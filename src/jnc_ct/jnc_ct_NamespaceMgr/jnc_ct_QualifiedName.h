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

#pragma once

namespace jnc {
namespace ct {

//..............................................................................

class QualifiedName
{
protected:
	sl::String m_first;
	sl::BoxList <sl::String> m_list;

public:
	QualifiedName ()
	{
	}

	QualifiedName (const sl::StringRef& name)
	{
		m_first = name;
	}

	QualifiedName (const QualifiedName& name)
	{
		copy (name);
	}

	QualifiedName&
	operator = (const QualifiedName& name)
	{
		copy (name);
		return *this;
	}

	void
	clear ()
	{
		m_first.clear ();
		m_list.clear ();
	}

	void
	parse (const sl::StringRef& name);

	void
	addName (const sl::StringRef& name);

	sl::String
	removeLastName ();

	bool
	isEmpty () const
	{
		return m_first.isEmpty ();
	}

	bool
	isSimple () const
	{
		return m_list.isEmpty ();
	}

	const sl::String&
	getFirstName () const
	{
		return m_first;
	}

	sl::ConstBoxList <sl::String>
	getNameList () const
	{
		return m_list;
	}

	const sl::String&
	getShortName () const
	{
		return !m_list.isEmpty () ? *m_list.getTail () : m_first;
	}

	sl::String
	getFullName () const;

	void
	copy (const QualifiedName& name);
};

//..............................................................................

} // namespace ct
} // namespace jnc
