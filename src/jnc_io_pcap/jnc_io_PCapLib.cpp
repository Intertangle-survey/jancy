#include "pch.h"
#include "jnc_io_PCapLib.h"

//.............................................................................

AXL_EXPORT
jnc::ext::ExtensionLib* 
jncExtensionLibMain (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_pcapLibCacheSlot = host->getLibCacheSlot (jnc::io::g_pcapLibGuid);
	return mt::getSimpleSingleton <jnc::io::PCapLib> ();
}

//.............................................................................
