// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"
#include "jnc_PropertyPtrType.h"

namespace jnc {

//.............................................................................

class Cast_PropertyPtr_FromDataPtr: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);

protected:
	bool
	llvmCast_DirectThunk (
		Variable* variable,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_FullClosure (
		StorageKind storageKind,
		const Value& opValue,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);
};

//.............................................................................

class Cast_PropertyPtr_Base: public CastOperator
{
public:
	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_FromFat: public Cast_PropertyPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Thin2Fat: public Cast_PropertyPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);

protected:
	bool
	llvmCast_NoThunkSimpleClosure (
		const Value& opValue,
		const Value& simpleClosureObjValue,
		PropertyType* srcPropertyType,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_DirectThunkNoClosure (
		Property* prop,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_DirectThunkSimpleClosure (
		Property* prop,
		const Value& simpleClosureObjValue,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	llvmCast_FullClosure (
		StorageKind storageKind,
		const Value& opValue,
		PropertyType* srcPropertyType,
		PropertyPtrType* dstPtrType,
		Value* resultValue
		);

	bool
	createClosurePropertyPtr (
		Property* prop,
		const Value& closureValue,
		PropertyPtrType* ptrType,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Weak2Normal: public Cast_PropertyPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_PropertyPtr_Thin2Thin: public Cast_PropertyPtr_Base
{
public:
	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

// safe / unsafe fn pointer -> safe fn pointer

class Cast_PropertyPtr: public Cast_Master
{
protected:
	Cast_PropertyPtr_FromDataPtr m_fromDataPtr;
	Cast_PropertyPtr_FromFat m_fromFat;
	Cast_PropertyPtr_Weak2Normal m_weak2Normal;
	Cast_PropertyPtr_Thin2Fat m_thin2Fat;
	Cast_PropertyPtr_Thin2Thin m_thin2Thin;

	CastOperator* m_operatorTable [PropertyPtrTypeKind__Count] [PropertyPtrTypeKind__Count];

public:
	Cast_PropertyPtr ();

	virtual
	CastOperator*
	getCastOperator (
		const Value& opValue,
		Type* type
		);
};

//.............................................................................

// data ref (EUnOp_Indir => data ptr cast => EUnOp_Addr)

class Cast_PropertyRef: public CastOperator
{
public:
	Cast_PropertyRef ()
	{
		m_opFlags = OpFlagKind_KeepRef;
	}

	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

	virtual
	bool
	llvmCast (
		StorageKind storageKind,
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

} // namespace jnc {
