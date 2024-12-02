
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* "%code requires" blocks.  */


	#include "ast.h"




/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TIF = 258,
     TELSE = 259,
     TWHILE = 260,
     TDO = 261,
     TFOR = 262,
     TBREAK = 263,
     TCONTINUE = 264,
     TRETURN = 265,
     TVAR = 266,
     TCONST = 267,
     TENUM = 268,
     TEXTERN = 269,
     TDOT = 270,
     TCOLON = 271,
     TCOMMA = 272,
     TIDENT = 273,
     TLITTRUE = 274,
     TLITFALSE = 275,
     TNULL = 276,
     TLITINT = 277,
     TLITREAL = 278,
     TLITSTRING = 279,
     TLPAR = 280,
     TRPAR = 281,
     TLCUR = 282,
     TRCUR = 283,
     TLSQU = 284,
     TRSQU = 285,
     TEQU = 286,
     TNEQ = 287,
     TLES = 288,
     TLEQ = 289,
     TGRT = 290,
     TGEQ = 291,
     TADD = 292,
     TSUB = 293,
     TMUL = 294,
     TDIV = 295,
     TMOD = 296,
     TAND = 297,
     TOR = 298,
     TXOR = 299,
     TNOT = 300,
     TASN = 301,
     TANY = 302,
     TBOOL = 303,
     TINT = 304,
     TREAL = 305,
     TSTRING = 306,
     TOBJECT = 307,
     TEOS = 308,
     TNEG = 309
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union 
YYSTYPE
{


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



} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



