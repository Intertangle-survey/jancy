#pragma once

#define _JNC_EXTENSIONLIB_H

#include "jnc_Module.h"

typedef struct jnc_ExtensionLib jnc_ExtensionLib;
typedef struct jnc_DynamicExtensionLibHost jnc_DynamicExtensionLibHost;

//.............................................................................

typedef
void
jnc_ExtensionLib_AddSourcesFunc (jnc_Module* module);

typedef
void
jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc (jnc_Module* module);

typedef
int
jnc_ExtensionLib_MapFunctionsFunc (jnc_Module* module);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

struct jnc_ExtensionLib
{
	jnc_ExtensionLib_AddSourcesFunc* m_addSourcesFunc;
	jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc* m_addOpaqueClassTypeInfosFunc;
	jnc_ExtensionLib_MapFunctionsFunc* m_mapFunctionsFunc;
};

//.............................................................................

typedef 
jnc_ExtensionLib*
jnc_DynamicExtensionLibMainFunc (jnc_DynamicExtensionLibHost* host);

JNC_SELECT_ANY
char
jnc_g_dynamicExtensionLibMainFuncName [] = "jncDynamicExtensionLibMain";

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
extern jnc_DynamicExtensionLibHost* jnc_g_dynamicExtensionLibHost;
#else if (defined _JNC_CORE)
extern jnc_DynamicExtensionLibHost jnc_g_dynamicExtensionLibHostImpl;
#endif

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#define JNC_DECLARE_LIB(LibPrefix) \
	JNC_EXTERN_C \
	jnc_ExtensionLib* \
	LibPrefix##_getLib ();

#define JNC_DEFINE_LIB(LibPrefix) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addSources (jnc_Module* module); \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addOpaqueClassTypeInfos (jnc_Module* module); \
	JNC_EXTERN_C \
	int \
	LibPrefix##_mapFunctions (jnc_Module* module); \
	JNC_EXTERN_C \
	jnc_ExtensionLib* \
	LibPrefix##_getLib () \
	{ \
		static jnc_ExtensionLib lib = \
		{ \
			LibPrefix##_addSources, \
			LibPrefix##_addOpaqueClassTypeInfos, \
			LibPrefix##_mapFunctions, \
		}; \
		return &lib; \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_SOURCE_FILE_TABLE(LibPrefix) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addSources (jnc_Module* module) \
	{

#define JNC_LIB_SOURCE_FILE(fileName, sourceVar) \
		jnc_Module_addSource (module, 0, fileName, sourceVar, sizeof (sourceVar) - 1);

#define JNC_LIB_FORCED_SOURCE_FILE(fileName, sourceVar) \
		jnc_Module_addSource (module, 1, fileName, sourceVar, sizeof (sourceVar) - 1);

#define JNC_LIB_FORCED_IMPORT(fileName) \
		jnc_Module_addImport (module, fileName);

#define JNC_END_LIB_SOURCE_FILE_TABLE() \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE(LibPrefix) \
	JNC_EXTERN_C \
	void \
	LibPrefix##_addOpaqueClassTypeInfos (jnc_Module* module) \
	{

#define JNC_LIB_OPAQUE_CLASS_TYPE_TABLE_ENTRY(TypePrefix) \
		jnc_Module_addOpaqueClassTypeInfo ( \
			module, \
			TypePrefix##_getQualifiedName (), \
			TypePrefix##_getOpaqueClassTypeInfo () \
			);

#define JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE() \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_LIB_FUNCTION_MAP(LibPrefix) \
	JNC_EXTERN_C \
	int \
	LibPrefix##_mapFunctions (jnc_Module* module) \
	{ \
		int result = 1; \
		jnc_Function* function = NULL; \
		jnc_Property* prop = NULL; \
		jnc_GlobalNamespace* global = jnc_Module_getGlobalNamespace (module); \
		jnc_Namespace* nspace = jnc_ModuleItem_getNamespace ((jnc_ModuleItem*) global); \
		size_t overloadIdx = 0; \

#define JNC_END_LIB_FUNCTION_MAP() \
		return 1; \
	}

//.............................................................................

#define JNC_DECLARE_TYPE_EX(TypePrefix, JncType) \
	JNC_EXTERN_C \
	const char* \
	TypePrefix##_getQualifiedName (); \
	JNC_EXTERN_C \
	JncType* \
	TypePrefix##_getType (jnc_Module* module); \
	JNC_EXTERN_C \
	int \
	TypePrefix##_mapFunctions ( \
		jnc_Module* module, \
		int isRequired \
		);

#define JNC_DECLARE_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX ( \
		TypePrefix, \
		jnc_DerivableType \
		)

#define JNC_DECLARE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_TYPE_EX ( \
		TypePrefix, \
		jnc_ClassType \
		) \
	const void* \
	TypePrefix##_getVTable ();

