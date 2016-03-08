#include "pch.h"
#include "jnc_ct_NamespaceMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

NamespaceMgr::NamespaceMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);

	sl::String jncName ("jnc");

	GlobalNamespace* global = &m_stdNamespaceArray [StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray [StdNamespace_Jnc];
	GlobalNamespace* internal = &m_stdNamespaceArray [StdNamespace_Internal];

	global->m_module = m_module;

	jnc->m_module = m_module;
	jnc->m_parentNamespace = global;
	jnc->m_name = jncName;
	jnc->m_qualifiedName = jncName;
	jnc->m_tag = jncName;
	jnc->m_flags |= ModuleItemFlag_Sealed;

	internal->m_module = m_module;
	internal->m_parentNamespace = global;
	internal->m_name = jncName;
	internal->m_qualifiedName = jncName;
	internal->m_tag = jncName;

	m_currentNamespace = global;
	m_currentScope = NULL;
	m_currentAccessKind = AccessKind_Public;
	m_sourcePosLockCount = 0;
}

void
NamespaceMgr::clear ()
{
	for (size_t i = 0; i < StdNamespace__Count; i++)
		m_stdNamespaceArray [i].clear ();

	m_globalNamespaceList.clear ();
	m_extensionNamespaceList.clear ();
	m_dynamicLibNamespaceList.clear ();
	m_scopeList.clear ();
	m_orphanList.clear ();
	m_namespaceStack.clear ();
	m_currentNamespace = &m_stdNamespaceArray [StdNamespace_Global];
	m_currentScope = NULL;
	m_sourcePosLockCount = 0;
	m_staticObjectValue.clear ();
}

bool
NamespaceMgr::addStdItems ()
{
	GlobalNamespace* global = &m_stdNamespaceArray [StdNamespace_Global];
	GlobalNamespace* jnc = &m_stdNamespaceArray [StdNamespace_Jnc];

	return
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uintptr_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_size_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint8_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uchar_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_byte_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint16_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_ushort_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_word_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint32_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_dword_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_uint64_t)) &&
		global->addItem (m_module->m_typeMgr.getStdTypedef (StdTypedef_qword_t)) &&
		global->addItem (jnc) &&
		jnc->addItem ("collectGarbage", m_module->m_functionMgr.getLazyStdFunction (StdFunc_CollectGarbage)) &&
		jnc->addItem ("Scheduler", m_module->m_typeMgr.getLazyStdType (StdType_Scheduler)) &&
		jnc->addItem ("AutomatonResult", m_module->m_typeMgr.getLazyStdType (StdType_AutomatonResult)) &&
		jnc->addItem ("AutomatonFunc", m_module->m_typeMgr.getLazyStdType (StdType_AutomatonFunc)) &&
		jnc->addItem ("Recognizer", m_module->m_typeMgr.getLazyStdType (StdType_Recognizer)) &&
		jnc->addItem ("DynamicLib", m_module->m_typeMgr.getLazyStdType (StdType_DynamicLib));
}

Orphan*
NamespaceMgr::createOrphan (
	OrphanKind orphanKind,
	FunctionType* functionType
	)
{
	Orphan* orphan = AXL_MEM_NEW (Orphan);
	orphan->m_orphanKind = orphanKind;
	orphan->m_functionType = functionType;
	m_orphanList.insertTail (orphan);
	return orphan;
}

bool
NamespaceMgr::resolveOrphans ()
{
	bool result;

	sl::Iterator <Orphan> it = m_orphanList.getHead ();
	for (; it; it++)
	{
		result = it->resolveOrphan ();
		if (!result)
			return false;
	}

	return true;
}

void
NamespaceMgr::setSourcePos (const Token::Pos& pos)
{
	if (!(m_module->getCompileFlags () & ModuleCompileFlag_DebugInfo) ||
		!m_currentScope ||
		m_sourcePosLockCount)
		return;

	llvm::DebugLoc llvmDebugLoc = m_module->m_llvmDiBuilder.getDebugLoc (m_currentScope, pos);
	m_module->m_llvmIrBuilder.setCurrentDebugLoc (llvmDebugLoc);
}

void
NamespaceMgr::openNamespace (Namespace* nspace)
{
	NamespaceStackEntry entry =
	{
		m_currentNamespace,
		m_currentScope,
		m_currentAccessKind
	};

	m_namespaceStack.append (entry);
	m_currentNamespace = nspace;
	m_currentAccessKind = AccessKind_Public; // always start with 'public'

	if (nspace->m_namespaceKind == NamespaceKind_Scope)
		m_currentScope =  (Scope*) nspace;
}

