#include "pch.h"
#include "jnc_FunctionMgr.h"
#include "jnc_Module.h"
#include "jnc_Parser.llk.h"

// #define _JNC_NO_JIT

namespace jnc {

//.............................................................................

FunctionMgr::FunctionMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	m_currentFunction = NULL;
	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));
}

void
FunctionMgr::clear ()
{
	m_functionList.clear ();
	m_propertyList.clear ();
	m_propertyTemplateList.clear ();
	m_scheduleLauncherFunctionList.clear ();
	m_thunkFunctionList.clear ();
	m_thunkPropertyList.clear ();
	m_dataThunkPropertyList.clear ();
	m_thunkFunctionMap.clear ();
	m_thunkPropertyMap.clear ();
	m_lazyStdFunctionList.clear ();
	m_scheduleLauncherFunctionMap.clear ();
	m_thisValue.clear ();
	m_staticConstructArray.clear ();

	m_currentFunction = NULL;
	memset (m_stdFunctionArray, 0, sizeof (m_stdFunctionArray));
	memset (m_lazyStdFunctionArray, 0, sizeof (m_lazyStdFunctionArray));
}

bool
FunctionMgr::callStaticConstructors ()
{
	bool result;

	Function* addDestructor = getStdFunction (StdFunction_AddStaticDestructor);
	Type* dtorType = m_module->m_typeMgr.getStdType (StdType_BytePtr);

	size_t count = m_staticConstructArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		NamedTypeBlock* namedTypeBlock = m_staticConstructArray [i];

		Function* destructor = namedTypeBlock->getStaticDestructor ();
		if (destructor)
		{
			Value dtorValue;
			result = 
				m_module->m_operatorMgr.castOperator (destructor, dtorType, &dtorValue) &&
				m_module->m_operatorMgr.callOperator (addDestructor, dtorValue);

			if (!result)
				return false;
		}

		Function* constructor = namedTypeBlock->getStaticConstructor ();
		ASSERT (constructor);

		result = m_module->m_operatorMgr.callOperator (constructor);
		if (!result)
			return false;
	}

	return true;
}

Value
FunctionMgr::overrideThisValue (const Value& value)
{
	Value prevThisValue = m_thisValue;
	m_thisValue = value;
	return prevThisValue;
}

Function*
FunctionMgr::createFunction (
	FunctionKind functionKind,
	const rtl::String& name,
	const rtl::String& qualifiedName,
	const rtl::String& tag,
	FunctionType* type
	)
{
	Function* function;
	switch (functionKind)
	{
	case FunctionKind_Thunk:
		function = AXL_MEM_NEW (ThunkFunction);
		m_thunkFunctionList.insertTail ((ThunkFunction*) function);
		break;

	case FunctionKind_ScheduleLauncher:
		function = AXL_MEM_NEW (ScheduleLauncherFunction);
		m_scheduleLauncherFunctionList.insertTail ((ScheduleLauncherFunction*) function);
		break;

	default:
		function = AXL_MEM_NEW (Function);
		m_functionList.insertTail (function);
	}

	function->m_module = m_module;
	function->m_functionKind = functionKind;
	function->m_name = name;
	function->m_qualifiedName = qualifiedName;
	function->m_tag = tag;
	function->m_type = type;
	function->m_typeOverload.addOverload (type);
	return function;
}

Property*
FunctionMgr::createProperty (
	PropertyKind propertyKind,
	const rtl::String& name,
	const rtl::String& qualifiedName,
	const rtl::String& tag
	)
{
	Property* prop;

	switch (propertyKind)
	{
	case PropertyKind_Thunk:
		prop = AXL_MEM_NEW (ThunkProperty);
		m_thunkPropertyList.insertTail ((ThunkProperty*) prop);
		break;

	case PropertyKind_DataThunk:
		prop = AXL_MEM_NEW (DataThunkProperty);
		m_dataThunkPropertyList.insertTail ((DataThunkProperty*) prop);
		break;

	default:
		prop = AXL_MEM_NEW (Property);
		m_propertyList.insertTail (prop);
	}

	prop->m_module = m_module;
	prop->m_propertyKind = propertyKind;
	prop->m_name = name;
	prop->m_qualifiedName = qualifiedName;
	prop->m_tag = tag;
	m_module->markForLayout (prop, true);
	return prop;
}

