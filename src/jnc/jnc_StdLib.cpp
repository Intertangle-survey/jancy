#include "pch.h"
#include "jnc_StdLib.h"
#include "jnc_Module.h"
#include "jnc_Runtime.h"

namespace jnc {

//.............................................................................

IfaceHdr*
StdLib::dynamicCastClassPtr (
	IfaceHdr* p,
	ClassType* type
	)
{
	if (!p)
		return NULL;

	if (p->m_object->m_type->cmp (type) == 0)
		return p;

	BaseTypeCoord coord;
	bool result = p->m_object->m_classType->findBaseTypeTraverse (type, &coord);
	if (!result)
		return NULL;

	IfaceHdr* p2 = (IfaceHdr*) ((uchar_t*) (p->m_object + 1) + coord.m_offset);
	ASSERT (p2->m_object == p->m_object);
	return p2;
}

IfaceHdr*
StdLib::strengthenClassPtr (IfaceHdr* p)
{
	if (!p)
		return NULL;

	ClassTypeKind classTypeKind = p->m_object->m_classType->getClassTypeKind ();
	return classTypeKind == ClassTypeKind_FunctionClosure || classTypeKind == ClassTypeKind_PropertyClosure ?
		((ClosureClassType*) p->m_object->m_type)->strengthen (p) :
		(!(p->m_object->m_flags & ObjHdrFlag_Dead)) ? p : NULL;
}

void*
StdLib::gcAllocate (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->gcAllocate (type, elementCount);
}

void*
StdLib::gcTryAllocate (
	Type* type,
	size_t elementCount
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->gcTryAllocate (type, elementCount);
}

void
StdLib::gcEnter ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);
	runtime->gcEnter ();
}

void
StdLib::gcLeave ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->gcLeave ();
}

void
StdLib::gcPulse ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->gcPulse ();
}

void
StdLib::runGc ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	runtime->runGc ();
}

size_t
StdLib::strLen (DataPtr ptr)
{
	char* p = (char*) ptr.m_p;
	if (!p)
		return 0;

	char* p0 = p;
	char* end = (char*) ptr.m_rangeEnd;
	while (*p && p < end)
		p++;

	return p - p0;
}

void
StdLib::memCpy (
	DataPtr dstPtr,
	DataPtr srcPtr,
	size_t size
	)
{
	if (dstPtr.m_p && srcPtr.m_p)
		memcpy (dstPtr.m_p, srcPtr.m_p, size);
}

DataPtr
StdLib::memCat (
	DataPtr ptr1,
	size_t size1,
	DataPtr ptr2,
	size_t size2
	)
{
	DataPtr resultPtr = { 0 };

	size_t totalSize = size1 + size2;
	char* p = (char*) AXL_MEM_ALLOC (totalSize + 1);
	if (!p)
		return resultPtr;

	p [totalSize] = 0; // ensure zero-termination just in case

	if (ptr1.m_p)
		memcpy (p, ptr1.m_p, size1);
	else
		memset (p, 0, size1);

	if (ptr2.m_p)
		memcpy (p + size1, ptr2.m_p, size2);
	else
		memset (p + size1, 0, size2);

	resultPtr.m_p = p;
	resultPtr.m_rangeBegin = p;
	resultPtr.m_rangeEnd = p + totalSize;
	resultPtr.m_object = jnc::getStaticObjHdr ();
	return resultPtr;
}

#if (_AXL_ENV == AXL_ENV_WIN)

intptr_t
StdLib::getCurrentThreadId ()
{
	return ::GetCurrentThreadId ();
}

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
};

DWORD
WINAPI
StdLib::threadProc (PVOID rawContext)
{
	ThreadContext* context = (ThreadContext*) rawContext;
	FunctionPtr ptr = context->m_ptr;
	Runtime* runtime = context->m_runtime;
	AXL_MEM_DELETE (context);

	ScopeThreadRuntime scopeRuntime (runtime);
	getTlsMgr ()->getTls (runtime); // register thread right away

	((void (__cdecl*) (IfaceHdr*)) ptr.m_pf) (ptr.m_closure);
	return 0;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext* context = AXL_MEM_NEW (ThreadContext);
	context->m_ptr = ptr;
	context->m_runtime = runtime;

	DWORD threadId;
	HANDLE h = ::CreateThread (NULL, 0, StdLib::threadProc, context, 0, &threadId);
	return h != NULL;
}

#elif (_AXL_ENV == AXL_ENV_POSIX)

intptr_t
StdLib::getCurrentThreadId ()
{
	return (intptr_t) pthread_self ();
}

struct ThreadContext
{
	FunctionPtr m_ptr;
	Runtime* m_runtime;
};

void*
StdLib::threadProc (void* rawContext)
{
	ThreadContext* context = (ThreadContext*) rawContext;
	FunctionPtr ptr = context->m_ptr;
	Runtime* runtime = context->m_runtime;
	AXL_MEM_DELETE (context);

	ScopeThreadRuntime scopeRuntime (runtime);
	getTlsMgr ()->getTls (runtime); // register thread right away

	((void (*) (IfaceHdr*)) ptr.m_pf) (ptr.m_closure);
	return NULL;
}

