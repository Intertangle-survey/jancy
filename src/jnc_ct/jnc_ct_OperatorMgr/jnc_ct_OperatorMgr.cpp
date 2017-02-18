//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#include "pch.h"
#include "jnc_ct_OperatorMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

OperatorMgr::OperatorMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	// operator tables

	memset (m_unaryOperatorTable, 0, sizeof (m_unaryOperatorTable));
	memset (m_binaryOperatorTable, 0, sizeof (m_binaryOperatorTable));
	memset (m_castOperatorTable, 0, sizeof (m_castOperatorTable));

	// unary arithmetics

	m_unaryOperatorTable [UnOpKind_Plus]     = &m_unOp_Plus;
	m_unaryOperatorTable [UnOpKind_Minus]    = &m_unOp_Minus;
	m_unaryOperatorTable [UnOpKind_BwNot]    = &m_unOp_BwNot;
	m_unaryOperatorTable [UnOpKind_LogNot]   = &m_unOp_LogNot;

	// pointer operators

	m_unaryOperatorTable [UnOpKind_Addr]     = &m_unOp_Addr;
	m_unaryOperatorTable [UnOpKind_Indir]    = &m_unOp_Indir;
	m_unaryOperatorTable [UnOpKind_Ptr]      = &m_unOp_Indir;

	// increment operators

	m_unOp_PreInc.m_opKind  = UnOpKind_PreInc;
	m_unOp_PreDec.m_opKind  = UnOpKind_PreDec;
	m_unOp_PostInc.m_opKind = UnOpKind_PostInc;
	m_unOp_PostDec.m_opKind = UnOpKind_PostDec;

	m_unaryOperatorTable [UnOpKind_PreInc]   = &m_unOp_PreInc;
	m_unaryOperatorTable [UnOpKind_PreDec]   = &m_unOp_PreDec;
	m_unaryOperatorTable [UnOpKind_PostInc]  = &m_unOp_PostInc;
	m_unaryOperatorTable [UnOpKind_PostDec]  = &m_unOp_PostDec;

	// binary arithmetics

	m_binaryOperatorTable [BinOpKind_Add]    = &m_binOp_Add;
	m_binaryOperatorTable [BinOpKind_Sub]    = &m_binOp_Sub;
	m_binaryOperatorTable [BinOpKind_Mul]    = &m_binOp_Mul;
	m_binaryOperatorTable [BinOpKind_Div]    = &m_binOp_Div;
	m_binaryOperatorTable [BinOpKind_Mod]    = &m_binOp_Mod;
	m_binaryOperatorTable [BinOpKind_Shl]    = &m_binOp_Shl;
	m_binaryOperatorTable [BinOpKind_Shr]    = &m_binOp_Shr;
	m_binaryOperatorTable [BinOpKind_BwAnd]  = &m_binOp_BwAnd;
	m_binaryOperatorTable [BinOpKind_BwXor]  = &m_binOp_BwXor;
	m_binaryOperatorTable [BinOpKind_BwOr]   = &m_binOp_BwOr;
	m_binaryOperatorTable [BinOpKind_At]     = &m_binOp_At;

	// binary logic operators

	m_binaryOperatorTable [BinOpKind_LogAnd] = &m_binOp_LogAnd;
	m_binaryOperatorTable [BinOpKind_LogOr]  = &m_binOp_LogOr;

	// comparison operators

	m_binaryOperatorTable [BinOpKind_Eq]     = &m_binOp_Eq;
	m_binaryOperatorTable [BinOpKind_Ne]     = &m_binOp_Ne;
	m_binaryOperatorTable [BinOpKind_Lt]     = &m_binOp_Lt;
	m_binaryOperatorTable [BinOpKind_Le]     = &m_binOp_Le;
	m_binaryOperatorTable [BinOpKind_Gt]     = &m_binOp_Gt;
	m_binaryOperatorTable [BinOpKind_Ge]     = &m_binOp_Ge;

	// indexing operator

	m_binaryOperatorTable [BinOpKind_Idx]    = &m_binOp_Idx;

	// assignment operators

	m_binOp_AddAssign.m_opKind = BinOpKind_AddAssign;
	m_binOp_SubAssign.m_opKind = BinOpKind_SubAssign;
	m_binOp_MulAssign.m_opKind = BinOpKind_MulAssign;
	m_binOp_DivAssign.m_opKind = BinOpKind_DivAssign;
	m_binOp_ModAssign.m_opKind = BinOpKind_ModAssign;
	m_binOp_ShlAssign.m_opKind = BinOpKind_ShlAssign;
	m_binOp_ShrAssign.m_opKind = BinOpKind_ShrAssign;
	m_binOp_AndAssign.m_opKind = BinOpKind_AndAssign;
	m_binOp_XorAssign.m_opKind = BinOpKind_XorAssign;
	m_binOp_OrAssign.m_opKind  = BinOpKind_OrAssign;
	m_binOp_AtAssign.m_opKind  = BinOpKind_AtAssign;

	m_binaryOperatorTable [BinOpKind_Assign]      = &m_binOp_Assign;
	m_binaryOperatorTable [BinOpKind_RefAssign]   = &m_binOp_RefAssign;
	m_binaryOperatorTable [BinOpKind_AddAssign]   = &m_binOp_AddAssign;
	m_binaryOperatorTable [BinOpKind_SubAssign]   = &m_binOp_SubAssign;
	m_binaryOperatorTable [BinOpKind_MulAssign]   = &m_binOp_MulAssign;
	m_binaryOperatorTable [BinOpKind_DivAssign]   = &m_binOp_DivAssign;
	m_binaryOperatorTable [BinOpKind_ModAssign]   = &m_binOp_ModAssign;
	m_binaryOperatorTable [BinOpKind_ShlAssign]   = &m_binOp_ShlAssign;
	m_binaryOperatorTable [BinOpKind_ShrAssign]   = &m_binOp_ShrAssign;
	m_binaryOperatorTable [BinOpKind_AndAssign]   = &m_binOp_AndAssign;
	m_binaryOperatorTable [BinOpKind_XorAssign]   = &m_binOp_XorAssign;
	m_binaryOperatorTable [BinOpKind_OrAssign]    = &m_binOp_OrAssign;
	m_binaryOperatorTable [BinOpKind_AtAssign]    = &m_binOp_AtAssign;

	// cast operators

	m_stdCastOperatorTable [StdCast_Copy] = &m_cast_Copy;
	m_stdCastOperatorTable [StdCast_SwapByteOrder] = &m_cast_SwapByteOrder;
	m_stdCastOperatorTable [StdCast_PtrFromInt] = &m_cast_PtrFromInt;
	m_stdCastOperatorTable [StdCast_Int] = &m_cast_Int;
	m_stdCastOperatorTable [StdCast_Fp] = &m_cast_Fp;
	m_stdCastOperatorTable [StdCast_FromVariant] = &m_cast_FromVariant;


	for (size_t i = 0; i < TypeKind__Count; i++)
		m_castOperatorTable [i] = &m_cast_Default;

	m_castOperatorTable [TypeKind_Bool] = &m_cast_Bool;

	for (size_t i = TypeKind_Int8; i <= TypeKind_Int64_u; i++)
		m_castOperatorTable [i] = &m_cast_Int;

	for (size_t i = TypeKind_Int16_be; i <= TypeKind_Int64_beu; i++)
		m_castOperatorTable [i] = &m_cast_BeInt;

	m_castOperatorTable [TypeKind_Float]       = &m_cast_Fp;
	m_castOperatorTable [TypeKind_Double]      = &m_cast_Fp;
	m_castOperatorTable [TypeKind_Variant]     = &m_cast_Variant;
	m_castOperatorTable [TypeKind_Array]       = &m_cast_Array;
	m_castOperatorTable [TypeKind_Enum]        = &m_cast_Enum;
	m_castOperatorTable [TypeKind_Struct]      = &m_cast_Struct;
	m_castOperatorTable [TypeKind_DataPtr]     = &m_cast_DataPtr;
	m_castOperatorTable [TypeKind_DataRef]     = &m_cast_DataRef;
	m_castOperatorTable [TypeKind_ClassPtr]    = &m_cast_ClassPtr;
	m_castOperatorTable [TypeKind_FunctionPtr] = &m_cast_FunctionPtr;
	m_castOperatorTable [TypeKind_FunctionRef] = &m_cast_FunctionRef;
	m_castOperatorTable [TypeKind_PropertyPtr] = &m_cast_PropertyPtr;
	m_castOperatorTable [TypeKind_PropertyRef] = &m_cast_PropertyRef;

	m_unsafeEnterCount = 0;
}

