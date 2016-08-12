#pragma once

#define _JNC_MODULE_H

#include "jnc_AttributeBlock.h"
#include "jnc_Namespace.h"
#include "jnc_Variable.h"
#include "jnc_Function.h"
#include "jnc_Property.h"
#include "jnc_ArrayType.h"
#include "jnc_BitFieldType.h"
#include "jnc_EnumType.h"
#include "jnc_DerivableType.h"
#include "jnc_StructType.h"
#include "jnc_UnionType.h"
#include "jnc_ClassType.h"
#include "jnc_Unit.h"

//.............................................................................

enum jnc_ModuleCompileFlag
{
	jnc_ModuleCompileFlag_DebugInfo                            = 0x0001,
	jnc_ModuleCompileFlag_McJit                                = 0x0002,
	jnc_ModuleCompileFlag_SimpleGcSafePoint                    = 0x0004,
	jnc_ModuleCompileFlag_GcSafePointInPrologue                = 0x0010,
	jnc_ModuleCompileFlag_GcSafePointInInternalPrologue        = 0x0020,
	jnc_ModuleCompileFlag_CheckStackOverflowInPrologue         = 0x0040,
	jnc_ModuleCompileFlag_CheckStackOverflowInInternalPrologue = 0x0080,
	jnc_ModuleCompileFlag_CheckDivByZero                       = 0x0100,
	jnc_ModuleCompileFlag_Documentation                        = 0x0200,
	jnc_ModuleCompileFlag_IgnoreOpaqueClassTypeInfo            = 0x0200,

	jnc_ModuleCompileFlag_StdFlags = 
		jnc_ModuleCompileFlag_GcSafePointInPrologue | 
		jnc_ModuleCompileFlag_GcSafePointInInternalPrologue |
		jnc_ModuleCompileFlag_CheckStackOverflowInPrologue |
		jnc_ModuleCompileFlag_CheckDivByZero
#if (_AXL_ENV == AXL_ENV_WIN && _AXL_CPU != AXL_CPU_X86)
		// SEH on amd64/ia64 relies on being able to walk the stack which is not as 
		// reliable as frame-based SEH on x86. therefore, use write barrier for 
		// safe points on windows if and only if it's x86 
		| jnc_ModuleCompileFlag_SimpleGcSafePoint
#elif (_AXL_ENV == AXL_ENV_POSIX)
		| jnc_ModuleCompileFlag_McJit
#endif
};

//.............................................................................

enum jnc_ModuleCompileState
{
	jnc_ModuleCompileState_Idle,
	jnc_ModuleCompileState_CalcLayout,
	jnc_ModuleCompileState_LayoutCalculated,
	jnc_ModuleCompileState_Compiling,
	jnc_ModuleCompileState_Compiled,
	jnc_ModuleCompileState_Jitting,
	jnc_ModuleCompileState_Jitted,
};

//.............................................................................

JNC_EXTERN_C
jnc_Module* 
jnc_Module_create ();

JNC_EXTERN_C
void
jnc_Module_destroy (jnc_Module* module);

JNC_EXTERN_C
void
jnc_Module_clear (jnc_Module* module);

JNC_EXTERN_C
int
jnc_Module_initialize (
	jnc_Module* module,
	const char* tag,
	uint_t compileFlags
	);

JNC_EXTERN_C
uint_t
jnc_Module_getCompileFlags (jnc_Module* module);

JNC_EXTERN_C
jnc_ModuleCompileState
jnc_Module_getCompileState (jnc_Module* module);

JNC_EXTERN_C
jnc_GlobalNamespace*
jnc_Module_getGlobalNamespace (jnc_Module* module);

JNC_EXTERN_C
jnc_Type*
jnc_Module_getPrimitiveType (
	jnc_Module* module,
	jnc_TypeKind typeKind
	);

JNC_EXTERN_C
jnc_Type*
jnc_Module_getStdType (
	jnc_Module* module,
	jnc_StdType stdType
	);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Module_findItem (
	jnc_Module* module,
	const char* name,
	const jnc_Guid* libGuid,
	size_t itemCacheSlot
	);