PropertyTemplate*
FunctionMgr::createPropertyTemplate ()
{
	PropertyTemplate* propertyTemplate = AXL_MEM_NEW (PropertyTemplate);
	propertyTemplate->m_module = m_module;
	m_propertyTemplateList.insertTail (propertyTemplate);
	return propertyTemplate;
}

bool
FunctionMgr::fireOnChanged ()
{
	Function* function = m_currentFunction;

	ASSERT (
		function->m_functionKind == FunctionKind_Setter &&
		function->m_property &&
		function->m_property->getType ()->getFlags () & PropertyTypeFlag_Bindable
		);

	Value propertyValue = function->m_property;

	if (function->m_thisType)
	{
		ASSERT (m_thisValue);
		propertyValue.insertToClosureHead (m_thisValue);
	}

	Value onChanged;

	return
		m_module->m_operatorMgr.getPropertyOnChanged (propertyValue, &onChanged) &&
		m_module->m_operatorMgr.memberOperator (&onChanged, "call") &&
		m_module->m_operatorMgr.callOperator (onChanged);
}

Function*
FunctionMgr::setCurrentFunction (Function* function)
{
	Function* prevFunction = m_currentFunction;
	m_currentFunction = function;
	return prevFunction;
}

bool
FunctionMgr::prologue (
	Function* function,
	const Token::Pos& pos
	)
{
	bool result;

	m_currentFunction = function;

	// create scope

	m_module->m_namespaceMgr.openNamespace (function->m_parentNamespace);

	function->m_scope = m_module->m_namespaceMgr.openScope (pos);

	if (function->m_extensionNamespace)
	{
		function->m_scope->m_usingSet.addGlobalNamespace (function->m_extensionNamespace);
		function->m_scope->m_usingSet.addExtensionNamespace (function->m_extensionNamespace);
	}

	if (function->m_type->getFlags () & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.enterUnsafeRgn ();

	// create entry block (gc roots come here)

	BasicBlock* entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");

	function->m_entryBlock = entryBlock;
	entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);
	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);
	m_module->m_controlFlowMgr.m_unreachableBlock = NULL;
	m_module->m_controlFlowMgr.m_flags = 0; // clear jump flag

	if (function->m_functionKind == FunctionKind_ModuleConstructor)
	{
		result = 
			m_module->m_variableMgr.allocateInitializeGlobalVariables () &&
			callStaticConstructors ();

		if (!result)
			return false;
	}

	function->getType ()->getCallConv ()->createArgVariables (function);

	// 'this' arg

	if (function->isMember ())
		createThisValue ();

	// static initializers

	if (function->m_functionKind == FunctionKind_StaticConstructor)
	{
		if (function->getProperty ())
			function->getProperty ()->initializeStaticFields ();
		else if (function->getParentType ())
			function->getParentType ()->initializeStaticFields ();		
	}

	return true;
}

