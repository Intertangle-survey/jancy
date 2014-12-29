#include "pch.h"
#include "jnc_StructType.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

StructField::StructField ()
{
	m_itemKind = ModuleItemKind_StructField;
	m_type = NULL;
	m_type_i = NULL;
	m_ptrTypeFlags = 0;
	m_bitFieldBaseType = NULL;
	m_bitCount = 0;
	m_offset = 0;
	m_llvmIndex = -1;
}

//.............................................................................

StructType::StructType ()
{
	m_typeKind = TypeKind_Struct;
	m_flags = TypeFlag_Pod | TypeFlag_StructRet;
	m_fieldAlignment = 8;
	m_fieldActualSize = 0;
	m_fieldAlignedSize = 0;
	m_lastBitFieldType = NULL;
	m_lastBitFieldOffset = 0;
}

void
StructType::prepareLlvmType ()
{
	m_llvmType = llvm::StructType::create (*m_module->getLlvmContext (), m_tag.cc ());
}

StructField*
StructType::createFieldImpl (
	const rtl::String& name,
	Type* type,
	size_t bitCount,
	uint_t ptrTypeFlags,
	rtl::BoxList <Token>* constructor,
	rtl::BoxList <Token>* initializer
	)
{
	if (m_flags & ModuleItemFlag_Sealed)
	{
		err::setFormatStringError ("'%s' is completed, cannot add fields to it", getTypeString ().cc ());
		return NULL;
	}

	StructField* field = m_module->m_typeMgr.createStructField (
		name, 
		type, 
		bitCount, 
		ptrTypeFlags, 
		constructor, 
		initializer
		);

	field->m_parentNamespace = this;

	if (!field->m_constructor.isEmpty () ||
		!field->m_initializer.isEmpty ())
	{
		m_initializedMemberFieldArray.append (field);
	}

	if (name.isEmpty ())
	{
		m_unnamedFieldArray.append (field);
	}
	else if (name [0] != '!') // internal field
	{
		bool result = addItem (field);
		if (!result)
			return NULL;
	}

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		field->m_type_i = (ImportType*) type;
		m_importFieldArray.append (field);
	}

	m_memberFieldArray.append (field);
	return field;
}

bool
StructType::append (StructType* type)
{
	bool result;

	rtl::Iterator <BaseTypeSlot> slot = type->m_baseTypeList.getHead ();
	for (; slot; slot++)
	{
		result = addBaseType (slot->m_type) != NULL;
		if (!result)
			return false;
	}

	rtl::Array <StructField*> fieldArray = type->getMemberFieldArray ();
	size_t count = fieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = fieldArray [i];
		result = field->m_bitCount ?
			createField (field->m_name, field->m_bitFieldBaseType, field->m_bitCount, field->m_ptrTypeFlags) != NULL:
			createField (field->m_name, field->m_type, 0, field->m_ptrTypeFlags) != NULL;

		if (!result)
			return false;
	}

	return true;
}

