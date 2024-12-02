%{
	#include "kiml.h"
	#include "ast.h"
	#include <stdio.h>
	
	AstProgram *root;
	
	union YYSTYPE;
	struct YYLTYPE;
	
	int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);
	
	void yyerror(YYLTYPE *yylloc, const char *s);
	
	KIMLUINT nSyntaxErrors = 0;
	KIMLCSTRING szFileName = "";
%}

%locations
%define api.pure
%error-verbose

%defines "kiml.parser.h"
%no-lines
%output "kiml.parser.cpp"

%union YYSTYPE {
	AstProgram *program;
	AstStatement *stmt;
	AstStatementList *stmtlist;
	AstExpression *expr;
	AstExpressionList *exprlist;
	AstParamList *paramlist;
	AstIdentList *identlist;
	KIMLTYPES datatype;
	
	KIMLBOOL bvalue;
	KIMLINT ivalue;
	KIMLFLOAT rvalue;
	KIMLCSTRING svalue;
}

%token TIO TVAR TSTACK TTAPE TCTRL TFUNC TEVENT TCONST TENUM
%token TIN TOUT TDECL TDEL TSET TPUSH TPOP TPEEK TCLEAR TSWAP TREAD TWRITE TNEXT TPREV TMOVE TGOTO TCALL TRETURN TEND THANDLE TUNHANDLE TRAISE
%token TIF TAT TBY

%token TDOT TCOLON TCOMMA
%token TIDENT TLITTRUE TLITFALSE TNULL TLITINT TLITREAL TLITSTRING
%token TLPAR TRPAR TEQU TNEQ TLES TLEQ TGRT TGEQ TADD TSUB TMUL TDIV TMOD TPOW TCAT TAND TOR TXOR TNOT
%token TCINT TCREAL TCOBJECT
%token TANY TBOOL TINT TREAL TSTRING TOBJECT
%token TEOS

%type <svalue> TIDENT TLITSTRING
%type <bvalue> TLITTRUE TLITFALSE
%type <ivalue> TLITINT
%type <rvalue> TLITREAL

%type <program> program
%type <stmt> stmt ioin ioout vardecl varset vardel stackpush stackpop stackpeek stackswap stackclear stackread stackwrite taperead tapewrite tapenext tapeprev tapemove ctrlgoto ctrlcall ctrlreturn ctrlend funcdecl funccall eventhandle eventunhandle eventraise label constdef enumdef
%type <stmtlist> stmtlist
%type <expr> expr binexpr unexpr func cast atom
%type <exprlist> exprlist exprlist_sep
%type <datatype> vartype rettype paramtype
%type <paramlist> paramlist paramlist_sep
%type <identlist> identlist

%left TCAT
%left TOR
%left TXOR
%left TAND
%left TEQU TNEQ
%left TLES TLEQ TGRT TGEQ
%left TADD TSUB
%left TMUL TDIV TMOD
%right TPOW
%right TNOT
%right TNEG

%start program

%%

program
	: stmtlist
		{ root = new AstProgram(); root->setStatements($1); }
	;

stmtlist
	: stmt
		{ $$ = new AstStatementList(@1); if ($1) $$->add($1); }
	| stmtlist stmt
		{ if ($2) $1->add($2); $$ = $1; }
	;

stmt
	: TEOS
		{ $$ = nullptr; }
	| ioin
		{ $$ = $1; }
	| ioout
		{ $$ = $1; }
	| vardecl
		{ $$ = $1; }
	| varset
		{ $$ = $1; }
	| vardel
		{ $$ = $1; }
	| stackpush
		{ $$ = $1; }
	| stackpop
		{ $$ = $1; }
	| stackpeek
		{ $$ = $1; }
	| stackswap
		{ $$ = $1; }
	| stackclear
		{ $$ = $1; }
	| stackread
		{ $$ = $1; }
	| stackwrite
		{ $$ = $1; }
	| taperead
		{ $$ = $1; }
	| tapewrite
		{ $$ = $1; }
	| tapenext
		{ $$ = $1; }
	| tapeprev
		{ $$ = $1; }
	| tapemove
		{ $$ = $1; }
	| ctrlgoto
		{ $$ = $1; }
	| ctrlcall
		{ $$ = $1; }
	| ctrlreturn
		{ $$ = $1; }
	| ctrlend
		{ $$ = $1; }
	| funcdecl
		{ $$ = $1; }
	| funccall
		{ $$ = $1; }
	| eventhandle
		{ $$ = $1; }
	| eventunhandle
		{ $$ = $1; }
	| eventraise
		{ $$ = $1; }
	| label
		{ $$ = $1; }
	| constdef
		{ $$ = $1; }
	| enumdef
		{ $$ = $1; }
	| error TEOS
		{ $$ = nullptr; yyerrok; }
	;
	
