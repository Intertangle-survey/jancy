#include "pch.h"
#include "jnc_sys_Lock.h"
#include "jnc_Runtime.h"

namespace jnc {
namespace sys {

//.............................................................................

JNC_BEGIN_TYPE_FUNCTION_MAP (Lock)
	JNC_MAP_CONSTRUCTOR (&sl::construct <Lock>)
	JNC_MAP_DESTRUCTOR (&sl::destruct <Lock>)
	JNC_MAP_FUNCTION ("lock", &Lock::lock)
	JNC_MAP_FUNCTION ("unlock", &Lock::unlock)
JNC_END_TYPE_FUNCTION_MAP ()

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
AXL_CDECL
Lock::lock ()
{
	GcHeap* gcHeap = getCurrentThreadGcHeap ();
	ASSERT (gcHeap);

	gcHeap->enterWaitRegion ();
	m_lock.lock ();
	gcHeap->leaveWaitRegion ();
}

//.............................................................................

} // namespace sys
} // namespace jnc
