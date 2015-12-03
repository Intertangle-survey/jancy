#include "pch.h"
#include "jnc_io_SshLib.h"

//.............................................................................

extern "C"
AXL_EXPORT
jnc::ext::ExtensionLib*
jncExtensionLibMain (jnc::ext::ExtensionLibHost* host)
{
	jnc::ext::g_extensionLibHost = host;
	jnc::io::g_sshLibCacheSlot = host->getLibCacheSlot (jnc::io::g_sshLibGuid);
	return mt::getSimpleSingleton <jnc::io::SshLib> ();
}

jnc::ext::ExtensionLib*
getExtensionLib ()
{
	return mt::getSimpleSingleton <jnc::io::SshLib> ();
}

//.............................................................................
