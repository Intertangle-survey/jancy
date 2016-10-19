// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_DerivableType.h"
#include "jnc_ct_BitFieldType.h"

namespace jnc {
namespace ct {

class StructType;
class UnionType;
struct FmtLiteral;

//..............................................................................

class StructField:
	public ModuleItem,
	public ModuleItemDecl,
	public ModuleItemInitializer
{
	friend class TypeMgr;
	friend class NamedTypeBlock;
	friend class DerivableType;
	friend class Property;
	friend class StructType;
	friend class UnionType;
	friend class ClassType;

protected:
	Type* m_type;
	uint_t m_ptrTypeFlags;
	sl::BoxList <Token> m_constructor;

	Type* m_bitFieldBaseType;
	size_t m_bitCount;
	size_t m_offset;
	uint_t m_llvmIndex;

public:
	StructField ();

	Type*
	getType ()
	{
		return m_type;
	}

	int
	getPtrTypeFlags ()
	{
		return m_ptrTypeFlags;
	}

	sl::ConstBoxList <Token>
	getConstructor ()
	{
		return m_constructor;
	}

	size_t
	getOffset ()
	{
		return m_offset;
	}

	uint_t
	getLlvmIndex ()
	{
		return m_llvmIndex;
	}

	virtual
	bool
	generateDocumentation (
		const sl::StringRef& outputDir,
		sl::String* itemXml,
		sl::String* indexXml
		);

};

//..............................................................................

enum StructTypeKind
{
	StructTypeKind_Normal,
	StructTypeKind_IfaceStruct,
	StructTypeKind_ClassStruct,
	StructTypeKind_UnionStruct,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class StructType: public DerivableType
{
	friend class TypeMgr;
	friend class ClassType;
	friend class UnionType;
	friend class Property;

protected:
	StructTypeKind m_structTypeKind;

	size_t m_fieldAlignment;
	size_t m_fieldActualSize;
	size_t m_fieldAlignedSize;

	sl::Array <llvm::Type*> m_llvmFieldTypeArray;
	BitFieldType* m_lastBitFieldType;
	size_t m_lastBitFieldOffset;

public:
	StructType ();

	StructTypeKind
	getStructTypeKind ()
	{
		return m_structTypeKind;
	}

	size_t
	getFieldAlignment ()
	{
		return m_fieldAlignment;
	}

	size_t
	getFieldActualSize ()
	{
		return m_fieldActualSize;
	}

	size_t
	getFieldAlignedSize ()
	{
		return m_fieldAlignedSize;
	}

	bool
	append (StructType* type);

	virtual
	bool
	compile ();

	virtual
	void
	markGcRoots (
		const void* p,
		rt::GcHeap* gcHeap
		);

protected:
	virtual
	StructField*
	createFieldImpl (
		const sl::StringRef& name,
		Type* type,
		size_t bitCount = 0,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		);

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();

	virtual
	bool
	calcLayout ();

	bool
	layoutField (
		llvm::Type* llvmType,
		size_t size,
		size_t alignment,
		size_t* offset,
		uint_t* llvmIndex
		);

	bool
	layoutField (
		Type* type,
		size_t* offset,
		uint_t* llvmIndex
		)
	{
		return
			type->ensureLayout () &&
			layoutField (
				type->getLlvmType (),
				type->getSize (),
				type->getAlignment (),
				offset,
				llvmIndex
				);
	}

	bool
	layoutBitField (
		Type* baseType,
		size_t bitCount,
		Type** type,
		size_t* offset,
		uint_t* llvmIndex
		);

	size_t
	getFieldOffset (size_t alignment);

	size_t
	setFieldActualSize (size_t size);

	ArrayType*
	insertPadding (size_t size);
};

//..............................................................................

} // namespace ct
} // namespace jnc
