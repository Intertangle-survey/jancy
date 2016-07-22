#pragma once

#define _JNC_DEF_H

//.............................................................................

#ifdef _MSC_VER
#	define JNC_SELECT_ANY __declspec (selectany)
#elif (defined __GNUC__)
#	define JNC_SELECT_ANY  __attribute__ ((weak))
#else
#	error unsupported compiler
#endif

#ifdef __cplusplus 
#	define JNC_EXTERN_C extern "C"
#else
#	define JNC_EXTERN_C
#endif

// inheriting which works for both C and C++

#ifdef __cplusplus 
#	define JNC_BEGIN_INHERITED_STRUCT(Struct, BaseStruct) \
	struct Struct: BaseStruct {
#else
#	define JNC_BEGIN_INHERITED_STRUCT (Struct, BaseStruct) \
	struct Struct { BaseStruct;
#endif

#define JNC_END_INHERITED_STRUCT() \
	};

//.............................................................................

// when compiling core libraries, we want to use actual implementation classes
// as to avoid unncecessary casts; otherwise, use opaque struct pointers

#ifdef _JNC_CORE

namespace jnc {
namespace ct {

class ModuleItemDecl;
class ModuleItem;
class Attribute;
class AttributeBlock;
class Namespace;
class GlobalNamespace;
class Variable;
class Function;
class Property;
class Typedef;
class Type;
class NamedType;
class BaseTypeSlot;
class DerivableType;
class ArrayType;
class BitFieldType;
class FunctionArg;
class FunctionType;
class PropertyType;
class EnumConst;
class EnumType;
class StructField;
class StructType;
class UnionType;
class MulticastClassType;
class McSnapshotClassType;
class ClassType;
class DataPtrType;
class ClassPtrType;
class FunctionPtrType;
class PropertyPtrType;
class Unit;
class Module;
class GcShadowStackFrameMap;

} // namespace ct

namespace rt {

class Runtime;
class GcHeap;

} // namespace rt
} // namespace jnc

typedef axl::sl::ListLink jnc_ListLink;
typedef axl::sl::Guid jnc_Guid;
typedef axl::err::ErrorHdr jnc_Error;
typedef jnc::ct::ModuleItemDecl jnc_ModuleItemDecl;
typedef jnc::ct::ModuleItem jnc_ModuleItem;
typedef jnc::ct::Attribute jnc_Attribute;
typedef jnc::ct::AttributeBlock jnc_AttributeBlock;
typedef jnc::ct::Namespace jnc_Namespace;
typedef jnc::ct::GlobalNamespace jnc_GlobalNamespace;
typedef jnc::ct::Variable jnc_Variable;
typedef jnc::ct::Function jnc_Function;
typedef jnc::ct::Property jnc_Property;
typedef jnc::ct::Typedef jnc_Typedef;
typedef jnc::ct::Type jnc_Type;
typedef jnc::ct::NamedType jnc_NamedType;
typedef jnc::ct::BaseTypeSlot jnc_BaseTypeSlot;
typedef jnc::ct::DerivableType jnc_DerivableType;
typedef jnc::ct::ArrayType jnc_ArrayType;
typedef jnc::ct::BitFieldType jnc_BitFieldType;
typedef jnc::ct::FunctionArg jnc_FunctionArg;
typedef jnc::ct::FunctionType jnc_FunctionType;
typedef jnc::ct::PropertyType jnc_PropertyType;
typedef jnc::ct::EnumConst jnc_EnumConst;
typedef jnc::ct::EnumType jnc_EnumType;
typedef jnc::ct::StructField jnc_StructField;
typedef jnc::ct::StructType jnc_StructType;
typedef jnc::ct::UnionType jnc_UnionType;
typedef jnc::ct::ClassType jnc_ClassType;
typedef jnc::ct::MulticastClassType jnc_MulticastClassType;
typedef jnc::ct::McSnapshotClassType jnc_McSnapshotClassType;
typedef jnc::ct::DataPtrType jnc_DataPtrType;
typedef jnc::ct::ClassPtrType jnc_ClassPtrType;
typedef jnc::ct::FunctionPtrType jnc_FunctionPtrType;
typedef jnc::ct::PropertyPtrType jnc_PropertyPtrType;
typedef jnc::ct::Unit jnc_Unit;
typedef jnc::ct::Module jnc_Module;
typedef jnc::rt::Runtime jnc_Runtime;
typedef jnc::rt::GcHeap jnc_GcHeap;
typedef jnc::ct::GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

#	define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#	define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID
#	define jnc_g_nullGuid axl::sl::g_nullGuid

namespace jnc {

axl::sl::String*
getTlsStringBuffer ();

} // namespace jnc

#else // _JNC_CORE

typedef struct jnc_Error jnc_Error;
typedef struct jnc_ModuleItemDecl jnc_ModuleItemDecl;
typedef struct jnc_ModuleItem jnc_ModuleItem;
typedef struct jnc_Attribute jnc_Attribute;
typedef struct jnc_AttributeBlock jnc_AttributeBlock;
typedef struct jnc_Namespace jnc_Namespace;
typedef struct jnc_GlobalNamespace jnc_GlobalNamespace;
typedef struct jnc_Variable jnc_Variable;
typedef struct jnc_Function jnc_Function;
typedef struct jnc_Property jnc_Property;
typedef struct jnc_Typedef jnc_Typedef;
typedef struct jnc_Type jnc_Type;
typedef struct jnc_NamedType jnc_NamedType;
typedef struct jnc_BaseTypeSlot jnc_BaseTypeSlot;
typedef struct jnc_DerivableType jnc_DerivableType;
typedef struct jnc_ArrayType jnc_ArrayType;
typedef struct jnc_BitFieldType jnc_BitFieldType;
typedef struct jnc_FunctionArg jnc_FunctionArg;
typedef struct jnc_FunctionType jnc_FunctionType;
typedef struct jnc_PropertyType jnc_PropertyType;
typedef struct jnc_EnumConst jnc_EnumConst;
typedef struct jnc_EnumType jnc_EnumType;
typedef struct jnc_StructField jnc_StructField;
typedef struct jnc_StructType jnc_StructType;
typedef struct jnc_UnionType jnc_UnionType;
typedef struct jnc_ClassType jnc_ClassType;
typedef struct jnc_MulticastClassType jnc_MulticastClassType;
typedef struct jnc_McSnapshotClassType jnc_McSnapshotClassType;
typedef struct jnc_DataPtrType jnc_DataPtrType;
typedef struct jnc_ClassPtrType jnc_ClassPtrType;
typedef struct jnc_FunctionPtrType jnc_FunctionPtrType;
typedef struct jnc_PropertyPtrType jnc_PropertyPtrType;
typedef struct jnc_Unit jnc_Unit;
typedef struct jnc_Module jnc_Module;
typedef struct jnc_Runtime jnc_Runtime;
typedef struct jnc_GcHeap jnc_GcHeap;
typedef struct jnc_GcShadowStackFrameMap jnc_GcShadowStackFrameMap;

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

#	ifdef _AXL_SL_LISTBASE_H

typedef axl::sl::ListLink jnc_ListLink;

#	else // _AXL_SL_LISTBASE_H

typedef struct jnc_ListLink jnc_ListLink;

struct jnc_ListLink
{
	jnc_ListLink* m_next;
	jnc_ListLink* m_prev;
};

#	endif // _AXL_SL_LISTBASE_H

#	ifdef _AXL_SL_GUID_H

typedef axl::sl::Guid jnc_Guid;

#		define JNC_GUID_INITIALIZER AXL_SL_GUID_INITIALIZER
#		define JNC_DEFINE_GUID AXL_SL_DEFINE_GUID

#	else // _AXL_SL_GUID_H

typedef struct jnc_Guid jnc_Guid;

struct jnc_Guid
{
	union
	{
		struct
		{
			uint32_t m_data1;
			uint16_t m_data2;
			uint16_t m_data3;
			uint8_t m_data4 [8];
		};

		struct
		{
			uint32_t m_long1;
			uint32_t m_long2;
			uint32_t m_long3;
			uint32_t m_long4;
		};
	};
};

#		define JNC_GUID_INITIALIZER(l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			{ { { l, s1, s2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } } }

#		define JNC_DEFINE_GUID(n, l, s1, s2, b1, b2, b3, b4, b5, b6, b7, b8) \
			extern JNC_SELECT_ANY const jnc_Guid n = \
			JNC_GUID_INITIALIZER (l, s1, s2, b1, b2,  b3,  b4,  b5,  b6,  b7,  b8)
#	endif // _AXL_SL_GUID_H

#endif // _JNC_CORE

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef struct jnc_ExtensionLib jnc_ExtensionLib;
typedef struct jnc_GcStats jnc_GcStats;
typedef struct jnc_GcSizeTriggers jnc_GcSizeTriggers;

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

#ifdef __cplusplus

namespace jnc {

//.............................................................................

typedef jnc_ListLink ListLink;
typedef jnc_Guid Guid;
typedef jnc_Error Error;
typedef jnc_ModuleItemDecl ModuleItemDecl;
typedef jnc_ModuleItem ModuleItem;
typedef jnc_Attribute Attribute;
typedef jnc_AttributeBlock AttributeBlock;
typedef jnc_Namespace Namespace;
typedef jnc_GlobalNamespace GlobalNamespace;
typedef jnc_Variable Variable;
typedef jnc_Function Function;
typedef jnc_Property Property;
typedef jnc_Typedef Typedef;
typedef jnc_Type Type;
typedef jnc_NamedType NamedType;
typedef jnc_BaseTypeSlot BaseTypeSlot;
typedef jnc_DerivableType DerivableType;
typedef jnc_ArrayType ArrayType;
typedef jnc_BitFieldType BitFieldType;
typedef jnc_FunctionArg FunctionArg;
typedef jnc_FunctionType FunctionType;
typedef jnc_PropertyType PropertyType;
typedef jnc_EnumConst EnumConst;
typedef jnc_EnumType EnumType;
typedef jnc_StructField StructField;
typedef jnc_StructType StructType;
typedef jnc_UnionType UnionType;
typedef jnc_ClassType ClassType;
typedef jnc_MulticastClassType MulticastClassType;
typedef jnc_McSnapshotClassType McSnapshotClassType;
typedef jnc_DataPtrType DataPtrType;
typedef jnc_ClassPtrType ClassPtrType;
typedef jnc_FunctionPtrType FunctionPtrType;
typedef jnc_PropertyPtrType PropertyPtrType;
typedef jnc_Unit Unit;
typedef jnc_Module Module;
typedef jnc_Runtime Runtime;
typedef jnc_GcHeap GcHeap;
typedef jnc_GcShadowStackFrameMap GcShadowStackFrameMap;
typedef jnc_ExtensionLib ExtensionLib;
typedef jnc_GcStats GcStats;
typedef jnc_GcSizeTriggers GcSizeTriggers;

//.............................................................................

} // namespace jnc

#endif // __cplusplus
