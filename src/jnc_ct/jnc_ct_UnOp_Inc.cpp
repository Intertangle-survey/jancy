#include "pch.h"
#include "jnc_ct_UnOp_Inc.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Type*
UnOp_PreInc::getResultType (const Value& opValue)
{
	return opValue.getType ();
}

bool
UnOp_PreInc::op (
	const Value& opValue,
	Value* resultValue
	)
{
	Value oneValue;
	oneValue.setConstInt32 (1, m_module);
	BinOpKind binOpKind = m_opKind == UnOpKind_PreInc ? BinOpKind_AddAssign : BinOpKind_SubAssign;
		
	bool result = m_module->m_operatorMgr.binaryOperator (binOpKind, opValue, oneValue);
	if (!result)
		return false;

	*resultValue = opValue;
	return true;
}

//.............................................................................

Type*
UnOp_PostInc::getResultType (const Value& opValue)
{
	Value oldValue;
	m_module->m_operatorMgr.prepareOperandType (opValue, &oldValue);
	return oldValue.getType ();
}

bool
UnOp_PostInc::op (
	const Value& opValue,
	Value* resultValue
	)
{
	bool result;

	Value oldValue;
	result = m_module->m_operatorMgr.prepareOperand (opValue, &oldValue);
	if (!result)
		return false;

	Value oneValue;
	oneValue.setConstInt32 (1, m_module);
	BinOpKind binOpKind = m_opKind == UnOpKind_PostInc ? BinOpKind_AddAssign : BinOpKind_SubAssign;
		
	result = m_module->m_operatorMgr.binaryOperator (binOpKind, opValue, oneValue);
	if (!result)
		return false;

	*resultValue = oldValue;
	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
