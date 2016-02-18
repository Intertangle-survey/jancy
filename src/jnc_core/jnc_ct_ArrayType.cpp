#include "pch.h"
#include "jnc_ct_ArrayType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ArrayType::ArrayType ()
{
	m_typeKind = TypeKind_Array;
	m_flags = TypeFlag_StructRet;
	m_elementType = NULL;
	m_elementType_i = NULL;
	m_rootType = NULL;
	m_elementCount = -1;
	m_parentUnit = NULL;
	m_parentNamespace = NULL;
}

Type*
ArrayType::getRootType ()
{
	if (!m_rootType)
		m_rootType = m_elementType->getTypeKind () == TypeKind_Array ?
			((ArrayType*) m_elementType)->getRootType () :
			m_elementType;

	return m_rootType;
}

void
ArrayType::prepareTypeString ()
{
	sl::String string;

	m_typeString.format (
		m_elementCount == -1 ? "%s []" : "%s [%d]",
		getRootType ()->getTypeString ().cc (),
		m_elementCount
		);

	Type* elementType = m_elementType;
	while (elementType->getTypeKind () == TypeKind_Array)
	{
		ArrayType* arrayType = (ArrayType*) elementType;
		m_typeString.appendFormat (" [%d]", arrayType->m_elementCount);
		elementType = arrayType->m_elementType;
	}
}

bool
ArrayType::calcLayout ()
{
	if (m_elementType_i)
		m_elementType = m_elementType_i->getActualType ();

	bool result = m_elementType->ensureLayout ();
	if (!result)
		return false;

	// ensure update

	m_rootType = NULL;
	m_typeString.clear ();

	uint_t rootTypeFlags = getRootType ()->getFlags ();
	if (rootTypeFlags & TypeFlag_Pod)
		m_flags |= TypeFlag_Pod;
	else if (rootTypeFlags & TypeFlag_GcRoot)
		m_flags |= TypeFlag_GcRoot;

	m_alignment = m_elementType->getAlignment ();

	// calculate size

	if (!m_elementCountInitializer.isEmpty ())
	{
		ASSERT (m_parentUnit && m_parentNamespace);
		m_module->m_namespaceMgr.openNamespace (m_parentNamespace);

		intptr_t value = 0;
		result = m_module->m_operatorMgr.parseConstIntegerExpression (
			m_parentUnit,
			m_elementCountInitializer,
			&value
			);

		if (!result)
			return false;

		if (value <= 0)
		{
			err::setFormatStringError ("invalid array size '%d'\n", value);
			lex::pushSrcPosError (
				m_parentUnit->getFilePath (),
				m_elementCountInitializer.getHead ()->m_pos
				);
			
			return false;
		}

		m_elementCount = value;
		m_module->m_namespaceMgr.closeNamespace ();
	}

	sl::String signature = createSignature (m_elementType, m_elementCount);
	m_module->m_typeMgr.updateTypeSignature (this, signature);

	m_size = m_elementType->getSize () * m_elementCount;
	if (m_size > TypeSizeLimit_StackAllocSize)
		m_flags |= TypeFlag_NoStack;

	return true;
}

void
ArrayType::markGcRoots (
	const void* p,
	rt::GcHeap* gcHeap
	)
{
	ASSERT (m_flags & TypeFlag_GcRoot);
	gcHeap->addRootArray (p, m_elementType, m_elementCount);
}

void
ArrayType::prepareLlvmDiType ()
{
	m_llvmDiType = m_module->m_llvmDiBuilder.createArrayType (this);
}

//.............................................................................

} // namespace ct
} // namespace jnc