void
FunctionMgr::createThisValue ()
{
	Function* function = m_currentFunction;
	ASSERT (function && function->isMember ());

	Value thisArgValue = function->getType ()->getCallConv ()->getThisArgValue (function);
	if (function->m_thisArgType->cmp (function->m_thisType) == 0)
	{
		if (function->m_thisType->getTypeKind () != TypeKind_DataPtr)
		{
			m_thisValue = thisArgValue;
		}
		else // make it lean
		{
			ASSERT (
				thisArgValue.getType ()->getTypeKind () == TypeKind_DataPtr &&
				((DataPtrType*) thisArgValue.getType ())->getPtrTypeKind () == DataPtrTypeKind_Normal);

			DataPtrType* ptrType = ((DataPtrType*) thisArgValue.getType ());
			ptrType = ptrType->getTargetType ()->getDataPtrType (DataPtrTypeKind_Lean, ptrType->getFlags ());

			Value ptrValue;
			m_module->m_llvmIrBuilder.createExtractValue (thisArgValue, 0, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast (ptrValue, ptrType, &ptrValue);
			m_thisValue.setLeanDataPtr (ptrValue.getLlvmValue (), ptrType, thisArgValue);
		}
	}
	else
	{
		ASSERT (function->m_storageKind == StorageKind_Override);
		
		if (function->m_thisArgDelta == 0)
		{
			m_module->m_llvmIrBuilder.createBitCast (thisArgValue, function->m_thisType, &m_thisValue);
		}
		else
		{
			Value ptrValue;
			m_module->m_llvmIrBuilder.createBitCast (thisArgValue, m_module->m_typeMgr.getStdType (StdType_BytePtr), &ptrValue);
			m_module->m_llvmIrBuilder.createGep (ptrValue, (int32_t) function->m_thisArgDelta, NULL, &ptrValue);
			m_module->m_llvmIrBuilder.createBitCast (ptrValue, function->m_thisType, &m_thisValue);
		}
	}
}

bool
FunctionMgr::epilogue ()
{
	bool result;

	Function* function = m_currentFunction;
	Scope* scope = m_module->m_namespaceMgr.getCurrentScope ();

	ASSERT (m_currentFunction && scope);

	if (function->m_functionKind == FunctionKind_Destructor)
	{
		ASSERT (m_thisValue);

		if (function->getProperty ())
		{
			Property* prop = function->getProperty ();
			result = prop->callMemberPropertyDestructors (m_thisValue);
		}
		else
		{
			ASSERT (function->getParentType ()->getTypeKind () == TypeKind_Class && m_thisValue);
			ClassType* classType = (ClassType*) function->getParentType ();

			result =
				classType->callMemberPropertyDestructors (m_thisValue) &&
				classType->callBaseTypeDestructors (m_thisValue);
		}

		if (!result)
			return false;
	}

	result = m_module->m_controlFlowMgr.checkReturn ();
	if (!result)
		return false;

	try
	{
		llvm::verifyFunction (*function->getLlvmFunction (), llvm::ReturnStatusAction);
	}
	catch (err::Error error)
	{
		err::setFormatStringError (
			"LLVM verification fail for '%s': %s",
			function->m_tag.cc (),
			error->getDescription ().cc ()
			);

		return false;
	}

	if (function->m_type->getFlags () & FunctionTypeFlag_Unsafe)
		m_module->m_operatorMgr.leaveUnsafeRgn ();

	m_module->m_namespaceMgr.closeScope ();
	m_module->m_namespaceMgr.closeNamespace ();

	m_currentFunction = NULL;
	m_thisValue.clear ();
	return true;
}

void
FunctionMgr::internalPrologue (
	Function* function,
	Value* argValueArray,
	size_t argCount
	)
{
	m_currentFunction = function;

	function->m_scope = m_module->m_namespaceMgr.openInternalScope ();
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvm::DebugLoc ());

	BasicBlock* entryBlock = m_module->m_controlFlowMgr.createBlock ("function_entry");
	BasicBlock* bodyBlock = m_module->m_controlFlowMgr.createBlock ("function_body");

	function->m_entryBlock = entryBlock;
	entryBlock->markEntry ();

	m_module->m_controlFlowMgr.setCurrentBlock (entryBlock);
	m_module->m_controlFlowMgr.jump (bodyBlock, bodyBlock);
	m_module->m_controlFlowMgr.m_unreachableBlock = NULL;
	m_module->m_controlFlowMgr.m_flags = 0;

	if (function->isMember ())
		createThisValue ();

	if (argCount)
	{
		llvm::Function::arg_iterator llvmArg = function->getLlvmFunction ()->arg_begin ();
		rtl::Array <FunctionArg*> argArray = function->getType ()->getArgArray ();

		CallConv* callConv = function->getType ()->getCallConv ();

		for (size_t i = 0; i < argCount; i++, llvmArg++)
			argValueArray [i] = callConv->getArgValue (argArray [i], llvmArg);
	}
}

void
FunctionMgr::internalEpilogue ()
{
	Function* function = m_currentFunction;

	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	if (!currentBlock->hasTerminator ())
	{
		Type* returnType = function->getType ()->getReturnType ();

		Value returnValue;
		if (returnType->getTypeKind () != TypeKind_Void)
			returnValue = returnType->getZeroValue ();

		m_module->m_controlFlowMgr.ret (returnValue);
	}

	m_module->m_namespaceMgr.closeScope ();

	m_currentFunction = NULL;
	m_thisValue.clear ();
}

