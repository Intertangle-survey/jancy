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
#include "jnc_ct_NamedTypeBlock.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

NamedTypeBlock::NamedTypeBlock(ModuleItem* parent)
{
	m_parent = parent;
	m_staticConstructor = NULL;
	m_staticDestructor = NULL;
	m_preconstructor = NULL;
	m_constructor = NULL;
	m_destructor = NULL;
}

Namespace*
NamedTypeBlock::getParentNamespaceImpl()
{
	ASSERT(
		m_parent->getItemKind() == ModuleItemKind_Property ||
		m_parent->getItemKind() == ModuleItemKind_Type &&
		(((Type*)m_parent)->getTypeKindFlags() & TypeKindFlag_Derivable));

	return  m_parent->getItemKind() == ModuleItemKind_Property ?
		(Namespace*)(Property*)m_parent :
		(Namespace*)(DerivableType*)m_parent;
}

Unit*
NamedTypeBlock::getParentUnitImpl()
{
	ASSERT(
		m_parent->getItemKind() == ModuleItemKind_Property ||
		m_parent->getItemKind() == ModuleItemKind_Type &&
		(((Type*)m_parent)->getTypeKindFlags() & TypeKindFlag_Derivable));

	return  m_parent->getItemKind() == ModuleItemKind_Property ?
		((Property*)m_parent)->getParentUnit() :
		((DerivableType*)m_parent)->getParentUnit();
}

Function*
NamedTypeBlock::createMethod(
	StorageKind storageKind,
	const sl::StringRef& name,
	FunctionType* shortType
	)
{
	sl::String qualifedName = getParentNamespaceImpl()->createQualifiedName(name);

	Function* function = m_parent->getModule()->m_functionMgr.createFunction(
		FunctionKind_Normal,
		name,
		qualifedName,
		shortType
		);

	function->m_storageKind = storageKind;
	function->m_declaratorName = name;

	bool result = addMethod(function);
	if (!result)
		return NULL;

	return function;
}

Function*
NamedTypeBlock::createUnnamedMethod(
	StorageKind storageKind,
	FunctionKind functionKind,
	FunctionType* shortType
	)
{
	sl::String name = getFunctionKindString(functionKind);
	sl::String qualifiedName = sl::formatString(
		"%s.%s",
		getParentNamespaceImpl()->getQualifiedName().sz(),
		name.sz()
		);

	Function* function = m_parent->getModule()->m_functionMgr.createFunction(
		functionKind,
		name,
		qualifiedName,
		shortType
		);

	function->m_storageKind = storageKind;

	bool result = addMethod(function);
	if (!result)
		return NULL;

	return function;
}

Property*
NamedTypeBlock::createProperty(
	StorageKind storageKind,
	const sl::StringRef& name,
	PropertyType* shortType
	)
{
	sl::String qualifiedName = getParentNamespaceImpl()->createQualifiedName(name);

	Property* prop = m_parent->getModule()->m_functionMgr.createProperty(name, qualifiedName);

	bool result =
		addProperty(prop) &&
		prop->create(shortType);

	if (!result)
		return NULL;

	return prop;
}

bool
NamedTypeBlock::initializeStaticFields()
{
	bool result;

	Module* module = m_parent->getModule();

	Unit* unit = getParentUnitImpl();
	if (unit)
		module->m_unitMgr.setCurrentUnit(unit);

	module->m_namespaceMgr.openNamespace(getParentNamespaceImpl());

	size_t count = m_initializedStaticFieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Variable* staticField = m_initializedStaticFieldArray[i];
		result = module->m_variableMgr.initializeVariable(staticField);
		if (!result)
			return false;
	}

	module->m_namespaceMgr.closeNamespace();

	return true;
}

bool
NamedTypeBlock::initializeMemberFields(const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule();

	Unit* unit = getParentUnitImpl();
	if (unit)
		module->m_unitMgr.setCurrentUnit(unit);

	module->m_namespaceMgr.openNamespace(getParentNamespaceImpl());

	size_t count = m_initializedMemberFieldArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_initializedMemberFieldArray[i];

		Value fieldValue;
		result = module->m_operatorMgr.getField(thisValue, field, NULL, &fieldValue);
		if (!result)
			return false;

		result = module->m_operatorMgr.parseInitializer(
			fieldValue,
			field->m_constructor,
			field->m_initializer
			);

		if (!result)
			return false;
	}

	module->m_namespaceMgr.closeNamespace();

	return true;
}

bool
NamedTypeBlock::callMemberFieldConstructors(const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_memberFieldConstructArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		StructField* field = m_memberFieldConstructArray[i];
		if (field->m_flags & ModuleItemFlag_Constructed)
		{
			field->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Value fieldValue;
		result = module->m_operatorMgr.getField(thisValue, field, NULL, &fieldValue);
		if (!result)
			return false;

		ASSERT(field->getType()->getTypeKindFlags() & TypeKindFlag_Derivable);
		DerivableType* type = (DerivableType*)field->getType();

		Function* constructor = type->getDefaultConstructor();
		ASSERT(constructor); // otherwise, would not be on member-field-construct array

		result = module->m_operatorMgr.callOperator(constructor, fieldValue);
		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::callMemberPropertyConstructors(const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_memberPropertyConstructArray.getCount();
	for (size_t i = 0; i < count; i++)
	{
		Property* prop = m_memberPropertyConstructArray[i];
		if (prop->m_flags & ModuleItemFlag_Constructed)
		{
			prop->m_flags &= ~ModuleItemFlag_Constructed;
			continue;
		}

		Function* constructor = prop->getConstructor();
		ASSERT(constructor);

		result = module->m_operatorMgr.callOperator(constructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

bool
NamedTypeBlock::callMemberPropertyDestructors(const Value& thisValue)
{
	bool result;

	Module* module = m_parent->getModule();

	size_t count = m_memberPropertyDestructArray.getCount();
	for (intptr_t i = count - 1; i >= 0; i--)
	{
		Property* prop = m_memberPropertyDestructArray[i];

		Function* destructor = prop->getDestructor();
		ASSERT(destructor);

		result = module->m_operatorMgr.callOperator(destructor, thisValue);
		if (!result)
			return false;
	}

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