void
OperatorMgr::clear ()
{
	m_unsafeEnterCount = 0;
}

Function*
OperatorMgr::getOverloadedUnaryOperator (
	UnOpKind opKind,
	const Value& opValue
	)
{
	Type* opType = prepareOperandType (opValue);

	if (opType->getTypeKind () == TypeKind_ClassPtr)
	{
		ClassPtrType* ptrType = (ClassPtrType*) opType;
		return ptrType->getTargetType ()->getUnaryOperator (opKind);
	}
	else if (opType->getTypeKindFlags () & TypeKindFlag_Derivable)
	{
		DerivableType* derivableType = (DerivableType*) opType;
		return derivableType->getUnaryOperator (opKind);
	}

	return NULL;
}

Type*
OperatorMgr::getUnaryOperatorResultType (
	UnOpKind opKind,
	const Value& rawOpValue
	)
{
	ASSERT ((size_t) opKind < UnOpKind__Count);

	Function* function = getOverloadedUnaryOperator (opKind, rawOpValue);
	if (function)
	{
		sl::BoxList <Value> argList;
		argList.insertTail (rawOpValue);
		return getCallOperatorResultType (function->getTypeOverload (), &argList);
	}

	UnaryOperator* op = m_unaryOperatorTable [opKind];
	ASSERT (op);

	Value opValue;
	prepareOperandType (rawOpValue, &opValue, op->getOpFlags ());

	return opValue.getType ()->getTypeKind () == TypeKind_Variant && opKind <= UnOpKind_Indir ?
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant) :
		op->getResultType (opValue);
}

