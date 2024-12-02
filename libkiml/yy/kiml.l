%{
	#include "kiml.h"
	#include "kiml.parser.h"
	#include "utils.h"
	
	#include <math.h>
	#include <vector>
	
	int yylineno = 1;
	int yycolumn = 1;
	
	#define YY_DECL extern int yylex(YYSTYPE *yylval, YYLTYPE *yylloc)
	
	#define YY_USER_ACTION \
	    yylloc->first_line = yylloc->last_line = yylineno; \
	    yylloc->first_column = yycolumn; yylloc->last_column = yycolumn+yyleng-1; \
	    yycolumn += yyleng;
	
	std::vector<KIMLCHAR> stringBuffer;
	
	// extern YYSTYPE yylval;
	// extern YYLTYPE yylloc;
	
	void yyerror(const char *s);
	void lexError(YYLTYPE *yylloc, const char *s);
%}

%option noyywrap
%option never-interactive

whitespace	[ \t]

digit		[0-9]
letter		[:alpha:]
hexdigit	[0-9A-Fa-f]

litint		{digit}+
litinthex	0x{hexdigit}+
litfloat	{digit}+\.{digit}*

ident		[A-Za-z_][A-Za-z_0-9]*

%x		INLITSTRING

%%

{whitespace}+	{ }

;[^\r\n]*	{ /* comment */ }

\\\r?\n		{ ++yylineno; yycolumn = 1; }

"io"		return TIO;
"var"		return TVAR;
"stack"		return TSTACK;
"tape"		return TTAPE;
"ctrl"		return TCTRL;
"func"		return TFUNC;
"event"		return TEVENT;
"const"		return TCONST;
"enum"		return TENUM;
"in"		return TIN;
"out"		return TOUT;
"decl"		return TDECL;
"set"		return TSET;
"del"		return TDEL;
"push"		return TPUSH;
"pop"		return TPOP;
"peek"		return TPEEK;
"swap"		return TSWAP;
"clear"		return TCLEAR;
"read"		return TREAD;
"write"		return TWRITE;
"next"		return TNEXT;
"prev"		return TPREV;
"move"		return TMOVE;
"goto"		return TGOTO;
"call"		return TCALL;
"return"	return TRETURN;
"end"		return TEND;
"handle"	return THANDLE;
"unhandle"	return TUNHANDLE;
"raise"		return TRAISE;
"if"		return TIF;
"at"		return TAT;
"by"		return TBY;
"and"		return TAND;
"or"		return TOR;
"xor"		return TXOR;
"not"		return TNOT;

"any"		return TANY;
"bool"		return TBOOL;
"int"		return TINT;
"real"		return TREAL;
"string"	return TSTRING;
"object"	return TOBJECT;

"true"		return TLITTRUE;
"false"		return TLITFALSE;
"null"		return TNULL;

"."		return TDOT;
":"		return TCOLON;
","		return TCOMMA;
"("		return TLPAR;
")"		return TRPAR;
"+"		return TADD;
"-"		return TSUB;
"*"		return TMUL;
"/"		return TDIV;
"%"		return TMOD;
"^"		return TPOW;
"&"		return TCAT;
"="		return TEQU;
"<>"		return TNEQ;
"<"		return TLES;
"<="		return TLEQ;
">"		return TGRT;
">="		return TGEQ;
"@"		return TCINT;
"#"		return TCREAL;
"$"		return TCOBJECT;

{litint}	{
			yylval->ivalue = atoi(yytext);
			return TLITINT;
		}
		
{litinthex}	{
			yylval->ivalue = (int)strtol(yytext, nullptr, 16);
			return TLITINT;
		}
		
{litfloat}	{
			yylval->rvalue = (float)atof(yytext);
			return TLITREAL;
		}

{ident}		{
			yylval->svalue = kimlstrdup(yytext);
			return TIDENT;
		}

\"		{
			BEGIN(INLITSTRING);
			stringBuffer.reserve(1024);
			stringBuffer.clear();
		}
<INLITSTRING>{
	\"		{
				BEGIN(INITIAL);
				char *str = new char[stringBuffer.size() + 1];
				if (stringBuffer.size())
					memcpy(str, (void *)&stringBuffer[0], stringBuffer.size());
				str[stringBuffer.size()] = 0;
				yylval->svalue = str;
				return TLITSTRING;
			}
	[^\\"\n]*	{ stringBuffer.insert(stringBuffer.end(), yytext, yytext + yyleng); }
	\\n		{ stringBuffer.push_back('\n'); }
	\\r		{ stringBuffer.push_back('\r'); }
	\\t		{ stringBuffer.push_back('\t'); }
	\\b		{ stringBuffer.push_back('\b'); }
	\\\"		{ stringBuffer.push_back('\"'); }
	\\\\		{ stringBuffer.push_back('\\'); }
	\\0		{ stringBuffer.push_back('\0'); }
	\\x{hexdigit}{2}	{ stringBuffer.push_back((char)strtol(yytext+2, nullptr, 16)); }
	\\u{hexdigit}{4}	{ int wc = (int)strtol(yytext+2, nullptr, 16); stringBuffer.push_back((char)wc); stringBuffer.push_back((char)(wc / 256)); }
	\r\n?|\n	{ lexError(yylloc, "Expected closing quotes."); BEGIN(INITIAL); return TEOS; }
	<<EOF>>		{ lexError(yylloc, "Expected closing quotes."); BEGIN(INITIAL); return TEOS; }
}

\r?\n		{ ++yylineno; yycolumn = 1; return TEOS; }

.		{ lexError(yylloc, "Unrecognized character."); return TEOS; }

%%

void lexError(YYLTYPE *yylloc, const char *s)
{
	fprintf(stderr, "Line %d, col %d: ", yylloc->first_line, yylloc->first_column);
	fprintf(stderr, "%s\n", s);
}