bool
StdLib::createThread (FunctionPtr ptr)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	ThreadContext* context = AXL_MEM_NEW (ThreadContext);
	context->m_ptr = ptr;
	context->m_runtime = runtime;

	pthread_t thread;
	int result = pthread_create (&thread, NULL, StdLib::threadProc, context);
	return result == 0;
}

#endif

DataPtr
StdLib::getLastError ()
{
	err::Error error = err::getError ();
	size_t size = error->m_size;

	void* p = AXL_MEM_ALLOC (size);
	memcpy (p , error, size);

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + size;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

DataPtr
StdLib::getErrorDescription (DataPtr errorPtr)
{
	err::ErrorData* error = (err::ErrorData*) errorPtr.m_p;
	rtl::String string = error->getDescription ();
	size_t length = string.getLength ();

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	memcpy (p, string.cc (), length);
	p [length] = 0;

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + length + 1;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

DataPtr
StdLib::format (
	DataPtr formatStringPtr,
	...
	)
{
	AXL_VA_DECL (va, formatStringPtr);

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	string.format_va ((const char*) formatStringPtr.m_p, va);
	size_t length = string.getLength ();

	char* p = (char*) AXL_MEM_ALLOC (length + 1);
	memcpy (p, string.cc (), length);
	p [length] = 0;

	jnc::DataPtr ptr = { 0 };
	ptr.m_p = p;
	ptr.m_rangeBegin = ptr.m_p;
	ptr.m_rangeEnd = (char*) ptr.m_p + length + 1;
	ptr.m_object = jnc::getStaticObjHdr ();

	return ptr;
}

void*
StdLib::getTls ()
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	return runtime->getTls () + 1;
}

size_t
StdLib::appendFmtLiteral_a (
	FmtLiteral* fmtLiteral,
	const char* p,
	size_t length
	)
{
	Runtime* runtime = getCurrentThreadRuntime ();
	ASSERT (runtime);

	size_t newLength = fmtLiteral->m_length + length;
	if (newLength < 64)
		newLength = 64;

	if (fmtLiteral->m_maxLength < newLength)
	{
		Module* module = runtime->getFirstModule ();
		ASSERT (module);

		size_t newMaxLength = rtl::getMinPower2Ge (newLength);
		ObjHdr* objHdr = AXL_MEM_NEW_EXTRA (ObjHdr, newMaxLength + 1);
		objHdr->m_scopeLevel = 0;
		objHdr->m_root = objHdr;
		objHdr->m_type = module->m_typeMgr.getPrimitiveType (TypeKind_Char);
		objHdr->m_flags = 0;

		char* buffer = (char*) (objHdr + 1);
		memcpy (buffer, fmtLiteral->m_p, fmtLiteral->m_length);

		fmtLiteral->m_p = buffer;
		fmtLiteral->m_maxLength = newMaxLength;
	}

	memcpy (fmtLiteral->m_p + fmtLiteral->m_length, p, length);
	fmtLiteral->m_length += length;
	fmtLiteral->m_p [fmtLiteral->m_length] = 0;

	return fmtLiteral->m_length;
}

void
StdLib::prepareFormatString (
	rtl::String* formatString,
	const char* fmtSpecifier,
	char defaultType
	)
{
	if (!fmtSpecifier)
	{
		char formatBuffer [2] = { '%', defaultType };
		formatString->copy (formatBuffer, 2);
		return;
	}

	formatString->clear ();

	if (fmtSpecifier [0] != '%')
		formatString->copy ('%');

	formatString->append (fmtSpecifier);

	size_t length = formatString->getLength ();
	if (!isalpha (formatString->cc () [length - 1]))
		formatString->append (defaultType);
}

size_t
StdLib::appendFmtLiteral_p (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	DataPtr ptr
	)
{
	if (!ptr.m_p)
		return appendFmtLiteral_a (fmtLiteral, "(null)", 6);

	char* p = (char*) ptr.m_p;
	while (*p && p < ptr.m_rangeEnd)
		p++;

	if (!fmtSpecifier || !*fmtSpecifier)
	{
		size_t length = p - (char*) ptr.m_p;
		return appendFmtLiteral_a (fmtLiteral, (char*) ptr.m_p, length);
	}

	char buffer1 [256];
	rtl::String formatString (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	prepareFormatString (&formatString, fmtSpecifier, 's');

	char buffer2 [256];
	rtl::String string (ref::BufKind_Stack, buffer2, sizeof (buffer2));

	if (p < ptr.m_rangeEnd) // null terminated
	{
		ASSERT (!*p);
		string.format (formatString, ptr.m_p);
	}
	else
	{
		char buffer3 [256];
		rtl::String nullTermString (ref::BufKind_Stack, buffer3, sizeof (buffer3));
		string.format (formatString, nullTermString.cc ());
	}

	return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
}

size_t
StdLib::appendFmtLiteralImpl (
	FmtLiteral* fmtLiteral,
	const char* fmtSpecifier,
	char defaultType,
	...
	)
{
	AXL_VA_DECL (va, defaultType);

	char buffer1 [256];
	rtl::String formatString (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	prepareFormatString (&formatString, fmtSpecifier,  defaultType);

	char buffer2 [256];
	rtl::String string (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	string.format_va (formatString, va);

	return appendFmtLiteral_a (fmtLiteral, string, string.getLength ());
}

//.............................................................................

} // namespace jnc {
