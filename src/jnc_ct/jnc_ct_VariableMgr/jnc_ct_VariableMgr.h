//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

#pragma once

#include "jnc_ct_Variable.h"

namespace jnc {
namespace ct {

class ClassType;
class Function;

//..............................................................................

class VariableMgr
{
	friend class Module;
	friend class Variable;

protected:
	Module* m_module;

	sl::StdList <Variable> m_variableList;
	sl::Array <Variable*> m_staticVariableArray;
	sl::Array <Variable*> m_staticGcRootArray;
	sl::Array <Variable*> m_globalStaticVariableArray;
	sl::Array <Variable*> m_liftedStackVariableArray;
	sl::Array <Variable*> m_tlsVariableArray;
	StructType* m_tlsStructType;

	Variable* m_stdVariableArray [StdVariable__Count];

public:
	VariableMgr ();

	Module*
	getModule ()
	{
		return m_module;
	}

	void
	clear ();

	void
	createStdVariables ();

	void
	finalizeLiftedStackVariables ();

	Variable*
	getStdVariable (StdVariable variable);

	sl::Array <Variable*>
	getStaticVariableArray ()
	{
		return m_staticVariableArray;
	}

	sl::Array <Variable*>
	getStaticGcRootArray ()
	{
		return m_staticGcRootArray;
	}

	sl::Array <Variable*>
	getGlobalStaticVariableArray ()
	{
		return m_globalStaticVariableArray;
	}

	sl::Array <Variable*>
	getTlsVariableArray ()
	{
		return m_tlsVariableArray;
	}

	StructType*
	getTlsStructType ()
	{
		ASSERT (m_tlsStructType);
		return m_tlsStructType;
	}

	Variable*
	createVariable (
		StorageKind storageKind,
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* type,
		uint_t ptrTypeFlags = 0,
		sl::BoxList <Token>* constructor = NULL,
		sl::BoxList <Token>* initializer = NULL
		);

	Variable*
	createSimpleStackVariable (
		const sl::StringRef& name,
		Type* type,
		uint_t ptrTypeFlags = 0
		)
	{
		return createVariable (StorageKind_Stack, name, name, type, ptrTypeFlags);
	}

	Variable*
	createSimpleStaticVariable (
		const sl::StringRef& name,
		const sl::StringRef& qualifiedName,
		Type* type,
		const Value& value = Value (),
		uint_t ptrTypeFlags = 0
		);

	Variable*
	createOnceFlagVariable (StorageKind storageKind = StorageKind_Static);

	Variable*
	createStaticDataPtrValidatorVariable (Variable* variable);

	Variable*
	createArgVariable (FunctionArg* arg);

	bool
	createTlsStructType ();

	bool
	allocateInitializeGlobalVariables ();

	bool
	initializeVariable (Variable* variable);

	void
	liftStackVariable (Variable* variable);

	bool
	finalizeDisposableVariable (Variable* variable);

protected:
	llvm::GlobalVariable*
	createLlvmGlobalVariable (
		Type* type,
		const sl::StringRef& tag,
		const Value& initValue = Value ()
		);

	void
	primeStaticClassVariable (Variable* variable);

	bool
	allocateHeapVariable (Variable* variable);
};

//..............................................................................

} // namespace ct
} // namespace jnc
