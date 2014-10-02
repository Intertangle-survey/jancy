#include "pch.h"
#include "jnc_AttributeBlock.h"

namespace jnc {

//.............................................................................

Attribute*
AttributeBlock::createAttribute (
	const rtl::String& name,
	Value* value
	)
{
	rtl::StringHashTableMapIterator <Attribute*> it = m_attributeMap.visit (name);
	if (it->m_value)
	{
		err::setFormatStringError ("redefinition of attribute '%s'", name.cc ()); // thanks a lot gcc
		return NULL;
	}

	Attribute* attribute = AXL_MEM_NEW (Attribute);
	attribute->m_module = m_module;
	attribute->m_name = name;
	attribute->m_value = value;
	m_attributeList.insertTail (attribute);
	it->m_value = attribute;
	return attribute;
}

//.............................................................................

} // namespace jnc {
