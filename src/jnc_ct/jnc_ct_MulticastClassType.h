// This file is part of AXL (R) Library
// Tibbo Technology Inc (C) 2004-2013. All rights reserved
// Author: Vladimir Gladkov

#pragma once

#include "jnc_ct_McSnapshotClassType.h"

namespace jnc {
namespace ct {

//.............................................................................

class MulticastClassType: public ClassType
{
	friend class TypeMgr;

protected:
	FunctionPtrType* m_targetType;
	McSnapshotClassType* m_snapshotType;
	StructField* m_fieldArray [MulticastFieldKind__Count];
	Function* m_methodArray [MulticastMethodKind__Count];

	ClassPtrTypeTuple* m_eventClassPtrTypeTuple;

public:
	MulticastClassType ();

	FunctionPtrType*
	getTargetType ()
	{
		return m_targetType;
	}

	FunctionType*
	getFunctionType ()
	{
		return m_targetType->getTargetType ();
	}

	StructField*
	getField (MulticastFieldKind field)
	{
		ASSERT (field < MulticastFieldKind__Count);
		return m_fieldArray [field];
	}

	Function*
	getMethod (MulticastMethodKind method)
	{
		ASSERT (method < MulticastMethodKind__Count);
		return m_methodArray [method];
	}

	McSnapshotClassType*
	getSnapshotType ()
	{
		return m_snapshotType;
	}

	virtual
	bool
	compile ()
	{
		return
			ClassType::compile () &&
			compileCallMethod ();
	}

protected:
	virtual
	void
	prepareTypeString ();

	virtual 
	void
	prepareDoxyLinkedText ();

	virtual 
	void
	prepareDoxyTypeString ();

	virtual
	bool
	calcLayout ()
	{
		return
			ClassType::calcLayout () &&
			m_snapshotType->ensureLayout ();
	}

	bool
	compileCallMethod ();
};

//.............................................................................

} // namespace ct
} // namespace jnc
