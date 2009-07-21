
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison GLR parsers in C
   
      Copyright (C) 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
   
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


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TAG_IDENTIFIER = 258,
     TAG_SORT_EXPR = 259,
     TAG_DATA_EXPR = 260,
     TAG_DATA_SPEC = 261,
     TAG_MULT_ACT = 262,
     TAG_PROC_EXPR = 263,
     TAG_PROC_SPEC = 264,
     TAG_PBES_SPEC = 265,
     TAG_STATE_FRM = 266,
     TAG_DATA_VARS = 267,
     TAG_ACTION_RENAME = 268,
     LMERGE = 269,
     ARROW = 270,
     LTE = 271,
     GTE = 272,
     CONS = 273,
     SNOC = 274,
     CONCAT = 275,
     EQ = 276,
     NEQ = 277,
     AND = 278,
     BARS = 279,
     IMP = 280,
     BINIT = 281,
     ELSE = 282,
     SLASH = 283,
     STAR = 284,
     PLUS = 285,
     MINUS = 286,
     EQUALS = 287,
     DOT = 288,
     COMMA = 289,
     COLON = 290,
     SEMICOLON = 291,
     QMARK = 292,
     EXCLAM = 293,
     AT = 294,
     HASH = 295,
     BAR = 296,
     LPAR = 297,
     RPAR = 298,
     PBRACK = 299,
     LBRACK = 300,
     RBRACK = 301,
     LANG = 302,
     RANG = 303,
     PBRACE = 304,
     LBRACE = 305,
     RBRACE = 306,
     KWSORT = 307,
     KWCONS = 308,
     KWMAP = 309,
     KWVAR = 310,
     KWEQN = 311,
     KWACT = 312,
     KWGLOB = 313,
     KWPROC = 314,
     KWPBES = 315,
     KWINIT = 316,
     KWSTRUCT = 317,
     BOOL = 318,
     POS = 319,
     NAT = 320,
     INT = 321,
     REAL = 322,
     LIST = 323,
     SET = 324,
     BAG = 325,
     CTRUE = 326,
     CFALSE = 327,
     DIV = 328,
     MOD = 329,
     IN = 330,
     LAMBDA = 331,
     FORALL = 332,
     EXISTS = 333,
     WHR = 334,
     END = 335,
     DELTA = 336,
     TAU = 337,
     SUM = 338,
     BLOCK = 339,
     ALLOW = 340,
     HIDE = 341,
     RENAME = 342,
     COMM = 343,
     VAL = 344,
     MU = 345,
     NU = 346,
     DELAY = 347,
     YALED = 348,
     NIL = 349,
     ID = 350,
     NUMBER = 351
   };
#endif


#ifndef YYSTYPE
typedef union YYSTYPE
{

/* Line 2638 of glr.c  */
#line 79 "mcrl2parser.yy"

  ATerm term;
  ATermAppl appl;
  ATermList list;



/* Line 2638 of glr.c  */
#line 154 "../include/mcrl2/core/detail/mcrl2parser.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
#endif

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{

  char yydummy;

} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



extern YYSTYPE mcrl2yylval;



