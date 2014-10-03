#include "pch.h"
#include "jnc_OperatorMgr.h"
#include "jnc_Closure.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

bool
OperatorMgr::createClosureObject (
	StorageKind storageKind,
	const Value& opValue, // thin function or property ptr with closure
	Type* thunkType, // function or property type
	bool isWeak,
	Value* resultValue
	)
{
	ASSERT (thunkType->getTypeKind () == TypeKind_Function || thunkType->getTypeKind () == TypeKind_Property);

	bool result;

	// choose reference function type

	FunctionType* srcFunctionType;
	if (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_FunctionPtr)
	{
		ASSERT (((FunctionPtrType*) opValue.getType ())->getPtrTypeKind () == FunctionPtrTypeKind_Thin);
		srcFunctionType = ((FunctionPtrType*) opValue.getType ())->getTargetType ();
	}
	else
	{
		ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_PropertyPtr);
		ASSERT (((PropertyPtrType*) opValue.getType ())->getPtrTypeKind () == PropertyPtrTypeKind_Thin);
		srcFunctionType = ((PropertyPtrType*) opValue.getType ())->getTargetType ()->getGetterType ();
	}

	char buffer1 [256];
	char buffer2 [256];

	rtl::Array <Type*> closureArgTypeArray (ref::BufKind_Stack, buffer1, sizeof (buffer1));
	rtl::Array <size_t> closureMap (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	size_t closureArgCount = 0;
	uint64_t weakMask = 0;

	// build closure arg type array & closure map

	Closure* closure = opValue.getClosure ();
	if (closure)
	{
		closureArgCount = closure->getArgValueList ()->getCount ();

		rtl::Array <FunctionArg*> srcArgArray = srcFunctionType->getArgArray ();
		size_t srcArgCount = srcArgArray.getCount ();

		if (closureArgCount > srcArgCount)
		{
			err::setFormatStringError ("closure is too big for '%s'", srcFunctionType->getTypeString ().cc ());
			return false;
		}

		closureArgTypeArray.setCount (closureArgCount);
		closureMap.setCount (closureArgCount);

		rtl::BoxIterator <Value> closureArgValue = closure->getArgValueList ()->getHead ();

		size_t j = 0;

		for (size_t i = 0; i < closureArgCount; closureArgValue++, i++)
		{
			if (closureArgValue->isEmpty ())
				continue;

			Type* type = closureArgValue->getType ();
			if (isWeakPtrType (type))
				weakMask |= (uint64_t) 2 << j; // account for function ptr field (at idx 0)

			closureArgTypeArray [j] = srcArgArray [i]->getType ();
			closureMap [j] = i;
			j++;
		}

		closureArgCount = j; // account for possible skipped args
	}

	// find or create closure class type

	ClosureClassType* closureType;

	if (thunkType->getTypeKind () == TypeKind_Function)
	{
		closureType = m_module->m_typeMgr.getFunctionClosureClassType (
			((FunctionPtrType*) opValue.getType ())->getTargetType (),
			(FunctionType*) thunkType,
			closureArgTypeArray,
			closureMap,
			closureArgCount,
			weakMask
			);
	}
	else
	{
		ASSERT (thunkType->getTypeKind () == TypeKind_Property);

		closureType = m_module->m_typeMgr.getPropertyClosureClassType (
			((PropertyPtrType*) opValue.getType ())->getTargetType (),
			(PropertyType*) thunkType,
			closureArgTypeArray,
			closureMap,
			closureArgCount,
			weakMask
			);
	}

	// create instance

	Value closureValue;
	result = m_module->m_operatorMgr.newOperator (storageKind, closureType, NULL, &closureValue);
	if (!result)
		return false;

	rtl::Iterator <StructField> field = closureType->getFieldList ().getHead ();

	// save function/property pointer

	Value pfnValue = opValue;
	pfnValue.clearClosure ();

	Value fieldValue;
	result =
		getClassField (closureValue, *field, NULL, &fieldValue) &&
		binaryOperator (BinOpKind_Assign, fieldValue, pfnValue);

	if (!result)
		return false;

	field++;

	// save closure arguments (if any)

	if (closure)
	{
		rtl::BoxIterator <Value> closureArgValue = closure->getArgValueList ()->getHead ();
		for (; closureArgValue; closureArgValue++)
		{
			if (closureArgValue->isEmpty ())
				continue;

			ASSERT (field);

			Value fieldValue;
			result =
				getClassField (closureValue, *field, NULL, &fieldValue) &&
				binaryOperator (BinOpKind_Assign, fieldValue, *closureArgValue);

			if (!result)
				return false;

			field++;
		}
	}

	*resultValue = closureValue;
	return true;
}

bool
OperatorMgr::createDataClosureObject (
	StorageKind storageKind,
	const Value& opValue, // data ptr
	PropertyType* thunkType, // function or property type
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_DataPtr);

	bool result;

	// find or create closure class type

	DataClosureClassType* closureType = m_module->m_typeMgr.getDataClosureClassType (
		((DataPtrType*) opValue.getType ())->getTargetType (),
		thunkType
		);

	// create instance

	Value closureValue;
	result = m_module->m_operatorMgr.newOperator (storageKind, closureType, NULL, &closureValue);
	if (!result)
		return false;

	rtl::Iterator <StructField> field = closureType->getFieldList ().getHead ();

	// save data pointer

	Value fieldValue;
	result =
		getClassField (closureValue, *field, NULL, &fieldValue) &&
		binaryOperator (BinOpKind_Assign, fieldValue, opValue);

	if (!result)
		return false;

	*resultValue = closureValue;
	return true;
}

//.............................................................................

} // namespace jnc {
