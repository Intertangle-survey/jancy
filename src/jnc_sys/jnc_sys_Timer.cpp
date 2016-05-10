#include "pch.h"
#include "jnc_sys_Timer.h"

namespace jnc {
namespace sys {

//.............................................................................

bool 
AXL_CDECL
Timer::start (
	rt::FunctionPtr ptr,
	uint64_t dueTime,
	uint_t interval
	)
{
	bool result;

	stop ();
	
	m_timerFuncPtr = ptr;
	m_dueTime = dueTime;
	m_interval = interval;
	m_stopEvent.reset ();

	result = m_thread.start ();
	if (!result)
	{
		m_timerFuncPtr = rt::g_nullFunctionPtr;
		return false;
	}

	return true;
}

void
AXL_CDECL
Timer::stop ()
{
	m_stopEvent.signal ();

	rt::enterWaitRegion (m_runtime);
	m_thread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);
		
	m_timerFuncPtr = rt::g_nullFunctionPtr;
}

void
Timer::threadFunc ()
{
	bool result;

	uint64_t timestamp = axl::sys::getTimestamp ();
	if (m_dueTime > timestamp)
	{
		uint_t delay = (uint_t) ((m_dueTime - timestamp) / 10000);
		result = m_stopEvent.wait (delay);
		if (result)
			return;
	}

	rt::callVoidFunctionPtr (m_runtime, m_timerFuncPtr);

	if (!m_interval || m_interval == -1)
		return;

	for (;;)
	{
		result = m_stopEvent.wait (m_interval);
		if (result)
			break;

		rt::callVoidFunctionPtr (m_runtime, m_timerFuncPtr);
	}
}

//.............................................................................

} // namespace sys
} // namespace jnc
