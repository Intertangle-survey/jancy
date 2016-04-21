#include "pch.h"
#include "jnc_io_Mailslot.h"

namespace jnc {
namespace io {

//.............................................................................

Mailslot::Mailslot ()
{
	m_runtime = rt::getCurrentThreadRuntime ();
	m_ioFlags = 0;
	m_incomingDataSize = 0;
	m_isOpen = false;
	m_syncId = 0;
}

bool
AXL_CDECL
Mailslot::open (rt::DataPtr namePtr)
{
	close ();

	sl::String_w name = "\\\\.\\mailslot\\";
	name += (const char*) namePtr.m_p;
	
	HANDLE mailslot = ::CreateMailslotW (name, 0, -1, NULL);
	if (mailslot == INVALID_HANDLE_VALUE)
	{
		ext::setError (::GetLastError ());
		return false;
	}

	m_file.m_file.attach (mailslot);
	m_isOpen = true;

	m_ioThreadEvent.reset ();
	m_readBuffer.setCount (Const_ReadBufferSize);
	m_incomingDataSize = 0;

	m_ioFlags = 0;
	m_ioThread.start ();
	return true;
}

void
AXL_CDECL
Mailslot::close ()
{
	if (!m_file.isOpen ())
		return;

	m_ioLock.lock ();
	m_ioFlags |= IoFlag_Closing;
	m_ioThreadEvent.signal ();
	m_ioLock.unlock ();

	rt::enterWaitRegion (m_runtime);
	m_ioThread.waitAndClose ();
	rt::leaveWaitRegion (m_runtime);

	m_file.close ();
	m_ioFlags = 0;
	m_incomingDataSize = 0;
	m_isOpen = false;
	m_syncId++;
}

void
Mailslot::fireMailslotEvent (
	MailslotEventKind eventKind,
	const err::ErrorHdr* error
	)
{
	JNC_BEGIN_CALL_SITE_NO_COLLECT (m_runtime, true);

	rt::DataPtr paramsPtr = rt::createData <MailslotEventParams> (m_runtime);
	MailslotEventParams* params = (MailslotEventParams*) paramsPtr.m_p;
	params->m_eventKind = eventKind;
	params->m_syncId = m_syncId;

	if (error)
		params->m_errorPtr = rt::memDup (error, error->m_size);

	rt::callMulticast (m_onMailslotEvent, paramsPtr);

	JNC_END_CALL_SITE ();
}

size_t
AXL_CDECL
Mailslot::read (
	rt::DataPtr ptr,
	size_t size
	)
{
	m_ioLock.lock ();
	if (m_ioFlags & IoFlag_IncomingData)
	{
		size_t result = readImpl (ptr.m_p, size);
		m_ioThreadEvent.signal ();
		m_ioLock.unlock ();

		return result;
	}
	else
	{
		Read read;
		read.m_buffer = ptr.m_p;
		read.m_size = size;
		m_readList.insertTail (&read);
		m_ioThreadEvent.signal ();
		m_ioLock.unlock ();

		rt::enterWaitRegion (m_runtime);
		read.m_completionEvent.wait ();
		rt::leaveWaitRegion (m_runtime);

		if (read.m_result == -1)
			ext::setError (read.m_error);

		return read.m_result;
	}
}

size_t
Mailslot::readImpl (
	void* p,
	size_t size
	)
{
	ASSERT (m_incomingDataSize);

	size_t copySize;

	if (size < m_incomingDataSize)
	{
		copySize = size;
		m_incomingDataSize -= size;
		m_ioFlags |= IoFlag_RemainingData;
		memcpy (p, m_readBuffer, copySize);
		memmove (m_readBuffer, m_readBuffer + copySize, m_incomingDataSize);
	}
	else
	{
		copySize = m_incomingDataSize;
		m_incomingDataSize = 0;
		m_ioFlags &= ~IoFlag_IncomingData;
		memcpy (p, m_readBuffer, copySize);
	}

	return copySize;
}

void
Mailslot::ioThreadFunc ()
{
	ASSERT (m_file.isOpen ());

	readLoop ();

	// wait for close, cancell all incoming reads

	for (;;)
	{
		m_ioLock.lock ();

		while (!m_readList.isEmpty ())
		{
			Read* read = m_readList.removeHead ();
			read->m_result = -1;
			read->m_error = err::Error (ERROR_CANCELLED);
			read->m_completionEvent.signal ();
		}

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		m_ioLock.unlock ();

		m_ioThreadEvent.wait ();
	}
}


void
Mailslot::readLoop ()
{
	sys::Event completionEvent;

	HANDLE waitTable [] =
	{
		m_ioThreadEvent.m_event,
		completionEvent.m_event,
	};

	uint64_t offset = 0;

	for (;;)
	{
		Read* read = NULL;
		void* readBuffer;
		size_t readSize;

		m_ioLock.lock ();

		if (m_ioFlags & IoFlag_Closing)
		{
			m_ioLock.unlock ();
			break;
		}

		if (m_ioFlags & IoFlag_RemainingData)
		{
			ASSERT (m_ioFlags & IoFlag_IncomingData);

			if (!m_readList.isEmpty ())
			{
				read = m_readList.removeHead ();
				readImpl (read->m_buffer, read->m_size);
			}
			else
			{
				fireMailslotEvent (MailslotEventKind_IncomingData);
			}

			m_ioFlags &= ~IoFlag_RemainingData;
			m_ioLock.unlock ();
			continue;
		}

		if (!m_readList.isEmpty ())
		{
			read = m_readList.removeHead ();
			readBuffer = read->m_buffer;
			readSize = read->m_size;
		}
		else if (!(m_ioFlags & IoFlag_IncomingData))
		{
			ASSERT (m_incomingDataSize == 0);
			readBuffer = m_readBuffer;
			readSize = m_readBuffer.getCount ();
		}
		else
		{
			readBuffer = NULL;
		}

		m_ioLock.unlock ();

		if (!readBuffer)
		{
			DWORD waitResult = ::WaitForSingleObject (m_ioThreadEvent.m_event, INFINITE);
			if (waitResult == WAIT_FAILED)
			{
				err::Error error = err::getLastSystemErrorCode ();
				fireMailslotEvent (MailslotEventKind_IoError, error);
				return;
			}
		}
		else
		{
			OVERLAPPED overlapped = { 0 };

			overlapped.hEvent = completionEvent.m_event;
			overlapped.Offset = (DWORD) offset;
			overlapped.OffsetHigh = (DWORD) (offset >> 32);

			bool_t result = m_file.m_file.read (readBuffer, readSize, NULL, &overlapped);
			if (!result)
			{
				if (read)
				{
					read->m_result = -1;
					read->m_error = err::getLastError ();
					read->m_completionEvent.signal ();
				}

				break;
			}

			for (;;) // cycle is needed case main thread can add new reads to m_readList
			{
				DWORD waitResult = ::WaitForMultipleObjects (2, waitTable, false, INFINITE);
				if (waitResult == WAIT_FAILED)
				{
					err::Error error = err::getLastSystemErrorCode ();
					fireMailslotEvent (MailslotEventKind_IoError, error);
					return;
				}

				m_ioLock.lock ();

				if (m_ioFlags & IoFlag_Closing)
				{
					m_ioLock.unlock ();
					return;
				}

				m_ioLock.unlock ();

				if (waitResult == WAIT_OBJECT_0 + 1)
					break;
			}

			readSize = m_file.m_file.getOverlappedResult (&overlapped);
			if (readSize == -1)
			{
				err::Error error = err::getLastError ();

				if (read)
				{
					read->m_result = -1;
					read->m_error = error;
					read->m_completionEvent.signal ();
				}

				fireMailslotEvent (MailslotEventKind_IoError, error);
				return;
			}

			if (read)
			{
				read->m_result = readSize;
				read->m_completionEvent.signal ();
			}
			else
			{
				m_ioLock.lock ();
				ASSERT (!(m_ioFlags & IoFlag_IncomingData));
				if (readSize)
				{
					m_ioFlags |= IoFlag_IncomingData;
					m_incomingDataSize = readSize;
				}

				m_ioLock.unlock ();

				if (readSize)
				{
					fireMailslotEvent (MailslotEventKind_IncomingData);
				}
				else
				{
					err::Error error (err::SystemErrorCode_InvalidDeviceState);
					fireMailslotEvent (MailslotEventKind_IoError, error);
					return;
				}
			}

			offset += readSize; // advance offset
		}
	}
}

//.............................................................................

} // namespace io
} // namespace jnc
