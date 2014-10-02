// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_PropertyType.h"
#include "jnc_FunctionPtrType.h"

namespace jnc {

struct IfaceHdr;

//.............................................................................

class PropertyPtrType: public Type
{
	friend class TypeMgr;

protected:
	PropertyPtrTypeKind m_ptrTypeKind;
	PropertyType* m_targetType;
	Namespace* m_anchorNamespace; // for dual pointers

public:
	PropertyPtrType ();

	PropertyPtrTypeKind
	getPtrTypeKind ()
	{
		return m_ptrTypeKind;
	}

	PropertyType*
	getTargetType ()
	{
		return m_targetType;
	}

	Namespace*
	getAnchorNamespace ()
	{
		return m_anchorNamespace;
	}

	bool
	isConstPtrType ();

	bool
	hasClosure ()
	{
		return m_ptrTypeKind == PropertyPtrTypeKind_Normal || m_ptrTypeKind == PropertyPtrTypeKind_Weak;
	}

	PropertyPtrType*
	getCheckedPtrType ()
	{
		return !(m_flags & PtrTypeFlagKind_Safe) ?
			m_targetType->getPropertyPtrType (m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlagKind_Safe) :
			this;
	}

	PropertyPtrType*
	getUnCheckedPtrType ()
	{
		return (m_flags & PtrTypeFlagKind_Safe) ?
			m_targetType->getPropertyPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlagKind_Safe) :
			this;
	}

	PropertyPtrType*
	getNormalPtrType ()
	{
		return (m_ptrTypeKind != PropertyPtrTypeKind_Normal) ?
			m_targetType->getPropertyPtrType (PropertyPtrTypeKind_Normal, m_flags) :
			this;
	}

	PropertyPtrType*
	getWeakPtrType ()
	{
		return (m_ptrTypeKind != PropertyPtrTypeKind_Weak) ?
			m_targetType->getPropertyPtrType (PropertyPtrTypeKind_Weak, m_flags) :
			this;
	}

	PropertyPtrType*
	getUnWeakPtrType ()
	{
		return (m_ptrTypeKind == PropertyPtrTypeKind_Weak) ?
			m_targetType->getPropertyPtrType (PropertyPtrTypeKind_Normal, m_flags) :
			this;
	}

	StructType*
	getPropertyPtrStructType ();

	static
	rtl::String
	createSignature (
		PropertyType* propertyType,
		TypeKind typeKind,
		PropertyPtrTypeKind ptrTypeKind,
		uint_t flags
		);

	virtual
	void
	gcMark (
		Runtime* runtime,
		void* p
		);

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareLlvmType ();

	virtual
	void
	prepareLlvmDiType ();
};

//.............................................................................

struct PropertyPtrTypeTuple: rtl::ListLink
{
	StructType* m_ptrStructType;
	PropertyPtrType* m_ptrTypeArray [2] [3] [3]; // ref x kind x unsafe / checked
};

//.............................................................................

inline
bool
isBindableType (Type* type)
{
	return
		type->getTypeKind () == TypeKind_PropertyRef &&
		(((PropertyPtrType*) type)->getTargetType ()->getFlags () & PropertyTypeFlagKind_Bindable) != 0;
}

//.............................................................................

// structure backing up property pointers, e.g.:
// int property* pxTest;
// int property weak* pxTest;

struct PropertyPtr
{
	void** m_pVTable;
	IfaceHdr* m_closure;
};

//.............................................................................

} // namespace jnc {
