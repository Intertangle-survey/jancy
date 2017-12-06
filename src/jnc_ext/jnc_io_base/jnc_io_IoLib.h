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
namespace io {

//..............................................................................

enum IoLibCacheSlot
{
	IoLibCacheSlot_Serial,
	IoLibCacheSlot_SerialPortDesc,
	IoLibCacheSlot_Address_ip4,
	IoLibCacheSlot_Address_ip6,
	IoLibCacheSlot_SocketAddress_ip4,
	IoLibCacheSlot_SocketAddress_ip6,
	IoLibCacheSlot_SocketAddress,
	IoLibCacheSlot_SocketAddressResolver,
	IoLibCacheSlot_SocketAddressResolverEventParams,
	IoLibCacheSlot_Socket,
	IoLibCacheSlot_SocketEventParams,
	IoLibCacheSlot_NetworkAdapterAddress,
	IoLibCacheSlot_NetworkAdapterDesc,
	IoLibCacheSlot_File,
	IoLibCacheSlot_MappedFile,
	IoLibCacheSlot_FileStream,
	IoLibCacheSlot_FileStreamEventParams,
	IoLibCacheSlot_NamedPipe,
	IoLibCacheSlot_Mailslot,
	IoLibCacheSlot_MailslotEventParams,
};

// {362FF8E2-1BDD-4319-AF8F-AD86C3917AC5}
JNC_DEFINE_GUID (
	g_ioLibGuid,
	0x362ff8e2, 0x1bdd, 0x4319, 0xaf, 0x8f, 0xad, 0x86, 0xc3, 0x91, 0x7a, 0xc5
	);

//..............................................................................

} // namespace io
} // namespace jnc
