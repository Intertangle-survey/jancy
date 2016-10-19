#pragma once

#define _JNC_VARIABLE_H

#include "jnc_Type.h"

/// \addtogroup variable
/// @{

//..............................................................................

JNC_EXTERN_C
int
jnc_Variable_hasInitializer (jnc_Variable* variable);

JNC_EXTERN_C
const char*
jnc_Variable_getInitializerString_v (jnc_Variable* variable);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#if (!defined _JNC_CORE && defined __cplusplus)

struct jnc_Variable: jnc_ModuleItem
{
	int
	hasInitializer ()
	{
		return jnc_Variable_hasInitializer (this);
	}

	const char*
	getInitializerString_v ()
	{
		return jnc_Variable_getInitializerString_v (this);
	}
};

#endif // _JNC_CORE

//..............................................................................

/// @}