bool
StructType::calcLayout ()
{
	bool result;

	result = resolveImportTypes ();
	if (!result)
		return false;

	rtl::Iterator <BaseTypeSlot> slotIt = m_baseTypeList.getHead ();
	for (; slotIt; slotIt++)
	{
		BaseTypeSlot* slot = *slotIt;

		result = slot->m_type->ensureLayout ();
		if (!result)
			return false;

		if (slot->m_type->getTypeKind () == TypeKind_Class)
		{
			err::setFormatStringError ("'%s' cannot be a base type of a struct", slot->m_type->getTypeString ().cc ());
			return false;
		}

		if (slot->m_type->getFlags () & TypeFlag_GcRoot)
		{
			m_gcRootBaseTypeArray.append (slot);
			m_flags |= TypeFlag_GcRoot;
		}

		if (slot->m_type->getConstructor ())
			m_baseTypeConstructArray.append (slot);

		result = layoutField (
			slot->m_type,
			&slot->m_offset,
			&slot->m_llvmIndex
			);

		if (!result)
			return false;
	}

	size_t count = m_memberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldArray [i];

		result = field->m_type->ensureLayout ();
		if (!result)
			return false;

		if (m_structTypeKind != StructTypeKind_IfaceStruct && field->m_type->getTypeKind () == TypeKind_Class)
		{
			err::setFormatStringError ("'%s' cannot be a field of a struct", field->m_type->getTypeString ().cc ());
			return false;
		}

		result = field->m_bitCount ?
			layoutBitField (
				field->m_bitFieldBaseType,
				field->m_bitCount,
				&field->m_type,
				&field->m_offset,
				&field->m_llvmIndex
				) :
			layoutField (
				field->m_type,
				&field->m_offset,
				&field->m_llvmIndex
				);

		if (!result)
			return false;
	}

	if (m_fieldAlignedSize > m_fieldActualSize)
		insertPadding (m_fieldAlignedSize - m_fieldActualSize);

	// scan members for gcroots and constructors (not for auxilary structs such as class iface)

	if (m_structTypeKind == StructTypeKind_Normal)
	{
		size_t count = m_memberFieldArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			StructField* field = m_memberFieldArray [i];
			Type* type = field->getType ();

			uint_t fieldTypeFlags = type->getFlags ();

			if (!(fieldTypeFlags & TypeFlag_Pod))
				m_flags &= ~TypeFlag_Pod;

			if (fieldTypeFlags & TypeFlag_GcRoot)
			{
				m_gcRootMemberFieldArray.append (field);
				m_flags |= TypeFlag_GcRoot;
			}

			if ((type->getTypeKindFlags () & TypeKindFlag_Derivable) && ((DerivableType*) type)->getConstructor ())
				m_memberFieldConstructArray.append (field);
		}

		count = m_memberPropertyArray.getCount ();
		for (size_t i = 0; i < count; i++)
		{
			Property* prop = m_memberPropertyArray [i];
			result = prop->ensureLayout ();
			if (!result)
				return false;

			if (prop->getConstructor ())
				m_memberPropertyConstructArray.append (prop);
		}
	}

	llvm::StructType* llvmStructType = (llvm::StructType*) getLlvmType ();
	llvmStructType->setBody (
		llvm::ArrayRef<llvm::Type*> (m_llvmFieldTypeArray, m_llvmFieldTypeArray.getCount ()),
		true
		);

	m_size = m_fieldAlignedSize;

	if (m_structTypeKind == StructTypeKind_Normal)
	{
		if (!m_staticConstructor && !m_initializedStaticFieldArray.isEmpty ())
		{
			result = createDefaultMethod (FunctionKind_StaticConstructor, StorageKind_Static);
			if (!result)
				return false;
		}

		if (!m_constructor &&
			(m_preConstructor ||
			!m_baseTypeConstructArray.isEmpty () ||
			!m_memberFieldConstructArray.isEmpty () ||
			!m_initializedMemberFieldArray.isEmpty () ||
			!m_memberPropertyConstructArray.isEmpty ()))
		{
			result = createDefaultMethod (FunctionKind_Constructor);
			if (!result)
				return false;
		}
	}

	return true;
}

