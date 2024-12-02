%{
  #include <kiml/kiml.h>
	#include <stdio.h>
  
	class AstProgram;
	AstProgram *root;
	
	union YYSTYPE;
	struct YYLTYPE;
	
	int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);
	
	void yyerror(YYLTYPE *yylloc, const char *s);
	
	KIMLUINT nSyntaxErrors = 0;
	KIMLCSTRING szFileName = "";
%}

%code requires {
	#include "ast.h"
}

%locations
%define api.pure
%error-verbose

%defines "simpl.parser.h"
%no-lines
%output "simpl.parser.cpp"

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

%token TIF TELSE TWHILE TDO TFOR TBREAK TCONTINUE TRETURN TVAR TCONST TENUM TEXTERN

%token TDOT TCOLON TCOMMA
%token TIDENT TLITTRUE TLITFALSE TNULL TLITINT TLITREAL TLITSTRING
%token TLPAR TRPAR TLCUR TRCUR TLSQU TRSQU TEQU TNEQ TLES TLEQ TGRT TGEQ TADD TSUB TMUL TDIV TMOD TAND TOR TXOR TNOT TASN
%token TANY TBOOL TINT TREAL TSTRING TOBJECT
%token TEOS

%type <svalue> TIDENT TLITSTRING
%type <bvalue> TLITTRUE TLITFALSE
%type <ivalue> TLITINT
%type <rvalue> TLITREAL

%type <program> program
%type <stmt> stmt func func_extern if while do break continue return exprstmt var const enum
%type <stmtlist> stmtlist stmtblock
%type <expr> expr binexpr unexpr funccall cast atom
%type <exprlist> exprlist exprlist_sep
%type <datatype> datatype
%type <paramlist> paramlist paramlist_sep
%type <identlist> identlist identlist_sep

%left TOR
%left TXOR
%left TAND
%left TEQU TNEQ
%left TLES TLEQ TGRT TGEQ
%left TADD TSUB
%left TMUL TDIV TMOD
%right TASN
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

stmtblock
  : TLCUR TRCUR
    { $$ = new AstStatementList(@1); }
  | TLCUR stmtlist TRCUR
    { $$ = $2; }
  ;
  
stmt
	: TEOS
		{ $$ = nullptr; }
  | stmtblock
    { $$ = $1; }
  | func
    { $$ = $1; }
  | func_extern
    { $$ = $1; }
	| if
		{ $$ = $1; }
	| while
		{ $$ = $1; }
	| do
		{ $$ = $1; }
	| break
		{ $$ = $1; }
	| continue
		{ $$ = $1; }
	| return
		{ $$ = $1; }
  | exprstmt
    { $$ = $1; }
	| var
		{ $$ = $1; }
	| const
		{ $$ = $1; }
	| enum
		{ $$ = $1; }
	| error TEOS
		{ $$ = nullptr; yyerrok; }
	;

func
  : datatype TIDENT TLPAR paramlist TRPAR stmtblock
    { $$ = new AstFunc(@1, $1, $2, $4, $6); }
  ;

func_extern
  : TEXTERN datatype TIDENT TLPAR paramlist TRPAR TEOS
    { $$ = new AstFunc(@1, $2, $3, $5, nullptr); }
  ;
  
if
  : TIF TLPAR expr TRPAR stmt
    { $$ = new AstIf(@1, $3, $5, nullptr); }
  | TIF TLPAR expr TRPAR stmt TELSE stmt
    { $$ = new AstIf(@1, $3, $5, $7); }
  ;

while
  : TWHILE TLPAR expr TRPAR stmt
    { $$ = new AstWhile(@1, $3, $5); }
  ;

do
  : TDO stmt TWHILE TLPAR expr TRPAR
    { $$ = new AstWhile(@1, $5, $2); }
  ;

break
  : TBREAK TEOS
    { $$ = new AstBreak(@1); }
  ;

continue
  : TCONTINUE TEOS
    { $$ = new AstContinue(@1); }
  ;

return
  : TRETURN TEOS
    { $$ = new AstReturn(@1, nullptr); }
  | TRETURN expr TEOS
    { $$ = new AstReturn(@1, $2); }
  ;

exprstmt
  : expr
    { $$ = new AstExprStatement(@1, $1); }
  ;
  
var
  : datatype TIDENT TEOS
    { $$ = new AstVar(@1, $1, $2, nullptr); }
  | datatype TIDENT TASN expr TEOS
    { $$ = new AstVar(@1, $1, $2, $4); }
  ;

const
	: TCONST datatype TIDENT TEQU expr TEOS
		{ $$ = new AstConst(@1, $2, $3, $5); }
	;
	
enum
	: TENUM TLCUR identlist TRCUR TEOS
		{ $$ = new AstEnum(@1, $3); }
	;
	
paramlist
	: /* empty */
		{ $$ = new AstParamList(); }
	| paramlist_sep
		{ $$ = $1; }
	;
	
paramlist_sep
	: datatype TIDENT
		{ $$ = new AstParamList(@1); $$->add($1, $2); }
	| paramlist_sep TCOMMA datatype TIDENT
		{ $1->add($3, $4); $$ = $1; }
	;

datatype
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
	
identlist
	: /* empty */
    { $$ = new AstIdentList(); }
  | identlist_sep
    { $$ = $1; }
	;
  
identlist_sep
  : TIDENT
    { $$ = new AstIdentList(@1); $$->add($1); }
  | identlist TCOMMA TIDENT
    { $1->add($3); $$ = $1; }
  ;
	
expr
	: atom
		{ $$ = $1; }
	| binexpr
		{ $$ = $1; }
	| unexpr
		{ $$ = $1; }
	| funccall
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
	| expr TAND expr
		{ $$ = new AstBinExpr(@1, BINOP_AND, $1, $3); }
	| expr TOR expr
		{ $$ = new AstBinExpr(@1, BINOP_OR, $1, $3); }
	| expr TXOR expr
		{ $$ = new AstBinExpr(@1, BINOP_XOR, $1, $3); }
	| expr TASN expr
		{ $$ = new AstBinExpr(@1, BINOP_ASN, $1, $3); }
	;
	
unexpr
	: TNOT expr
		{ $$ = new AstUnExpr(@1, UNOP_NOT, $2); }
	| TSUB expr %prec TNEG
		{ $$ = new AstUnExpr(@1, UNOP_NEG, $2); }
	;
	
funccall
	: TIDENT TLPAR exprlist TRPAR
		{ $$ = new AstExprFunc(@1, $1, $3); }
	;
  
cast
  : TLPAR datatype TRPAR expr
    { $$ = new AstCast(@1, $2, $4); }
  ;
	
%%

void yyerror(YYLTYPE *yylloc, const char *s)
{
	++nSyntaxErrors;
	fprintf(stderr, "%s[%d:%d]: ", szFileName, yylloc->first_line, yylloc->first_column);
	fprintf(stderr, "%s\n", s);
}
