#include "pch.h"
#include "jnc_ClassType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

JNC_EXTERN_C
const char*
jnc_getClassPtrTypeKindString (jnc_ClassPtrTypeKind ptrTypeKind)
{
	static const char* stringTable [jnc_ClassPtrTypeKind__Count] = 
	{
		"strong", // EClassPtrType_Normal = 0,
		"weak",   // EClassPtrType_Weak,
	};

	return (size_t) ptrTypeKind < jnc_ClassPtrTypeKind__Count ? 
		stringTable [ptrTypeKind] : 
		"undefined-class-ptr-kind";
}

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_MulticastClassType_getTargetType (jnc_MulticastClassType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_multicastClassTypeFuncTable->m_getTargetTypeFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_MulticastClassType_getMethod (
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
	)
{
	return jnc_g_dynamicExtensionLibHost->m_multicastClassTypeFuncTable->m_getMethodFunc (type, method);
}

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_McSnapshotClassType_getTargetType (jnc_McSnapshotClassType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_mcSnapshotClassTypeFuncTable->m_getTargetTypeFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshotClassType_getMethod (
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	)
{
	return jnc_g_dynamicExtensionLibHost->m_mcSnapshotClassTypeFuncTable->m_getMethodFunc (type, method);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_ClassTypeKind
jnc_ClassType_getClassTypeKind (jnc_ClassType* type)
{
	return type->getClassTypeKind ();
}

JNC_EXTERN_C
jnc_StructType*
jnc_ClassType_getIfaceStructType (jnc_ClassType* type)
{
	return type->getIfaceStructType ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_MulticastClassType_getTargetType (jnc_MulticastClassType* type)
{
	return type->getTargetType ();
}

JNC_EXTERN_C
jnc_Function*
jnc_MulticastClassType_getMethod (
	jnc_MulticastClassType* type,
	jnc_MulticastMethodKind method
	)
{
	return type->getMethod (method);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_FunctionPtrType*
jnc_McSnapshotClassType_getTargetType (jnc_McSnapshotClassType* type)
{
	return type->getTargetType ();
}

JNC_EXTERN_C
jnc_Function*
jnc_McSnapshotClassType_getMethod (
	jnc_McSnapshotClassType* type,
	jnc_McSnapshotMethodKind method
	)
{
	return type->getMethod (method);
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB

//.............................................................................
