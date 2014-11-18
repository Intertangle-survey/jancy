#include "pch.h"
#include "jnc_CastOp_DataPtr.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

CastKind
Cast_DataPtr_FromArray::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	if (isArrayRefType (opValue.getType ()))
	{
		Value ptrValue = m_module->m_operatorMgr.prepareOperandType (opValue, OpFlag_ArrayRefToPtr);
		return m_module->m_operatorMgr.getCastKind (ptrValue, type);
	}

	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Array);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	ArrayType* srcType = (ArrayType*) opValue.getType ();	
	DataPtrType* dstType = (DataPtrType*) type;

	Type* arrayElementType = srcType->getElementType ();
	Type* ptrDataType = dstType->getTargetType ();

	return
		arrayElementType->cmp (ptrDataType) == 0 ? CastKind_Implicit :
		(arrayElementType->getFlags () & TypeFlag_Pod) ?
			ptrDataType->getTypeKind () == TypeKind_Void ? CastKind_Implicit :
			(ptrDataType->getFlags () & TypeFlag_Pod) ? CastKind_Explicit : CastKind_None : CastKind_None;
}

bool
Cast_DataPtr_FromArray::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKind () == TypeKind_Array);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	ArrayType* srcType = (ArrayType*) opValue.getType ();
	DataPtrType* dstType = (DataPtrType*) type;

	const Value& savedOpValue = m_module->m_constMgr.saveValue (opValue);
	void* p = savedOpValue.getConstData ();

	// #pragma AXL_TODO ("create a global constant holding the array")

	if (dstType->getPtrTypeKind () == DataPtrTypeKind_Normal)
	{
		DataPtr* ptr = (DataPtr*) dst;
		ptr->m_p = p;
		ptr->m_rangeBegin = p;
		ptr->m_rangeEnd = (char*) p + srcType->getSize ();
		ptr->m_object = getStaticObjHdr ();
	}
	else // thin or lean
	{
		*(void**) dst = p;
	}

	return true;
}

bool
Cast_DataPtr_FromArray::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	if (isArrayRefType (opValue.getType ()))
	{
		Value ptrValue;

		return 
			m_module->m_operatorMgr.prepareOperand (opValue, &ptrValue, OpFlag_ArrayRefToPtr) &&
			m_module->m_operatorMgr.castOperator (ptrValue, type, resultValue);
	}

	err::setFormatStringError ("casting from array to pointer is currently only implemented for constants");
	return false;
}

//.............................................................................

CastKind
Cast_DataPtr_Base::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	DataPtrType* srcType = (DataPtrType*) opValue.getType ();
	DataPtrType* dstType = (DataPtrType*) type;
	Type* srcDataType = srcType->getTargetType ();
	Type* dstDataType = dstType->getTargetType ();

	if (srcType->isConstPtrType () && !dstType->isConstPtrType ())
		return CastKind_None; // const vs non-const mismatch

#pragma AXL_TODO ("develop safe data pointer casts when non-POD is involved")
#if 0
	if ((dstDataType->getFlags () & TypeFlag_Pod) != (srcDataType->getFlags () & TypeFlag_Pod))
		return CastKind_None; // pod vs non-pod mismatch
#endif

	if (srcDataType->cmp (dstDataType) == 0 || dstDataType->getTypeKind () == TypeKind_Void)
		return CastKind_Implicit;

	if (srcDataType->getTypeKind () != TypeKind_Struct)
		return CastKind_Explicit;

	return ((StructType*) srcDataType)->findBaseTypeTraverse (dstDataType) ?
		CastKind_Implicit :
		CastKind_Explicit;
}

size_t
Cast_DataPtr_Base::getOffset (
	DataPtrType* srcType,
	DataPtrType* dstType,
	BaseTypeCoord* coord
	)
{
	Type* srcDataType = srcType->getTargetType ();
	Type* dstDataType = dstType->getTargetType ();

	if (srcDataType->cmp (dstDataType) == 0 ||
		srcDataType->getTypeKind () != TypeKind_Struct ||
		dstDataType->getTypeKind () != TypeKind_Struct)
	{
		return 0;
	}

	StructType* srcStructType = (StructType*) srcDataType;
	StructType* dstStructType = (StructType*) dstDataType;

	if (!srcStructType->findBaseTypeTraverse (dstStructType, coord))
	{
		if (dstStructType->getFlags () & TypeFlag_Pod)
			return 0;

		err::setFormatStringError (
			"cannot statically cast '%s' to '%s' (use dynamic cast maybe?)", 
			srcType->getTypeString ().cc (),
			dstType->getTypeString ().cc ()
			);
		return -1;
	}

	return coord->m_offset;
}