bool
OperatorMgr::getUnaryOperatorResultType (
	UnOpKind opKind,
	const Value& rawOpValue,
	Value* resultValue
	)
{
	Type* resultType = getUnaryOperatorResultType (opKind, rawOpValue);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::unaryOperator (
	UnOpKind opKind,
	const Value& rawOpValue,
	Value* resultValue
	)
{
	ASSERT ((size_t) opKind < UnOpKind__Count);

	Function* function = getOverloadedUnaryOperator (opKind, rawOpValue);
	if (function)
	{
		sl::BoxList <Value> argList;
		argList.insertTail (rawOpValue);
		return callOperator (function, &argList, resultValue);
	}

	Value opValue;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	UnaryOperator* op = m_unaryOperatorTable [opKind];
	ASSERT (op);

	bool result = prepareOperand (rawOpValue, &opValue, op->getOpFlags ());
	if (!result)
		return false;

	if (opValue.getType ()->getTypeKind () == TypeKind_Variant && opKind <= UnOpKind_Indir)
	{
		Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_VariantUnaryOperator);
		Value opKindValue (opKind, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int));
		return callOperator (function, opKindValue, opValue, resultValue);
	}

	return op->op (opValue, resultValue);
}

Type*
OperatorMgr::getBinaryOperatorResultType (
	BinOpKind opKind,
	const Value& rawOpValue1,
	const Value& rawOpValue2
	)
{
	ASSERT ((size_t) opKind < BinOpKind__Count);

	Function* function = getOverloadedBinaryOperator (opKind, rawOpValue1);
	if (function)
	{
		sl::BoxList <Value> argList;
		argList.insertTail (rawOpValue1);
		argList.insertTail (rawOpValue2);
		return getCallOperatorResultType (function->getTypeOverload (), &argList);
	}

	BinaryOperator* op = m_binaryOperatorTable [opKind];
	ASSERT (op);

	Value opValue1;
	Value opValue2;
	prepareOperandType (rawOpValue1, &opValue1, op->getOpFlags1 ());
	prepareOperandType (rawOpValue2, &opValue2, op->getOpFlags2 ());

	if (opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge)
		return m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);

	if ((opValue1.getType ()->getTypeKind () == TypeKind_Variant ||
		opValue2.getType ()->getTypeKind () == TypeKind_Variant) && opKind < BinOpKind_Eq)
		return m_module->m_typeMgr.getPrimitiveType (TypeKind_Variant);

	return op->getResultType (opValue1, opValue2);
}

bool
OperatorMgr::getBinaryOperatorResultType (
	BinOpKind opKind,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	Type* resultType = getBinaryOperatorResultType (opKind, rawOpValue1, rawOpValue2);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

Function*
OperatorMgr::getOverloadedBinaryOperator (
	BinOpKind opKind,
	const Value& opValue
	)
{
	Type* opType = prepareOperandType (opValue);
	if (opType->getTypeKind () == TypeKind_ClassPtr)
	{
		ClassPtrType* ptrType = (ClassPtrType*) opType;
		return ptrType->getTargetType ()->getBinaryOperator (opKind);
	}
	else if (opType->getTypeKindFlags () & TypeKindFlag_Derivable)
	{
		DerivableType* derivableType = (DerivableType*) opType;
		return derivableType->getBinaryOperator (opKind);
	}

	return NULL;
}

bool
OperatorMgr::binaryOperator (
	BinOpKind opKind,
	const Value& rawOpValue1,
	const Value& rawOpValue2,
	Value* resultValue
	)
{
	ASSERT ((size_t) opKind < BinOpKind__Count);

	Function* function = getOverloadedBinaryOperator (opKind, rawOpValue1);
	if (function)
	{
		if (function->getFlags () & MulticastMethodFlag_InaccessibleViaEventPtr)
		{
			Value opValue1;
			prepareOperandType (rawOpValue1, &opValue1);

			if (opValue1.getType ()->getTypeKind () == TypeKind_ClassPtr &&
				((ClassPtrType*) opValue1.getType ())->isEventPtrType ())
			{
				err::setFormatStringError ("'%s' is inaccessible via 'event' pointer", getBinOpKindString (function->getBinOpKind ()));
				return false;
			}
		}

		sl::BoxList <Value> argList;
		argList.insertTail (rawOpValue1);
		argList.insertTail (rawOpValue2);
		return callOperator (function, &argList, resultValue);
	}

	Value opValue1;
	Value opValue2;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	BinaryOperator* op = m_binaryOperatorTable [opKind];
	ASSERT (op);

	bool result =
		prepareOperand (rawOpValue1, &opValue1, op->getOpFlags1 ()) &&
		prepareOperand (rawOpValue2, &opValue2, op->getOpFlags2 ());

	if (!result)
		return false;

	if (opKind <= BinOpKind_Ge &&
		(opValue1.getType ()->getTypeKind () == TypeKind_Variant ||
		opValue2.getType ()->getTypeKind () == TypeKind_Variant))
	{
		StdFunc stdFunc = opKind >= BinOpKind_Eq && opKind <= BinOpKind_Ge ?
			StdFunc_VariantRelationalOperator :
			StdFunc_VariantBinaryOperator;

		Function* function = m_module->m_functionMgr.getStdFunction (stdFunc);
		Value opKindValue (opKind, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int));
		return callOperator (function, opKindValue, opValue1, opValue2, resultValue);
	}

	return op->op (opValue1, opValue2, resultValue);
}