vartype
	: TBOOL
		{ $$ = KIML_BOOL; }
	| TINT
		{ $$ = KIML_INT; }
	| TREAL
		{ $$ = KIML_REAL; }
	| TSTRING
		{ $$ = KIML_STRING; }
	| TOBJECT
		{ $$ = KIML_OBJECT; }
	;
	
ioin
	: TIO TDOT TIN vartype TEOS
		{ $$ = new AstIoIn(@1, $4); }
	| TIO TDOT TIN TIDENT TEOS
		{ $$ = new AstIoIn(@1, $4); }
	;

ioout
	: TIO TDOT TOUT expr TEOS
		{ $$ = new AstIoOut(@1, $4); }
	;
	
vardecl
	: TVAR TDOT TDECL vartype TIDENT TEOS
		{ $$ = new AstVarDecl(@1, $4, $5); }
	| TVAR TDOT TDECL vartype TIDENT TEQU expr TEOS
		{ $$ = new AstVarDecl(@1, $4, $5, $7); }
	;
	
varset
	: TVAR TDOT TSET TIDENT TEQU expr TEOS
		{ $$ = new AstVarSet(@1, $4, $6); }
	;

vardel
	: TVAR TDOT TDEL TIDENT TEOS
		{ $$ = new AstVarDel(@1, $4); }
	;

stackpush
	: TSTACK TDOT TPUSH expr TEOS
		{ $$ = new AstStackPush(@1, $4); }
	;

stackpop
	: TSTACK TDOT TPOP TEOS
		{ $$ = new AstStackPop(@1); }
	| TSTACK TDOT TPOP TIDENT TEOS
		{ $$ = new AstStackPop(@1, $4); }
	;

stackpeek
	: TSTACK TDOT TPEEK TIDENT TEOS
		{ $$ = new AstStackPeek(@1, $4); }
	;

stackswap
	: TSTACK TDOT TSWAP TEOS
		{ $$ = new AstStackSwap(@1); }
	;

stackclear
	: TSTACK TDOT TCLEAR TEOS
		{ $$ = new AstStackClear(@1); }
	| TSTACK TDOT TCLEAR TLITINT TEOS
		{ $$ = new AstStackClear(@1, $4); }
	;

stackread
	: TSTACK TDOT TREAD TIDENT TAT expr TEOS
		{ $$ = new AstStackRead(@1, $4, $6); }
	;

stackwrite
	: TSTACK TDOT TWRITE expr TAT expr TEOS
		{ $$ = new AstStackWrite(@1, $4, $6); }
	;

taperead
	: TTAPE TDOT TREAD vartype TEOS
		{ $$ = new AstTapeRead(@1, $4); }
	| TTAPE TDOT TREAD TIDENT TEOS
		{ $$ = new AstTapeRead(@1, $4); }
	| TTAPE TDOT TREAD vartype TAT expr TEOS
		{ $$ = new AstTapeRead(@1, $4, $6); }
	| TTAPE TDOT TREAD TIDENT TAT expr TEOS
		{ $$ = new AstTapeRead(@1, $4, $6); }
	;
	
tapewrite
	: TTAPE TDOT TWRITE expr TEOS
		{ $$ = new AstTapeWrite(@1, $4); }
	| TTAPE TDOT TWRITE expr TAT expr TEOS
		{ $$ = new AstTapeWrite(@1, $4, $6); }
	;
	
tapenext
	: TTAPE TDOT TNEXT TEOS
		{ $$ = new AstTapeNext(@1); }
	;
	
