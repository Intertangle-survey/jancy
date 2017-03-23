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

#include "jnc_std_Map.h"

namespace jnc {
namespace std {

JNC_DECLARE_OPAQUE_CLASS_TYPE (HashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (StringHashTable)
JNC_DECLARE_OPAQUE_CLASS_TYPE (VariantHashTable)

//..............................................................................

typedef
size_t
HashFunc (Variant key);

typedef
bool
IsEqualFunc (
	Variant key1,
	Variant key2
	);

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HashIndirect
{
protected:
	HashFunc* m_func;

public:
	HashIndirect (HashFunc* func = NULL)
	{
		m_func = func;
	}

	size_t
	operator () (const Variant& key) const
	{
		ASSERT (m_func);
		return m_func (key);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class IsEqualIndirect
{
protected:
	IsEqualFunc* m_func;

public:
	IsEqualIndirect (IsEqualFunc* func = NULL)
	{
		m_func = func;
	}

	bool
	operator () (
		const Variant& key1,
		const Variant& key2
		) const
	{
		ASSERT (m_func);
		return m_func (key1, key2);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class HashTable: public IfaceHdr
{
public:
	Map m_map;

protected:
	sl::HashTable <Variant, DataPtr, HashIndirect, IsEqualIndirect> m_hashTable;

public:
	HashTable (
		HashFunc* hashFunc,
		IsEqualFunc* isEqualFunc
		):
		m_hashTable (HashIndirect (hashFunc), IsEqualIndirect (isEqualFunc))
	{
	}

	void
	JNC_CDECL
	clear ()
	{
		m_map.clear ();
		m_hashTable.clear ();
	}

	static
	DataPtr
	visit (
		HashTable* self,
		Variant key
		)
	{
		return self->visitImpl (key);
	}

	static
	DataPtr
	find (
		HashTable* self,
		Variant key
		)
	{
		return self->m_hashTable.findValue (key, g_nullPtr);
	}

	void
	JNC_CDECL
	remove (DataPtr entryPtr);

protected:
	DataPtr
	visitImpl (Variant key);
};

//..............................................................................

class StringHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		DataPtr m_keyPtr;
		Variant m_value;
	};

	typedef sl::StringHashTable <Entry*> Map;

protected:
	sl::StdList <Entry> m_list;
	Map m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		DataPtr keyPtr,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		DataPtr keyPtr,
		Variant value
		);

	bool
	JNC_CDECL
	remove (DataPtr keyPtr);
};

//..............................................................................

class VariantHashTable: public IfaceHdr
{
protected:
	struct Entry: sl::ListLink
	{
		Variant m_key;
		Variant m_value;
	};

	typedef sl::DuckTypeHashTable <Variant, Entry*> Map;

protected:
	sl::StdList <Entry> m_list;
	Map m_map;

public:
	void
	JNC_CDECL
	markOpaqueGcRoots (GcHeap* gcHeap);

	bool
	JNC_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	JNC_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	JNC_CDECL
	find (
		Variant key,
		DataPtr valuePtr
		);

	void
	JNC_CDECL
	insert (
		Variant key,
		Variant value
		);

	bool
	JNC_CDECL
	remove (Variant key);
};

//..............................................................................

} // namespace std
} // namespace jnc
