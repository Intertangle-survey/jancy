#include "pch.h"
#include "jnc_ct_ScheduleLauncherFunction.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

bool
ScheduleLauncherFunction::compile ()
{
	bool result;

	size_t argCount = m_type->getArgArray ().getCount ();

	char buffer [256];
	sl::Array <Value> argValueArray (ref::BufKind_Stack, buffer, sizeof (buffer));
	argValueArray.setCount (argCount);

	m_module->m_functionMgr.internalPrologue (this, argValueArray, argCount);

	Value functionPtrValue = argValueArray [0];
	Value schedulerValue = argValueArray [1];
	Value scheduleValue;

	result = m_module->m_operatorMgr.memberOperator (schedulerValue, "schedule", &scheduleValue);
	if (!result)
		return false;

	if (argCount > 2)
	{
		sl::BoxList <Value> argList;
		for (size_t i = 2; i < argCount; i++)
			argList.insertTail (argValueArray [i]);

		result = m_module->m_operatorMgr.closureOperator (&functionPtrValue, &argList);
		if (!result)
			return false;
	}

	result = m_module->m_operatorMgr.callOperator (scheduleValue, functionPtrValue);
	if (!result)
		return false;

	m_module->m_functionMgr.internalEpilogue ();
	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
