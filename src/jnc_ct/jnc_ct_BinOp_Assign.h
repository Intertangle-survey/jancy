// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_BinOp.h"

namespace jnc {
namespace ct {

//.............................................................................

class BinOp_Assign: public BinaryOperator
{
public:
	BinOp_Assign ()
	{
		m_opKind = BinOpKind_Assign;
		m_opFlags1 = OpFlag_KeepRef;
		m_opFlags2 = OpFlag_KeepEnum | OpFlag_KeepBool;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return opValue1.getType ();
	}

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);
};

//.............................................................................

class BinOp_OpAssign: public BinaryOperator
{
public:
	BinOp_OpAssign ()
	{
		m_opFlags1 = OpFlag_KeepRef;
		m_opFlags2 = OpFlag_KeepEnum;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		return opValue1.getType ();
	}

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		);
};

//.............................................................................

class BinOp_RefAssign: public BinaryOperator
{
public:
	BinOp_RefAssign ()
	{
		m_opKind = BinOpKind_RefAssign;
		m_opFlags1 = OpFlag_KeepRef;
	}

	virtual
	Type*
	getResultType (
		const Value& opValue1,
		const Value& opValue2
		)
	{
		err::setFormatStringError ("'%s' has no overloaded ':=' operator", opValue1.getType ()->getTypeString ().cc ());
		return NULL;
	}

	virtual
	bool
	op (
		const Value& opValue1,
		const Value& opValue2,
		Value* resultValue
		)
	{
		err::setFormatStringError ("'%s' has no overloaded ':=' operator", opValue1.getType ()->getTypeString ().cc ());
		return false;
	}
};

//.............................................................................

} // namespace ct
} // namespace jnc
