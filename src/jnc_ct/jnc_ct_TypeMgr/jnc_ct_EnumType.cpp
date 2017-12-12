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
#include "jnc_ct_EnumType.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//..............................................................................

const char*
getEnumTypeFlagString (EnumTypeFlag flag)
{
	static const char* stringTable [] =
	{
		"exposed",   // EnumTypeFlag_Exposed = 0x0010000
		"bitflag",   // EnumTypeFlag_BitFlag = 0x0020000
	};

	size_t i = sl::getLoBitIdx32 (flag >> 12);

	return i < countof (stringTable) ?
		stringTable [i] :
		"undefined-enum-type-flag";
}

sl::String
getEnumTypeFlagString (uint_t flags)
{
	sl::String string;

	if (flags & EnumTypeFlag_Exposed)
		string = "exposed ";

	if (flags & EnumTypeFlag_BitFlag)
		string += "bitflag ";

	if (!string.isEmpty ())
		string.chop (1);

	return string;
}

//..............................................................................

bool
EnumConst::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock ();

	itemXml->format (
		"<enumvalue id='%s'>\n"
		"<name>%s</name>\n",
		doxyBlock->getRefId ().sz (),
		m_name.sz ()
		);

	if (!m_initializer.isEmpty ())
		itemXml->appendFormat (
			"<initializer>= %s</initializer>\n",
			getInitializerString ().sz ()
			);

	itemXml->append (doxyBlock->getDescriptionString ());
	itemXml->append ("</enumvalue>\n");

	return true;
}

//..............................................................................

EnumType::EnumType ()
{
	m_typeKind = TypeKind_Enum;
	m_flags = TypeFlag_Pod;
	m_baseType = NULL;
}

EnumConst*
EnumType::createConst (
	const sl::StringRef& name,
	sl::BoxList <Token>* initializer
	)
{
	EnumConst* enumConst = AXL_MEM_NEW (EnumConst);
	enumConst->m_module = m_module;
	enumConst->m_parentUnit = m_parentUnit;
	enumConst->m_parentEnumType = this;
	enumConst->m_name = name;
	enumConst->m_tag = m_tag.isEmpty () ? name : m_tag + "." + name;

	if (initializer)
		enumConst->m_initializer.takeOver (initializer);

	m_constList.insertTail (enumConst);
	m_constArray.append (enumConst);

	bool result = addItem (enumConst);
	if (!result)
		return NULL;

	return enumConst;
}

bool
EnumType::calcLayout ()
{
	bool result;

	if (!(m_baseType->getTypeKindFlags () & TypeKindFlag_Integer))
	{
		err::setFormatStringError ("enum base type must be integer type");
		return false;
	}

	m_size = m_baseType->getSize ();
	m_alignment = m_baseType->getAlignment ();

	// assign values to consts

	if (m_parentUnit)
		m_module->m_unitMgr.setCurrentUnit (m_parentUnit);

	m_module->m_namespaceMgr.openNamespace (this);

	if (m_flags & EnumTypeFlag_BitFlag)
	{
		int64_t value = 1;

		sl::Iterator <EnumConst> constIt = m_constList.getHead ();
		for (; constIt; constIt++)
		{
			if (!constIt->m_initializer.isEmpty ())
			{
				result = m_module->m_operatorMgr.parseConstIntegerExpression (constIt->m_initializer, &value);
				if (!result)
					return false;
			}

#if (JNC_PTR_SIZE == 4)
			if (value > 0xffffffff && m_baseType->getSize () < 8)
			{
				err::setFormatStringError ("enum const '%lld' is too big", value);
				return false;
			}
#endif

			constIt->m_value = value;
			constIt->m_flags |= EnumConstFlag_ValueReady;

			value = value ? 2 << sl::getHiBitIdx64 (value) : 1;
		}
	}
	else
	{
		int64_t value = 0;

		sl::Iterator <EnumConst> constIt = m_constList.getHead ();
		for (; constIt; constIt++, value++)
		{
			if (!constIt->m_initializer.isEmpty ())
			{
				result = m_module->m_operatorMgr.parseConstIntegerExpression (constIt->m_initializer, &value);
				if (!result)
					return false;
			}

#if (JNC_PTR_SIZE == 4)
			if (value > 0xffffffff && m_baseType->getSize () < 8)
			{
				err::setFormatStringError ("enum const '%lld' is too big", value);
				return false;
			}
#endif

			constIt->m_value = value;
			constIt->m_flags |= EnumConstFlag_ValueReady;
		}
	}

	m_module->m_namespaceMgr.closeNamespace ();

	return true;
}

bool
EnumType::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	DoxyBlock* doxyBlock = getDoxyBlock ();

	sl::String memberXml;
	bool result = Namespace::generateMemberDocumentation (outputDir, &memberXml, indexXml, false);
	if (!result)
		return false;

	itemXml->format (
		"<memberdef kind='enum' id='%s'"
		">\n<name>%s</name>\n",
		doxyBlock->getRefId ().sz (),
		m_name.sz ()
		);

	uint_t flags = m_flags;
	if (m_name.isEmpty ())
		flags &= ~EnumTypeFlag_Exposed; // unnamed enums imply 'exposed' anyway

	sl::String modifierString = getEnumTypeFlagString (flags);
	if (!modifierString.isEmpty ())
		itemXml->appendFormat ("<modifiers>%s</modifiers>\n", modifierString.sz ());

	itemXml->append (memberXml);

	sl::String footnoteXml = doxyBlock->getFootnoteString ();
	if (!footnoteXml.isEmpty ())
		itemXml->append (footnoteXml);

	itemXml->append (doxyBlock->getImportString ());
	itemXml->append (doxyBlock->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</memberdef>\n");

	return true;
}

//..............................................................................

} // namespace ct
} // namespace jnc