bool
Cast_DataPtr_Base::getOffsetUnsafePtrValue (
	const Value& ptrValue,
	DataPtrType* srcType,
	DataPtrType* dstType,
	Value* resultValue
	)
{
	BaseTypeCoord coord;
	
	size_t offset = getOffset (srcType, dstType, &coord);
	if (offset == -1)
		return false;

	if (dstType->getPtrTypeKind () != DataPtrTypeKind_Thin)
		dstType = dstType->getTargetType ()->getDataPtrType_c ();

	if (!coord.m_llvmIndexArray.isEmpty ())
	{
		coord.m_llvmIndexArray.insert (0, 0);
		m_module->m_llvmIrBuilder.createGep (
			ptrValue,
			coord.m_llvmIndexArray,
			coord.m_llvmIndexArray.getCount (),
			dstType,
			resultValue
			);

		return true;
	}

	ASSERT (offset == 0);

	m_module->m_llvmIrBuilder.createBitCast (ptrValue, dstType, resultValue);
	return true;
}

//.............................................................................

bool
Cast_DataPtr_Normal2Normal::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	size_t offset = getOffset ((DataPtrType*) opValue.getType (), (DataPtrType*) type, NULL);
	if (offset == -1)
		return false;

	DataPtr* dstPtr = (DataPtr*) dst;
	DataPtr* srcPtr = (DataPtr*) opValue.getConstData ();
	dstPtr->m_p = (char*) srcPtr->m_p + offset;
	dstPtr->m_rangeBegin = srcPtr->m_rangeBegin;
	dstPtr->m_rangeEnd = srcPtr->m_rangeEnd;
	dstPtr->m_object = srcPtr->m_object;
	return true;
}

bool
Cast_DataPtr_Normal2Normal::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	if (type->getFlags () & PtrTypeFlag_Safe)
		m_module->m_operatorMgr.checkDataPtrRange (opValue);

	Value ptrValue;
	Value rangeBeginValue;
	Value rangeEndValue;
	Value scopeLevelValue;

	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, NULL, &ptrValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 1, NULL, &rangeBeginValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 2, NULL, &rangeEndValue);
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 3, NULL, &scopeLevelValue);

	bool result = getOffsetUnsafePtrValue (ptrValue, (DataPtrType*) opValue.getType (), (DataPtrType*) type, &ptrValue);
	if (!result)
		return false;

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "create safe data pointer");

	Value tmpValue = type->getUndefValue ();
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, ptrValue, 0, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, rangeBeginValue, 1, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, rangeEndValue, 2, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, scopeLevelValue, 3, type, resultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Lean2Normal::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	DataPtrTypeKind srcPtrTypeKind = ((DataPtrType*) opValue.getType ())->getPtrTypeKind ();
	ASSERT (srcPtrTypeKind == DataPtrTypeKind_Lean);

	size_t offset = getOffset ((DataPtrType*) opValue.getType (), (DataPtrType*) type, NULL);
	if (offset == -1)
		return false;

	DataPtr* dstPtr = (DataPtr*) dst;
	const void* src = opValue.getConstData ();

	dstPtr->m_p = (char*) src + offset;
	dstPtr->m_rangeBegin = NULL;
	dstPtr->m_rangeEnd = (void*) -1;
	dstPtr->m_object = getStaticObjHdr ();
	return true;
}

bool
Cast_DataPtr_Lean2Normal::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	DataPtrTypeKind srcPtrTypeKind = ((DataPtrType*) opValue.getType ())->getPtrTypeKind ();
	ASSERT (srcPtrTypeKind == DataPtrTypeKind_Lean);

	Value ptrValue;
	bool result = getOffsetUnsafePtrValue (opValue, (DataPtrType*) opValue.getType (), (DataPtrType*) type, &ptrValue);
	if (!result)
		return false;

	Value rangeBeginValue;
	Value rangeEndValue;
	Value objHdrValue;

	m_module->m_operatorMgr.getLeanDataPtrRange (opValue, &rangeBeginValue, &rangeEndValue);

	if (type->getFlags () & PtrTypeFlag_Safe)
		m_module->m_operatorMgr.checkDataPtrRange (opValue);

	m_module->m_operatorMgr.getLeanDataPtrObjHdr (opValue, &objHdrValue);

	LlvmScopeComment comment (&m_module->m_llvmIrBuilder, "create safe data pointer");

	Value tmpValue = type->getUndefValue ();
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, ptrValue, 0, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, rangeBeginValue, 1, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, rangeEndValue, 2, NULL, &tmpValue);
	m_module->m_llvmIrBuilder.createInsertValue (tmpValue, objHdrValue, 3, type, resultValue);
	return true;
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Normal2Thin::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	size_t offset = getOffset ((DataPtrType*) opValue.getType (), (DataPtrType*) type, NULL);
	if (offset == -1)
		return false;

	*(char**) dst = *(char**) opValue.getConstData () + offset;
	return true;
}