void
NamespaceMgr::closeNamespace ()
{
	if (m_namespaceStack.isEmpty ())
		return;
	
	NamespaceStackEntry entry = m_namespaceStack.getBackAndPop ();

	if (m_currentNamespace->m_namespaceKind == NamespaceKind_Global) // for others not really needed
		m_currentNamespace->m_usingSet.clear ();

	m_currentNamespace = entry.m_namespace;
	m_currentScope = entry.m_scope;
	m_currentAccessKind = entry.m_accessKind;
}

Scope*
NamespaceMgr::openInternalScope ()
{
	Function* function = m_module->m_functionMgr.getCurrentFunction ();
	ASSERT (function);

	m_module->m_gcShadowStackMgr.releaseTmpGcRoots (); // make sure tmp-gc-root-scope has no children

	Scope* scope = AXL_MEM_NEW (Scope);
	scope->m_module = m_module;
	scope->m_function = function;
	scope->m_parentNamespace = m_currentNamespace;

	if (m_currentScope)
	{
		// propagate parent scope traits
		scope->m_flags |= m_currentScope->m_flags & (ScopeFlag_Finalizable | ScopeFlag_StaticThrow);
		scope->m_gcShadowStackFrameMap = m_currentScope->m_gcShadowStackFrameMap;
	}
	else 
	{
		scope->m_flags = ScopeFlag_Function;
		if (function->getType ()->getFlags () & FunctionTypeFlag_ErrorCode)
			scope->m_flags |= ScopeFlag_StaticThrow;
	}

	m_scopeList.insertTail (scope);

	openNamespace (scope);
	return scope;
}

Scope*
NamespaceMgr::openScope (
	const Token::Pos& pos,
	uint_t flags	
	)
{
	Scope* parentScope = m_currentScope;
	Scope* scope = openInternalScope ();
	scope->m_pos = pos;
	scope->m_flags |= flags;

	if (scope->m_parentNamespace == scope->m_function->getScope ())
		scope->m_flags |= ScopeFlag_Function;

	if (m_module->getCompileFlags () & ModuleCompileFlag_DebugInfo)
		scope->m_llvmDiScope = (llvm::DIScope) m_module->m_llvmDiBuilder.createLexicalBlock (parentScope, pos);

	setSourcePos (pos);

	if (flags & ScopeFlag_CatchAhead)
	{
		scope->m_catchBlock = m_module->m_controlFlowMgr.createBlock ("catch_block", BasicBlockFlag_Catch);

		if (ScopeFlag_FinallyAhead)
			scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock ("catch_finally_block", BasicBlockFlag_Finally);

		m_module->m_controlFlowMgr.setJmp (scope->m_catchBlock, &scope->m_prevSjljFrameValue);
	}
	else if (flags & ScopeFlag_Disposable)
	{
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock ("dispose_finally_block", BasicBlockFlag_Finally);
		m_module->m_controlFlowMgr.setJmpFinally (scope->m_finallyBlock, &scope->m_prevSjljFrameValue);

		Type* type = m_module->m_typeMgr.getPrimitiveType (TypeKind_Int);
		scope->m_disposeLevelVariable = m_module->m_variableMgr.createSimpleStackVariable ("dispose_level", type);
		m_module->m_llvmIrBuilder.createStore (type->getZeroValue (), scope->m_disposeLevelVariable);
	}
	else if (flags & ScopeFlag_FinallyAhead)
	{
		scope->m_finallyBlock = m_module->m_controlFlowMgr.createBlock ("finally_block", BasicBlockFlag_Finally);
		m_module->m_controlFlowMgr.setJmpFinally (scope->m_finallyBlock, &scope->m_prevSjljFrameValue);
	}
	
	if (flags & ScopeFlag_Nested)
	{
		if (parentScope->m_flags & (ScopeFlag_Try | ScopeFlag_Catch | ScopeFlag_Finally | ScopeFlag_Nested))
		{
			err::setFormatStringError ("'nestedscope' can only be used in regular scopes (not 'try', 'catch', 'finally' or 'nestedscope')");
			return false;
		}

		scope->m_flags |= parentScope->m_flags & ScopeFlag_Function; // propagate function flag
	}

	return scope;
}

void
NamespaceMgr::closeScope ()
{
	ASSERT (m_currentScope);
	m_module->m_gcShadowStackMgr.finalizeScope (m_currentScope);
	
	uint_t flags = m_currentScope->m_flags;

	if (flags & ScopeFlag_Disposable) 
	{
		m_currentScope->m_flags &= ~ScopeFlag_Disposable; // prevent recursion
		m_module->m_controlFlowMgr.closeDisposeFinally ();
	}
	else if (flags & ScopeFlag_Try && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead)))
	{
		m_currentScope->m_flags &= ~ScopeFlag_Try; // prevent recursion
		m_module->m_controlFlowMgr.closeTry ();
	}
	else if ((flags & ScopeFlag_Catch) && !(flags & ScopeFlag_FinallyAhead))
	{
		m_module->m_controlFlowMgr.closeCatch ();
	}
	else if (flags & ScopeFlag_Finally) 
	{
		m_module->m_controlFlowMgr.closeFinally ();
	}

	closeNamespace ();

	if ((flags & ScopeFlag_Nested) && !(flags & (ScopeFlag_CatchAhead | ScopeFlag_FinallyAhead)))
		closeScope ();
}

