// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_FunctionType.h"

namespace jnc {

struct IfaceHdr;

//.............................................................................

class FunctionPtrType: public Type
{
	friend class TypeMgr;

protected:
	FunctionPtrTypeKind m_ptrTypeKind;
	FunctionType* m_targetType;
	ClassType* m_multicastType;
	rtl::String m_typeModifierString;

public:
	FunctionPtrType ();

	FunctionPtrTypeKind
	getPtrTypeKind ()
	{
		return m_ptrTypeKind;
	}

	FunctionType*
	getTargetType ()
	{
		return m_targetType;
	}

	bool
	hasClosure ()
	{
		return m_ptrTypeKind == FunctionPtrTypeKind_Normal || m_ptrTypeKind == FunctionPtrTypeKind_Weak;
	}

	FunctionPtrType*
	getCheckedPtrType ()
	{
		return !(m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType (m_typeKind, m_ptrTypeKind, m_flags | PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getUnCheckedPtrType ()
	{
		return (m_flags & PtrTypeFlag_Safe) ?
			m_targetType->getFunctionPtrType (m_typeKind, m_ptrTypeKind, m_flags & ~PtrTypeFlag_Safe) :
			this;
	}

	FunctionPtrType*
	getNormalPtrType ()
	{
		return (m_ptrTypeKind != FunctionPtrTypeKind_Normal) ?
			m_targetType->getFunctionPtrType (FunctionPtrTypeKind_Normal, m_flags) :
			this;
	}

	FunctionPtrType*
	getWeakPtrType ()
	{
		return (m_ptrTypeKind != FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType (FunctionPtrTypeKind_Weak, m_flags) :
			this;
	}

	FunctionPtrType*
	getUnWeakPtrType ()
	{
		return (m_ptrTypeKind == FunctionPtrTypeKind_Weak) ?
			m_targetType->getFunctionPtrType (FunctionPtrTypeKind_Normal, m_flags) :
			this;
	}

	ClassType*
	getMulticastType ();

	rtl::String
	getTypeModifierString ();

	static
	rtl::String
	createSignature (
		FunctionType* functionType,
		TypeKind typeKind,
		FunctionPtrTypeKind ptrTypeKind,
		uint_t flags
		);

	virtual
	void
	markGcRoots (
		const void* p,
		GcHeap* gcHeap
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

	virtual
	bool
	calcLayout ()
	{
		return m_targetType->ensureLayout ();	
	}
};

//.............................................................................

struct FunctionPtrTypeTuple: rtl::ListLink
{
	FunctionPtrType* m_ptrTypeArray [2] [3] [2]; // ref x kind x checked
};

//.............................................................................

} // namespace jnc {
