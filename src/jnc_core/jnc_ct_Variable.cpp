#include "pch.h"
#include "jnc_ct_Variable.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

Variable::Variable ()
{
	m_itemKind = ModuleItemKind_Variable;
	m_type = NULL;
	m_type_i = NULL;
	m_ptrTypeFlags = 0;
	m_scope = NULL;
	m_tlsField = NULL;
	m_staticData = NULL;
	m_llvmGlobalVariable = NULL;
	m_stackInitializeBlock = NULL;
	m_llvmBeforeStackInitialize = NULL;

	m_llvmValue = NULL;
}

LeanDataPtrValidator*
Variable::getLeanDataPtrValidator ()
{
	if (m_leanDataPtrValidator)
		return m_leanDataPtrValidator;

	Value originValue (this);
	m_leanDataPtrValidator = AXL_REF_NEW (LeanDataPtrValidator);
	m_leanDataPtrValidator->m_originValue = originValue;
	m_leanDataPtrValidator->m_rangeBeginValue = originValue;
	m_leanDataPtrValidator->m_rangeLength = m_type->getSize ();
	return m_leanDataPtrValidator;
}

void*
Variable::getStaticData ()
{
	ASSERT (m_storageKind == StorageKind_Static);
	
	if (m_staticData)
		return m_staticData;

	llvm::ExecutionEngine* llvmExecutionEngine = m_module->getLlvmExecutionEngine ();

	m_staticData = (m_module->getCompileFlags () & ModuleCompileFlag_McJit) ?
		(void*) llvmExecutionEngine->getGlobalValueAddress (m_llvmGlobalVariable->getName ()) :
		(void*) llvmExecutionEngine->getPointerToGlobal (m_llvmGlobalVariable);

	return m_staticData;
}

llvm::Value*
Variable::getLlvmValue ()
{
	if (m_llvmValue)
		return m_llvmValue;

	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function && m_storageKind == StorageKind_Thread);

	BasicBlock* prevBlock = m_module->m_controlFlowMgr.setCurrentBlock (function->getEntryBlock ());

	Value ptrValue;
	m_llvmValue = m_module->m_llvmIrBuilder.createAlloca (
		m_type,
		m_qualifiedName,
		NULL,
		&ptrValue
		);

	m_module->m_controlFlowMgr.setCurrentBlock (prevBlock);
	function->addTlsVariable (this);
	return m_llvmValue;
}

bool
Variable::calcLayout ()
{
	if (m_type_i)
		m_type = m_type_i->getActualType ();

	return m_type->ensureLayout ();
}

bool
Variable::isInitializationNeeded ()
{
	return 
		!m_constructor.isEmpty () ||
		!m_initializer.isEmpty () || 
		m_type->getTypeKind () == TypeKind_Class; // static class variable
}

//.............................................................................

} // namespace ct
} // namespace jnc
