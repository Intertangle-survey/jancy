#include "pch.h"
#include "jnc_Recognizer.h"
#include "jnc_StdLib.h"

namespace jnc {

//.............................................................................

void
AXL_CDECL
Recognizer::construct (FunctionPtr automatonFuncPtr)
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lexemeLengthLimit = 128;
	m_currentOffset = 0;
	
	char* buffer = (char*) AXL_MEM_ALLOC (m_lexemeLengthLimit);

	m_lexeme.m_p = buffer;
	m_lexeme.m_rangeBegin = buffer;
	m_lexeme.m_rangeEnd = buffer + m_lexemeLengthLimit;
	m_lexeme.m_object = getStaticObjHdr ();

	setAutomatonFunc (automatonFuncPtr);
}

void
AXL_CDECL
Recognizer::setAutomatonFunc (FunctionPtr automatonFuncPtr)
{
	m_automatonFuncPtr = automatonFuncPtr;
	m_stateId = 0;
	m_lastAcceptStateId = -1;

	if (automatonFuncPtr.m_p)
		((AutomatonFunc*) automatonFuncPtr.m_p) (automatonFuncPtr.m_closure, this, -1);
}

void
AXL_CDECL
Recognizer::reset ()
{
	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lastAcceptLexemeLength = 0;
	m_lexemeOffset = 0;
	m_lexemeLength = 0;
	m_currentOffset = 0;
}

bool 
AXL_CDECL
Recognizer::write (
	DataPtr ptr,
	size_t length
	)
{
	if (!m_automatonFuncPtr.m_p)
	{
		err::setError (err::SystemErrorCode_InvalidDeviceState);
		return false;	
	}

	if (length == -1)
		length = StdLib::strLen (ptr);

	return length ? writeImpl ((uchar_t*) ptr.m_p, length) : forceEof ();
}

bool
Recognizer::writeImpl (
	uchar_t* p, 
	size_t length
	)
{
	bool result;

	uchar_t* end = p + length;

	while (p < end)
	{
		uchar_t c = *p++;
		m_currentOffset++;

		// append lexeme

		((uchar_t*) m_lexeme.m_p) [m_lexemeLength++] = c;
		if (m_lexemeLength >= m_lexemeLengthLimit)
		{
			err::setStringError ("lexeme too long");
			return false;
		}

		size_t newStateId = m_transitionTable [m_stateId * 256 + c];
		if (newStateId != -1)
		{
			result = gotoState (newStateId);
			if (!result)
				return false;
		}
		else if (m_lastAcceptStateId != -1)
		{
			result = rollback ();
			if (!result)
				return false;
		}
		else
		{
			err::setStringError ("unrecognized lexeme");
			return false;
		}
	}
	
	return true;
}

bool
Recognizer::forceEof ()
{
	for (;;)
	{
		if (!m_lexemeLength) // just matched
			return true;

		if (m_lastAcceptStateId == -1)
		{
			err::setStringError ("unrecognized lexeme");
			return false;
		}

		if (m_lastAcceptLexemeLength >= m_lexemeLength)
			return match (m_lastAcceptStateId);

		bool result = rollback ();
		if (!result)
			return false;
	}
}

bool
Recognizer::gotoState (size_t stateId)
{
	m_stateId = stateId;
			
	uint_t flags = m_stateFlagTable [stateId];
	if (!(flags & RecognizerStateFlag_Accept))
		return true;

	if (flags & RecognizerStateFlag_Final)
		return match (stateId);

	m_lastAcceptStateId = stateId;
	m_lastAcceptLexemeLength = m_lexemeLength;
	return true;
}

bool
Recognizer::rollback ()
{
	ASSERT (m_lastAcceptStateId != -1 && m_lastAcceptLexemeLength);
	
	uchar_t* chunk = (uchar_t*) m_lexeme.m_p + m_lastAcceptLexemeLength;
	size_t chunkLength = m_lexemeLength - m_lastAcceptLexemeLength;

	m_currentOffset = m_lexemeOffset + m_lastAcceptLexemeLength;
	m_lexemeLength = m_lastAcceptLexemeLength;

	size_t savedLexemeLength = m_lexemeLength;
	uchar_t savedChar = ((uchar_t*) m_lexeme.m_p) [m_lexemeLength];
	
	bool result = match (m_lastAcceptStateId);
	if (!result)
		return false;

	((uchar_t*) m_lexeme.m_p) [savedLexemeLength] = savedChar;

	return chunkLength ? writeImpl (chunk, chunkLength) : true;
}

bool
Recognizer::match (size_t stateId)
{
	((uchar_t*) m_lexeme.m_p) [m_lexemeLength] = 0;
	bool result = ((AutomatonFunc*) m_automatonFuncPtr.m_p) (m_automatonFuncPtr.m_closure, this, stateId);
	if (!result)
		return false;

	m_stateId = 0;
	m_lastAcceptStateId = -1;
	m_lexemeOffset = m_currentOffset;
	m_lexemeLength = 0;
	return true;
}

//.............................................................................

} // namespace jnc