Function*
FunctionMgr::getDirectThunkFunction (
	Function* targetFunction,
	FunctionType* thunkFunctionType,
	bool hasUnusedClosure
	)
{
	if (!hasUnusedClosure && targetFunction->m_type->cmp (thunkFunctionType) == 0)
		return targetFunction;

	char signatureChar = 'D';

	if (hasUnusedClosure)
	{
		signatureChar = 'U';
		thunkFunctionType = thunkFunctionType->getStdObjectMemberMethodType ();
	}

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		signatureChar,
		targetFunction,
		thunkFunctionType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Function*> thunk = m_thunkFunctionMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ThunkFunction* thunkFunction = (ThunkFunction*) createFunction (FunctionKind_Thunk, thunkFunctionType);
	thunkFunction->m_storageKind = StorageKind_Static;
	thunkFunction->m_tag = "directThunkFunction";
	thunkFunction->m_signature = signature;
	thunkFunction->m_targetFunction = targetFunction;

	thunk->m_value = thunkFunction;

	m_module->markForCompile (thunkFunction);
	return thunkFunction;
}

Property*
FunctionMgr::getDirectThunkProperty (
	Property* targetProperty,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
	if (!hasUnusedClosure && targetProperty->m_type->cmp (thunkPropertyType) == 0)
		return targetProperty;

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetProperty,
		thunkPropertyType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ThunkProperty* thunkProperty = (ThunkProperty*) createProperty (PropertyKind_Thunk);
	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;
	thunkProperty->m_tag = "g_directThunkProperty";

	bool result = thunkProperty->create (targetProperty, thunkPropertyType, hasUnusedClosure);
	if (!result)
		return NULL;

	thunk->m_value = thunkProperty;

	thunkProperty->ensureLayout ();
	return thunkProperty;
}

Property*
FunctionMgr::getDirectDataThunkProperty (
	Variable* targetVariable,
	PropertyType* thunkPropertyType,
	bool hasUnusedClosure
	)
{
	bool result;

	rtl::String signature;
	signature.format (
		"%c%x.%s",
		hasUnusedClosure ? 'U' : 'D',
		targetVariable,
		thunkPropertyType->getSignature ().cc ()
		);

	rtl::StringHashTableMapIterator <Property*> thunk = m_thunkPropertyMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	DataThunkProperty* thunkProperty = (DataThunkProperty*) createProperty (PropertyKind_DataThunk);
	thunkProperty->m_storageKind = StorageKind_Static;
	thunkProperty->m_signature = signature;
	thunkProperty->m_targetVariable = targetVariable;
	thunkProperty->m_tag = "g_directDataThunkProperty";

	if (hasUnusedClosure)
		thunkPropertyType = thunkPropertyType->getStdObjectMemberPropertyType ();

	result = thunkProperty->create (thunkPropertyType);
	if (!result)
		return NULL;

	thunk->m_value = thunkProperty;

	thunkProperty->ensureLayout ();
	m_module->markForCompile (thunkProperty);
	return thunkProperty;
}

Function*
FunctionMgr::getScheduleLauncherFunction (
	FunctionPtrType* targetFunctionPtrType,
	ClassPtrTypeKind schedulerPtrTypeKind
	)
{
	rtl::String signature = targetFunctionPtrType->getSignature ();
	if (schedulerPtrTypeKind == ClassPtrTypeKind_Weak)
		signature += ".w";

	rtl::StringHashTableMapIterator <Function*> thunk = m_scheduleLauncherFunctionMap.visit (signature);
	if (thunk->m_value)
		return thunk->m_value;

	ClassPtrType* schedulerPtrType = ((ClassType*) m_module->m_typeMgr.getStdType (StdType_Scheduler))->getClassPtrType (schedulerPtrTypeKind);

	rtl::Array <FunctionArg*> argArray  = targetFunctionPtrType->getTargetType ()->getArgArray ();
	argArray.insert (0, targetFunctionPtrType->getSimpleFunctionArg ());
	argArray.insert (1, schedulerPtrType->getSimpleFunctionArg ());

	FunctionType* launcherType = m_module->m_typeMgr.getFunctionType (argArray);
	ScheduleLauncherFunction* launcherFunction = (ScheduleLauncherFunction*) createFunction (FunctionKind_ScheduleLauncher, launcherType);
	launcherFunction->m_storageKind = StorageKind_Static;
	launcherFunction->m_tag = "scheduleLauncherFunction";
	launcherFunction->m_signature = signature;

	thunk->m_value = launcherFunction;

	m_module->markForCompile (launcherFunction);
	return launcherFunction;
}