Type*
getConditionalNumericOperatorResultType (
	const Value& trueValue,
	Type* trueType,
	const Value& falseValue,
	Type* falseType
	)
{
	if (trueType->getTypeKind () == TypeKind_Enum &&
		(trueType->getFlags () & EnumTypeFlag_BitFlag) &&
		falseValue.isZero ())
		return trueType;

	if (falseType->getTypeKind () == TypeKind_Enum &&
		(falseType->getFlags () & EnumTypeFlag_BitFlag) &&
		trueValue.isZero ())
		return falseType;

	return getArithmeticOperatorResultType (trueType, falseType);
}

Type*
OperatorMgr::getConditionalOperatorResultType (
	const Value& trueValue,
	const Value& falseValue
	)
{
	Type* resultType;

	Type* trueType = trueValue.getType ();
	Type* falseType = falseValue.getType ();

	if (trueType->getTypeKind () == TypeKind_Array)
		trueType = ((ArrayType*) trueType)->getElementType ()->getDataPtrType ();

	if (falseType->getTypeKind () == TypeKind_Array)
		falseType = ((ArrayType*) falseType)->getElementType ()->getDataPtrType ();

	if (trueType->cmp (falseType) == 0)
	{
		resultType = trueType;
	}
	else
	{
		uint_t trueFlags = OpFlag_KeepBool | OpFlag_KeepEnum;
		uint_t falseFlags = OpFlag_KeepBool | OpFlag_KeepEnum;

		if (isArrayRefType (trueType))
			trueFlags |= OpFlag_ArrayRefToPtr;

		if (isArrayRefType (falseType))
			falseFlags |= OpFlag_ArrayRefToPtr;

		trueType = prepareOperandType (trueType, trueFlags);
		falseType = prepareOperandType (falseType, falseFlags);

		resultType =
			trueType->cmp (falseType) == 0 ? trueType :
			(trueType->getTypeKindFlags () & falseType->getTypeKindFlags () & TypeKindFlag_Numeric) ?
				getConditionalNumericOperatorResultType (trueValue, trueType, falseValue, falseType) :
				prepareOperandType (trueType);
	}

	// if it's a lean data pointer, fatten it

	if ((resultType->getTypeKindFlags () & TypeKindFlag_DataPtr) &&
		((DataPtrType*) resultType)->getPtrTypeKind () == DataPtrTypeKind_Lean)
	{
		resultType = ((DataPtrType*) resultType)->getTargetType ()->getDataPtrType (
			resultType->getTypeKind (),
			DataPtrTypeKind_Normal,
			resultType->getFlags ()
			);
	}

	bool result =
		checkCastKind (trueValue, resultType) &&
		checkCastKind (falseValue, resultType);

	return result ? resultType : NULL;
}

bool
OperatorMgr::getConditionalOperatorResultType (
	const Value& trueValue,
	const Value& falseValue,
	Value* resultValue
	)
{
	Type* resultType = getConditionalOperatorResultType (trueValue, falseValue);
	if (!resultType)
		return false;

	resultValue->setType (resultType);
	return true;
}

bool
OperatorMgr::conditionalOperator (
	const Value& rawTrueValue,
	const Value& rawFalseValue,
	BasicBlock* thenBlock,
	BasicBlock* phiBlock,
	Value* resultValue
	)
{
	bool result;

	Value trueValue;
	Value falseValue;

	Type* resultType = getConditionalOperatorResultType (rawTrueValue, rawFalseValue);
	if (!resultType)
		return false;

	result = castOperator (rawFalseValue, resultType, &falseValue);
	if (!result)
		return false;

	BasicBlock* elseBlock = m_module->m_controlFlowMgr.getCurrentBlock (); // might have changed

	m_module->m_controlFlowMgr.jump (phiBlock, thenBlock);

	result = castOperator (rawTrueValue, resultType, &trueValue);
	if (!result)
		return false;

	thenBlock = m_module->m_controlFlowMgr.getCurrentBlock (); // might have changed

	m_module->m_controlFlowMgr.follow (phiBlock);
	m_module->m_llvmIrBuilder.createPhi (trueValue, thenBlock, falseValue, elseBlock, resultValue);
	return true;
}

