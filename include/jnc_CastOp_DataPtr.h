// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_CastOp.h"
#include "jnc_DataPtrType.h"

namespace jnc {

class BaseTypeCoord;

//.............................................................................

// array -> ptr

class Cast_DataPtr_FromArray: public CastOperator
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
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

// data ptr -> data ptr

class Cast_DataPtr_Base: public CastOperator
{
public:
	Cast_DataPtr_Base ()
	{
		m_opFlags = OpFlag_KeepDerivableRef;
	}

	virtual
	CastKind
	getCastKind (
		const Value& opValue,
		Type* type
		);

protected:
	size_t
	getOffset (
		DataPtrType* srcType,
		DataPtrType* dstType,
		BaseTypeCoord* coord
		);

	bool
	getOffsetUnsafePtrValue (
		const Value& ptrValue, 
		DataPtrType* srcType,
		DataPtrType* dstType,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Normal2Normal: public Cast_DataPtr_Base
{
public:
	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Lean2Normal: public Cast_DataPtr_Base
{
public:
	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Normal2Thin: public Cast_DataPtr_Base
{
public:
	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Lean2Thin: public Cast_DataPtr_Base
{
public:
	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		)
	{
		ASSERT (false); // there are no lean pointer constants
		return true;
	}

	virtual
	bool
	llvmCast (
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//. . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class Cast_DataPtr_Thin2Thin: public Cast_DataPtr_Lean2Thin
{
public:
	virtual
	bool
	constCast (
		const Value& opValue,
		Type* type,
		void* dst
		);
};

//.............................................................................

// data ptr master cast

class Cast_DataPtr: public Cast_Master
{
protected:
	Cast_DataPtr_FromArray m_fromArray;
	Cast_DataPtr_Normal2Normal m_normal2Normal;
	Cast_DataPtr_Lean2Normal m_lean2Normal;
	Cast_DataPtr_Normal2Thin m_normal2Thin;
	Cast_DataPtr_Lean2Thin m_lean2Thin;
	Cast_DataPtr_Thin2Thin m_thin2Thin;

	CastOperator* m_operatorTable [DataPtrTypeKind__Count] [DataPtrTypeKind__Count];

public:
	Cast_DataPtr ();

	virtual
	CastOperator*
	getCastOperator (
		const Value& opValue,
		Type* type
		);
};

//.............................................................................

// data ref (EUnOp_Indir => data ptr cast => EUnOp_Addr)

class Cast_DataRef: public CastOperator
{
public:
	Cast_DataRef ()
	{
		m_opFlags = OpFlag_KeepRef;
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
		const Value& opValue,
		Type* type,
		Value* resultValue
		);
};

//.............................................................................

} // namespace jnc {