bool
FunctionMgr::injectTlsPrologues ()
{
	rtl::Iterator <Function> functionIt = m_functionList.getHead ();
	for (; functionIt; functionIt++)
	{
		Function* function = *functionIt;
		if (function->getEntryBlock () && function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	rtl::Iterator <ThunkFunction> thunkFunctionIt = m_thunkFunctionList.getHead ();
	for (; thunkFunctionIt; thunkFunctionIt++)
	{
		Function* function = *thunkFunctionIt;
		if (function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	rtl::Iterator <ScheduleLauncherFunction> scheduleLauncherFunctionIt = m_scheduleLauncherFunctionList.getHead ();
	for (; scheduleLauncherFunctionIt; scheduleLauncherFunctionIt++)
	{
		Function* function = *scheduleLauncherFunctionIt;
		if (function->isTlsRequired ())
			injectTlsPrologue (function);
	}

	return true;
}

void
FunctionMgr::injectTlsPrologue (Function* function)
{
	BasicBlock* block = function->getEntryBlock ();
	ASSERT (block);

	m_module->m_controlFlowMgr.setCurrentBlock (block);
	m_module->m_llvmIrBuilder.setInsertPoint (block->getLlvmBlock ()->begin ());

	Function* getTls = getStdFunction (StdFunction_GetTls);
	
	Value tlsValue;
	m_module->m_llvmIrBuilder.createCall (getTls, getTls->getType (), &tlsValue);

	// tls variables used in this function

	rtl::Array <TlsVariable> tlsVariableArray = function->getTlsVariableArray ();
	size_t count = tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = tlsVariableArray [i].m_variable->getTlsField ();
		ASSERT (field);

		Value ptrValue;
		m_module->m_llvmIrBuilder.createGep2 (tlsValue, field->getLlvmIndex (), NULL, &ptrValue);
		tlsVariableArray [i].m_llvmAlloca->replaceAllUsesWith (ptrValue.getLlvmValue ());
	}

	// unfortunately, erasing could not be safely done inside the above loop (cause of InsertPoint)
	// so just have a dedicated loop for erasing alloca's

	count = tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
		tlsVariableArray [i].m_llvmAlloca->eraseFromParent ();
}

void
llvmFatalErrorHandler (
	void* context,
	const std::string& errorString,
	bool shouldGenerateCrashDump
	)
{
	throw err::createStringError (errorString.c_str ());
}

bool
FunctionMgr::jitFunctions ()
{
#ifdef _JNC_NO_JIT
	err::setFormatStringError ("LLVM jitting is disabled");
	return false;
#endif

	llvm::ScopedFatalErrorHandler scopeErrorHandler (llvmFatalErrorHandler);

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine ();
	
	try
	{
		rtl::Iterator <Function> functionIt = m_functionList.getHead ();
		for (; functionIt; functionIt++)
		{
			Function* function = *functionIt;
			if (!function->getEntryBlock ())
				continue;

			llvm::Function* llvmFunction = function->getLlvmFunction ();
			function->m_machineCode = llvmExecutionEngine->getPointerToFunction (llvmFunction);
		}

		llvmExecutionEngine->finalizeObject ();
	}
	catch (err::Error error)
	{
		err::setFormatStringError ("LLVM jitting failed: %s", error->getDescription ().cc ());
		return false;
	}

	return true;
}

Function*
FunctionMgr::getStdFunction (StdFunction func)
{
	ASSERT ((size_t) func < StdFunction__Count);

	if (m_stdFunctionArray [func])
		return m_stdFunctionArray [func];

	// 8 is enough for all the std functions

	Type* argTypeArray [8] = { 0 }; 
	llvm::Type* llvmArgTypeArray [8] = { 0 };

	Type* returnType;
	FunctionType* functionType;
	Function* function;
	const StdItemSource* source;

	switch (func)
	{
	case StdFunction_TryAllocateClass:
		returnType = m_module->m_typeMgr.getStdType (StdType_BoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateClass", functionType);
		break;

	case StdFunction_AllocateClass:
		returnType = m_module->m_typeMgr.getStdType (StdType_BoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.allocateClass", functionType);
		break;

	case StdFunction_TryAllocateData:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataBoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateData", functionType);
		break;

	case StdFunction_AllocateData:
		returnType = m_module->m_typeMgr.getStdType (StdType_DataBoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.allocateData", functionType);
		break;

	case StdFunction_TryAllocateArray:
		returnType = m_module->m_typeMgr.getStdType (StdType_DynamicArrayBoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 2);
		function = createFunction (FunctionKind_Internal, "jnc.tryAllocateClass", functionType);
		break;

	case StdFunction_AllocateArray:
		returnType = m_module->m_typeMgr.getStdType (StdType_DynamicArrayBoxPtr);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 1);
		function = createFunction (FunctionKind_Internal, "jnc.allocateArray", functionType);
		break;

	case StdFunction_LlvmMemcpy:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemcpy", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [1]->getLlvmType ();
		llvmArgTypeArray [2] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memcpy,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 3)
			);
		break;

	case StdFunction_LlvmMemmove:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemmove", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [1]->getLlvmType ();
		llvmArgTypeArray [2] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memmove,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 3)
			);
		break;

	case StdFunction_LlvmMemset:
		returnType = m_module->m_typeMgr.getPrimitiveType (TypeKind_Void);
		argTypeArray [0] = m_module->m_typeMgr.getStdType (StdType_BytePtr);
		argTypeArray [1] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int8);
		argTypeArray [2] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [3] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32);
		argTypeArray [4] = m_module->m_typeMgr.getPrimitiveType (TypeKind_Bool);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, argTypeArray, 5);
		function = createFunction (FunctionKind_Internal, "jnc.llvmMemset", functionType);

		llvmArgTypeArray [0] = argTypeArray [0]->getLlvmType ();
		llvmArgTypeArray [1] = argTypeArray [2]->getLlvmType ();
		function->m_llvmFunction = llvm::Intrinsic::getDeclaration (
			m_module->getLlvmModule (), 
			llvm::Intrinsic::memset,
			llvm::ArrayRef <llvm::Type*> (llvmArgTypeArray, 2)
			);

		break;

	case StdFunction_GetTls:
		returnType = m_module->m_variableMgr.getTlsStructType ()->getDataPtrType (DataPtrTypeKind_Thin);
		functionType = m_module->m_typeMgr.getFunctionType (returnType, NULL, 0);
		function = createFunction (FunctionKind_Internal, "jnc.getTls", functionType);
		break;

	case StdFunction_StrLen:
	case StdFunction_StrCmp:
	case StdFunction_StriCmp:
	case StdFunction_StrChr:
	case StdFunction_StrCat:
	case StdFunction_StrDup:
	case StdFunction_MemCmp:
	case StdFunction_MemChr:
	case StdFunction_MemCpy:
	case StdFunction_MemSet:
	case StdFunction_MemCat:
	case StdFunction_MemDup:
	case StdFunction_Rand:
	case StdFunction_Printf:
	case StdFunction_Atoi:
