// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ImportType.h"

namespace jnc {

//.............................................................................

class BitFieldType: public Type
{
	friend class TypeMgr;

protected:
	Type* m_baseType;
	ImportType* m_baseType_i;
	size_t m_bitOffset;
	size_t m_bitCount;

public:
	BitFieldType ();

	Type*
	getBaseType ()
	{
		return m_baseType;
	}

	ImportType*
	getBaseType_i ()
	{
		return m_baseType_i;
	}

	size_t
	getBitOffset ()
	{
		return m_bitOffset;
	}

	size_t
	getBitCount ()
	{
		return m_bitCount;
	}

	static
	rtl::String
	createSignature (
		Type* baseType,
		size_t bitOffset,
		size_t bitCount
		)
	{
		return rtl::String::format_s (
			"B%s:%d:%d",
			baseType->getSignature ().cc (), // thanks a lot gcc
			bitOffset,
			bitOffset + bitCount
			);
	}

protected:
	virtual
	void
	prepareTypeString ();

	virtual
	void
	prepareLlvmType ()
	{
		m_llvmType = m_baseType->getLlvmType ();
	}

	virtual
	void
	prepareLlvmDiType ()
	{
		m_llvmDiType = m_baseType->getLlvmDiType ();
	}

	virtual
	bool
	calcLayout ();
};

//.............................................................................

} // namespace jnc {
