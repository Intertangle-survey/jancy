#include "pch.h"
#include "jnc_ct_BitFieldType.h"

namespace jnc {
namespace ct {

//.............................................................................

BitFieldType::BitFieldType ()
{
	m_typeKind = TypeKind_BitField;
	m_flags = TypeFlag_Pod;
	m_baseType = NULL;
	m_bitOffset = 0;
	m_bitCount = 0;
}

void
BitFieldType::prepareTypeString ()
{
	m_typeString.format (
		"%s:%d:%d",
		m_baseType->getTypeString ().cc (),
		m_bitOffset,
		m_bitOffset + m_bitCount
		);
}

bool
BitFieldType::calcLayout ()
{
	TypeKind typeKind = m_baseType->getTypeKind ();
	if (typeKind < TypeKind_Int8 || typeKind > TypeKind_Int64_beu)
	{
		err::setFormatStringError ("bit field can only be used with integer types");
		return false;
	}

	m_size = m_baseType->getSize ();
	m_alignment = m_baseType->getAlignment ();
	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc

