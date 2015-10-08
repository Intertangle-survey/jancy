#include "pch.h"
#include "jnc_io_IoLib.h"

//.............................................................................

AXL_EXPORT
jnc::ext::ExtensionLib* 
jncExtensionLibMain (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_ioLibCacheSlot = host->getLibCacheSlot (jnc::io::g_ioLibGuid);
	return mt::getSimpleSingleton <jnc::io::IoLib> ();
}

//.............................................................................
