#include "pch.h"
#include "jnc_DerivableType.h"

#ifdef _JNC_DYNAMIC_EXTENSION_LIB
#	include "jnc_DynamicExtensionLibHost.h"
#	include "jnc_ExtensionLib.h"
#elif defined (_JNC_CORE)
#	include "jnc_rt_Runtime.h"
#	include "jnc_ct_Module.h"
#endif

//.............................................................................

#ifdef _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_DerivableType*
jnc_BaseTypeSlot_getType (jnc_BaseTypeSlot* baseType)
{
	return jnc_g_dynamicExtensionLibHost->m_baseTypeSlotFuncTable->m_getTypeFunc (baseType);
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getOffset (jnc_BaseTypeSlot* baseType)
{
	return jnc_g_dynamicExtensionLibHost->m_baseTypeSlotFuncTable->m_getOffsetFunc (baseType);
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getVTableIndex (jnc_BaseTypeSlot* baseType)
{
	return jnc_g_dynamicExtensionLibHost->m_baseTypeSlotFuncTable->m_getVTableIndexFunc (baseType);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getNamespaceFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticConstructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getStaticConstructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticDestructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getStaticDestructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getPreConstructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getConstructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getDestructorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getUnaryOperatorFunc (type, opKind);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getBinaryOperatorFunc (type, opKind);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCallOperatorFunc (type);
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	)
{
	return jnc_g_dynamicExtensionLibHost->m_derivableTypeFuncTable->m_getCastOperatorFunc (type, idx);
}

#else // _JNC_DYNAMIC_EXTENSION_LIB

JNC_EXTERN_C
jnc_DerivableType*
jnc_BaseTypeSlot_getType (jnc_BaseTypeSlot* baseType)
{
	return baseType->getType ();
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getOffset (jnc_BaseTypeSlot* baseType)
{
	return baseType->getOffset ();
}

JNC_EXTERN_C
size_t
jnc_BaseTypeSlot_getVTableIndex (jnc_BaseTypeSlot* baseType)
{
	return baseType->getVTableIndex ();
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

JNC_EXTERN_C
jnc_Namespace*
jnc_DerivableType_getNamespace (jnc_DerivableType* type)
{
	return type;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticConstructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getStaticConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no static constructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getStaticDestructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getStaticDestructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no static destructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getPreConstructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getPreConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no preconstructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getConstructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getConstructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no constructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getDestructor (jnc_DerivableType* type)
{
	jnc_Function* function = type->getDestructor ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no destructor", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getUnaryOperator (
	jnc_DerivableType* type,
	jnc_UnOpKind opKind	
	)
{
	jnc_Function* function = type->getUnaryOperator ((jnc::UnOpKind) opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc_getUnOpKindString (opKind));
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getBinaryOperator (
	jnc_DerivableType* type,
	jnc_BinOpKind opKind
	)
{
	jnc_Function* function = type->getBinaryOperator ((jnc::BinOpKind) opKind);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator %s", type->getTypeString ().cc (), jnc_getBinOpKindString (opKind));
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCallOperator (jnc_DerivableType* type)
{
	jnc_Function* function = type->getCallOperator ();
	if (!function)
	{
		err::setFormatStringError ("'%s' has no operator ()", type->getTypeString ().cc ());
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getCastOperator (
	jnc_DerivableType* type,
	size_t idx
	)
{
	jnc_Function* function = type->getCastOperator (idx);
	if (!function)
	{
		err::setFormatStringError ("'%s' has no cast operator #%d", type->getTypeString ().cc (), idx);
		return NULL;
	}

	return function;
}

JNC_EXTERN_C
size_t 
jnc_DerivableType_getBaseTypeCount (jnc_DerivableType* type)
{
	return type->getBaseTypeArray ().getCount ();
}

JNC_EXTERN_C
jnc_BaseTypeSlot*
jnc_DerivableType_getBaseType (
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getBaseTypeArray () [index];
}

JNC_EXTERN_C
size_t
jnc_DerivableType_findBaseTypeOffset (
	jnc_DerivableType* type,
	jnc_Type* baseType
	)
{
	return type->findBaseTypeOffset (baseType);
}

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberFieldCount (jnc_DerivableType* type)
{
	return type->getMemberFieldArray ().getCount ();
}

JNC_EXTERN_C
jnc_StructField*
jnc_DerivableType_getMemberField (
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getMemberFieldArray () [index];
}

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberMethodCount (jnc_DerivableType* type)
{
	return type->getMemberMethodArray ().getCount ();
}

JNC_EXTERN_C
jnc_Function*
jnc_DerivableType_getMemberMethod (
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getMemberMethodArray () [index];
}

JNC_EXTERN_C
size_t
jnc_DerivableType_getMemberPropertyCount (jnc_DerivableType* type)
{
	return type->getMemberPropertyArray ().getCount ();
}

JNC_EXTERN_C
jnc_Property*
jnc_DerivableType_getMemberProperty (
	jnc_DerivableType* type,
	size_t index
	)
{
	return type->getMemberPropertyArray () [index];
}

#endif // _JNC_DYNAMIC_EXTENSION_LIB
