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

#include "pch.h"
#include "test.h"

//..............................................................................

namespace c2jnc {

void
testInt32 (jnc::Module* module)
{
	printf ("Running c2jnc.testInt32...\n");

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction ("c2jnc.funcInt32");
	ASSERT (jncFunc);

	FuncInt32* funcPtr = (FuncInt32*) jncFunc->getMachineCode ();
	int32_t retval = funcPtr (-1, -2, -3, -4, -5, -6, -7, -8);
	ASSERT (retval == -36);
}

void
testInt64 (jnc::Module* module)
{
	printf ("Running c2jnc.testInt64...\n");

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction ("c2jnc.funcInt64");
	ASSERT (jncFunc);

	FuncInt64* funcPtr = (FuncInt64*) jncFunc->getMachineCode ();
	int64_t retval = funcPtr (-1, -2, -3, -4, -5, -6, -7, -8);
	ASSERT (retval == -36);
}

void
testStruct32 (jnc::Module* module)
{
	printf ("Running c2jnc.testStruct32...\n");

	struct32 s1 = { -1 };
	struct32 s2 = { -2 };
	struct32 s3 = { -3 };
	struct32 s4 = { -4 };
	struct32 s5 = { -5 };
	struct32 s6 = { -6 };
	struct32 s7 = { -7 };
	struct32 s8 = { -8 };

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction ("c2jnc.funcStruct32");
	ASSERT (jncFunc);

	FuncStruct32* funcPtr = (FuncStruct32*) jncFunc->getMachineCode ();;
	struct32 retval = funcPtr(s1, s2, s3, s4, s5, s6, s7, s8);
	ASSERT (retval.m_a == -36);
}

void
testStruct64 (jnc::Module* module)
{
	printf ("Running c2jnc.testStruct64...\n");

	struct64 s1 = { -1 };
	struct64 s2 = { -2 };
	struct64 s3 = { -3 };
	struct64 s4 = { -4 };
	struct64 s5 = { -5 };
	struct64 s6 = { -6 };
	struct64 s7 = { -7 };
	struct64 s8 = { -8 };

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction ("c2jnc.funcStruct64");
	ASSERT (jncFunc);

	FuncStruct64* funcPtr = (FuncStruct64*) jncFunc->getMachineCode ();;
	struct64 retval = funcPtr (s1, s2, s3, s4, s5, s6, s7, s8);
	ASSERT (retval.m_a == -36);
}

void
testStruct128 (jnc::Module* module)
{
	printf ("Running c2jnc.testStruct128...\n");

	struct128 s1 = { -1, -2 };
	struct128 s2 = { -3, -4 };
	struct128 s3 = { -5, -6 };
	struct128 s4 = { -7, -8 };

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction ("c2jnc.funcStruct128");
	ASSERT (jncFunc);

	FuncStruct128* funcPtr = (FuncStruct128*) jncFunc->getMachineCode ();;
	struct128 retval = funcPtr (s1, s2, s3, s4);
	ASSERT (retval.m_a == -16 && retval.m_b == -20);
}

//..............................................................................

} // namespace c2jnc

