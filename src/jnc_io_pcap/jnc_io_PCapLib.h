#pragma once

namespace jnc {
namespace io {

//.............................................................................

// {72C7158B-F297-4F88-83A7-96E7FB548B29}
JNC_DEFINE_GUID (
	g_pcapLibGuid,
	0x72c7158b, 0xf297, 0x4f88, 0x83, 0xa7, 0x96, 0xe7, 0xfb, 0x54, 0x8b, 0x29
	);

enum PCapLibCacheSlot
{	
	PCapLibCacheSlot_PCap,
	PCapLibCacheSlot_PCapEventParams,
	PCapLibCacheSlot_PCapAddress,
	PCapLibCacheSlot_PCapDeviceDesc,
};

//.............................................................................

} // namespace io
} // namespace jnc
