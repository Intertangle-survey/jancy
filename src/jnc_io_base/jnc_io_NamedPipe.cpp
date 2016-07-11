#include "pch.h"
#include "jnc_io_NamedPipe.h"

namespace jnc {
namespace io {

//.............................................................................

NamedPipe::NamedPipe ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId = 0;
}

bool
AXL_CDECL
NamedPipe::open (
	DataPtr namePtr,
	size_t backLog
	)
{
	bool result;

	close ();

	if (backLog == 0)
		backLog = Const_DefBackLog;

	if (backLog > Const_MaxBackLog)
		backLog = Const_MaxBackLog;
	
	const char* name = (const char*) namePtr.m_p;

	sl::String_w pipeName = name;
	sl::Array <axl::io::win::NamedPipe> pipeArray;

	pipeArray.setCount (backLog);

	for (size_t i = 0; i < backLog; i++)
	{
		axl::io::win::NamedPipe pipe;
		result = pipe.create (
			pipeName,
			PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 
			PIPE_UNLIMITED_INSTANCES,
			Const_TxBufferSize,
			Const_RxBufferSize,
			Const_Timeout,
			NULL
			);

		if (!result)
		{
			ext::propagateLastError ();
			return false;
		}

		pipeArray [i].attach (pipe.detach ());
	}

	m_pipeName = pipeName;
	m_pipeArray = pipeArray;
	pipeArray.release ();	
	m_listenArray.clear ();
	m_isOpen = true;
	m_ioFlags = IoFlag_Opened;
	m_ioThreadEvent.reset ();
	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
NamedPipe::close ()
{
	if (!m_isOpen)
		return;

	m_ioLock.lock ();
	m_ioFlags &= ~IoFlag_Opened;
	m_ioFlags |= IoFlag_Closing;
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);

	m_pipeArray.clear ();
	m_ioFlags = 0;
	m_isOpen = false;
	m_syncId++;
}

FileStream*
AXL_CDECL
NamedPipe::accept ()
{
	m_ioLock.lock ();
	if (!(m_ioFlags & IoFlag_Opened))
	{
		ext::setError (err::SystemErrorCode_InvalidDeviceState);
		return NULL;
	}

	HANDLE hPipe;

	if (!m_pendingAcceptArray.isEmpty ())
	{
		size_t i = m_pendingAcceptArray [0];
		m_pendingAcceptArray.remove (0);
		m_listenArray.append (i);
		hPipe = m_pipeArray [i].detach ();
		m_ioThreadEvent.signal ();
		m_ioLock.unlock ();
	}
	else
	{
		Accept accept;
		m_acceptList.insertTail (&accept);
		m_ioThreadEvent.signal ();
		m_ioLock.unlock ();

		rt::enterWaitRegion (m_runtime);
		accept.m_completionEvent.wait ();
		rt::leaveWaitRegion (m_runtime);

		if (accept.m_hPipe == INVALID_HANDLE_VALUE)
		{
			ext::setError (accept.m_error);
			return NULL;
		}

		hPipe = accept.m_hPipe;
	}

	FileStream* fileStream = rt::createClass <FileStream> (m_runtime);
	fileStream->m_file.m_file.attach (hPipe);
	fileStream->m_readBuffer.setCount (FileStream::Const_ReadBufferSize);
	fileStream->m_isOpen = true;
	fileStream->m_ioFlags |= FileStream::IoFlag_Opened;
	fileStream->m_ioThreadEvent.reset ();
	fileStream->m_ioThread.start ();
	return fileStream;
}

void
NamedPipe::ioThreadFunc ()
{
	listenLoop ();

	sl::AuxList <Accept> acceptList;
	acceptList.takeOver (&m_acceptList);
	m_ioLock.unlock ();
	
	while (!acceptList.isEmpty ())
	{
		Accept* accept = acceptList.removeHead ();
		accept->m_error = err::Error (ERROR_CANCELLED);
		accept->m_completionEvent.signal ();
	}
}

void
NamedPipe::listenLoop ()
{
	bool result;

	size_t pipeCount = m_pipeArray.getCount ();
	ASSERT (pipeCount);

	char buffer1 [256];
	sl::Array <sys::Event> eventArray (ref::BufKind_Stack, buffer1, sizeof (buffer1));

	char buffer2 [256];
	sl::Array <OVERLAPPED> overlappedArray (ref::BufKind_Stack, buffer2, sizeof (buffer2));
	eventArray.setCount (pipeCount);

	char buffer3 [256];
	sl::Array <HANDLE> waitArray (ref::BufKind_Stack, buffer3, sizeof (buffer3));
	waitArray.setCount (pipeCount + 1);
	waitArray [0] = m_ioThreadEvent.m_event;

	char buffer4 [256];
	sl::Array <size_t> waitMapArray (ref::BufKind_Stack, buffer4, sizeof (buffer4));
	waitMapArray.setCount (pipeCount);

	for (size_t i = 0, j = 1; i < pipeCount; i++, j++)
	{
		overlappedArray [i].hEvent = eventArray [i].m_event;
		waitArray [j] = eventArray [i].m_event;
		waitMapArray [i] = i;

		result = m_pipeArray [i].connect (&overlappedArray [i]);
		ASSERT (result); // ignore result
	}

	for (;;)
	{
		size_t waitCount = waitArray.getCount ();
		ASSERT (waitCount);

		DWORD waitResult = ::WaitForMultipleObjects (waitCount, waitArray, false, -1);
		
		m_ioLock.lock ();
		if (waitResult == WAIT_FAILED)
		{
			return;
		}
		else if (waitResult == WAIT_OBJECT_0)
		{
			if (m_ioFlags & IoFlag_Closing)
				return;

			sl::Array <size_t> listenArray = m_listenArray;
			m_listenArray.clear ();
			m_ioLock.unlock ();

			size_t listenCount = listenArray.getCount ();
			for (size_t i = 0; i < listenCount; i++)
			{
				size_t pipeIdx = listenArray [i];
				result = m_pipeArray [pipeIdx].create (
					m_pipeName,
					PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
					PIPE_TYPE_BYTE | PIPE_READMODE_BYTE, 
					PIPE_UNLIMITED_INSTANCES,
					Const_TxBufferSize,
					Const_RxBufferSize,
					Const_Timeout,
					NULL
					) && 
					m_pipeArray [pipeIdx].connect (&overlappedArray [pipeIdx]);

				if (result)
				{
					waitArray.append (eventArray [pipeIdx].m_event);
					waitMapArray.append (pipeIdx);
				}
			}
		}
		else
		{
			size_t waitMapIdx = waitResult - 1;
			ASSERT (waitMapIdx < waitMapArray.getCount ());

			size_t pipeIdx = waitMapArray [waitMapIdx];
			ASSERT (pipeIdx < pipeCount && m_pipeArray [pipeIdx].isOpen ());

			if (!m_acceptList.isEmpty ())
			{
				Accept* accept = m_acceptList.removeHead ();
				m_ioLock.unlock ();

				accept->m_hPipe = m_pipeArray [pipeIdx].detach ();
				accept->m_completionEvent.signal ();
			}
			else
			{
				m_pendingAcceptArray.append (pipeIdx);
				m_ioLock.unlock ();

				rt::callMulticast (m_runtime, m_onIncomingConnectionEvent, m_syncId);

				waitArray.remove (waitResult);
				waitMapArray.remove (waitMapIdx);
			}
		}
	}
}

//.............................................................................

} // namespace io
} // namespace jnc