bool
StructType::compile ()
{
	bool result;

	if (m_staticConstructor && !(m_staticConstructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultStaticConstructor ();
		if (!result)
			return false;
	}

	if (m_constructor && !(m_constructor->getFlags () & ModuleItemFlag_User))
	{
		result = compileDefaultConstructor ();
		if (!result)
			return false;
	}

	return true;
}

bool
StructType::layoutField (
	llvm::Type* llvmType,
	size_t size,
	size_t alignment,
	size_t* offset_o,
	uint_t* llvmIndex
	)
{
	if (alignment > m_alignment)
		m_alignment = AXL_MIN (alignment, m_fieldAlignment);

	size_t offset = getFieldOffset (alignment);
	if (offset > m_fieldActualSize)
		insertPadding (offset - m_fieldActualSize);

	*offset_o = offset;
	*llvmIndex = (uint_t) m_llvmFieldTypeArray.getCount ();

	m_lastBitFieldType = NULL;
	m_lastBitFieldOffset = 0;

	m_llvmFieldTypeArray.append (llvmType);
	setFieldActualSize (offset + size);
	return true;
}

bool
StructType::layoutBitField (
	Type* baseType,
	size_t bitCount,
	Type** type_o,
	size_t* offset_o,
	uint_t* llvmIndex
	)
{
	size_t bitOffset = getBitFieldBitOffset (baseType, bitCount);
	BitFieldType* type = m_module->m_typeMgr.getBitFieldType (baseType, bitOffset, bitCount);
	if (!type)
		return false;

	*type_o = type;
	m_lastBitFieldType = type;

	if (bitOffset)
	{
		*offset_o = m_lastBitFieldOffset;
		*llvmIndex = (uint_t) m_llvmFieldTypeArray.getCount () - 1;
		return true;
	}

	size_t alignment = type->getAlignment ();
	if (alignment > m_alignment)
		m_alignment = AXL_MIN (alignment, m_fieldAlignment);

	size_t offset = getFieldOffset (alignment);
	m_lastBitFieldOffset = offset;

	if (offset > m_fieldActualSize)
		insertPadding (offset - m_fieldActualSize);

	*offset_o = offset;
	*llvmIndex = (uint_t) m_llvmFieldTypeArray.getCount ();

	m_llvmFieldTypeArray.append (type->getLlvmType ());
	setFieldActualSize (offset + type->getSize ());
	return true;
}

size_t
StructType::getFieldOffset (size_t alignment)
{
	size_t offset = m_fieldActualSize;

	if (alignment > m_fieldAlignment)
		alignment = m_fieldAlignment;

	size_t mod = offset % alignment;
	if (mod)
		offset += alignment - mod;

	return offset;
}

size_t
StructType::getBitFieldBitOffset (
	Type* type,
	size_t bitCount
	)
{
	if (!m_lastBitFieldType || m_lastBitFieldType->getBaseType ()->cmp (type) != 0)
		return 0;

	size_t lastBitOffset =
		m_lastBitFieldType->getBitOffset () +
		m_lastBitFieldType->getBitCount ();

	return lastBitOffset + bitCount <= type->getSize () * 8 ? lastBitOffset : 0;
}

size_t
StructType::setFieldActualSize (size_t size)
{
	if (m_fieldActualSize >= size)
		return m_fieldAlignedSize;

	m_fieldActualSize = size;
	m_fieldAlignedSize = size;

	size_t mod = size % m_alignment;
	if (mod)
		m_fieldAlignedSize += m_alignment - mod;

	return m_fieldAlignedSize;
}

ArrayType*
StructType::insertPadding (size_t size)
{
	ArrayType* type = m_module->m_typeMgr.getArrayType (TypeKind_Int8_u, size);
	m_llvmFieldTypeArray.append (type->getLlvmType ());
	return type;
}

void
StructType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createEmptyStructType (this);
	m_module->m_llvmDiBuilder.setStructTypeBody (this);
}

void
StructType::gcMark (
	Runtime* runtime,
	void* _p
	)
{
	if (m_stdType == StdType_FmtLiteral) // special handling
	{
		gcMarkFmtLiteral (runtime, (FmtLiteral*) _p);
		return;
	}

	char* p = (char*) _p;

	size_t count = m_gcRootBaseTypeArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		BaseTypeSlot* slot = m_gcRootBaseTypeArray [i];
		slot->getType ()->gcMark (runtime, p + slot->getOffset ());
	}

	count = m_gcRootMemberFieldArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_gcRootMemberFieldArray [i];
		field->getType ()->gcMark (runtime, p + field->getOffset ());
	}
}

void
StructType::gcMarkFmtLiteral (
	Runtime* runtime,
	FmtLiteral* literal
	)
{
	if (!literal->m_p)
		return;

	ObjHdr* object = ((ObjHdr*) literal->m_p) - 1;
	ASSERT (object->m_scopeLevel == 0);
	object->gcMarkData (runtime);
}

//.............................................................................

} // namespace jnc {