#define JNC_DECLARE_OPAQUE_CLASS_TYPE(TypePrefix) \
	JNC_DECLARE_CLASS_TYPE (TypePrefix) \
	JNC_EXTERN_C \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo ();

//.............................................................................

#define JNC_DEFINE_TYPE_EX(TypePrefix, JncType, verify, qualifiedName, libGuid, cacheSlot) \
	JNC_EXTERN_C \
	const char* \
	TypePrefix##_getQualifiedName () \
	{ \
		return qualifiedName; \
	} \
	JNC_EXTERN_C \
	JncType* \
	TypePrefix##_getType (jnc_Module* module) \
	{ \
		jnc_ModuleItem* item = jnc_Module_findItem (module, qualifiedName, &(libGuid), cacheSlot); \
		return item ? verify (item, qualifiedName) : NULL; \
	}

#define JNC_DEFINE_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_DEFINE_TYPE_EX ( \
		TypePrefix, \
		jnc_DerivableType, \
		jnc_verifyModuleItemIsDerivableType, \
		qualifiedName, \
		libGuid, \
		cacheSlot \
		)

#define JNC_DEFINE_CLASS_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_DEFINE_TYPE_EX ( \
		TypePrefix, \
		jnc_ClassType, \
		jnc_verifyModuleItemIsClassType, \
		qualifiedName, \
		libGuid, \
		cacheSlot \
		)

#define JNC_DEFINE_OPAQUE_CLASS_TYPE_EX(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc, isNonCreatable) \
	JNC_DEFINE_CLASS_TYPE (TypePrefix, qualifiedName, libGuid, cacheSlot) \
	JNC_EXTERN_C \
	const jnc_OpaqueClassTypeInfo* \
	TypePrefix##_getOpaqueClassTypeInfo () \
	{ \
		static jnc_OpaqueClassTypeInfo typeInfo = \
		{ \
			sizeof (Type), \
			(jnc_MarkOpaqueGcRootsFunc*) pvoid_cast (markOpaqueGcRootsFunc), \
			isNonCreatable, \
		}; \
		return &typeInfo; \
	}

#define JNC_DEFINE_OPAQUE_CLASS_TYPE(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc) \
	JNC_DEFINE_OPAQUE_CLASS_TYPE_EX (TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc, 0)

#define JNC_DEFINE_OPAQUE_CLASS_TYPE_NC(TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc) \
	JNC_DEFINE_OPAQUE_CLASS_TYPE_EX (TypePrefix, qualifiedName, libGuid, cacheSlot, Type, markOpaqueGcRootsFunc, 1)

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_DECLARE_TYPE_STATIC_METHODS_EX(TypePrefix, JncType) \
	static \
	const char* \
	getQualifiedName () \
	{ \
		return TypePrefix##_getQualifiedName (); \
	} \
	static \
	JncType* \
	getType (jnc::Module* module) \
	{ \
		return TypePrefix##_getType (module); \
	} \
	static \
	bool \
	mapFunctions ( \
		jnc::Module* module, \
		bool isRequired \
		) \
	{ \
		return TypePrefix##_mapFunctions (module, isRequired) != 0; \
	}

#define JNC_DECLARE_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_TYPE_STATIC_METHODS_EX ( \
		TypePrefix, \
		jnc::DerivableType \
		)

#define JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_TYPE_STATIC_METHODS_EX ( \
		TypePrefix, \
		jnc::ClassType \
		) \
	static \
	const void* \
	getVTable () \
	{ \
		return TypePrefix##_getVTable (); \
	}

#define JNC_DECLARE_OPAQUE_CLASS_TYPE_STATIC_METHODS(TypePrefix) \
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (TypePrefix) \
	static \
	const jnc_OpaqueClassTypeInfo* \
	getOpaqueClassTypeInfo () \
	{ \
		return TypePrefix##_getOpaqueClassTypeInfo (); \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_CLASS_TYPE_VTABLE(TypePrefix) \
	const void* \
	TypePrefix##_getVTable () \
	{ \
		static const void* const vtable [] = \
		{

#define JNC_CLASS_TYPE_VTABLE_ENTRY(function) \
			pvoid_cast (function),

#define JNC_END_CLASS_TYPE_VTABLE() \
			NULL, \
		}; \
		return vtable; \
	}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#define JNC_BEGIN_TYPE_FUNCTION_MAP(TypePrefix) \
	int \
	TypePrefix##_mapFunctions ( \
		jnc_Module* module, \
		int isRequired \
		) \
	{ \
		int result = 1; \
		jnc_Function* function = NULL; \
		jnc_Property* prop = NULL; \
		jnc_Namespace* nspace = NULL; \
		size_t overloadIdx = 0; \
		jnc_DerivableType* type = (jnc_DerivableType*) TypePrefix##_getType (module); \
		if (!type) \
			return !isRequired; \
		nspace = jnc_ModuleItem_getNamespace ((jnc_ModuleItem*) type);

#define JNC_END_TYPE_FUNCTION_MAP() \
		return 1; \
	}

