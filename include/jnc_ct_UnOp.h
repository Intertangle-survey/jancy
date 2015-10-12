// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_UnOpKind.h"
#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

//.............................................................................

enum OpFlag
{
	OpFlag_KeepDataRef      = 0x01,
	OpFlag_KeepClassRef	    = 0x02,
	OpFlag_KeepFunctionRef  = 0x04,
	OpFlag_KeepPropertyRef  = 0x08,
	OpFlag_KeepDerivableRef = 0x10,
	OpFlag_KeepBool         = 0x20,
	OpFlag_KeepEnum         = 0x40,
	OpFlag_ArrayRefToPtr    = 0x80,
	OpFlag_LoadArrayRef     = 0x100,
	
	OpFlag_KeepRef          = 
		OpFlag_KeepDataRef | 
		OpFlag_KeepClassRef | 
		OpFlag_KeepFunctionRef | 
		OpFlag_KeepPropertyRef,
};

//.............................................................................

class UnaryOperator
{
	friend class OperatorMgr;

protected:
	Module* m_module;
	UnOpKind m_opKind;
	uint_t m_opFlags;

public:
	UnaryOperator ();

	Module*
	getModule ()
	{
		return m_module;
	}

	UnOpKind 
	getOpKind ()
	{
		return m_opKind;
	}

	int 
	getOpFlags ()
	{
		return m_opFlags;
	}

	bool
	getResultType (
		const Value& opValue,
		Value* resultValue
		);

	virtual
	Type*
	getResultType (const Value& opValue) = 0;

	virtual
	bool
	op (
		const Value& opValue,
		Value* resultValue
		) = 0;

	err::Error
	setOperatorError (Type* opType)
	{
		return err::setFormatStringError (
			"unary '%s' cannot be applied to '%s'",
			getUnOpKindString (m_opKind),
			opType->getTypeString ().cc ()
			);
	}

	err::Error
	setOperatorError (const Value& opValue)
	{
		return setOperatorError (opValue.getType ());
	}
};

//.............................................................................

} // namespace ct
} // namespace jnc
