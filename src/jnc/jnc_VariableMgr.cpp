#include "pch.h"
#include "jnc_VariableMgr.h"
#include "jnc_Parser.llk.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

VariableMgr::VariableMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
	
	m_tlsStructType = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	getStdVariable (StdVariable_GcShadowStackTop); // this variable is required even if it's not used
}

void
VariableMgr::clear ()
{
	m_variableList.clear ();
	m_aliasList.clear ();

	m_staticVariableArray.clear ();
	m_staticGcRootArray.clear ();
	m_globalStaticVariableArray.clear ();

	m_tlsVariableArray.clear ();
	m_tlsStructType = NULL;

	memset (m_stdVariableArray, 0, sizeof (m_stdVariableArray));
	getStdVariable (StdVariable_GcShadowStackTop); // this variable is required even if it's not used
}

Variable*
VariableMgr::getStdVariable (StdVariable stdVariable)
{
	ASSERT ((size_t) stdVariable < StdVariable__Count);

	if (m_stdVariableArray [stdVariable])
		return m_stdVariableArray [stdVariable];

	Variable* variable;

	switch (stdVariable)
	{
	case StdVariable_GcShadowStackTop:
		variable = createVariable (
			StorageKind_Thread,
			"g_gcShadowStackTop",
			"jnc.g_gcShadowStackTop",
			m_module->m_typeMgr.getStdType (StdType_BytePtr)
			);
		break;

	case StdVariable_GcSafePointTrigger:
		variable = createVariable (
			StorageKind_Static,
			"g_gcSafePointTrigger",
			"jnc.g_gcSafePointTrigger",
			m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr)->getDataPtrType_c ()
			);
		break;

	default:
		ASSERT (false);
		variable = NULL;
	}

	m_stdVariableArray [stdVariable] = variable;
	return variable;
}

Variable*
VariableMgr::createVariable (
	StorageKind storageKind,
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	uint_t ptrTypeFlags,
	rtl::BoxList <Token>* constructor,
	rtl::BoxList <Token>* initializer
	)
{
	bool result;

	Variable* variable = AXL_MEM_NEW (Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_tag = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = storageKind;
	variable->m_ptrTypeFlags = ptrTypeFlags;
	variable->m_scope = m_module->m_namespaceMgr.getCurrentScope ();

	if (constructor)
		variable->m_constructor.takeOver (constructor);

	if (initializer)
		variable->m_initializer.takeOver (initializer);

	m_variableList.insertTail (variable);

	Value ptrValue;

	switch (storageKind)
	{
	case StorageKind_Static:
		m_staticVariableArray.append (variable);

		if (!m_module->m_namespaceMgr.getCurrentScope ())
		{
			m_globalStaticVariableArray.append (variable);
			break;
		}

		variable->m_llvmGlobalVariable = createLlvmGlobalVariable (type, qualifiedName);
		variable->m_llvmValue = variable->m_llvmGlobalVariable;
		
		if (type->getFlags () & TypeFlag_GcRoot)
			m_staticGcRootArray.append (variable);

		break;

	case StorageKind_Thread:
		m_tlsVariableArray.append (variable);
		break;

	case StorageKind_Stack:
		m_module->m_llvmIrBuilder.createAlloca (type, qualifiedName, NULL, &ptrValue);
		variable->m_llvmValue = ptrValue.getLlvmValue ();
		break;

	case StorageKind_Heap:
		result = allocateHeapVariable (variable);
		if (!result)
			return false;
		break;

	default:
		ASSERT (false);
	}

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		variable->m_type_i = (ImportType*) type;
		m_module->markForLayout (variable);
	}

	return variable;
}

Variable*
VariableMgr::createSimpleStaticVariable (
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	const Value& value,
	uint_t ptrTypeFlags
	)
{
	ASSERT (type->getTypeKind () != TypeKind_Class);

	Variable* variable = AXL_MEM_NEW (Variable);
	variable->m_module = m_module;
	variable->m_name = name;
	variable->m_qualifiedName = qualifiedName;
	variable->m_tag = qualifiedName;
	variable->m_type = type;
	variable->m_storageKind = StorageKind_Static;
	variable->m_ptrTypeFlags = ptrTypeFlags;
	variable->m_scope = m_module->m_namespaceMgr.getCurrentScope ();
	variable->m_llvmGlobalVariable = createLlvmGlobalVariable (type, qualifiedName, value);
	variable->m_llvmValue = variable->m_llvmGlobalVariable;

	if (type->getFlags () & TypeFlag_GcRoot)	
		m_staticGcRootArray.append (variable);

	m_variableList.insertTail (variable);
	return variable;
}

