#include "pch.h"
#include "jnc_StdType.h"

namespace jnc {

//.............................................................................

const char*
getStdTypeName (StdType stdType)
{
	const char* nameTable [StdType__Count] =
	{
		NULL,               // StdType_BytePtr,
		NULL,               // StdType_ByteConstPtr,
		NULL,               // StdType_SimpleIfaceHdr,
		NULL,               // StdType_SimpleIfaceHdrPtr,
		NULL,               // StdType_Box,
		NULL,               // StdType_BoxPtr,
		NULL,               // StdType_DataBox,
		NULL,               // StdType_DataBoxPtr,
		NULL,               // StdType_DynamicArrayBox,
		NULL,               // StdType_DynamicArrayBoxPtr,
		NULL,               // StdType_StaticDataBox,
		NULL,               // StdType_StaticDataBoxPtr,
		NULL,               // StdType_AbstractClass,
		NULL,               // StdType_AbstractClassPtr,
		NULL,               // StdType_SimpleFunction,
		NULL,               // StdType_SimpleMulticast,
		NULL,               // StdType_SimpleEventPtr,
		NULL,               // StdType_Binder,
		NULL,               // StdType_ReactorBindSite,
		"Scheduler",        // StdType_Scheduler,
		"Recognizer",       // StdType_Recognizer,
		"AutomatonResult",  // StdType_AutomatonResult,
		"AutomatonFunc",    // StdType_AutomatonFunc,
		"Library",          // StdType_DynamicLib,
		NULL,               // StdType_FmtLiteral,
	};

	ASSERT ((size_t) stdType < StdType__Count);
	return nameTable [stdType];
}

const StdItemSource*
getStdTypeSource (StdType stdType)
{
	#include "jnc_StdTypes.jnc.cpp"
	#include "jnc_Recognizer.jnc.cpp"
	#include "jnc_DynamicLib.jnc.cpp"

	static StdItemSource sourceTable [StdType__Count] =
	{
		{ NULL },                            // StdType_BytePtr,
		{ NULL },                            // StdType_ByteConstPtr,
		{ NULL },                            // StdType_SimpleIfaceHdr,
		{ NULL },                            // StdType_SimpleIfaceHdrPtr,
		{ NULL },                            // StdType_Box,
		{ NULL },                            // StdType_BoxPtr,
		{ NULL },                            // StdType_DataBox,
		{ NULL },                            // StdType_DataBoxPtr,
		{ NULL },                            // StdType_DynamicArrayBox,
		{ NULL },                            // StdType_DynamicArrayBoxPtr,
		{ NULL },                            // StdType_StaticDataBox,
		{ NULL },                            // StdType_StaticDataBoxPtr,
		{ NULL },                            // StdType_AbstractClass,
		{ NULL },                            // StdType_AbstractClassPtr,
		{ NULL },                            // StdType_SimpleFunction,
		{ NULL },                            // StdType_SimpleMulticast,
		{ NULL },                            // StdType_SimpleEventPtr,
		{ NULL },                            // StdType_Binder,
		{                                    // StdType_ReactorBindSite,
			reactorBindSiteTypeSrc,
			lengthof (reactorBindSiteTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Scheduler,
			schedulerTypeSrc,
			lengthof (schedulerTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_Recognizer,
			recognizerTypeSrc,
			lengthof (recognizerTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_AutomatonResult,
			automatonResultTypeSrc,
			lengthof (automatonResultTypeSrc),
			StdNamespace_Jnc,
		},
		{ NULL },                            // StdType_AutomatonFunc,
		{                                    // StdType_DynamicLib,
			dynamicLibTypeSrc,
			lengthof (dynamicLibTypeSrc),
			StdNamespace_Jnc,
		},
		{                                    // StdType_FmtLiteral,
			fmtLiteralTypeSrc,
			lengthof (fmtLiteralTypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Int64,
			int64Int64TypeSrc,
			lengthof (int64Int64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Fp64Fp64,
			fp64Fp64TypeSrc,
			lengthof (fp64Fp64TypeSrc),
			StdNamespace_Internal,
		},
		{                                    // StdType_Int64Fp64,
			int64Fp64TypeSrc,
			lengthof (int64Fp64TypeSrc),
			StdNamespace_Internal,
		},

		{                                    // StdType_Fp64Int64,
			fp64Int64TypeSrc,
			lengthof (fp64Int64TypeSrc),
			StdNamespace_Internal,
		},
	};

	ASSERT ((size_t) stdType < StdType__Count);
	return &sourceTable [stdType];
}

//.............................................................................

} // namespace jnc {