tapeprev
	: TTAPE TDOT TPREV TEOS
		{ $$ = new AstTapePrev(@1); }
	;
	
tapemove
	: TTAPE TDOT TMOVE expr TEOS
		{ $$ = new AstTapeMove(@1, $4); }
	;

ctrlgoto
	: TCTRL TDOT TGOTO TIDENT TEOS
		{ $$ = new AstCtrlGoto(@1, $4); }
	| TCTRL TDOT TGOTO TIDENT TIF expr TEOS
		{ $$ = new AstCtrlGoto(@1, $4, $6); }
	;
	
ctrlcall
	: TCTRL TDOT TCALL TIDENT TEOS
		{ $$ = new AstCtrlCall(@1, $4); }
	| TCTRL TDOT TCALL TIDENT TIF expr TEOS
		{ $$ = new AstCtrlCall(@1, $4, $6); }
	;
	
ctrlreturn
	: TCTRL TDOT TRETURN TEOS
		{ $$ = new AstCtrlReturn(@1); }
	;
	
ctrlend
	: TCTRL TDOT TEND TEOS
		{ $$ = new AstCtrlEnd(@1); }
	;

funcdecl
	: TFUNC TDOT TDECL rettype TIDENT TLPAR paramlist TRPAR TEQU TIDENT TEOS
		{ $$ = new AstFuncDecl(@1, $5, $4, $7, $10); }
	| TFUNC TDOT TDECL rettype TIDENT TLPAR paramlist TRPAR TEOS
		{ $$ = new AstFuncDecl(@1, $5, $4, $7); }
	;
	
paramlist
	: /* empty */
		{ $$ = new AstParamList(); }
	| paramlist_sep
		{ $$ = $1; }
	;
	
paramlist_sep
	: paramtype
		{ $$ = new AstParamList(@1); $$->add($1); }
	| paramtype TIDENT
		{ $$ = new AstParamList(@1); $$->add($1); }
	| paramlist_sep TCOMMA paramtype
		{ $1->add($3); $$ = $1; }
	| paramlist_sep TCOMMA paramtype TIDENT
		{ $1->add($3); $$ = $1; }
	;

rettype
	: TANY
		{ $$ = KIML_ANYTYPE; }
	| TBOOL
		{ $$ = KIML_BOOL; }
	| TINT
		{ $$ = KIML_INT; }
	| TREAL
		{ $$ = KIML_REAL; }
	| TSTRING
		{ $$ = KIML_STRING; }
	| TOBJECT
		{ $$ = KIML_OBJECT; }
	;

paramtype
	: TANY
		{ $$ = KIML_ANYTYPE; }
	| TBOOL
		{ $$ = KIML_BOOL; }
	| TINT
		{ $$ = KIML_INT; }
	| TREAL
		{ $$ = KIML_REAL; }
	| TSTRING
		{ $$ = KIML_STRING; }
	| TOBJECT
		{ $$ = KIML_OBJECT; }
	;
	
funccall
	: TFUNC TDOT TCALL TIDENT TLPAR exprlist TRPAR TEOS
		{ $$ = new AstFuncCall(@1, $4, $6); }
	;
	
exprlist
	: /* empty */
		{ $$ = new AstExpressionList(); }
	| exprlist_sep
		{ $$ = $1; }
	;

exprlist_sep
	: expr
		{ $$ = new AstExpressionList(@1); $$->add($1); }
	| exprlist TCOMMA expr
		{ $1->add($3); $$ = $1; }
	;

eventhandle
	: TEVENT TDOT THANDLE TIDENT TBY TIDENT TEOS
		{ $$ = new AstEventHandle(@1, $4, $6); }
	;
	
eventunhandle
	: TEVENT TDOT TUNHANDLE TIDENT TBY TIDENT TEOS
		{ $$ = new AstEventUnhandle(@1, $4, $6); }
	;
	
eventraise
	: TEVENT TDOT TRAISE TIDENT TEOS
		{ $$ = new AstEventRaise(@1, $4); }
	;

label
	: TIDENT TCOLON TEOS
		{ $$ = new AstLabel(@1, $1); }
	;
	
constdef
	: TCONST TIDENT TEQU expr TEOS
		{ $$ = new AstConstDef(@1, $2, $4); }
	;
	
