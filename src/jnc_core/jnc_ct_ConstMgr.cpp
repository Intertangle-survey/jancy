#include "pch.h"
#include "jnc_ct_ConstMgr.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

ConstMgr::ConstMgr ()
{
	m_module = Module::getCurrentConstructedModule ();
	ASSERT (m_module);
}

void
ConstMgr::clear ()
{
	m_valueList.clear ();
	m_constList.clear ();
	m_constDataPtrValidatorList.clear ();
}

Const*
ConstMgr::createConst (
	const sl::String& name,
	const sl::String& qualifiedName,
	const Value& value
	)
{
	Const* cnst = AXL_MEM_NEW (Const);
	cnst->m_name = name;
	cnst->m_qualifiedName = qualifiedName;
	cnst->m_tag = qualifiedName;
	cnst->m_value = value;
	m_constList.insertTail (cnst);

	return cnst;
}

const Value& 
ConstMgr::saveLiteral (
	const char* p,
	size_t length
	)
{
	if (length == -1)
		length = strlen (p);

	Value value;
	value.setCharArray (p, length + 1, m_module);
	return saveValue (value);
}

rt::DataPtrValidator*
ConstMgr::createConstDataPtrValidator (
	const void* p,
	Type* type
	)
{
	ConstDataPtrValidatorEntry* entry = AXL_MEM_NEW (ConstDataPtrValidatorEntry);
	entry->m_box.m_flags = rt::BoxFlag_StaticData | rt::BoxFlag_DataMark | rt::BoxFlag_WeakMark;
	entry->m_box.m_type = type;
	entry->m_validator.m_validatorBox = &entry->m_box;
	entry->m_validator.m_targetBox = &entry->m_box;
	entry->m_validator.m_rangeBegin = p;
	entry->m_validator.m_rangeEnd = (char*) p + type->getSize ();
	m_constDataPtrValidatorList.insertTail (entry);

	return &entry->m_validator;
}

//.............................................................................

} // namespace ct
} // namespace jnc