#if (_AXL_ENV == AXL_ENV_POSIX)
		ASSERT (sourceTable [func].m_p);
		function = parseStdFunction (
			sourceTable [func].m_stdNamespace,
			sourceTable [func].m_p,
			sourceTable [func].m_length
			);

		ASSERT (!function->m_llvmFunction);
		function->m_tag += "_jnc"; // as to avoid mapping conflicts
		break;
#endif

	case StdFunction_DynamicSizeOf:
	case StdFunction_DynamicCountOf:
	case StdFunction_DynamicCastDataPtr:
	case StdFunction_DynamicCastClassPtr:
	case StdFunction_DynamicCastVariant:
	case StdFunction_StrengthenClassPtr:
	case StdFunction_GcSafePoint:
	case StdFunction_CollectGarbage:
	case StdFunction_CreateThread:
	case StdFunction_Sleep:
	case StdFunction_GetTimestamp:
	case StdFunction_GetCurrentThreadId:
	case StdFunction_Throw:
	case StdFunction_GetLastError:
	case StdFunction_SetPosixError:
	case StdFunction_SetStringError:
	case StdFunction_AssertionFailure:
	case StdFunction_AddStaticDestructor:
	case StdFunction_AddStaticClassDestructor:
	case StdFunction_Format:
	case StdFunction_AppendFmtLiteral_a:
	case StdFunction_AppendFmtLiteral_p:
	case StdFunction_AppendFmtLiteral_i32:
	case StdFunction_AppendFmtLiteral_ui32:
	case StdFunction_AppendFmtLiteral_i64:
	case StdFunction_AppendFmtLiteral_ui64:
	case StdFunction_AppendFmtLiteral_f:
	case StdFunction_AppendFmtLiteral_v:
	case StdFunction_AppendFmtLiteral_s:
	case StdFunction_AppendFmtLiteral_sr:
	case StdFunction_AppendFmtLiteral_cb:
	case StdFunction_AppendFmtLiteral_cbr:
	case StdFunction_AppendFmtLiteral_br:
	case StdFunction_TryCheckDataPtrRangeDirect:
	case StdFunction_CheckDataPtrRangeDirect:
	case StdFunction_TryCheckDataPtrRangeIndirect:
	case StdFunction_CheckDataPtrRangeIndirect:
	case StdFunction_TryCheckNullPtr:
	case StdFunction_CheckNullPtr:
	case StdFunction_TryLazyGetLibraryFunction:
	case StdFunction_LazyGetLibraryFunction:
		source = getStdFunctionSource (func);
			
		ASSERT (source->m_p);
		function = parseStdFunction (
			source->m_stdNamespace,
			source->m_p,
			source->m_length
			);
		break;

	case StdFunction_SimpleMulticastCall:
		function = ((MulticastClassType*) m_module->m_typeMgr.getStdType (StdType_SimpleMulticast))->getMethod (MulticastMethodKind_Call);
		break;

	default:
		ASSERT (false);
		function = NULL;
	}

	m_stdFunctionArray [func] = function;
	return function;
}