bool
Cast_DataPtr_Normal2Thin::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	Value ptrValue;
	m_module->m_llvmIrBuilder.createExtractValue (opValue, 0, NULL, &ptrValue);
	return getOffsetUnsafePtrValue (ptrValue, (DataPtrType*) opValue.getType (), (DataPtrType*) type, resultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Lean2Thin::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	return getOffsetUnsafePtrValue (opValue, (DataPtrType*) opValue.getType (), (DataPtrType*) type, resultValue);
}

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

bool
Cast_DataPtr_Thin2Thin::constCast (
	const Value& opValue,
	Type* type,
	void* dst
	)
{
	ASSERT (opValue.getType ()->getTypeKindFlags () & TypeKindFlag_DataPtr);
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	size_t offset = getOffset ((DataPtrType*) opValue.getType (), (DataPtrType*) type, NULL);
	if (offset == -1)
		return false;

	*(char**) dst = *(char**) opValue.getConstData () + offset;
	return true;
}

//.............................................................................

Cast_DataPtr::Cast_DataPtr ()
{
	memset (m_operatorTable, 0, sizeof (m_operatorTable));

	m_operatorTable [DataPtrTypeKind_Normal] [DataPtrTypeKind_Normal] = &m_normal2Normal;
	m_operatorTable [DataPtrTypeKind_Normal] [DataPtrTypeKind_Thin]   = &m_normal2Thin;
	m_operatorTable [DataPtrTypeKind_Lean] [DataPtrTypeKind_Normal]   = &m_lean2Normal;
	m_operatorTable [DataPtrTypeKind_Lean] [DataPtrTypeKind_Thin]     = &m_lean2Thin;
	m_operatorTable [DataPtrTypeKind_Thin] [DataPtrTypeKind_Thin]     = &m_thin2Thin;

	m_opFlags = OpFlag_KeepDerivableRef;
}

CastOperator*
Cast_DataPtr::getCastOperator (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_DataPtr);

	DataPtrType* dstPtrType = (DataPtrType*) type;
	DataPtrTypeKind dstPtrTypeKind = dstPtrType->getPtrTypeKind ();

	Type* srcType = opValue.getType ();
	if (isArrayRefType (srcType) || srcType->getTypeKind () == TypeKind_Array)
	{
		return &m_fromArray;
	}
	else if (!(srcType->getTypeKindFlags () & TypeKindFlag_DataPtr))
	{
		return NULL;
	}

	DataPtrType* srcPtrType = (DataPtrType*) srcType;
	DataPtrTypeKind srcPtrTypeKind = srcPtrType->getPtrTypeKind ();

	if (dstPtrTypeKind == DataPtrTypeKind_Normal)
	{
#pragma AXL_TODO ("develop safe data pointer casts when non-POD is involved")
#if 0
		if ((srcPtrType->getTargetType ()->getFlags () & TypeFlag_Pod) !=
			(dstPtrType->getTargetType ()->getFlags () & TypeFlag_Pod))
			return NULL;
#endif

		if (srcPtrType->isConstPtrType () && !dstPtrType->isConstPtrType ())
			return NULL;
	}

	ASSERT ((size_t) srcPtrTypeKind < DataPtrTypeKind__Count);
	ASSERT ((size_t) dstPtrTypeKind < DataPtrTypeKind__Count);

	return m_operatorTable [srcPtrTypeKind] [dstPtrTypeKind];
}

//.............................................................................

CastKind
Cast_DataRef::getCastKind (
	const Value& opValue,
	Type* type
	)
{
	ASSERT (type->getTypeKind () == TypeKind_DataRef);

	Type* intermediateSrcType = m_module->m_operatorMgr.getUnaryOperatorResultType (UnOpKind_Addr, opValue);
	if (!intermediateSrcType)
		return CastKind_None;

	DataPtrType* ptrType = (DataPtrType*) type;
	DataPtrType* intermediateDstType = ptrType->getTargetType ()->getDataPtrType (
		TypeKind_DataPtr,
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);

	return m_module->m_operatorMgr.getCastKind (intermediateSrcType, intermediateDstType);
}

bool
Cast_DataRef::llvmCast (
	const Value& opValue,
	Type* type,
	Value* resultValue
	)
{
	ASSERT (type->getTypeKind () == TypeKind_DataRef);

	DataPtrType* ptrType = (DataPtrType*) type;
	DataPtrType* intermediateType = ptrType->getTargetType ()->getDataPtrType (
		TypeKind_DataPtr,
		ptrType->getPtrTypeKind (),
		ptrType->getFlags ()
		);

	Value intermediateValue;

	return
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Addr, opValue, &intermediateValue) &&
		m_module->m_operatorMgr.castOperator (&intermediateValue, intermediateType) &&
		m_module->m_operatorMgr.unaryOperator (UnOpKind_Indir, intermediateValue, resultValue);
}

//.............................................................................

} // namespace jnc {
