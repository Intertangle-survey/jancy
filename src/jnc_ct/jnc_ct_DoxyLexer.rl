// warning C4065: switch statement contains 'default' but no 'case' labels

#pragma warning (disable: 4065)

namespace jnc {
namespace ct {

//.............................................................................

%%{

machine dox;
write data;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# definitions
#

# regular char (non-whitespace and non-escape)

rc = [^ \t\r\n\\];
ws = [ \t\r]+;

# . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .
#
# main machine
#

main := |*

'\\enum'          { createToken (DoxyTokenKind_Enum); };
'\\struct'        { createToken (DoxyTokenKind_Struct); };
'\\union'         { createToken (DoxyTokenKind_Union); };
'\\class'         { createToken (DoxyTokenKind_Class); };
'\\fn'            { createToken (DoxyTokenKind_Fn); };
'\\group'         { createToken (DoxyTokenKind_Group); };
'\\defgroup'      { createToken (DoxyTokenKind_DefGroup); };
'\\ingroup'       { createToken (DoxyTokenKind_InGroup); };
'\\addtogroup'    { createToken (DoxyTokenKind_AddToGroup); };
'\\title'         { createToken (DoxyTokenKind_Title); };
'\\brief'         { createToken (DoxyTokenKind_Brief); };

'\\' rc*          { createToken (DoxyTokenKind_OtherCommand); };

'@{'              { createToken (DoxyTokenKind_OpeningBrace); };
'@}'              { createToken (DoxyTokenKind_ClosingBrace); };

rc ([^\n\\]* rc)? { createTextToken (); };

'\n'              { createToken ('\n'), newLine (ts + 1); };
ws                ;

*|;

}%%

//.............................................................................

void
DoxyLexer::init ()
{
	%% write init;
}

void
DoxyLexer::exec ()
{
	%% write exec;
}

//.............................................................................

} // namespace ct
} // namespace jnc