bool
VariableMgr::initializeVariable (Variable* variable)
{
	switch (variable->m_storageKind)
	{
	case StorageKind_Static:
		if (variable->m_type->getTypeKind () == TypeKind_Class)
			primeStaticClassVariable (variable);
		break;

	case StorageKind_Thread:
	case StorageKind_Heap:
		break;

	case StorageKind_Stack:
		if (variable->m_type->getFlags () & TypeFlag_GcRoot)
		{
			m_module->m_operatorMgr.zeroInitialize (variable);
			m_module->m_operatorMgr.markStackGcRoot (variable, variable->m_type);
		}
		else if ((variable->m_type->getTypeKindFlags () & TypeKindFlag_Aggregate) || variable->m_initializer.isEmpty ())
		{
			m_module->m_operatorMgr.zeroInitialize (variable);
		}
		break;

	default:
		ASSERT (false);
	};

	return m_module->m_operatorMgr.parseInitializer (
		variable,
		variable->m_itemDecl->getParentUnit (),
		variable->m_constructor,
		variable->m_initializer
		);
}

llvm::GlobalVariable*
VariableMgr::createLlvmGlobalVariable (
	Type* type,
	const char* tag,
	const Value& initValue
	)
{
	llvm::Constant* llvmInitConstant = initValue ? 
		(llvm::Constant*) initValue.getLlvmValue () :
		(llvm::Constant*) type->getZeroValue ().getLlvmValue ();

	return new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		type->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmInitConstant,
		tag
		);
}

void
VariableMgr::primeStaticClassVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Static && variable->m_type->getTypeKind () == TypeKind_Class);

	Function* primeStaticClass = m_module->m_functionMgr.getStdFunction (StdFunc_PrimeStaticClass);

	Value argValueArray [2];
	m_module->m_llvmIrBuilder.createBitCast (
		variable->m_llvmValue,  
		m_module->m_typeMgr.getStdType (StdType_BoxPtr), 
		&argValueArray [0]
		);

	argValueArray [1].createConst (
		&variable->m_type, 
		m_module->m_typeMgr.getStdType (StdType_BytePtr)
		);

	m_module->m_llvmIrBuilder.createCall (
		primeStaticClass, 
		primeStaticClass->getType (),
		argValueArray, 
		2,
		NULL
		);

	Value ifaceValue;
	m_module->m_llvmIrBuilder.createGep2 (variable->m_llvmValue, 1, NULL, &ifaceValue);
	variable->m_llvmValue = ifaceValue.getLlvmValue ();

	Function* destructor = ((ClassType*) variable->m_type)->getDestructor ();
	if (destructor)
	{
		Function* addDestructor = m_module->m_functionMgr.getStdFunction (StdFunc_AddStaticClassDestructor);

		Value argValueArray [2];

		m_module->m_llvmIrBuilder.createBitCast (destructor, m_module->m_typeMgr.getStdType (StdType_BytePtr), &argValueArray [0]);
		m_module->m_llvmIrBuilder.createBitCast (variable, m_module->m_typeMgr.getStdType (StdType_AbstractClassPtr), &argValueArray [1]);
		m_module->m_llvmIrBuilder.createCall (addDestructor, addDestructor->getType (), argValueArray, countof (argValueArray), NULL);
	}
}

Variable*
VariableMgr::createOnceFlagVariable (StorageKind storageKind)
{
	return createVariable (
		storageKind,
		"onceFlag",
		"onceFlag",
		m_module->m_typeMgr.getPrimitiveType (TypeKind_Int32),
		storageKind == StorageKind_Static ? PtrTypeFlag_Volatile : 0
		);
}

