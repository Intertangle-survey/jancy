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

enum AsyncIoEvent
{
	AsyncIoEvent_IoError             = 0x0001,
	AsyncIoEvent_IncomingData        = 0x0002,
	AsyncIoEvent_ReceiveBufferFull   = 0x0004,
	AsyncIoEvent_TransmitBufferReady = 0x0008,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AsyncIoOption
{
	AsyncIoOption_KeepReadBlockSize  = 0x01,
	AsyncIoOption_KeepWriteBlockSize = 0x02,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class AsyncIoDevice
{
protected:
	enum IoThreadFlag
	{
		IoThreadFlag_Closing  = 0x0001,
		IoThreadFlag_Datagram = 0x0002,
	};

	struct Wait: sl::ListLink
	{
		uint_t m_mask;
	};

	struct SyncWait: Wait
	{
		sys::Event* m_event;
	};

	struct AsyncWait: Wait
	{
		FunctionPtr m_handlerPtr;
		handle_t m_handle;
	};

#if (_AXL_OS_WIN)
	struct OverlappedRead: sl::ListLink
	{
		axl::io::win::StdOverlapped m_overlapped;
		sl::Array <char> m_buffer;
		size_t m_paramSize;
	};
#endif

	struct ReadWriteMeta: sl::ListLink
	{
		size_t m_dataSize;
		size_t m_paramSize;
	};

public:
	uint_t m_options;
	uint_t m_activeEvents;
	DataPtr m_ioErrorPtr;

	bool m_isOpen;

protected:
	Runtime* m_runtime;
	sys::Lock m_lock;
	volatile uint_t m_ioThreadFlags;
	sl::AuxList <SyncWait> m_syncWaitList;
	sl::StdList <AsyncWait> m_asyncWaitList;
	sl::StdList <AsyncWait> m_pendingAsyncWaitList;
	sl::HandleTable <AsyncWait*> m_asyncWaitMap;

#if (_JNC_OS_WIN)
	sys::Event m_ioThreadEvent;
#else
	axl::io::psx::Pipe m_ioThreadSelfPipe; // for self-pipe trick
#endif

	// buffers are not always used, but we get a cleaner impl this way

	sl::CircularBuffer m_readBuffer;
	sl::CircularBuffer m_writeBuffer;
	sl::Array <char> m_readOverflowBuffer;

	sl::StdList <ReadWriteMeta> m_readMetaList;
	sl::StdList <ReadWriteMeta> m_writeMetaList;
	sl::StdList <ReadWriteMeta> m_freeReadWriteMetaList;

#if (_AXL_OS_WIN)
	sl::StdList <OverlappedRead> m_activeOverlappedReadList;
	sl::StdList <OverlappedRead> m_freeOverlappedReadList;
	sl::Array <char> m_overlappedWriteBlock;
#endif

public:
	AsyncIoDevice ();

protected:
	void
	markOpaqueGcRoots (jnc::GcHeap* gcHeap);

	handle_t
	wait (
		uint_t eventMask,
		FunctionPtr handlerPtr
		);

	bool
	cancelWait (handle_t handle);

	uint_t
	blockingWait (
		uint_t eventMask,
		uint_t timeout
		);

	void
	open ();

	void
	close ();

	void
	wakeIoThread ();

	void
	sleepIoThread ();

	bool
	setReadParallelism (
		uint_t* p,
		uint_t count
		);

	bool
	setReadBlockSize (
		size_t* p,
		size_t size
		);

	bool
	setReadBufferSize (
		size_t* p,
		size_t size
		);

	bool
	setWriteBufferSize (
		size_t* p,
		size_t size
		);

	void
	cancelAllWaits ();

	size_t
	processWaitLists_l ();

	void
	setEvents_l (uint_t events);

	void
	setEvents (uint_t events)
	{
		m_lock.lock ();
		setEvents_l (events);
	}

	void
	setErrorEvent_l (
		uint_t event,
		const err::Error& error
		)
	{
		m_lock.unlock ();
		setErrorEvent (event, error);
	}

	void
	setIoErrorEvent_l (const err::Error& error)
	{
		setErrorEvent_l (AsyncIoEvent_IoError, error);
	}

	void
	setIoErrorEvent_l ()
	{
		setErrorEvent_l (AsyncIoEvent_IoError, err::getLastError ());
	}

	void
	setErrorEvent (
		uint_t event,
		const err::Error& error
		);

	void
	setIoErrorEvent (const err::Error& error)
	{
		setErrorEvent (AsyncIoEvent_IoError, error);
	}

	void
	setIoErrorEvent ()
	{
		setErrorEvent (AsyncIoEvent_IoError, err::getLastError ());
	}

	bool
	isReadBufferValid ();

	bool
	isWriteBufferValid ();

	size_t
	bufferedRead (
		DataPtr ptr,
		size_t size,
		sl::Array <char>* params = NULL
		);

	size_t
	bufferedReadImpl_l (
		void* p,
		size_t size,
		sl::Array <char>* params = NULL
		);

	size_t
	bufferedWrite (
		DataPtr dataPtr,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		);

	void
	addToReadBuffer (
		const void* p,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		);

	void
	getNextWriteBlock (
		sl::Array <char>* data,
		sl::Array <char>* params = NULL
		);

	void
	updateReadWriteBufferEvents ();

#if (_JNC_OS_WIN)
	OverlappedRead*
	createOverlappedRead (size_t paramSize = 0)
	{
		return !m_freeOverlappedReadList.isEmpty () &&
			m_freeOverlappedReadList.getHead ()->m_paramSize >= paramSize ?
			m_freeOverlappedReadList.removeHead () :
			AXL_MEM_NEW_EXTRA (OverlappedRead, paramSize);
	}
#endif

	ReadWriteMeta*
	createReadWriteMeta (
		size_t dataSize,
		const void* params,
		size_t paramSize
		);
};

//..............................................................................

} // namespace io
} // namespace jnc