Function*
FunctionMgr::parseStdFunction (
	StdNamespace stdNamespace,
	const char* source,
	size_t length
	)
{
	bool result;

	Lexer lexer;
	lexer.create ("jnc_StdFunctions.jnc", source, length);

	if (stdNamespace)
		m_module->m_namespaceMgr.openStdNamespace (stdNamespace);

	Parser parser (m_module);
	parser.create (SymbolKind_normal_item_declaration);
	parser.m_stage = Parser::StageKind_Pass2; // no imports

	for (;;)
	{
		const Token* token = lexer.getToken ();

		result = parser.parseToken (token);
		if (!result)
		{
			dbg::trace ("parse std function error: %s\n", err::getLastError ()->getDescription ().cc ());
			ASSERT (false);
		}

		if (token->m_token == TokenKind_Eof) // EOF token must be parsed
			break;

		lexer.nextToken ();
	}

	if (stdNamespace)
		m_module->m_namespaceMgr.closeNamespace ();

	ModuleItem* item = parser.m_lastDeclaredItem;
	ASSERT (item && item->getItemKind () == ModuleItemKind_Function);

	result = m_module->postParseStdItem ();
	ASSERT (result);

	return (Function*) item;
}

LazyStdFunction*
FunctionMgr::getLazyStdFunction (StdFunction func)
{
	ASSERT ((size_t) func < StdFunction__Count);

	if (m_lazyStdFunctionArray [func])
		return m_lazyStdFunctionArray [func];

	const char* name = getStdFunctionName (func);
	ASSERT (name);

	LazyStdFunction* function = AXL_MEM_NEW (LazyStdFunction);
	function->m_module = m_module;
	function->m_name = name;
	function->m_func = func;
	m_lazyStdFunctionList.insertTail (function);
	m_lazyStdFunctionArray [func] = function;
	return function;
}

//.............................................................................

} // namespace jnc {

