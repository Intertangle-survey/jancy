#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"

namespace jnc {
namespace std {
		
//.............................................................................

struct Error: err::ErrorHdr
{
	JNC_BEGIN_TYPE_MAP ("std.Error", g_stdLibCacheSlot, StdLibCacheSlot_Error)
		JNC_MAP_CONST_PROPERTY ("m_description", getDescription_s)
	JNC_END_TYPE_MAP ()

public:
	DataPtr
	getDescription ();

protected:
	static
	DataPtr
	getDescription_s (DataPtr selfPtr)
	{
		return ((Error*) selfPtr.m_p)->getDescription ();
	}
};

//.............................................................................

} // namespace std
} // namespace jnc
