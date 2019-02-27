//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#define _JNC_NAMESPACE_H

#include "jnc_ModuleItem.h"

/**

\defgroup namespace Namespace
	\ingroup module-subsystem
	\import{jnc_Namespace.h}

\addtogroup namespace
@{

\struct jnc_Namespace
	\verbatim

	Opaque structure used as a handle to Jancy namespace.

	Use functions from the :ref:`Namespace<cid-namespace>` to access and manage the contents of this structure.

	\endverbatim

*/

//..............................................................................

enum jnc_NamespaceKind
{
	jnc_NamespaceKind_Undefined,
	jnc_NamespaceKind_Global,
	jnc_NamespaceKind_Scope,
	jnc_NamespaceKind_Type,
	jnc_NamespaceKind_Extension,
	jnc_NamespaceKind_Property,
	jnc_NamespaceKind_PropertyTemplate,
	jnc_NamespaceKind_DynamicLib,
	jnc_NamespaceKind__Count,
};

typedef enum jnc_NamespaceKind jnc_NamespaceKind;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
const char*
jnc_getNamespaceKindString(jnc_NamespaceKind namespaceKind);

//..............................................................................

JNC_EXTERN_C
size_t
jnc_Namespace_getItemCount(jnc_Namespace* nspace);

JNC_EXTERN_C
jnc_ModuleItem*
jnc_Namespace_getItem(
	jnc_Namespace* nspace,
	size_t index
	);

JNC_EXTERN_C
jnc_Variable*
jnc_Namespace_findVariable(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	);

JNC_EXTERN_C
jnc_Function*
jnc_Namespace_findFunction(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	);

JNC_EXTERN_C
jnc_Property*
jnc_Namespace_findProperty(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequired
	);

JNC_EXTERN_C
jnc_ClassType*
jnc_Namespace_findClassType(
	jnc_Namespace* nspace,
	const char* name,
	bool_t isRequire
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_Namespace
{
	size_t
	getItemCount()
	{
		return jnc_Namespace_getItemCount(this);
	}

	jnc_ModuleItem*
	getItem(size_t index)
	{
		return jnc_Namespace_getItem(this, index);
	}

	jnc_Variable*
	findVariable(
		const char* name,
		bool isRequired = false
		)
	{
		return jnc_Namespace_findVariable(this, name, isRequired);
	}

	jnc_Function*
	findFunction(
		const char* name,
		bool isRequired = false
		)
	{
		return jnc_Namespace_findFunction(this, name, isRequired);
	}

	jnc_Property*
	findProperty(
		const char* name,
		bool isRequired = false
		)
	{
		return jnc_Namespace_findProperty(this, name, isRequired);
	}

	jnc_ClassType*
	findClassType(
		const char* name,
		bool isRequired = false
		)
	{
		return jnc_Namespace_findClassType(this, name, isRequired);
	}
};
#endif // _JNC_CORE

//..............................................................................

#if (!defined _JNC_CORE && defined __cplusplus)
struct jnc_GlobalNamespace: jnc_ModuleItem
{
};
#endif // _JNC_CORE

//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//..............................................................................

typedef jnc_NamespaceKind NamespaceKind;

const NamespaceKind
	NamespaceKind_Undefined        = jnc_NamespaceKind_Undefined,
	NamespaceKind_Global           = jnc_NamespaceKind_Global,
	NamespaceKind_Scope            = jnc_NamespaceKind_Scope,
	NamespaceKind_Type             = jnc_NamespaceKind_Type,
	NamespaceKind_Extension        = jnc_NamespaceKind_Extension,
	NamespaceKind_Property         = jnc_NamespaceKind_Property,
	NamespaceKind_PropertyTemplate = jnc_NamespaceKind_PropertyTemplate,
	NamespaceKind_DynamicLib       = jnc_NamespaceKind_DynamicLib,
	NamespaceKind__Count           = jnc_NamespaceKind__Count;

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
const char*
getNamespaceKindString(NamespaceKind namespaceKind)
{
	return jnc_getNamespaceKindString(namespaceKind);
}

//..............................................................................

} // namespace jnc

#endif // __cplusplus

/// @}
