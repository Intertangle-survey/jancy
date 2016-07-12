#include "pch.h"
#include "jnc_Module.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif (defined _JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Namespace*
jnc_Module_getGlobalNamespace (jnc_Module* module)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_getGlobalNamespaceFunc (module);
}

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_findItemFunc (module, name, libGuid, itemCacheSlot);
}

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_mapFunctionFunc (module, function, p);
}

JNC_EXTERN_C
bool
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addImportFunc (module, fileName);
}

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t size
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_addSourceFunc (module, fileName, source, size);
}

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_verifyModuleItemIsDerivableTypeFunc (item, name);
}

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc_g_dynamicExtensionLibHost->m_moduleFuncTable->m_verifyModuleItemIsClassTypeFunc (item, name);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_Namespace*
jnc_Module_getGlobalNamespace (jnc_Module* module)
{
	return module->m_namespaceMgr.getGlobalNamespace ();
}

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	)
{
	return module->m_extensionLibMgr.findItem (name, *libGuid, itemCacheSlot);
}

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	)
{
	module->mapFunction (function, p);
}

JNC_EXTERN_C
bool
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	)
{
	return module->m_importMgr.addImport (fileName);
}

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	const char* fileName,
	const char* source,
	size_t length
	)
{
	module->m_importMgr.addSource (fileName, axl::sl::StringRef (source, length));
}

JNC_EXTERN_C
jnc_DerivableType*
jnc_verifyModuleItemIsDerivableType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc::ct::verifyModuleItemIsDerivableType (item, name);
}

JNC_EXTERN_C
jnc_ClassType*
jnc_verifyModuleItemIsClassType (
	jnc_ModuleItem* item,
	const char* name
	)
{
	return jnc::ct::verifyModuleItemIsClassType (item, name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