Variable*
VariableMgr::createStaticDataPtrValidatorVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Static);

	// create static box

	StructType* boxType = (StructType*) m_module->m_typeMgr.getStdType (StdType_StaticDataBox);

	uintptr_t flags = BoxFlag_StaticData | BoxFlag_DataMark | BoxFlag_WeakMark;

	Value variablePtrValue;
	m_module->m_llvmIrBuilder.createBitCast (
		variable, 
		m_module->m_typeMgr.getStdType (StdType_BytePtr), 
		&variablePtrValue
		);

	ASSERT (llvm::isa <llvm::Constant> (variablePtrValue.getLlvmValue ()));

	llvm::Constant* llvmMemberArray [4];
	llvmMemberArray [0] = Value::getLlvmConst (m_module->m_typeMgr.getStdType (StdType_BytePtr), &variable->m_type);
	llvmMemberArray [1] = Value::getLlvmConst (m_module->m_typeMgr.getPrimitiveType (TypeKind_IntPtr_u), &flags);
	llvmMemberArray [2] = (llvm::Constant*) variablePtrValue.getLlvmValue ();

	llvm::Constant* llvmBoxConst = llvm::ConstantStruct::get (
		(llvm::StructType*) boxType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmMemberArray, 3)
		);

	rtl::String boxTag = variable->m_tag + ".box";

	llvm::GlobalVariable* llvmBoxVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		boxType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmBoxConst,
		boxTag.cc ()
		);

	// now validator

	StructType* validatorType = (StructType*) m_module->m_typeMgr.getStdType (StdType_DataPtrValidator);

	Value boxPtrValue;
	m_module->m_llvmIrBuilder.createBitCast (
		llvmBoxVariable, 
		m_module->m_typeMgr.getStdType (StdType_BoxPtr), 
		&boxPtrValue
		);

	size_t size = variable->m_type->getSize ();

	ASSERT (llvm::isa <llvm::Constant> (boxPtrValue.getLlvmValue ()));

	llvmMemberArray [0] = (llvm::Constant*) boxPtrValue.getLlvmValue ();
	llvmMemberArray [1] = (llvm::Constant*) boxPtrValue.getLlvmValue ();
	llvmMemberArray [2] = (llvm::Constant*) variablePtrValue.getLlvmValue ();
	llvmMemberArray [3] = Value::getLlvmConst (m_module->m_typeMgr.getPrimitiveType (TypeKind_SizeT), &size);

	llvm::Constant* llvmValidatorConst = llvm::ConstantStruct::get (
		(llvm::StructType*) validatorType->getLlvmType (),
		llvm::ArrayRef <llvm::Constant*> (llvmMemberArray, 4)
		);

	rtl::String validatorTag = variable->m_tag + ".validator";

	llvm::GlobalVariable* llvmValidatorVariable = new llvm::GlobalVariable (
		*m_module->getLlvmModule (),
		validatorType->getLlvmType (),
		false,
		llvm::GlobalVariable::InternalLinkage,
		llvmValidatorConst,
		validatorTag.cc ()
		);

	Variable* validatorVariable = AXL_MEM_NEW (Variable);
	validatorVariable->m_module = m_module;
	validatorVariable->m_name = validatorTag;
	validatorVariable->m_qualifiedName = validatorTag;
	validatorVariable->m_tag = validatorTag;
	validatorVariable->m_type = validatorType;
	validatorVariable->m_storageKind = StorageKind_Static;
	validatorVariable->m_ptrTypeFlags = 0;
	validatorVariable->m_scope = NULL;
	validatorVariable->m_llvmValue = llvmValidatorVariable;
	m_variableList.insertTail (validatorVariable);

	LeanDataPtrValidator* validator = variable->getLeanDataPtrValidator ();
	validator->m_validatorValue = validatorVariable;
	return validatorVariable;
}

bool
VariableMgr::allocateHeapVariable (Variable* variable)
{
	Value ptrValue;
	
	bool result = m_module->m_operatorMgr.gcHeapAllocate (variable->m_type, &ptrValue);
	if (!result)
		return NULL;

	if (variable->m_type->getTypeKind () == TypeKind_Class)
	{
		variable->m_llvmValue = ptrValue.getLlvmValue ();
	}
	else
	{
		Value variableValue;
		Value validatorValue;

		m_module->m_llvmIrBuilder.createExtractValue (ptrValue, 0, NULL, &variableValue);
		m_module->m_llvmIrBuilder.createExtractValue (ptrValue, 1, NULL, &validatorValue);
		m_module->m_llvmIrBuilder.createBitCast (variableValue, variable->m_type->getDataPtrType_c (), &variableValue);
		variable->m_llvmValue = variableValue.getLlvmValue ();

		LeanDataPtrValidator* validator = variable->getLeanDataPtrValidator ();
		validator->m_validatorValue = validatorValue;
	}

	m_module->m_operatorMgr.markStackGcRoot (variable, variable->m_type->getDataPtrType_c (), StackGcRootKind_Scope, variable->m_scope);
	return true;
}