void
OperatorMgr::forceCast (
	const Value& value,
	Type* dstType,
	Value* resultValue
	)
{
	Type* srcType = value.getType ();

	if (srcType->getSize () >= dstType->getSize ())
	{
		Value tmpValue;
		m_module->m_llvmIrBuilder.createAlloca (srcType, "tmp", NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createStore (value, tmpValue);
		m_module->m_llvmIrBuilder.createBitCast (tmpValue, dstType->getDataPtrType_c (), &tmpValue);
		m_module->m_llvmIrBuilder.createLoad (tmpValue, dstType, resultValue);
	}
	else
	{
		Value tmpValue, tmpValue2;
		m_module->m_llvmIrBuilder.createAlloca (dstType, "tmp", NULL, &tmpValue);
		m_module->m_llvmIrBuilder.createBitCast (tmpValue, srcType->getDataPtrType_c (), &tmpValue2);
		m_module->m_llvmIrBuilder.createStore (value, tmpValue2);
		m_module->m_llvmIrBuilder.createLoad (tmpValue, dstType, resultValue);
	}
}

bool
OperatorMgr::castOperator (
	OperatorDynamism dynamism,
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	bool result;

	if (rawOpValue.getValueKind () == ValueKind_Null)
	{
		if ((type->getTypeKindFlags () & TypeKindFlag_Ptr) && (type->getFlags () & PtrTypeFlag_Safe))
		{
			setCastError (rawOpValue, type);
			return false;
		}

		if (type->getTypeKind () == TypeKind_Void)
			resultValue->setNull (m_module);
		else
			*resultValue = type->getZeroValue ();

		return true;
	}

	TypeKind typeKind = type->getTypeKind ();
	ASSERT ((size_t) typeKind < TypeKind__Count);

	CastOperator* op = m_castOperatorTable [typeKind];
	ASSERT (op); // there is always a default

	Value opValue;
	Value unusedResultValue;

	if (!resultValue)
		resultValue = &unusedResultValue;

	result = prepareOperand (rawOpValue, &opValue, op->getOpFlags ());
	if (!result)
		return false;

	Type* opType = opValue.getType ();
	if (opType->cmp (type) == 0) // identity, try to shortcut
	{
		if (opValue.hasLlvmValue ())
		{
			*resultValue = opValue;
			return true;
		}

		if (opValue.getValueKind () == ValueKind_Property)
		{
			ASSERT (type->getTypeKind () == TypeKind_PropertyPtr);
			return getPropertyThinPtr (opValue.getProperty (), opValue.getClosure (), (PropertyPtrType*) type, resultValue);
		}

		// nope, need to go through full cast
	}

	if (opType->getTypeKind () == TypeKind_Variant)
		return m_stdCastOperatorTable [StdCast_FromVariant]->cast (opValue, type, resultValue);

	if (dynamism != OperatorDynamism_Dynamic)
		return op->cast (opValue, type, resultValue);

	typeKind = type->getTypeKind ();
	switch (typeKind)
	{
	case TypeKind_DataPtr:
		return dynamicCastDataPtr (opValue, (DataPtrType*) type, resultValue);

	case TypeKind_ClassPtr:
		return dynamicCastClassPtr (opValue, (ClassPtrType*) type, resultValue);

	default:
		err::setFormatStringError ("cannot dynamically cast to '%s'", type->getTypeString ().sz ());
		return false;
	}
}

bool
OperatorMgr::castOperator (
	OperatorDynamism dynamism,
	const Value& opValue,
	TypeKind typeKind,
	Value* resultValue
	)
{
	Type* type = m_module->m_typeMgr.getPrimitiveType (typeKind);
	return castOperator (dynamism, opValue, type, resultValue);
}

bool
OperatorMgr::dynamicCastDataPtr (
	const Value& opValue,
	DataPtrType* type,
	Value* resultValue
	)
{
	if (!(opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr))
	{
		err::setFormatStringError (
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType ()->getTypeString ().sz (),
			type->getTypeString ().sz ()
			);
		return false;
	}

	if (((DataPtrType*) opValue.getType ())->isConstPtrType () && !type->isConstPtrType ())
	{
		setCastError (opValue, type);
		return false;
	}

	Value ptrValue;
	bool result = castOperator (
		opValue,
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Void)->getDataPtrType (DataPtrTypeKind_Normal, PtrTypeFlag_Const),
		&ptrValue
		);

	if (!result)
		return false;

	Type* targetType = type->getTargetType ();
	Value typeValue (&targetType, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_DynamicCastDataPtr);

	result = callOperator (function, ptrValue, typeValue, resultValue);
	if (!result)
		return false;

	resultValue->overrideType (type);
	return true;
}

bool
OperatorMgr::dynamicCastClassPtr (
	const Value& opValue,
	ClassPtrType* type,
	Value* resultValue
	)
{
	if (!(opValue.getType ()->getTypeKindFlags () & TypeKindFlag_ClassPtr))
	{
		err::setFormatStringError (
			"cannot dynamically cast '%s' to '%s'",
			opValue.getType ()->getTypeString ().sz (),
			type->getTypeString ().sz ()
			);
		return false;
	}

	if (((ClassPtrType*) opValue.getType ())->isConstPtrType () && !type->isConstPtrType ())
	{
		setCastError (opValue, type);
		return false;
	}

	Value ptrValue;
	m_module->m_llvmIrBuilder.createBitCast (opValue, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &ptrValue);

	Type* targetType = type->getTargetType ();
	Value typeValue (&targetType, m_module->m_typeMgr.getStdType (StdType_BytePtr));

	Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_DynamicCastClassPtr);
	m_module->m_llvmIrBuilder.createCall2 (
		function,
		function->getType (),
		ptrValue,
		typeValue,
		&ptrValue
		);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, type, resultValue);
	return true;
}