namespace jnc2c {

//..............................................................................

int32_t
funcInt32 (
	int32_t a1,
	int32_t a2,
	int32_t a3,
	int32_t a4,
	int32_t a5,
	int32_t a6,
	int32_t a7,
	int32_t a8
	)
{
	ASSERT (a1 == -1);
	ASSERT (a2 == -2);
	ASSERT (a3 == -3);
	ASSERT (a4 == -4);
	ASSERT (a5 == -5);
	ASSERT (a6 == -6);
	ASSERT (a7 == -7);
	ASSERT (a8 == -8);

	return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
}

int64_t
funcInt64 (
	int32_t a1,
	int32_t a2,
	int32_t a3,
	int32_t a4,
	int32_t a5,
	int32_t a6,
	int32_t a7,
	int32_t a8
	)
{
	ASSERT (a1 == -1);
	ASSERT (a2 == -2);
	ASSERT (a3 == -3);
	ASSERT (a4 == -4);
	ASSERT (a5 == -5);
	ASSERT (a6 == -6);
	ASSERT (a7 == -7);
	ASSERT (a8 == -8);

	return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8;
}

struct32
funcStruct32 (
	struct32 s1,
	struct32 s2,
	struct32 s3,
	struct32 s4,
	struct32 s5,
	struct32 s6,
	struct32 s7,
	struct32 s8
	)
{
	ASSERT (s1.m_a == -1);
	ASSERT (s2.m_a == -2);
	ASSERT (s3.m_a == -3);
	ASSERT (s4.m_a == -4);
	ASSERT (s5.m_a == -5);
	ASSERT (s6.m_a == -6);
	ASSERT (s7.m_a == -7);
	ASSERT (s8.m_a == -8);

	struct32 retval;

	retval.m_a =
		s1.m_a + s2.m_a + s3.m_a + s4.m_a +
		s5.m_a + s6.m_a + s7.m_a + s8.m_a;

	return retval;
}

struct64
funcStruct64 (
	struct64 s1,
	struct64 s2,
	struct64 s3,
	struct64 s4,
	struct64 s5,
	struct64 s6,
	struct64 s7,
	struct64 s8
	)
{
	ASSERT (s1.m_a == -1);
	ASSERT (s2.m_a == -2);
	ASSERT (s3.m_a == -3);
	ASSERT (s4.m_a == -4);
	ASSERT (s5.m_a == -5);
	ASSERT (s6.m_a == -6);
	ASSERT (s7.m_a == -7);
	ASSERT (s8.m_a == -8);

	struct64 retval;

	retval.m_a =
		s1.m_a + s2.m_a + s3.m_a + s4.m_a +
		s5.m_a + s6.m_a + s7.m_a + s8.m_a;

	return retval;
}

struct128
funcStruct128 (
	struct128 s1,
	struct128 s2,
	struct128 s3,
	struct128 s4
	)
{
	ASSERT (s1.m_a == -1);
	ASSERT (s1.m_b == -2);
	ASSERT (s2.m_a == -3);
	ASSERT (s2.m_b == -4);
	ASSERT (s3.m_a == -5);
	ASSERT (s3.m_b == -6);
	ASSERT (s4.m_a == -7);
	ASSERT (s4.m_b == -8);

	struct128 retval;
	retval.m_a = s1.m_a + s2.m_a + s3.m_a + s4.m_a;
	retval.m_b = s1.m_b + s2.m_b + s3.m_b + s4.m_b;
	return retval;
}

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

void
test (
	jnc::Module* module,
	const char* funcName
	)
{
	printf ("Running %s...\n", funcName);

	jnc::Function* jncFunc = module->getGlobalNamespace ()->getNamespace ()->findFunction (funcName);
	ASSERT (jncFunc);

	TestFunc* funcPtr = (TestFunc*) jncFunc->getMachineCode ();
	funcPtr ();
}

//..............................................................................

} // namespace jnc2c

//..............................................................................

JNC_DEFINE_LIB (
	TestLib,
	sl::g_nullGuid,
	"TestLib",
	"Jancy ABI-compatibility test library"
	)

JNC_BEGIN_LIB_FUNCTION_MAP (TestLib)
	JNC_MAP_FUNCTION ("jnc2c.funcInt32",     &jnc2c::funcInt32)
	JNC_MAP_FUNCTION ("jnc2c.funcInt64",     &jnc2c::funcInt64)
	JNC_MAP_FUNCTION ("jnc2c.funcStruct32",  &jnc2c::funcStruct32)
	JNC_MAP_FUNCTION ("jnc2c.funcStruct64",  &jnc2c::funcStruct64)
	JNC_MAP_FUNCTION ("jnc2c.funcStruct128", &jnc2c::funcStruct128)
JNC_END_LIB_FUNCTION_MAP ()

JNC_BEGIN_LIB_SOURCE_FILE_TABLE (TestLib)
JNC_END_LIB_SOURCE_FILE_TABLE ()

JNC_BEGIN_LIB_OPAQUE_CLASS_TYPE_TABLE (TestLib)
JNC_END_LIB_OPAQUE_CLASS_TYPE_TABLE ()

//..............................................................................