enumdef
	: TENUM identlist TEOS
		{ $$ = new AstEnumDef(@1, $2); }
	;
	
identlist
	: TIDENT
		{ $$ = new AstIdentList(@1); $$->add($1); }
	| identlist TIDENT
		{ $$->add($2); }
	;
	
expr
	: atom
		{ $$ = $1; }
	| binexpr
		{ $$ = $1; }
	| unexpr
		{ $$ = $1; }
	| func
		{ $$ = $1; }
	| cast
		{ $$ = $1; }
	;

atom
	: TIDENT
		{ $$ = new AstIdent(@1, $1); }
	| TLITTRUE
		{ $$ = new AstConstTrue(@1); }
	| TLITFALSE
		{ $$ = new AstConstFalse(@1); }
	| TNULL
		{ $$ = new AstConstNull(@1); }
	| TLITINT
		{ $$ = new AstConstInt(@1, $1); }
	| TLITREAL
		{ $$ = new AstConstReal(@1, $1); }
	| TLITSTRING
		{ $$ = new AstConstString(@1, $1); }
	| TLPAR expr TRPAR
		{ $$ = $2; }
	;
	
binexpr
	: expr TEQU expr
		{ $$ = new AstBinExpr(@1, BINOP_EQU, $1, $3); }
	| expr TNEQ expr
		{ $$ = new AstBinExpr(@1, BINOP_NEQ, $1, $3); }
	| expr TLES expr
		{ $$ = new AstBinExpr(@1, BINOP_LES, $1, $3); }
	| expr TLEQ expr
		{ $$ = new AstBinExpr(@1, BINOP_LEQ, $1, $3); }
	| expr TGRT expr
		{ $$ = new AstBinExpr(@1, BINOP_GRT, $1, $3); }
	| expr TGEQ expr
		{ $$ = new AstBinExpr(@1, BINOP_GEQ, $1, $3); }
	| expr TADD expr
		{ $$ = new AstBinExpr(@1, BINOP_ADD, $1, $3); }
	| expr TSUB expr
		{ $$ = new AstBinExpr(@1, BINOP_SUB, $1, $3); }
	| expr TMUL expr
		{ $$ = new AstBinExpr(@1, BINOP_MUL, $1, $3); }
	| expr TDIV expr
		{ $$ = new AstBinExpr(@1, BINOP_DIV, $1, $3); }
	| expr TMOD expr
		{ $$ = new AstBinExpr(@1, BINOP_MOD, $1, $3); }
	| expr TPOW expr
		{ $$ = new AstBinExpr(@1, BINOP_POW, $1, $3); }
	| expr TCAT expr
		{ $$ = new AstBinExpr(@1, BINOP_CAT, $1, $3); }
	| expr TAND expr
		{ $$ = new AstBinExpr(@1, BINOP_AND, $1, $3); }
	| expr TOR expr
		{ $$ = new AstBinExpr(@1, BINOP_OR, $1, $3); }
	| expr TXOR expr
		{ $$ = new AstBinExpr(@1, BINOP_XOR, $1, $3); }
	;
	
unexpr
	: TNOT expr
		{ $$ = new AstUnExpr(@1, UNOP_NOT, $2); }
	| TSUB expr %prec TNEG
		{ $$ = new AstUnExpr(@1, UNOP_NEG, $2); }
	;
	
func
	: TIDENT TLPAR exprlist TRPAR
		{ $$ = new AstExprFunc(@1, $1, $3); }
	;
	
cast
	: TCINT TLPAR expr TRPAR
		{ $$ = new AstCast(@1, KIML_INT, $3); }
	| TCREAL TLPAR expr TRPAR
		{ $$ = new AstCast(@1, KIML_REAL, $3); }
	| TCOBJECT TLPAR expr TRPAR
		{ $$ = new AstCast(@1, KIML_OBJECT, $3); }
	;
	
%%

void yyerror(YYLTYPE *yylloc, const char *s)
{
	++nSyntaxErrors;
	fprintf(stderr, "%s[%d:%d]: ", szFileName, yylloc->first_line, yylloc->first_column);
	fprintf(stderr, "%s\n", s);
}
