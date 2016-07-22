#include "pch.h"
#include "jnc_AttributeBlock.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_Attribute_getItemDecl (jnc_Attribute* attribute)
{
	return attribute;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_ModuleItemDecl*
jnc_AttributeBlock_getItemDecl (jnc_AttributeBlock* block)
{
	return block;
}

JNC_EXTERN_C
size_t
jnc_AttributeBlock_getAttributeCount (jnc_AttributeBlock* block)
{
	return block->getAttributeArray ().getCount ();
}

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_getAttribute (
	jnc_AttributeBlock* block,
	size_t index
	)
{
	return block->getAttributeArray () [index];
}

JNC_EXTERN_C
jnc_Attribute*
jnc_AttributeBlock_findAttribute (
	jnc_AttributeBlock* block,
	const char* name
	)
{
	return block->findAttribute (name);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
