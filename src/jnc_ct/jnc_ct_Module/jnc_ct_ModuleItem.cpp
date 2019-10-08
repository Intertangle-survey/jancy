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

#include "pch.h"
#include "jnc_ct_ModuleItem.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

ModuleItemDecl::ModuleItemDecl()
{
	m_storageKind = StorageKind_Undefined;
	m_accessKind = AccessKind_Public; // public by default
	m_parentNamespace = NULL;
	m_attributeBlock = NULL;
	m_doxyBlock = NULL;
}

void
ModuleItemDecl::prepareQualifiedName()
{
	ASSERT(m_qualifiedName.isEmpty());
	m_qualifiedName = m_parentNamespace ? m_parentNamespace->createQualifiedName(m_name) : m_name;
}

sl::String
ModuleItemDecl::getDoxyLocationString()
{
	if (!m_parentUnit)
		return sl::String();

	sl::String string;

	string.format("<location file='%s' line='%d' col='%d'/>\n",
		m_parentUnit->getFileName().sz(),
		m_pos.m_line + 1,
		m_pos.m_col + 1
		);

	return string;
}

//..............................................................................

bool
ModuleItemBodyDecl::setBody(
	const lex::LineCol& pos,
	const sl::StringRef& body
	)
{
	if (!canSetBody())
		return false;

	m_bodyPos = pos;
	m_body = body;
	return true;
}

bool
ModuleItemBodyDecl::setTokenList(sl::BoxList<Token>* tokenList)
{
	if (!canSetBody())
		return false;

	m_bodyPos = tokenList->getHead()->m_pos;
	sl::takeOver(&m_tokenList, tokenList);
	return true;
}

bool
ModuleItemBodyDecl::canSetBody()
{
	if (!m_body.isEmpty() || !m_tokenList.isEmpty())
	{
		err::setFormatStringError("'%s' already has a body", getQualifiedName().sz());
		return false;
	}

	if (m_storageKind == StorageKind_Abstract)
	{
		err::setFormatStringError("'%s' is abstract and hence cannot have a body", getQualifiedName().sz());
		return false;
	}

	return true;
}

//..............................................................................

ModuleItem::ModuleItem()
{
	m_module = NULL;
	m_itemKind = ModuleItemKind_Undefined;
	m_flags = 0;
}

ModuleItemDecl*
ModuleItem::getDecl()
{
	switch (m_itemKind)
	{
	case ModuleItemKind_Namespace:
		return (GlobalNamespace*)this;

	case ModuleItemKind_Scope:
		return (Scope*)this;

	case ModuleItemKind_Type:
		return (((Type*)this)->getTypeKindFlags() & TypeKindFlag_Named) ?
			(NamedType*)this :
			NULL;

	case ModuleItemKind_Typedef:
		return (Typedef*)this;

	case ModuleItemKind_Alias:
		return (Alias*)this;

	case ModuleItemKind_Const:
		return (Const*)this;

	case ModuleItemKind_Variable:
		return (Variable*)this;

	case ModuleItemKind_FunctionArg:
		return (FunctionArg*)this;

	case ModuleItemKind_Function:
		return (Function*)this;

	case ModuleItemKind_FunctionOverload:
		return (FunctionOverload*)this;

	case ModuleItemKind_Property:
		return (Property*)this;

	case ModuleItemKind_PropertyTemplate:
		return (PropertyTemplate*)this;

	case ModuleItemKind_EnumConst:
		return (EnumConst*)this;

	case ModuleItemKind_Field:
		return (Field*)this;

	case ModuleItemKind_BaseTypeSlot:
		return (BaseTypeSlot*)this;

	case ModuleItemKind_Orphan:
		return (Orphan*)this;

	default:
		return NULL;
	}
}

Namespace*
ModuleItem::getNamespace()
{
	switch (m_itemKind)
	{
	case ModuleItemKind_Namespace:
		return (GlobalNamespace*)this;

	case ModuleItemKind_Scope:
		return (Scope*)this;

	case ModuleItemKind_Typedef:
		return ((Typedef*)this)->getType()->getNamespace();

	case ModuleItemKind_Type:
		return (((Type*)this)->getTypeKindFlags() & TypeKindFlag_Named) ?
			(NamedType*)this :
			NULL;

	case ModuleItemKind_Property:
		return (Property*)this;

	case ModuleItemKind_PropertyTemplate:
		return (PropertyTemplate*)this;

	default:
		return NULL;
	}
}

Type*
ModuleItem::getType()
{
	using namespace jnc;

	switch (m_itemKind)
	{
	case ModuleItemKind_Type:
		return (ct::Type*)this;

	case ModuleItemKind_Typedef:
		return ((ct::Typedef*)this)->getType();

	case ModuleItemKind_Const:
		return ((ct::Const*)this)->getValue().getType();

	case ModuleItemKind_Variable:
		return ((ct::Variable*)this)->getType();

	case ModuleItemKind_FunctionArg:
		return ((ct::FunctionArg*)this)->getType();

	case ModuleItemKind_Function:
		return ((ct::Function*)this)->getType();

	case ModuleItemKind_Property:
		return ((ct::Property*)this)->getType();

	case ModuleItemKind_PropertyTemplate:
		return ((ct::PropertyTemplate*)this)->calcType();

	case ModuleItemKind_EnumConst:
		return ((ct::EnumConst*)this)->getParentEnumType();

	case ModuleItemKind_Field:
		return ((ct::Field*)this)->getType();

	case ModuleItemKind_BaseTypeSlot:
		return ((ct::BaseTypeSlot*)this)->getType();

	case ModuleItemKind_Orphan:
		return ((ct::Orphan*)this)->getFunctionType();

	case ModuleItemKind_Alias:
		return ((ct::Alias*)this)->getTargetItem()->getType();

	case ModuleItemKind_Lazy:
		return jnc_ModuleItem_getType(((ct::LazyModuleItem*)this)->getActualItem());

	default:
		return NULL;
	}
}

sl::String
ModuleItem::createDoxyRefId()
{
	sl::String refId = getModuleItemKindString(m_itemKind);
	refId.replace('-', '_');

	ModuleItemDecl* decl = getDecl();
	ASSERT(decl);

	sl::String name = decl->getQualifiedName();
	if (!name.isEmpty())
	{
		refId.appendFormat("_%s", name.sz());
		refId.replace('.', '_');
	}

	refId.makeLowerCase();

	return m_module->m_doxyModule.adjustRefId(refId);
}

//..............................................................................

} // namespace ct
} // namespace jnc
