// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_Value.h"

namespace jnc {
namespace ct {

class FunctionType;
class FunctionPtrType;
class PropertyType;
class PropertyPtrType;

//..............................................................................

class Closure: public ref::RefCount
{
protected:
	sl::BoxList <Value> m_argValueList;
	Value* m_thisArgValue;
	size_t m_thisArgIdx;

public:
	Closure ()
	{
		m_thisArgValue = NULL;
		m_thisArgIdx = -1;
	}

	sl::BoxList <Value>*
	getArgValueList ()
	{
		return &m_argValueList;
	}

	bool
	isMemberClosure ()
	{
		return m_thisArgIdx != -1;
	}

	size_t
	getThisArgIdx ()
	{
		return m_thisArgIdx;
	}

	Value
	getThisArgValue ();

	void
	setThisArgIdx (size_t thisArgIdx);

	void
	insertThisArgValue (const Value& thisValue);

	bool
	isSimpleClosure ()
	{
		return isMemberClosure () && m_argValueList.getCount () == 1;
	}

	size_t
	append (const sl::ConstBoxList <Value>& argValueList);

	bool
	apply (sl::BoxList <Value>* argValueList);

	Type*
	getClosureType (Type* type);

	FunctionPtrType*
	getFunctionClosureType (Function* function); // choose the best overload

	FunctionPtrType*
	getFunctionClosureType (FunctionPtrType* ptrType);

	PropertyPtrType*
	getPropertyClosureType (PropertyPtrType* ptrType);

protected:
	bool
	getArgTypeArray (
		Module* module,
		sl::Array <FunctionArg*>* argArray
		);
};

//..............................................................................

} // namespace ct
} // namespace jnc