void
VariableMgr::liftStackVariable (Variable* variable)
{
	ASSERT (variable->m_storageKind == StorageKind_Stack);
	ASSERT (llvm::isa <llvm::AllocaInst> (variable->m_llvmValue));

	variable->m_storageKind = StorageKind_Heap;

	llvm::AllocaInst* llvmAlloca = (llvm::AllocaInst*) (llvm::AllocaInst*) variable->m_llvmValue;
	BasicBlock* currentBlock = m_module->m_controlFlowMgr.getCurrentBlock ();
	m_module->m_llvmIrBuilder.setInsertPoint (llvmAlloca);
	
	bool result = allocateHeapVariable (variable);
	ASSERT (result);

	m_module->m_llvmIrBuilder.setInsertPoint (currentBlock);

	llvmAlloca->replaceAllUsesWith (variable->m_llvmValue);
	llvmAlloca->eraseFromParent ();
}

Variable*
VariableMgr::createArgVariable (FunctionArg* arg)
{
	Variable* variable = createSimpleStackVariable (
		arg->getName (),
		arg->getType (),
		arg->getPtrTypeFlags ()
		);

	variable->m_parentUnit = arg->getParentUnit ();
	variable->m_pos = *arg->getPos ();
	variable->m_flags |= ModuleItemFlag_User;
		
	if ((m_module->getCompileFlags () & ModuleCompileFlag_DebugInfo) &&
		(variable->getFlags () & ModuleItemFlag_User))
	{
		variable->m_llvmDiDescriptor = m_module->m_llvmDiBuilder.createLocalVariable (
			variable,
			llvm::dwarf::DW_TAG_arg_variable
			);

		m_module->m_llvmDiBuilder.createDeclare (variable);
	}

	// arg variables are not initialized (stored to directly), so mark gc root manually

	if (variable->m_type->getFlags () & TypeFlag_GcRoot)
		m_module->m_operatorMgr.markStackGcRoot (variable, variable->m_type, StackGcRootKind_Function);
	
	return variable;
}

Alias*
VariableMgr::createAlias (
	const rtl::String& name,
	const rtl::String& qualifiedName,
	Type* type,
	rtl::BoxList <Token>* initializer
	)
{
	ASSERT (initializer);

	Alias* alias = AXL_MEM_NEW (Alias);
	alias->m_name = name;
	alias->m_qualifiedName = qualifiedName;
	alias->m_tag = qualifiedName;
	alias->m_type = type;
	alias->m_initializer.takeOver (initializer);

	m_aliasList.insertTail (alias);

	if (type->getTypeKindFlags () & TypeKindFlag_Import)
	{
		alias->m_type_i = (ImportType*) type;
		m_module->markForLayout (alias);
	}

	return alias;
}

bool
VariableMgr::createTlsStructType ()
{
	bool result;

	StructType* type = m_module->m_typeMgr.createStructType ("Tls", "jnc.Tls");

	size_t count = m_tlsVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_tlsVariableArray [i];

		if (variable->m_type->getTypeKindFlags () & TypeKindFlag_Aggregate)
		{
			err::setFormatStringError ("'thread' variables cannot have aggregate type '%s'",  variable->m_type->getTypeString ().cc ());
			return false;
		}

		variable->m_tlsField = type->createField (variable->m_type);
	}

	result = type->ensureLayout ();
	if (!result)
		return false;

	m_tlsStructType = type;
	return true;
}

bool
VariableMgr::allocateInitializeGlobalVariables ()
{
	bool result;

	size_t count = m_globalStaticVariableArray.getCount ();
	for (size_t i = 0; i < count; i++)
	{
		Variable* variable = m_globalStaticVariableArray [i];
		ASSERT (!variable->m_llvmValue);

		variable->m_llvmGlobalVariable = createLlvmGlobalVariable (variable->m_type, variable->m_qualifiedName);
		variable->m_llvmValue = variable->m_llvmGlobalVariable;

		if (variable->m_type->getFlags () & TypeFlag_GcRoot)
			m_staticGcRootArray.append (variable);

		result = initializeVariable (variable);
		if (!result)
			return false;
	}

	return true;
}

//.............................................................................

} // namespace jnc {
