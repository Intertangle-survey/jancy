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

class UsbInterface;

JNC_DECLARE_OPAQUE_CLASS_TYPE (UsbDevice)

//..............................................................................

class UsbDevice: public IfaceHdr
{
	friend class UsbInterface;

public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS (UsbDevice)

protected:
	bool m_isOpen;
	bool m_isAutoDetachKernelDriverEnabled;

protected:
	axl::io::UsbDevice m_device;

public:
	UsbDevice ()
	{
		m_isOpen = false;
		m_isAutoDetachKernelDriverEnabled = false;
	}

	~UsbDevice ()
	{
		close ();
	}

	axl::io::UsbDevice*
	getDevice ()
	{
		return &m_device;
	}

	void
	setDevice (libusb_device* srcDevice)
	{
		m_device.setDevice (srcDevice);
	}

	void
	takeOver (axl::io::UsbDevice* srcDevice)
	{
		m_isOpen = srcDevice->isOpen ();
		m_device.takeOver (srcDevice);
	}

	void
	JNC_CDECL
	close ()
	{
		m_device.close ();
		m_isOpen = false;
	}

	bool
	JNC_CDECL
	open ();

	static
	DataPtr
	JNC_CDECL
	getDeviceDesc (UsbDevice* self);

	static
	DataPtr
	JNC_CDECL
	getActiveConfigurationDesc (UsbDevice* self);

	uint8_t
	JNC_CDECL
	getConfigurationId ()
	{
		return m_isOpen ? m_device.getConfiguration () : 0;
	}

	bool
	JNC_CDECL
	setConfigurationId (uint8_t configurationId)
	{
		return m_isOpen ? m_device.setConfiguration (configurationId) : false;
	}

	uint8_t
	JNC_CDECL
	getBus ()
	{
		return m_device.getBusNumber ();
	}

	uint8_t
	JNC_CDECL
	getAddress ()
	{
		return m_device.getDeviceAddress ();
	}

	uint8_t
	JNC_CDECL
	getSpeed ()
	{
		return m_device.getDeviceSpeed ();
	}

	bool
	JNC_CDECL
	setAutoDetachKernelDriverEnabled (bool isEnabled);

	bool
	JNC_CDECL
	isKernelDriverActive (uint_t interfaceId)
	{
		return m_device.isKernelDriverActive (interfaceId);
	}

	bool
	JNC_CDECL
	attachKernelDriver (uint_t interfaceId);

	bool
	JNC_CDECL
	detachKernelDriver (uint_t interfaceId);

	UsbInterface*
	JNC_CDECL
	claimInterface (
		uint8_t interfaceId,
		uint8_t altSettingId
		);

	static
	DataPtr
	JNC_CDECL
	getStringDesc (
		UsbDevice* self,
		uint8_t stringId
		);
};

//..............................................................................

DataPtr
createUsbDeviceArray (DataPtr countPtr);

UsbDevice*
openUsbDevice (
	uint_t vendorId,
	uint_t productId
	);

//..............................................................................

} // namespace io
} // namespace jnc
