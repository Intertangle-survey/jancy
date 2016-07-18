// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_RuntimeStructs.h"
#include "jnc_Variant.h"
#include "jnc_OpKind.h"

namespace jnc {
namespace rt {

//.............................................................................

Variant
variantUnaryOperator (
	UnOpKind opKind,
	const Variant& op
	);

Variant
variantBinaryOperator (
	BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	);

bool
variantRelationalOperator (
	BinOpKind opKind,
	const Variant& op1,
	const Variant& op2
	);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

// the most used op

inline
bool
isVariantEqual (
	const Variant& op1,
	const Variant& op2
	)
{
	return variantRelationalOperator (BinOpKind_Eq, op1, op2);
}

//.............................................................................

class HashVariant
{
public:
	uintptr_t
	operator () (const Variant& variant);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class CmpVariant 
{
public:
	int operator () (
		const Variant& variant1,
		const Variant& variant2
		)
	{
		return !isVariantEqual (variant1, variant2);
	}
};

//.............................................................................

} // namespace rt
} // namespace jnc
