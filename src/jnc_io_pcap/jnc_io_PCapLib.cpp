#include "pch.h"
#include "jnc_io_PCapLib.h"

#ifdef _JDK_FOUND
#	include "jnc_ext_JavaJniImpl.h"
#endif

//.............................................................................

extern "C"
AXL_EXPORT
jnc::ext::ExtensionLib* 
jncExtensionLibMain (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_pcapLibCacheSlot = host->getLibCacheSlot (jnc::io::g_pcapLibGuid);
	return mt::getSimpleSingleton <jnc::io::PCapLib> ();
}

jnc::ext::ExtensionLib*
getExtensionLib ()
{
	return mt::getSimpleSingleton <jnc::io::PCapLib> ();
}

//.............................................................................
