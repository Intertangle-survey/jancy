#include "pch.h"
#include "jnc_Orphan.h"
#include "jnc_Module.h"

namespace jnc {

//.............................................................................

Orphan::Orphan ()
{
	m_itemKind = ModuleItemKind_Orphan;
	m_orphanKind = OrphanKind_Undefined;
	m_functionType = NULL;
}

bool
Orphan::setBody (rtl::BoxList <Token>* tokenList)
{
	if (!m_body.isEmpty ())
	{
		err::setFormatStringError ("'%s' already has a body", m_tag.cc ());
		return false;
	}

	m_body.takeOver (tokenList);
	return true;
}

bool
Orphan::resolveOrphan ()
{
	ModuleItem* item = m_parentNamespace->findItemTraverse (m_declaratorName);
	if (!item)
	{
		err::setFormatStringError ("unresolved orphan '%s'", m_tag.cc ()); // thanks a lot gcc
		return false;
	}

	switch (m_orphanKind)
	{
	case OrphanKind_Function:
		return adoptOrphanFunction (item);

	case OrphanKind_Reactor:
		return adoptOrphanReactor (item);

	default:
		ASSERT (false);
		return true;
	}
}

Function*
Orphan::getItemUnnamedMethod (ModuleItem* item)
{
	if (item->getItemKind () == ModuleItemKind_Property)
	{
		Property* prop = (Property*) item;
		switch (m_functionKind)
		{
		case FunctionKind_Constructor:
			return prop->getConstructor ();

		case FunctionKind_StaticConstructor:
			return prop->getStaticConstructor ();

		case FunctionKind_Destructor:
			return prop->getDestructor ();

		case FunctionKind_Getter:
			return prop->getGetter ();

		case FunctionKind_Setter:
			return prop->getSetter ();
		}
	}
	else if (
		item->getItemKind () == ModuleItemKind_Type &&
		(((Type*) item)->getTypeKindFlags () & TypeKindFlagKind_Derivable))
	{
		DerivableType* type = (DerivableType*) item;
		switch (m_functionKind)
		{
		case FunctionKind_PreConstructor:
			return type->getPreConstructor ();

		case FunctionKind_Constructor:
			return type->getConstructor ();

		case FunctionKind_StaticConstructor:
			return type->getStaticConstructor ();

		case FunctionKind_Destructor:
			return type->getTypeKind () == TypeKind_Class ? ((ClassType*) type)->getDestructor () : NULL;

		case FunctionKind_UnaryOperator:
			return type->getUnaryOperator (m_unOpKind);

		case FunctionKind_BinaryOperator:
			return type->getBinaryOperator (m_binOpKind);

		case FunctionKind_CallOperator:
			return type->getCallOperator ();
		}
	}

	return NULL;
}

bool
Orphan::adoptOrphanFunction (ModuleItem* item)
{
	Function* originFunction = NULL;

	ModuleItemKind itemKind = item->getItemKind ();

	if (m_functionKind == FunctionKind_Named)
	{
		if (itemKind != ModuleItemKind_Function)
		{
			err::setFormatStringError ("'%s' is not a function", m_tag.cc ());
			return false;
		}

		originFunction = (Function*) item;
	}
	else
	{
		originFunction = getItemUnnamedMethod (item);
		if (!originFunction)
		{
			err::setFormatStringError ("'%s' has no '%s'", item->m_tag.cc (), getFunctionKindString (m_functionKind));
			return false;
		}
	}

	bool result =
		m_functionType->ensureLayout () &&
		originFunction->getTypeOverload ()->ensureLayout ();

	if (!result)
		return false;

	originFunction = originFunction->findShortOverload (m_functionType);
	if (!originFunction)
	{
		err::setFormatStringError ("'%s': overload not found", m_tag.cc ());
		return false;
	}

	if (!(originFunction->m_flags & ModuleItemFlagKind_User))
	{
		err::setFormatStringError ("'%s' is a compiler-generated function", m_tag.cc ());
		return false;
	}

	ASSERT (originFunction->m_functionKind == m_functionKind);

	copySrcPos (originFunction);

	return
		copyArgNames (originFunction->getType ()) &&
		originFunction->setBody (&m_body) &&
		verifyStorageKind (originFunction);
}

bool
Orphan::adoptOrphanReactor (ModuleItem* item)
{
	Type* itemType = NULL;

	ModuleItemKind itemKind = item->getItemKind ();
	switch (itemKind)
	{
	case ModuleItemKind_Variable:
		itemType = ((Variable*) item)->getType ();
		break;

	case ModuleItemKind_StructField:
		itemType = ((StructField*) item)->getType ();
		break;
	}

	if (!itemType || !isClassType (itemType, ClassTypeKind_Reactor))
	{
		err::setFormatStringError ("'%s' is not a reactor", m_tag.cc ());
		return false;
	}

	ReactorClassType* originType = (ReactorClassType*) itemType ;
	Function* originStart = originType->getMethod (ReactorMethodKind_Start);

	copySrcPos (originType);
	copySrcPos (originStart);

	return
		copyArgNames (originStart->getType ()) &&
		originType->setBody (&m_body) &&
		verifyStorageKind (originStart);
}

bool
Orphan::copyArgNames (FunctionType* targetFunctionType)
{
	// copy arg names and make sure orphan funciton does not override default values

	rtl::Array <FunctionArg*> dstArgArray = targetFunctionType->getArgArray ();
	rtl::Array <FunctionArg*> srcArgArray = m_functionType->getArgArray ();

	size_t argCount = dstArgArray.getCount ();

	size_t iDst = 0;
	size_t iSrc = 0;

	if (targetFunctionType->isMemberMethodType ())
		iDst++;

	for (; iDst < argCount; iDst++, iSrc++)
	{
		FunctionArg* dstArg = dstArgArray [iDst];
		FunctionArg* srcArg = srcArgArray [iSrc];

		if (!srcArg->m_initializer.isEmpty ())
		{
			err::setFormatStringError ("redefinition of default value for '%s'", srcArg->m_name.cc ());
			return false;
		}

		dstArg->m_name = srcArg->m_name;
		dstArg->m_qualifiedName = srcArg->m_qualifiedName;
		dstArg->m_tag = srcArg->m_tag;
	}

	return true;
}

bool
Orphan::verifyStorageKind (ModuleItemDecl* targetDecl)
{
	if (!m_storageKind || m_storageKind == targetDecl->getStorageKind ())
		return true;

	err::setFormatStringError ("storage specifier mismatch for orphan '%s'", m_tag.cc ());
	return false;
}

void
Orphan::copySrcPos (ModuleItemDecl* targetDecl)
{
	targetDecl->m_parentUnit = m_parentUnit;
	targetDecl->m_pos = m_pos;
}

//.............................................................................

} // namespace jnc {
