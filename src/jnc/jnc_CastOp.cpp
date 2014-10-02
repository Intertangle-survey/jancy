#include "pch.h"
#include "jnc_CastOp.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

err::Error
setCastError (
	const Value& opValue,
	Type* dstType
	)
{
	Type* srcType = opValue.getType ();
	if (!srcType)
	{
		ASSERT (opValue.getValueKind () == ValueKind_Function);
		Function* function = (Function*) opValue.getFunction ();

		return err::setFormatStringError (
			"not enough information to select one of %d overloads of '%s'",
			function->getOverloadCount (),
			function->m_tag.cc () // thanks a lot gcc
			);
	}

	return err::setFormatStringError (
		"cannot convert from '%s' to '%s'",
		opValue.getValueKind () == ValueKind_Null ? "null" : opValue.getType ()->getTypeString ().cc (), 
		dstType->getTypeString ().cc ()
		);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
castOperator (
	Module* module,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	return module->m_operatorMgr.castOperator (opValue, type, resultValue);
}

//.............................................................................

CastOperator::CastOperator()
{
	m_module = getCurrentThreadModule ();
	ASSERT (m_module);

	m_opFlags = 0;
}

bool
CastOperator::cast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue_o
	)
{
	ValueKind opValueKind = opValue.getValueKind ();
	if (opValueKind != ValueKind_Const)
		return llvmCast (storageKind, opValue, type, resultValue_o);
	
	Value resultValue;
	
	bool result = 
		resultValue.createConst (NULL, type) && 
		constCast (opValue, type, resultValue.getConstData ());

	if (!result)
		return false;

	*resultValue_o = resultValue;
	return true;
}

bool
CastOperator::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	// fail by default; if const-cast is supported then override and implement

	err::setFormatStringError (
		"cannot convert constant from '%s' to '%s'",
		opValue.getType ()->getTypeString ().cc (), 
		type->getTypeString ().cc ()
		);

	return false;
}

//.............................................................................

bool
Cast_Copy::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	size_t srcSize = opValue.getType ()->getSize ();
	size_t dstSize = type->getSize ();

	ASSERT (srcSize == dstSize);

	memcpy (dst, opValue.getConstData (), dstSize);
	return true;
}

bool
Cast_Copy::llvmCast (
	StorageKind storageKind,
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	m_module->m_llvmIrBuilder.createBitCast (opValue, type, resultValue);
	return true;
}

//.............................................................................

CastKind
Cast_Master::getCastKind (
	const Value& rawOpValue,
	Type* type
	)
{
	if (!rawOpValue.getType ())
		return CastKind_None;

	CastOperator* op = getCastOperator (rawOpValue, type);
	if (!op)
		return CastKind_None;

	Value opValue = rawOpValue;

	uint_t opFlags = op->getOpFlags ();
	if (opFlags != m_opFlags)
		m_module->m_operatorMgr.prepareOperandType (&opValue, opFlags);

	return op->getCastKind (opValue, type);
}

bool
Cast_Master::constCast (
	const Value& rawOpValue,
	Type* type,
	void* dst
	)
{
	CastOperator* op = getCastOperator (rawOpValue, type);
	if (!op)
	{
		setCastError (rawOpValue, type);
		return false;
	}

	Value opValue = rawOpValue;

	uint_t opFlags = op->getOpFlags ();
	if (opFlags != m_opFlags)
	{
		bool result = m_module->m_operatorMgr.prepareOperand (&opValue, opFlags);
		if (!result)
			return false;
	}

	return op->constCast (opValue, type, dst);
}

bool
Cast_Master::llvmCast (
	StorageKind storageKind,
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	CastOperator* op = getCastOperator (rawOpValue, type);
	if (!op)
	{
		setCastError (rawOpValue, type);
		return false;
	}

	Value opValue = rawOpValue;

	uint_t opFlags = op->getOpFlags ();
	if (opFlags != m_opFlags)
	{
		bool result = m_module->m_operatorMgr.prepareOperand (&opValue, opFlags);
		if (!result)
			return false;
	}
		
	return op->llvmCast (storageKind, opValue, type, resultValue);
}

//.............................................................................

CastKind
Cast_SuperMaster::getCastKind (
	const Value& rawOpValue,
	Type* type
	)
{
	if (!rawOpValue.getType ())
		return CastKind_None;

	CastOperator* operator1 = NULL;
	CastOperator* operator2 = NULL;
	Type* intermediateType = NULL;

	bool result = getCastOperators (
		rawOpValue, 
		type,
		&operator1, 
		&operator2, 
		&intermediateType
		);

	if (!result)
		return CastKind_None;

	ASSERT (operator1);

	Value opValue = rawOpValue;

	uint_t opFlags1 = operator1->getOpFlags ();
	if (opFlags1 != m_opFlags)
		m_module->m_operatorMgr.prepareOperandType (&opValue, opFlags1);

	if (!operator2) 
		return operator1->getCastKind (opValue, type);
	
	CastKind castKind1 = operator1->getCastKind (opValue, intermediateType);
	CastKind castKind2 = operator2->getCastKind (intermediateType, type);
	return AXL_MIN (castKind1, castKind2);
}

bool
Cast_SuperMaster::constCast (
	const Value& rawOpValue,
	Type* type,
	void* dst
	)
{
	CastOperator* operator1 = NULL;
	CastOperator* operator2 = NULL;
	Type* intermediateType = NULL;

	bool result = getCastOperators (
		rawOpValue, 
		type,
		&operator1, 
		&operator2, 
		&intermediateType
		);

	if (!result)
	{
		setCastError (rawOpValue, type);
		return false;
	}

	ASSERT (operator1);

	Value srcValue = rawOpValue;

	uint_t opFlags1 = operator1->getOpFlags ();
	if (opFlags1 != m_opFlags)
	{
		bool result = m_module->m_operatorMgr.prepareOperand (&srcValue, opFlags1);
		if (!result)
			return false;
	}

	if (!operator2) 
		return operator1->constCast (srcValue, type, dst);

	Value tmpValue;
	return 
		tmpValue.createConst (NULL, intermediateType) &&
		operator1->constCast (srcValue, intermediateType, tmpValue.getConstData ()) &&
		operator2->constCast (tmpValue, type, dst);
}

bool
Cast_SuperMaster::llvmCast (
	StorageKind storageKind,
	const Value& rawOpValue,
	Type* type,
	Value* resultValue
	)
{
	CastOperator* operator1 = NULL;
	CastOperator* operator2 = NULL;
	Type* intermediateType = NULL;

	bool result = getCastOperators (
		rawOpValue, 
		type,
		&operator1, 
		&operator2, 
		&intermediateType
		);

	if (!result)
	{
		setCastError (rawOpValue, type);
		return false;
	}

	ASSERT (operator1);

	Value opValue = rawOpValue;

	uint_t opFlags1 = operator1->getOpFlags ();
	if (opFlags1 != m_opFlags)
	{
		bool result = m_module->m_operatorMgr.prepareOperand (&opValue, opFlags1);
		if (!result)
			return false;
	}

	if (!operator2) 
		return operator1->llvmCast (storageKind, opValue, type, resultValue);

	Value tmpValue;
	return 
		operator1->llvmCast (storageKind, opValue, intermediateType, &tmpValue) &&
		operator2->llvmCast (storageKind, tmpValue, type, resultValue);
}

//.............................................................................

} // namespace jnc {
