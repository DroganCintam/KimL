
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

#include "ast.h"

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TIO = 258,
     TVAR = 259,
     TSTACK = 260,
     TTAPE = 261,
     TCTRL = 262,
     TFUNC = 263,
     TEVENT = 264,
     TCONST = 265,
     TENUM = 266,
     TIN = 267,
     TOUT = 268,
     TDECL = 269,
     TDEL = 270,
     TSET = 271,
     TPUSH = 272,
     TPOP = 273,
     TPEEK = 274,
     TCLEAR = 275,
     TSWAP = 276,
     TREAD = 277,
     TWRITE = 278,
     TNEXT = 279,
     TPREV = 280,
     TMOVE = 281,
     TGOTO = 282,
     TCALL = 283,
     TRETURN = 284,
     TEND = 285,
     THANDLE = 286,
     TUNHANDLE = 287,
     TRAISE = 288,
     TIF = 289,
     TAT = 290,
     TBY = 291,
     TDOT = 292,
     TCOLON = 293,
     TCOMMA = 294,
     TIDENT = 295,
     TLITTRUE = 296,
     TLITFALSE = 297,
     TNULL = 298,
     TLITINT = 299,
     TLITREAL = 300,
     TLITSTRING = 301,
     TLPAR = 302,
     TRPAR = 303,
     TEQU = 304,
     TNEQ = 305,
     TLES = 306,
     TLEQ = 307,
     TGRT = 308,
     TGEQ = 309,
     TADD = 310,
     TSUB = 311,
     TMUL = 312,
     TDIV = 313,
     TMOD = 314,
     TPOW = 315,
     TCAT = 316,
     TAND = 317,
     TOR = 318,
     TXOR = 319,
     TNOT = 320,
     TCINT = 321,
     TCREAL = 322,
     TCOBJECT = 323,
     TANY = 324,
     TBOOL = 325,
     TINT = 326,
     TREAL = 327,
     TSTRING = 328,
     TOBJECT = 329,
     TEOS = 330,
     TNEG = 331
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



