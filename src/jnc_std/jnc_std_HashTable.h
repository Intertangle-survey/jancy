#pragma once

#include "jnc_ext_ExtensionLib.h"
#include "jnc_std_StdLibGlobals.h"
#include "jnc_rt_VariantUtils.h"

namespace jnc {
namespace std {

//.............................................................................

class StringHashTable: public rt::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (StringHashTable, &StringHashTable::markOpaqueGcRoots)

	JNC_BEGIN_CLASS_TYPE_MAP ("std.StringHashTable", g_stdLibCacheSlot, StdLibTypeCacheSlot_StringHashTable)
		JNC_MAP_CONSTRUCTOR (&sl::construct <StringHashTable>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <StringHashTable>)
		JNC_MAP_CONST_PROPERTY ("m_isEmpty",  &StringHashTable::isEmpty)
		JNC_MAP_FUNCTION ("clear",  &StringHashTable::clear)
		JNC_MAP_FUNCTION ("find", &StringHashTable::find)
		JNC_MAP_FUNCTION ("insert", &StringHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &StringHashTable::remove)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	struct Entry: sl::ListLink
	{
		rt::DataPtr m_keyPtr;
		rt::Variant m_value;
	};

	typedef sl::StringHashTableMap <Entry*> StringHashTableMap;

protected:
	sl::StdList <Entry> m_list;
	StringHashTableMap m_map;

public:
	void
	AXL_CDECL
	markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap);

	bool 
	AXL_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	AXL_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	AXL_CDECL
	find (
		rt::DataPtr keyPtr,
		rt::DataPtr valuePtr
		);

	void
	AXL_CDECL
	insert (
		rt::DataPtr keyPtr,
		rt::Variant value
		);

	bool
	AXL_CDECL
	remove (rt::DataPtr keyPtr);
};

//.............................................................................

class VariantHashTable: public rt::IfaceHdr
{
public:
	JNC_OPAQUE_CLASS_TYPE_INFO (VariantHashTable, &VariantHashTable::markOpaqueGcRoots)

	JNC_BEGIN_CLASS_TYPE_MAP ("std.VariantHashTable", g_stdLibCacheSlot, StdLibTypeCacheSlot_VariantHashTable)
		JNC_MAP_CONSTRUCTOR (&sl::construct <VariantHashTable>)
		JNC_MAP_DESTRUCTOR (&sl::destruct <VariantHashTable>)
		JNC_MAP_CONST_PROPERTY ("m_isEmpty",  &VariantHashTable::isEmpty)
		JNC_MAP_FUNCTION ("clear",  &VariantHashTable::clear)
		JNC_MAP_FUNCTION ("find", &VariantHashTable::find)
		JNC_MAP_FUNCTION ("insert", &VariantHashTable::insert)
		JNC_MAP_FUNCTION ("remove", &VariantHashTable::remove)
	JNC_END_CLASS_TYPE_MAP ()

protected:
	struct Entry: sl::ListLink
	{
		rt::Variant m_key;
		rt::Variant m_value;
	};

	typedef sl::HashTableMap <rt::Variant, Entry*, rt::HashVariant, rt::CmpVariant> VariantHashTableMap;

protected:
	sl::StdList <Entry> m_list;
	VariantHashTableMap m_map;

public:
	void
	AXL_CDECL
	markOpaqueGcRoots (jnc::rt::GcHeap* gcHeap);

	bool
	AXL_CDECL
	isEmpty ()
	{
		return m_list.isEmpty ();
	}

	void
	AXL_CDECL
	clear ()
	{
		m_list.clear ();
		m_map.clear ();
	}

	bool
	AXL_CDECL
	find (
		rt::Variant key,
		rt::DataPtr valuePtr
		);

	void
	AXL_CDECL
	insert (
		rt::Variant key,
		rt::Variant value
		);

	bool
	AXL_CDECL
	remove (rt::Variant key);
};

//.............................................................................

} // namespace std
} // namespace jnc