//.............................................................................

#define JNC_MAP_TYPE_EX(TypePrefix, isRequired) \
	result = TypePrefix##_mapFunctions (module, isRequired); \
	if (!result) \
		return 0;

#define JNC_MAP_TYPE(TypePrefix) \
	JNC_MAP_TYPE_EX(TypePrefix, 0)

#define JNC_MAP_TYPE_REQ(TypePrefix) \
	JNC_MAP_TYPE_EX(TypePrefix, 1)

#define JNC_MAP(function, p) \
	jnc_Module_mapFunction (module, function, pvoid_cast (p));

#define JNC_MAP_OVERLOAD(p) \
	if (function) \
	{ \
		jnc_Function* overload = jnc_Function_getOverload (function, ++overloadIdx); \
		if (!overload) \
			return 0; \
		JNC_MAP (overload, p) \
	}

#define JNC_MAP_PRECONSTRUCTOR(p) \
	function = jnc_DerivableType_getPreconstructor (type); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CONSTRUCTOR(p) \
	function = jnc_DerivableType_getConstructor (type); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_DESTRUCTOR(p) \
	function = jnc_DerivableType_getDestructor (type); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_UNARY_OPERATOR(opKind, p) \
	function = jnc_DerivableType_getUnaryOperator (type, opKind); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_BINARY_OPERATOR(opKind, p) \
	function = jnc_DerivableType_getBinaryOperator (type, opKind); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CALL_OPERATOR(p) \
	function = jnc_DerivableType_getCallOperator (); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_CAST_OPERATOR(i, p) \
	function = jnc_DerivableType_getCastOperator (i); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p)

#define JNC_MAP_FUNCTION(name, p) \
	function = jnc_Namespace_findFunction (nspace, name); \
	if (function) \
	{ \
		overloadIdx = 0; \
		JNC_MAP (function, p) \
	}

#define JNC_MAP_PROPERTY_GETTER(prop, p) \
	function = jnc_Property_getGetter (prop); \
	ASSERT (function); \
	overloadIdx = 0; \
	JNC_MAP (function, p);

#define JNC_MAP_PROPERTY_SETTER(prop, p) \
	function = jnc_Property_getSetter (prop); \
	if (!function) \
		return 0; \
	overloadIdx = 0; \
	JNC_MAP (function, p);

#define JNC_MAP_PROPERTY(name, getter, setter) \
	prop = jnc_Namespace_findProperty (nspace, name); \
	if (!prop) \
		return 0; \
	JNC_MAP_PROPERTY_GETTER (prop, getter); \
	JNC_MAP_PROPERTY_SETTER (prop, setter);

#define JNC_MAP_CONST_PROPERTY(name, getter) \
	prop = jnc_Namespace_findProperty (nspace, name); \
	if (!prop) \
		return 0; \
	JNC_MAP_PROPERTY_GETTER (prop, getter);

#define JNC_MAP_AUTOGET_PROPERTY(name, setter) \
	prop = jnc_Namespace_findProperty (nspace, name); \
	if (!prop) \
		return 0; \
	JNC_MAP_PROPERTY_SETTER (prop, setter);

//.............................................................................

// standard libraries

JNC_DECLARE_LIB (jnc_CoreLib)
JNC_DECLARE_LIB (jnc_StdLib)
JNC_DECLARE_LIB (jnc_SysLib)

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

// implicit tail-padding (might lead to ABI-incompatibility if omitted)

#if (_AXL_CPP == AXL_CPP_MSC)

template <typename T>
class BaseTailPadding
{
	// microsoft compiler does not re-use tail padding
};

#else 

template <typename T>
class BaseTailPadding
{
private:
	struct TailPaddingCheck: T
	{
		char m_field; // this field might be allocated in T's tail-padding
	};

	char m_tailPadding [sizeof (T) - offsetof (TailPaddingCheck, m_field)];
};

#endif

//.............................................................................

typedef jnc_ExtensionLib_AddSourcesFunc ExtensionLib_AddSourcesFunc;
typedef jnc_ExtensionLib_AddOpaqueClassTypeInfosFunc ExtensionLib_AddOpaqueClassTypeInfosFunc;
typedef jnc_ExtensionLib_MapFunctionsFunc ExtensionLib_MapFunctionsFunc;

typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_DynamicExtensionLibHost DynamicExtensionLibHost;
typedef jnc_DynamicExtensionLibMainFunc DynamicExtensionLibMainFunc;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline 
ExtensionLib*
StdLib_getLib ()
{
	return jnc_StdLib_getLib ();
}

inline 
ExtensionLib*
SysLib_getLib ()
{
	return jnc_SysLib_getLib ();
}

//.............................................................................

} // namespace jnc

#endif __cplusplus