AccessKind
NamespaceMgr::getAccessKind (Namespace* targetNamespace)
{
	Namespace* nspace = m_currentNamespace;

	if (!targetNamespace->isNamed ())
	{
		for (; nspace; nspace = nspace->m_parentNamespace)
		{
			if (nspace == targetNamespace)
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	if (targetNamespace->m_namespaceKind != NamespaceKind_Type)
	{
		for (; nspace; nspace = nspace->m_parentNamespace)
		{
			if (!nspace->isNamed ())
				continue;

			if (nspace == targetNamespace ||
				targetNamespace->m_qualifiedName.cmp (nspace->m_qualifiedName) == 0 ||
				targetNamespace->m_friendSet.find (nspace->m_qualifiedName))
				return AccessKind_Protected;
		}

		return AccessKind_Public;
	}

	NamedType* targetType = (NamedType*) targetNamespace;

	for (; nspace; nspace = nspace->m_parentNamespace)
	{
		if (!nspace->isNamed ())
			continue;

		if (nspace == targetNamespace ||
			targetNamespace->m_qualifiedName.cmp (nspace->m_qualifiedName) == 0 ||
			targetNamespace->m_friendSet.find (nspace->m_qualifiedName))
			return AccessKind_Protected;

		if (nspace->m_namespaceKind == NamespaceKind_Type)
		{
			NamedType* type = (NamedType*) nspace;
			TypeKind typeKind = type->getTypeKind ();
			if (typeKind == TypeKind_Class || typeKind == TypeKind_Struct)
			{
				bool result = ((DerivableType*) type)->findBaseTypeTraverse (targetType);
				if (result)
					return AccessKind_Protected;
			}
		}
	}

	return AccessKind_Public;
}

GlobalNamespace*
NamespaceMgr::createGlobalNamespace (
	const sl::String& name,
	Namespace* parentNamespace
	)
{
	if (!parentNamespace)
		parentNamespace = &m_stdNamespaceArray [StdNamespace_Global];

	sl::String qualifiedName = parentNamespace->createQualifiedName (name);

	GlobalNamespace* nspace = AXL_MEM_NEW (GlobalNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = parentNamespace;
	m_globalNamespaceList.insertTail (nspace);
	return nspace;
}

ExtensionNamespace*
NamespaceMgr::createExtensionNamespace (
	const sl::String& name,
	DerivableType* type,
	Namespace* parentNamespace
	)
{
	if (!parentNamespace)
		parentNamespace = &m_stdNamespaceArray [StdNamespace_Global];

	sl::String qualifiedName = type->createQualifiedName (name);

	ExtensionNamespace* nspace = AXL_MEM_NEW (ExtensionNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = parentNamespace;
	nspace->m_type = type;
	m_extensionNamespaceList.insertTail (nspace);
	return nspace;
}

DynamicLibNamespace*
NamespaceMgr::createDynamicLibNamespace (ClassType* dynamicLibType)
{
	sl::String name = "lib";
	sl::String qualifiedName = dynamicLibType->getQualifiedName () + (utf32_t) '.' + name;

	DynamicLibNamespace* nspace = AXL_MEM_NEW (DynamicLibNamespace);
	nspace->m_module = m_module;
	nspace->m_name = name;
	nspace->m_qualifiedName = qualifiedName;
	nspace->m_tag = qualifiedName;
	nspace->m_parentNamespace = dynamicLibType;
	nspace->m_dynamicLibType = dynamicLibType;
	m_dynamicLibNamespaceList.insertTail (nspace);
	return nspace;
}

Scope*
NamespaceMgr::findBreakScope (size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
	{
		if (scope->m_breakBlock)
		{
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findContinueScope (size_t level)
{
	size_t i = 0;
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
	{
		if (scope->m_continueBlock)
		{
			i++;
			if (i >= level)
				break;
		}
	}

	return scope;
}

Scope*
NamespaceMgr::findCatchScope ()
{
	Scope* scope = m_currentScope;
	for (; scope; scope = scope->getParentScope ())
	{
		if (scope->m_flags & (ScopeFlag_CatchAhead | ScopeFlag_Try))
			break;
	}

	if (!scope || scope->m_catchBlock)
		return scope;

	scope->m_catchBlock = m_module->m_controlFlowMgr.createBlock ("catch_block");
	return scope;
}

//.............................................................................

} // namespace ct
} // namespace jnc
