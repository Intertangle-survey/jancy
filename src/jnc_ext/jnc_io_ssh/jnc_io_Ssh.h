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

#include "jnc_io_AsyncIoDevice.h"
#include "jnc_io_SocketBase.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE (SshChannel)

//..............................................................................

enum SshEvent
{
	SshEvent_TcpDisconnected          = 0x0010,
	SshEvent_TcpConnectCompleted      = 0x0020,
	SshEvent_SshHandshakeCompleted    = 0x0040,
	SshEvent_SshAuthenticateError     = 0x0080,
	SshEvent_SshAuthenticateCompleted = 0x0100,
	SshEvent_SshChannelOpened         = 0x0200,
	SshEvent_SshPtyRequestCompleted   = 0x0400,
	SshEvent_SshConnectCompleted      = 0x0800,

	SshEvent_FullyConnected =
		SshEvent_TcpConnectCompleted |
		SshEvent_SshHandshakeCompleted |
		SshEvent_SshAuthenticateCompleted |
		SshEvent_SshChannelOpened |
		SshEvent_SshPtyRequestCompleted |
		SshEvent_SshConnectCompleted,
};

//..............................................................................

class FreeLibSsh2Session
{
public:
	void
	operator () (LIBSSH2_SESSION* session)
	{
		if (session)
			libssh2_session_free (session);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class FreeLibSsh2Channel
{
public:
	void
	operator () (LIBSSH2_CHANNEL* channel)
	{
		if (channel)
			libssh2_channel_free (channel);
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

typedef sl::Handle <LIBSSH2_SESSION*, FreeLibSsh2Session> SshSessionHandle;
typedef sl::Handle <LIBSSH2_CHANNEL*, FreeLibSsh2Channel> SshChannelHandle;

//..............................................................................

struct SshChannelHdr: IfaceHdr
{
	size_t m_readBlockSize;
	size_t m_readBufferSize;
	size_t m_writeBufferSize;
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class SshChannel:
	public SshChannelHdr,
	public SocketBase
{
	friend class IoThread;

protected:
	enum Def
	{
		Def_ReadBlockSize   = 4 * 1024,
		Def_ReadBufferSize  = 16 * 1024,
		Def_WriteBufferSize = 16 * 1024,
		Def_Options         = 0,
	};

	enum IoFlag
	{
		IoFlag_ResizePty = 0x0010,
	};

	class IoThread: public sys::ThreadImpl <IoThread>
	{
	public:
		void
		threadFunc ()
		{
			containerof (this, SshChannel, m_ioThread)->ioThreadFunc ();
		}
	};

	struct ConnectParams
	{
		sl::String m_userName;
		sl::Array <char> m_privateKey;
		sl::String m_password; // or private key passphrase
		sl::String m_channelType;
		sl::String m_processType;
		sl::String m_ptyType;
	};

protected:
	SshSessionHandle m_sshSession;
	SshChannelHandle m_sshChannel;

	jnc::io::SocketAddress m_localAddress;
	jnc::io::SocketAddress m_remoteAddress;
	volatile uint_t m_ptyWidth;
	volatile uint_t m_ptyHeight;

	ConnectParams* m_connectParams;
	IoThread m_ioThread;

public:
	SshChannel ();

	~SshChannel ()
	{
		close ();
	}

	void
	JNC_CDECL
	markOpaqueGcRoots (jnc::GcHeap* gcHeap)
	{
		AsyncIoDevice::markOpaqueGcRoots (gcHeap);
	}

	static
	SocketAddress
	JNC_CDECL
	getAddress (SshChannel* self)
	{
		return self->SocketBase::getAddress ();
	}

	static
	SocketAddress
	JNC_CDECL
	getPeerAddress (SshChannel* self)
	{
		return self->SocketBase::getPeerAddress ();
	}

	bool
	JNC_CDECL
	setReadBlockSize (size_t size)
	{
		return AsyncIoDevice::setReadBlockSize (&m_readBlockSize, size ? size : Def_ReadBlockSize);
	}

	bool
	JNC_CDECL
	setReadBufferSize (size_t size)
	{
		return AsyncIoDevice::setReadBufferSize (&m_readBufferSize, size ? size : Def_ReadBufferSize);
	}

	bool
	JNC_CDECL
	setWriteBufferSize (size_t size)
	{
		return AsyncIoDevice::setWriteBufferSize (&m_writeBufferSize, size ? size : Def_WriteBufferSize);
	}

	bool
	JNC_CDECL
	setOptions (uint_t flags)
	{
		return SocketBase::setOptions (flags);
	}


	bool
	JNC_CDECL
	open_0 (uint16_t family);

	bool
	JNC_CDECL
	open_1 (DataPtr addressPtr);

	void
	JNC_CDECL
	close ();

	bool
	JNC_CDECL
	connect (
		DataPtr addressPtr,
		DataPtr userNamePtr,
		DataPtr privateKeyPtr,
		size_t privateKeySize,
		DataPtr passwordPtr,
		DataPtr channelTypePtr,
		DataPtr processTypePtr,
		DataPtr ptyTypePtr,
		uint_t ptyWidth,
		uint_t ptyHeight
		);

	bool
	JNC_CDECL
	authenticate (
		DataPtr userNamePtr,
		DataPtr privateKeyPtr,
		size_t privateKeySize,
		DataPtr passwordPtr
		);

	bool
	JNC_CDECL
	resizePty (
		uint_t ptyWidth,
		uint_t ptyHeight
		);

	size_t
	JNC_CDECL
	read (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedRead (ptr, size);
	}

	size_t
	JNC_CDECL
	write (
		DataPtr ptr,
		size_t size
		)
	{
		return bufferedWrite (ptr, size);
	}

	handle_t
	JNC_CDECL
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		)
	{
		return AsyncIoDevice::wait (eventMask, handlerPtr);
	}

	bool
	JNC_CDECL
	cancelWait (handle_t handle)
	{
		return AsyncIoDevice::cancelWait (handle);
	}

	uint_t
	JNC_CDECL
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		)
	{
		return AsyncIoDevice::blockingWait (eventMask, timeout);
	}

protected:
	void
	ioThreadFunc ();

	err::Error
	getLastSshError ();

	int
	sshAsyncLoop (int result);

	bool
	sshConnect ();

	void
	sshReadWriteLoop ();
};

//..............................................................................

} // namespace io
} // namespace jnc
