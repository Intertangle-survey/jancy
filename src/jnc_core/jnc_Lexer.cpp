#include "pch.h"
#include "jnc_Lexer.h"
#include "jnc_Lexer.rl.cpp"

namespace jnc {

//.............................................................................

size_t 
decodeByteString (
	rtl::Array <char>* buffer,
	int radix, // must be 2 or 16
	const char* p,
	size_t length
	)
{

	ASSERT (radix == 2 || radix == 16);
	
	enum State
	{
		State_Space = 0,
		State_Byte,
	};

	State state = State_Space;

	if (length == -1)
		length = rtl::StringDetails::calcLength (p);

	buffer->clear ();
	buffer->reserve (length / 2); // good estimate no matter the radix
	
	char byteBuffer [16] = { 0 }; // big enough to fit byte in any radix
	char* byteEnd;
	size_t byteLength;
	
	size_t maxByteLength = 16 / radix;

	uchar_t x;

	const char* end = p + length;
	for (; p < end; p++)
	{
		bool_t isSpace = isspace(*p);

		switch (state)
		{
		case State_Space:
			if (isSpace)
				break;

			byteBuffer [0] = *p;
			byteLength = 1;
			state = State_Byte;
			break;

		case State_Byte:
			if (!isSpace)
			{
				byteBuffer [byteLength++] = *p;
				if (byteLength < maxByteLength)
					break;
			}

			byteBuffer [byteLength] = 0;
			x = (uchar_t) strtoul (byteBuffer, &byteEnd, radix);

			if (byteEnd == &byteBuffer [byteLength])
				buffer->append (x);
			else
				p = end; // not a propert byte string anymore, break the loop

			state = State_Space;
			break;
		}
	}

	if (state == State_Byte)
	{
		byteBuffer [byteLength] = 0;
		x = (uchar_t) strtoul (byteBuffer, &byteEnd, radix);

		if (byteEnd == &byteBuffer [byteLength])
			buffer->append (x);
	}

	return buffer->getCount ();
}

//.............................................................................

Token*
Lexer::createKeywordTokenEx (
	int tokenKind,
	int param
	)
{
	Token* token = createToken (tokenKind);
	token->m_data.m_integer = param;
	return token;
}

Token*
Lexer::createStringToken (
	int tokenKind,
	size_t left,
	size_t right,
	bool useEscapeEncoding
	)
{
	Token* token = createToken (tokenKind);
	ASSERT (token->m_pos.m_length >= left + right);

	size_t length = token->m_pos.m_length - (left + right);
	if (useEscapeEncoding)
		token->m_data.m_string = enc::EscapeEncoding::decode (ts + left, length);
	else
		token->m_data.m_string.copy (ts + left, length);

	return token;
}

Token*
Lexer::createHexLiteralToken (int radix)
{
	Token* token = createToken (TokenKind_HexLiteral);
	ASSERT (token->m_pos.m_length >= 4);
	decodeByteString (&token->m_data.m_binData, radix, ts + 3, token->m_pos.m_length - 4);
	return token;
}

Token*
Lexer::createCharToken (int tokenKind)
{
	Token* token = createToken (tokenKind);
	ASSERT (token->m_pos.m_length >= 2);

	char buffer [256];
	rtl::String string (ref::BufKind_Stack, buffer, sizeof (buffer));
	enc::EscapeEncoding::decode (&string, ts + 1, token->m_pos.m_length - 2);
		
	token->m_data.m_integer = string [0];
	return token;
}

Token*
Lexer::createIntegerToken (
	int radix,
	size_t left
	)
{
	Token* token = createToken (TokenKind_Integer);
	token->m_data.m_int64_u = _strtoui64 (ts + left, NULL, radix);
	return token;
}

Token*
Lexer::createFpToken ()
{
	Token* token = createToken (TokenKind_Fp);
	token->m_data.m_double = strtod (ts, NULL);
	return token;
}

Token*
Lexer::createConstIntegerToken (int value)
{
	Token* token = createToken (TokenKind_Integer);
	token->m_data.m_integer = value;
	return token;
}

// formatting literals

Token*
Lexer::preCreateFmtLiteralToken ()
{
	ASSERT (!m_fmtLiteralToken);
	m_fmtLiteralToken = preCreateToken (0);
	return m_fmtLiteralToken;
}

Token*
Lexer::createFmtLiteralToken (
	int tokenKind,
	int param
	)
{
	ASSERT (m_fmtLiteralToken);
	Token* token = m_fmtLiteralToken;

	size_t left = token->m_pos.m_length;
	size_t right = te - ts;

	m_fmtLiteralToken = NULL;

	token->m_pos.m_length = te - token->m_pos.m_p;
	ASSERT (token->m_pos.m_length >= left + right);

	token->m_token = tokenKind;
	token->m_data.m_string = enc::EscapeEncoding::decode (
		token->m_pos.m_p + left,
		token->m_pos.m_length - (left + right));
	token->m_data.m_integer = param;
	return token;
}

Token*
Lexer::createFmtSimpleIdentifierToken ()
{
	createFmtLiteralToken (TokenKind_FmtLiteral, false);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	Token* token = createStringToken (TokenKind_Identifier, 1, 0);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken ();
	return token;
}

Token*
Lexer::createFmtIndexToken ()
{
	createFmtLiteralToken (TokenKind_FmtLiteral, true);

	// important: prevent stop () -- otherwise we could feed half-created fmt-literal token to the parser

	size_t prevTokenizeLimit = m_tokenizeLimit;
	m_tokenizeLimit = -1;

	Token* token = createIntegerToken (10, 1);

	m_tokenizeLimit = prevTokenizeLimit;

	preCreateFmtLiteralToken ();
	return token;
}

Token*
Lexer::createFmtSpecifierToken ()
{
	ASSERT (*ts == ';');
	ts++;

	while (ts < te && (*ts == ' ' || *ts == '\t'))
		ts++;

	return ts < te ? createStringToken (TokenKind_FmtSpecifier) : NULL;
}

void
Lexer::onLeftParentheses ()
{
	if (!m_parenthesesLevelStack.isEmpty ())
		m_parenthesesLevelStack [m_parenthesesLevelStack.getCount () - 1]++;

	createToken ('(');
}

bool
Lexer::onRightParentheses ()
{
	if (!m_parenthesesLevelStack.isEmpty ())
	{
		size_t i = m_parenthesesLevelStack.getCount () - 1;
		if (m_parenthesesLevelStack [i] == 1)
		{
			m_parenthesesLevelStack.pop ();
			preCreateFmtLiteralToken ();
			return false;
		}

		m_parenthesesLevelStack [i]--;
	}

	createToken (')');
	return true;
}

bool
Lexer::onSemicolon ()
{
	ASSERT (*ts == ';');

	if (!m_parenthesesLevelStack.isEmpty ())
	{
		size_t i = m_parenthesesLevelStack.getCount () - 1;
		if (m_parenthesesLevelStack [i] == 1)
		{
			p = ts - 1; // need to reparse semicolon with 'fmt_spec' machine
			return false;
		}
	}

	createToken (';');
	return true;
}

void
Lexer::terminateFmtSpecifier ()
{
	ASSERT (!m_parenthesesLevelStack.isEmpty () && m_fmtLiteralToken == NULL);
	m_parenthesesLevelStack.pop ();
	preCreateFmtLiteralToken ();
}

//.............................................................................

} // namespace jnc {