CastKind
OperatorMgr::getCastKind (
	const Value& rawOpValue,
	Type* type
	)
{
	if (rawOpValue.getValueKind () == ValueKind_Null)
		return (type->getTypeKindFlags () & TypeKindFlag_Nullable) ? CastKind_Implicit : CastKind_None;

	TypeKind typeKind = type->getTypeKind ();
	ASSERT ((size_t) typeKind < TypeKind__Count);

	CastOperator* op = m_castOperatorTable [typeKind];
	ASSERT (op); // there is always a default

	Value opValue;
	prepareOperandType (
		rawOpValue,
		&opValue,
		op->getOpFlags ()
		);

	Type* opType = opValue.getType ();
	return
		opType->cmp (type) == 0 ? CastKind_Identitiy :
		opType->getTypeKind () == TypeKind_Variant ? CastKind_ImplicitCrossFamily :
		op->getCastKind (opValue, type);
}

CastKind
OperatorMgr::getArgCastKind (
	FunctionType* functionType,
	FunctionArg* const* actualArgArray,
	size_t actualArgCount
	)
{
	sl::Array <FunctionArg*> formalArgArray = functionType->getArgArray ();
	size_t formalArgCount = formalArgArray.getCount ();

	if (actualArgCount > formalArgCount && !(functionType->getFlags () & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray [argCount - 1]->getInitializer ().isEmpty ())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	for (size_t i = 0; i < argCount; i++)
	{
		Type* formalArgType = formalArgArray [i]->getType ();
		Type* actualArgType = actualArgArray [i]->getType ();

		CastKind castKind = getCastKind (actualArgType, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getArgCastKind (
	FunctionType* functionType,
	const Value* argValueArray,
	size_t actualArgCount
	)
{
	sl::Array <FunctionArg*> formalArgArray = functionType->getArgArray ();
	size_t formalArgCount = formalArgArray.getCount ();

	if (actualArgCount > formalArgCount && !(functionType->getFlags () & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray [argCount - 1]->getInitializer ().isEmpty ())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	for (size_t i = 0; i < argCount; i++)
	{
		Type* formalArgType = formalArgArray [i]->getType ();
		Type* actualArgType = argValueArray [i].getType ();

		CastKind castKind = getCastKind (actualArgType, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getArgCastKind (
	FunctionType* functionType,
	const sl::ConstBoxList <Value>& argList
	)
{
	size_t actualArgCount = argList.getCount ();

	sl::Array <FunctionArg*> formalArgArray = functionType->getArgArray ();
	size_t formalArgCount = formalArgArray.getCount ();

	if (actualArgCount > formalArgCount && !(functionType->getFlags () & FunctionTypeFlag_VarArg))
		return CastKind_None;

	size_t argCount = formalArgCount;
	while (actualArgCount < argCount)
	{
		if (formalArgArray [argCount - 1]->getInitializer ().isEmpty ())
			return CastKind_None;

		argCount--;
	}

	CastKind worstCastKind = CastKind_Identitiy;

	sl::BoxIterator <Value> arg = argList.getHead ();
	for (size_t i = 0; i < argCount; i++, arg++)
	{
		Type* formalArgType = formalArgArray [i]->getType ();

		CastKind castKind = getCastKind (*arg, formalArgType);
		if (!castKind)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

CastKind
OperatorMgr::getFunctionCastKind (
	FunctionType* srcType,
	FunctionType* dstType
	)
{
	CastKind argCastKind = getArgCastKind (srcType, dstType->getArgArray ());
	if (!argCastKind)
		return CastKind_None;

	Type* srcReturnType = srcType->getReturnType ();
	Type* dstReturnType = dstType->getReturnType ();

	if (dstReturnType->getTypeKind () == TypeKind_Void)
		return argCastKind;

	CastKind returnCastKind = getCastKind (srcReturnType, dstReturnType);
	return AXL_MIN (argCastKind, returnCastKind);
}

CastKind
OperatorMgr::getPropertyCastKind (
	PropertyType* srcType,
	PropertyType* dstType
	)
{
	CastKind castKind = getFunctionCastKind (srcType->getGetterType (), dstType->getGetterType ());
	if (!castKind)
		return CastKind_None;

	FunctionTypeOverload* srcSetterType = srcType->getSetterType ();
	FunctionTypeOverload* dstSetterType = dstType->getSetterType ();

	CastKind worstCastKind = castKind;

	size_t count = dstSetterType->getOverloadCount ();
	for (size_t i = 0; i < count; i++)
	{
		FunctionType* dstOverload = dstSetterType->getOverload (i);

		size_t j = srcSetterType->chooseOverload (dstOverload->getArgArray (), &castKind);
		if (j == -1)
			return CastKind_None;

		if (castKind < worstCastKind)
			worstCastKind = castKind;
	}

	return worstCastKind;
}

bool
OperatorMgr::checkCastKind (
	const Value& opValue,
	Type* type
	)
{
	CastKind castKind = getCastKind (opValue, type);
	if (castKind <= CastKind_Explicit)
	{
		setCastError (opValue, type, castKind);
		return false;
	}

	return true;
}

bool
OperatorMgr::sizeofOperator (
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
	)
{
	if (dynamism == OperatorDynamism_Dynamic)
	{
		Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_DynamicSizeOf);
		return callOperator (function, opValue, resultValue);
	}

	Type* type = opValue.getType ();
	if (type->getTypeKind () == TypeKind_DataRef)
		type = ((DataPtrType*) type)->getTargetType ();

	resultValue->setConstSizeT (type->getSize (), m_module);
	return true;
}

bool
OperatorMgr::countofOperator (
	OperatorDynamism dynamism,
	const Value& rawOpValue,
	Value* resultValue
	)
{
	if (dynamism == OperatorDynamism_Dynamic)
	{
		Value opValue;
		bool result = prepareOperand (rawOpValue, &opValue);
		if (!result)
			return false;

		Type* type = opValue.getType ();
		if (type->getTypeKind () != TypeKind_DataPtr)
		{
			err::setFormatStringError ("'dynamic countof' operator is only applicable to data pointers, not to '%s'", type->getTypeString ().sz ());
			return false;
		}

		type = ((DataPtrType*) type)->getTargetType ();
		Value typeValue (&type, m_module->m_typeMgr.getStdType (StdType_BytePtr));
		Function* function = m_module->m_functionMgr.getStdFunction (StdFunc_DynamicCountOf);
		return callOperator (function, opValue, typeValue, resultValue);
	}

	Type* type = rawOpValue.getType ();
	if (type->getTypeKind () == TypeKind_DataRef)
		type = ((DataPtrType*) type)->getTargetType ();

	if (type->getTypeKind () != TypeKind_Array)
	{
		err::setFormatStringError ("'countof' operator is only applicable to arrays, not to '%s'", type->getTypeString ().sz ());
		return false;
	}

	resultValue->setConstSizeT (((ArrayType*) type)->getElementCount (), m_module);
	return true;
}

bool
OperatorMgr::typeofOperator (
	OperatorDynamism dynamism,
	const Value& opValue,
	Value* resultValue
	)
{
	Type* type = opValue.getType ();
	if (type->getTypeKind () == TypeKind_DataRef)
		type = ((DataPtrType*) type)->getTargetType ();

	resultValue->setNull (m_module);
	return true;
}

void
OperatorMgr::prepareOperandType (
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
	)
{
	if (opValue.isEmpty ())
	{
		*resultValue = opValue;
		return;
	}

	Value value = opValue;

	for (;;)
	{
		Type* type = value.getType ();
		Type* prevType = type;

		TypeKind typeKind = type->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_DataRef:
			if (!(opFlags & OpFlag_KeepDataRef))
			{
				DataPtrType* ptrType = (DataPtrType*) type;
				Type* targetType = ptrType->getTargetType ();
				TypeKind targetTypeKind = targetType->getTypeKind ();

				if (targetTypeKind == TypeKind_BitField)
				{
					BitFieldType* bitFieldType = (BitFieldType*) targetType;
					value = bitFieldType->getBaseType ();
				}
				else if (targetTypeKind != TypeKind_Array)
				{
					bool b1 = (targetType->getTypeKindFlags () & TypeKindFlag_Derivable) && (opFlags & OpFlag_KeepDerivableRef);
					bool b2 = targetTypeKind == TypeKind_Variant && (opFlags & OpFlag_KeepVariantRef);

					if (!b1 && !b2)
					{
						value = ((DataPtrType*) type)->getTargetType ();
					}
				}
				else if (opFlags & OpFlag_LoadArrayRef)
				{
					value = ((DataPtrType*) type)->getTargetType ();
				}
				else if (opFlags & OpFlag_ArrayRefToPtr)
				{
					DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

					ArrayType* arrayType = (ArrayType*) targetType;
					value = arrayType->getElementType ()->getDataPtrType (
						ptrType->getAnchorNamespace (),
						TypeKind_DataPtr,
						ptrTypeKind == DataPtrTypeKind_Thin ? DataPtrTypeKind_Thin : DataPtrTypeKind_Lean,
						ptrType->getFlags ()
						);
				}
			}

			break;

		case TypeKind_ClassRef:
			if (!(opFlags & OpFlag_KeepClassRef))
			{
				ClassPtrType* ptrType = (ClassPtrType*) type;
				ClassType* targetType = ptrType->getTargetType ();
				value = targetType->getClassPtrType (
					ptrType->getAnchorNamespace (),
					TypeKind_ClassPtr,
					ptrType->getPtrTypeKind (),
					ptrType->getFlags ()
					);
			}

			break;

		case TypeKind_FunctionRef:
			if (!(opFlags & OpFlag_KeepFunctionRef))
			{
				FunctionPtrType* ptrType = (FunctionPtrType*) value.getClosureAwareType (); // important: take closure into account!
				if (!ptrType)
					break;

				FunctionType* targetType = ptrType->getTargetType ();
				value = targetType->getFunctionPtrType (ptrType->getPtrTypeKind (), ptrType->getFlags ());
			}

			break;

		case TypeKind_PropertyRef:
			if (!(opFlags & OpFlag_KeepPropertyRef))
			{
				PropertyPtrType* ptrType = (PropertyPtrType*) value.getClosureAwareType ();
				if (!ptrType)
					break;

				PropertyType* targetType = ptrType->getTargetType ();
				if (!targetType->isIndexed ())
					value = targetType->getReturnType ();
			}

			break;

		case TypeKind_Bool:
			if (!(opFlags & OpFlag_KeepBool))
				value = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8);

			break;

		case TypeKind_Enum:
			if (!(opFlags & OpFlag_KeepEnum))
				value = ((EnumType*) type)->getBaseType ();

			break;
		}

		if (value.getType () == type)
			break;
	}

	*resultValue = value;
}

Type*
OperatorMgr::prepareOperandType (
	const Value& opValue,
	uint_t opFlags
	)
{
	Value resultValue;
	prepareOperandType (opValue, &resultValue, opFlags);
	return resultValue.getType ();
}

bool
OperatorMgr::prepareOperand (
	const Value& opValue,
	Value* resultValue,
	uint_t opFlags
	)
{
	bool result;

	if (opValue.isEmpty ())
	{
		*resultValue = opValue;
		return true;
	}

	Value value = opValue;
	for (;;)
	{
		Type* type = value.getType ();

		TypeKind typeKind = type->getTypeKind ();
		switch (typeKind)
		{
		case TypeKind_DataRef:
			if (!(opFlags & OpFlag_KeepDataRef))
			{
				DataPtrType* ptrType = (DataPtrType*) type;
				Type* targetType = ptrType->getTargetType ();
				TypeKind targetTypeKind = targetType->getTypeKind ();

				if (targetTypeKind != TypeKind_Array)
				{
					bool b1 = (targetType->getTypeKindFlags () & TypeKindFlag_Derivable) && (opFlags & OpFlag_KeepDerivableRef);
					bool b2 = targetTypeKind == TypeKind_Variant && (opFlags & OpFlag_KeepVariantRef);

					if (!b1 && !b2)
					{
						result = loadDataRef (&value);
						if (!result)
							return false;
					}
				}
				else if (opFlags & OpFlag_LoadArrayRef)
				{
					result = loadDataRef (&value);
					if (!result)
						return false;
				}
				else if (opFlags & OpFlag_ArrayRefToPtr)
				{
					DataPtrTypeKind ptrTypeKind = ptrType->getPtrTypeKind ();

					ArrayType* arrayType = (ArrayType*) ptrType->getTargetType ();
					type = arrayType->getElementType ()->getDataPtrType (
						ptrType->getAnchorNamespace (),
						TypeKind_DataPtr,
						ptrTypeKind == DataPtrTypeKind_Thin ? DataPtrTypeKind_Thin : DataPtrTypeKind_Lean,
						ptrType->getFlags ()
						);

					Value prevValue = value;
					m_module->m_llvmIrBuilder.createGep2 (value, 0, type, &value);

					if (ptrTypeKind != DataPtrTypeKind_Thin)
					{
						if (ptrTypeKind == DataPtrTypeKind_Normal)
							value.setLeanDataPtrValidator (prevValue);
						else if (prevValue.getValueKind () == ValueKind_Variable) // EDataPtrType_Lean
							value.setLeanDataPtrValidator (prevValue);
						else
							value.setLeanDataPtrValidator (prevValue.getLeanDataPtrValidator ());
					}
				}
			}

			break;

		case TypeKind_ClassRef:
			if (!(opFlags & OpFlag_KeepClassRef))
			{
				ClassPtrType* ptrType = (ClassPtrType*) type;
				ClassType* targetType = ptrType->getTargetType ();
				value.overrideType (targetType->getClassPtrType (
					ptrType->getAnchorNamespace (),
					TypeKind_ClassPtr,
					ptrType->getPtrTypeKind (),
					ptrType->getFlags ())
					);
			}

			break;

		case TypeKind_FunctionRef:
			if (!(opFlags & OpFlag_KeepFunctionRef))
			{
				FunctionPtrType* ptrType = (FunctionPtrType*) type;
				FunctionType* targetType = ptrType->getTargetType ();
				value.overrideType (targetType->getFunctionPtrType (ptrType->getPtrTypeKind (), ptrType->getFlags ()));
			}

			break;

		case TypeKind_PropertyRef:
			if (!(opFlags & OpFlag_KeepPropertyRef))
			{
				PropertyPtrType* ptrType = (PropertyPtrType*) value.getClosureAwareType ();
				if (!ptrType)
					return false;

				PropertyType* targetType = ptrType->getTargetType ();
				if (!targetType->isIndexed ())
				{
					result = getProperty (value, &value);
					if (!result)
						return false;
				}
			}

			break;

		case TypeKind_Bool:
			if (!(opFlags & OpFlag_KeepBool))
			{
				result = m_castIntFromBool.cast (value, m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8), &value);
				if (!result)
					return false;
			}

			break;

		case TypeKind_Enum:
			if (!(opFlags & OpFlag_KeepEnum))
				value.overrideType (((EnumType*) type)->getBaseType ());

			break;
		}

		if (value.getType () == type)
			break;
	}

	*resultValue = value;
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