JNC_EXTERN_C
void
jnc_Module_mapFunction (
	jnc_Module* module,
	jnc_Function* function,
	void* p
	);

JNC_EXTERN_C
void
jnc_Module_addSource (
	jnc_Module* module,
	int isForced,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length = -1
	);

JNC_EXTERN_C
void
jnc_Module_addImportDir (
	jnc_Module* module,
	const char* dir
	);

JNC_EXTERN_C
void
jnc_Module_addImport (
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
void
jnc_Module_addOpaqueClassTypeInfo (
	jnc_Module* module,
	const char* qualifiedName,
	const jnc_OpaqueClassTypeInfo* info
	);

JNC_EXTERN_C
void
jnc_Module_addStaticLib (
	jnc_Module* module,
	jnc_ExtensionLib* lib
	);

JNC_EXTERN_C
int
jnc_Module_parse (
	jnc_Module* module,
	jnc_ExtensionLib* lib,
	const char* fileName,
	const char* source,
	size_t length
	);

JNC_EXTERN_C
int
jnc_Module_parseFile (
	jnc_Module* module,
	const char* fileName
	);

JNC_EXTERN_C
int
jnc_Module_parseImports (jnc_Module* module);

JNC_EXTERN_C
int
jnc_Module_calcLayout (jnc_Module* module);

JNC_EXTERN_C
int
jnc_Module_compile (jnc_Module* module);

JNC_EXTERN_C
int
jnc_Module_jit (jnc_Module* module);

JNC_EXTERN_C
int
jnc_Module_generateDocumentation (
	jnc_Module* module,
	const char* outputDir
	);

JNC_EXTERN_C
const char*
jnc_Module_createLlvmIrString_v (jnc_Module* module);

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Module
{
	static
	jnc_Module* 
	create ()
	{
		return jnc_Module_create ();
	}

	void
	destroy ()
	{
		jnc_Module_destroy (this);
	}

	void
	clear ()
	{
		jnc_Module_clear (this);
	}

	bool
	initialize (
		const char* tag,
		uint_t compileFlags = jnc_ModuleCompileFlag_StdFlags
		)
	{
		return jnc_Module_initialize (this, tag, compileFlags) != 0;
	}

	uint_t
	getCompileFlags ()
	{
		return jnc_Module_getCompileFlags (this);
	}

	jnc_ModuleCompileState
	getCompileState ()
	{
		return jnc_Module_getCompileState (this);
	}

	jnc_GlobalNamespace*
	getGlobalNamespace ()
	{
		return jnc_Module_getGlobalNamespace (this);
	}

	jnc_Type*
	getPrimitiveType (jnc_TypeKind typeKind)
	{
		return jnc_Module_getPrimitiveType (this, typeKind);
	}

	jnc_Type*
	getStdType (jnc_StdType stdType)
	{
		return jnc_Module_getStdType (this, stdType);
	}

	jnc_ModuleItem*
	findItem (
		const char* name,
		const jnc_Guid* libGuid = NULL,
		size_t itemCacheSlot = -1
		)
	{
		return jnc_Module_findItem (this, name, libGuid, itemCacheSlot);
	}

	void
	mapFunction (
		jnc_Function* function,
		void* p
		)
	{
		jnc_Module_mapFunction (this, function, p);
	}


	void
	addSource (
		bool isForced,
		jnc_ExtensionLib* lib,
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource (this, isForced, lib, fileName, source, length);
	}

	void
	addSource (
		jnc_ExtensionLib* lib,
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		jnc_Module_addSource (this, false, lib, fileName, source, length);
	}

	void
	addImportDir (const char* dir)
	{
		jnc_Module_addImportDir (this, dir);
	}

	void
	addImport (const char* fileName)
	{
		jnc_Module_addImport (this, fileName);
	}

	void
	addOpaqueClassTypeInfo (
		const char* qualifiedName,
		const jnc_OpaqueClassTypeInfo* info
		)
	{
		jnc_Module_addOpaqueClassTypeInfo (this, qualifiedName, info);
	}

	void
	addStaticLib (jnc_ExtensionLib* lib)
	{
		jnc_Module_addStaticLib (this, lib);
	}

	bool
	parse (
		jnc_ExtensionLib* lib,
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		return jnc_Module_parse (this, lib, fileName, source, length) != 0;
	}

	bool
	parse (
		const char* fileName,
		const char* source,
		size_t length = -1
		)
	{
		return jnc_Module_parse (this, NULL, fileName, source, length) != 0;
	}

	bool
	parseFile (const char* fileName)
	{
		return jnc_Module_parseFile (this, fileName) != 0;
	}

	bool
	parseImports ()
	{
		return jnc_Module_parseImports (this) != 0;
	}

	bool
	calcLayout ()
	{
		return jnc_Module_calcLayout (this) != 0;
	}

	bool
	compile ()
	{
		return jnc_Module_compile (this) != 0;
	}

	bool
	jit ()
	{
		return jnc_Module_jit (this) != 0;
	}

	const char*
	createLlvmIrString_v ()
	{
		return jnc_Module_createLlvmIrString_v (this);
	}

	bool
	generateDocumentation (const char* outputDir)
	{
		return jnc_Module_generateDocumentation (this, outputDir) != 0;
	}
};
#endif // _JNC_CORE

//.............................................................................

JNC_EXTERN_C
void
jnc_initialize ();

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_ModuleCompileState ModuleCompileState;

const ModuleCompileState
	ModuleCompileState_Idle             = jnc_ModuleCompileState_Idle,
	ModuleCompileState_CalcLayout       = jnc_ModuleCompileState_CalcLayout, 
	ModuleCompileState_LayoutCalculated = jnc_ModuleCompileState_LayoutCalculated,
	ModuleCompileState_Compiling        = jnc_ModuleCompileState_Compiling,
	ModuleCompileState_Compiled         = jnc_ModuleCompileState_Compiled,
	ModuleCompileState_Jitting          = jnc_ModuleCompileState_Jitting,
	ModuleCompileState_Jitted           = jnc_ModuleCompileState_Jitted;

//.............................................................................

typedef jnc_ModuleCompileFlag ModuleCompileFlag;

const ModuleCompileFlag
	ModuleCompileFlag_DebugInfo                            = jnc_ModuleCompileFlag_DebugInfo,
	ModuleCompileFlag_McJit                                = jnc_ModuleCompileFlag_McJit,
	ModuleCompileFlag_SimpleGcSafePoint                    = jnc_ModuleCompileFlag_SimpleGcSafePoint,
	ModuleCompileFlag_GcSafePointInPrologue                = jnc_ModuleCompileFlag_GcSafePointInPrologue,
	ModuleCompileFlag_GcSafePointInInternalPrologue        = jnc_ModuleCompileFlag_GcSafePointInInternalPrologue,
	ModuleCompileFlag_CheckStackOverflowInPrologue         = jnc_ModuleCompileFlag_CheckStackOverflowInPrologue,
	ModuleCompileFlag_CheckStackOverflowInInternalPrologue = jnc_ModuleCompileFlag_CheckStackOverflowInInternalPrologue,
	ModuleCompileFlag_CheckDivByZero                       = jnc_ModuleCompileFlag_CheckDivByZero,
	ModuleCompileFlag_Documentation                        = jnc_ModuleCompileFlag_Documentation,
	ModuleCompileFlag_IgnoreOpaqueClassTypeInfo            = jnc_ModuleCompileFlag_IgnoreOpaqueClassTypeInfo,
	ModuleCompileFlag_StdFlags                             = jnc_ModuleCompileFlag_StdFlags;

//.............................................................................

class AutoModule
{
protected:
	Module* m_module;

public:
	AutoModule ()
	{
		m_module = jnc_Module_create ();
	}

	~AutoModule ()
	{
		if (m_module)
			jnc_Module_destroy (m_module);
	}
	
	operator Module* ()
	{
		return m_module;
	}

	Module* 
	operator -> ()
	{
		return m_module;
	}
	
	Module*
	p ()
	{
		return m_module;
	}
};

//.............................................................................

inline
void
initialize ()
{
	jnc_initialize ();
}

//.............................................................................

} // namespace jnc

#endif // __cplusplus
