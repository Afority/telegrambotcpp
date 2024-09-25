/*
 * Copyright 1993, 2000 Christopher Seiwald.
 *
 * This file is part of Jam - see jam.c for Copyright information.
 */

/*  This file is ALSO:
 *  Copyright 2022 René Ferdinand Rivera Morell
 *  Copyright 2001-2004 David Abrahams.
 *  Distributed under the Boost Software License, Version 1.0.
 *  (See accompanying file LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)
 */

/*
 * jamgram.yy - jam grammar
 *
 * 04/13/94 (seiwald) - added shorthand L0 for null list pointer
 * 06/01/94 (seiwald) - new 'actions existing' does existing sources
 * 08/23/94 (seiwald) - Support for '+=' (append to variable)
 * 08/31/94 (seiwald) - Allow ?= as alias for "default =".
 * 09/15/94 (seiwald) - if conditionals take only single arguments, so
 *			that 'if foo == bar' gives syntax error (use =).
 * 02/11/95 (seiwald) - when scanning arguments to rules, only treat
 *			punctuation keywords as keywords.  All arg lists
 *			are terminated with punctuation keywords.
 *
 * 09/11/00 (seiwald) - Support for function calls:
 *
 *		Rules now return lists (LIST *), rather than void.
 *
 *		New "[ rule ]" syntax evals rule into a LIST.
 *
 *		Lists are now generated by compile_list() and
 *		compile_append(), and any other rule that indirectly
 *		makes a list, rather than being built directly here,
 *		so that lists values can contain rule evaluations.
 *
 *		New 'return' rule sets the return value, though
 *		other statements also may have return values.
 *
 *		'run' production split from 'block' production so
 *		that empty blocks can be handled separately.
 */

%token ARG STRING

%left `||` `|`
%left `&&` `&`
%left `=` `!=` `in`
%left `<` `<=` `>` `>=`
%left `!`

%{
#include "jam.h"

#include "lists.h"
#include "parse.h"
#include "scan.h"
#include "compile.h"
#include "object.h"
#include "rules.h"

# define YYINITDEPTH 5000 /* for C++ parsing */
# define YYMAXDEPTH 10000	/* for OSF and other less endowed yaccs */

# define F0 -1
static PARSE * P0 = nullptr;
# define S0 (OBJECT *)0

# define pappend( l,r )    	parse_make( PARSE_APPEND,l,r,P0,S0,S0,0 )
# define peval( c,l,r )	    parse_make( PARSE_EVAL,l,r,P0,S0,S0,c )
# define pfor( s,l,r,x )    parse_make( PARSE_FOREACH,l,r,P0,s,S0,x )
# define pif( l,r,t )       parse_make( PARSE_IF,l,r,t,S0,S0,0 )
# define pincl( l )         parse_make( PARSE_INCLUDE,l,P0,P0,S0,S0,0 )
# define plist( s )         parse_make( PARSE_LIST,P0,P0,P0,s,S0,0 )
# define plocal( l,r,t )    parse_make( PARSE_LOCAL,l,r,t,S0,S0,0 )
# define pmodule( l,r )     parse_make( PARSE_MODULE,l,r,P0,S0,S0,0 )
# define pclass( l,r )      parse_make( PARSE_CLASS,l,r,P0,S0,S0,0 )
# define pnull()            parse_make( PARSE_NULL,P0,P0,P0,S0,S0,0 )
# define pon( l,r )         parse_make( PARSE_ON,l,r,P0,S0,S0,0 )
# define prule( s,p )       parse_make( PARSE_RULE,p,P0,P0,s,S0,0 )
# define prules( l,r )      parse_make( PARSE_RULES,l,r,P0,S0,S0,0 )
# define pset( l,r,a )      parse_make( PARSE_SET,l,r,P0,S0,S0,a )
# define pset1( l,r,t,a )   parse_make( PARSE_SETTINGS,l,r,t,S0,S0,a )
# define psetc( s,p,a,l )   parse_make( PARSE_SETCOMP,p,a,P0,s,S0,l )
# define psete( s,l,s1,f )  parse_make( PARSE_SETEXEC,l,P0,P0,s,s1,f )
# define pswitch( l,r )     parse_make( PARSE_SWITCH,l,r,P0,S0,S0,0 )
# define pwhile( l,r )      parse_make( PARSE_WHILE,l,r,P0,S0,S0,0 )
# define preturn( l )       parse_make( PARSE_RETURN,l,P0,P0,S0,S0,0 )
# define pbreak()           parse_make( PARSE_BREAK,P0,P0,P0,S0,S0,0 )
# define pcontinue()        parse_make( PARSE_CONTINUE,P0,P0,P0,S0,S0,0 )

# define pnode( l,r )       parse_make( F0,l,r,P0,S0,S0,0 )
# define psnode( s,l )      parse_make( F0,l,P0,P0,s,S0,0 )

%}

%%

run	: /* empty */
		/* do nothing */
	| rules
		{ parse_save( $1.parse ); }
	;

/*
 * block - zero or more rules
 * rules - one or more rules
 * rule - any one of jam's rules
 * right-recursive so rules execute in order.
 */

block	: null
                { $$.parse = $1.parse; }
	| rules
		{ $$.parse = $1.parse; }
	;

rules	: rule
		{ $$.parse = $1.parse; }
	| rule rules
		{ $$.parse = prules( $1.parse, $2.parse ); }
	| `local` { yymode( SCAN_ASSIGN ); } list assign_list_opt `;` { yymode( SCAN_NORMAL ); } block
		{ $$.parse = plocal( $3.parse, $4.parse, $7.parse ); }
	;

null    : /* empty */
        { $$.parse = pnull(); }
        ;

assign_list_opt : `=` { yymode( SCAN_PUNCT ); } list
                { $$.parse = $3.parse; $$.number = ASSIGN_SET; }
        | null
		{ $$.parse = $1.parse; $$.number = ASSIGN_APPEND; }
        ;

arglist_opt : `(` lol `)`
                { $$.parse = $2.parse; }
        |
                { $$.parse = P0; }
        ;

local_opt : `local`
                { $$.number = 1; }
          | /* empty */
                { $$.number = 0; }
          ;

else_opt : `else` rule
                { $$.parse = $2.parse; }
          | /* empty */
                { $$.parse = pnull(); }

rule	: `{` block `}`
		{ $$.parse = $2.parse; }
	| `include` { yymode( SCAN_PUNCT ); } list `;`
		{ $$.parse = pincl( $3.parse ); yymode( SCAN_NORMAL ); }
	| ARG { yymode( SCAN_PUNCT ); } lol `;`
		{ $$.parse = prule( $1.string, $3.parse ); yymode( SCAN_NORMAL ); }
	| arg assign { yymode( SCAN_PUNCT ); } list `;`
		{ $$.parse = pset( $1.parse, $4.parse, $2.number ); yymode( SCAN_NORMAL ); }
	| arg `on` { yymode( SCAN_ASSIGN ); } list assign { yymode( SCAN_PUNCT ); } list `;`
		{ $$.parse = pset1( $1.parse, $4.parse, $7.parse, $5.number ); yymode( SCAN_NORMAL ); }
	| `return` { yymode( SCAN_PUNCT ); } list `;`
		{ $$.parse = preturn( $3.parse ); yymode( SCAN_NORMAL ); }
    | `break` `;`
        { $$.parse = pbreak(); }
    | `continue` `;`
        { $$.parse = pcontinue(); }
	| `for` local_opt ARG `in` { yymode( SCAN_PUNCT ); } list `{` { yymode( SCAN_NORMAL ); } block `}`
		{ $$.parse = pfor( $3.string, $6.parse, $9.parse, $2.number ); }
	| `switch` { yymode( SCAN_PUNCT ); } list `{` { yymode( SCAN_NORMAL ); } cases `}`
		{ $$.parse = pswitch( $3.parse, $6.parse ); }
	| `if` { yymode( SCAN_CONDB ); } expr `{` { yymode( SCAN_NORMAL ); } block `}` else_opt
		{ $$.parse = pif( $3.parse, $6.parse, $8.parse ); }
	| `module` { yymode( SCAN_PUNCT ); } list `{` { yymode( SCAN_NORMAL ); } block `}`
		{ $$.parse = pmodule( $3.parse, $6.parse ); }
	| `class` { yymode( SCAN_PUNCT ); } lol `{` { yymode( SCAN_NORMAL ); } block `}`
		{ $$.parse = pclass( $3.parse, $6.parse ); }
	| `while` { yymode( SCAN_CONDB ); } expr { yymode( SCAN_NORMAL ); } `{` block `}`
		{ $$.parse = pwhile( $3.parse, $6.parse ); }
     | local_opt `rule` { yymode( SCAN_PUNCT ); } ARG { yymode( SCAN_PARAMS ); } arglist_opt { yymode( SCAN_NORMAL ); } rule
		{ $$.parse = psetc( $4.string, $8.parse, $6.parse, $1.number ); }
	| `on` arg rule
		{ $$.parse = pon( $2.parse, $3.parse ); }
	| `actions` eflags ARG bindlist `{`
		{ yymode( SCAN_STRING ); }
	  STRING
		{ yymode( SCAN_NORMAL ); }
	  `}`
		{ $$.parse = psete( $3.string,$4.parse,$7.string,$2.number ); }
	;

/*
 * assign - = or +=
 */

assign	: `=`
		{ $$.number = ASSIGN_SET; }
	| `+=`
		{ $$.number = ASSIGN_APPEND; }
	| `?=`
		{ $$.number = ASSIGN_DEFAULT; }
	| `default` `=`
		{ $$.number = ASSIGN_DEFAULT; }
	;

/*
 * expr - an expression for if
 */
expr	: arg
		{ $$.parse = peval( EXPR_EXISTS, $1.parse, pnull() ); yymode( SCAN_COND ); }
	| expr `=` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_EQUALS, $1.parse, $4.parse ); }
	| expr `!=` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_NOTEQ, $1.parse, $4.parse ); }
	| expr `<` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_LESS, $1.parse, $4.parse ); }
	| expr `<=` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_LESSEQ, $1.parse, $4.parse ); }
	| expr `>` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_MORE, $1.parse, $4.parse ); }
	| expr `>=` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_MOREEQ, $1.parse, $4.parse ); }
	| expr `&` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_AND, $1.parse, $4.parse ); }
	| expr `&&` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_AND, $1.parse, $4.parse ); }
	| expr `|` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_OR, $1.parse, $4.parse ); }
	| expr `||` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_OR, $1.parse, $4.parse ); }
	| arg `in` { yymode( SCAN_PUNCT ); } list
		{ $$.parse = peval( EXPR_IN, $1.parse, $4.parse ); yymode( SCAN_COND ); }
	| `!` { yymode( SCAN_CONDB ); } expr
		{ $$.parse = peval( EXPR_NOT, $3.parse, pnull() ); }
	| `(` { yymode( SCAN_CONDB ); } expr `)`
		{ $$.parse = $3.parse; }
	;


/*
 * cases - action elements inside a 'switch'
 * case - a single action element inside a 'switch'
 * right-recursive rule so cases can be examined in order.
 */

cases	: /* empty */
		{ $$.parse = P0; }
	| case cases
		{ $$.parse = pnode( $1.parse, $2.parse ); }
	;

case	: `case` { yymode( SCAN_CASE ); } ARG `:` { yymode( SCAN_NORMAL ); } block
		{ $$.parse = psnode( $3.string, $6.parse ); }
	;

/*
 * lol - list of lists
 * right-recursive rule so that lists can be added in order.
 */

lol	: list
		{ $$.parse = pnode( P0, $1.parse ); }
	| list `:` lol
		{ $$.parse = pnode( $3.parse, $1.parse ); }
	;

/*
 * list - zero or more args in a LIST
 * listp - list (in puncutation only mode)
 * arg - one ARG or function call
 */

list	: listp
		{ $$.parse = $1.parse; }
	;

listp	: /* empty */
		{ $$.parse = pnull(); }
	| listp arg
        	{ $$.parse = pappend( $1.parse, $2.parse ); }
	;

arg	: ARG
		{ $$.parse = plist( $1.string ); }
	| `[` { $$.number = yymode( SCAN_CALL ); } func `]`
		{ $$.parse = $3.parse; yymode( $2.number ); }
	;

/*
 * func - a function call (inside [])
 * This needs to be split cleanly out of 'rule'
 */

func	: ARG { yymode( SCAN_PUNCT ); } lol
		{ $$.parse = prule( $1.string, $3.parse ); }
	| `on` arg ARG { yymode( SCAN_PUNCT ); } lol
		{ $$.parse = pon( $2.parse, prule( $3.string, $5.parse ) ); }
	| `on` arg `return` { yymode( SCAN_PUNCT ); } list
		{ $$.parse = pon( $2.parse, $5.parse ); }
	;


/*
 * eflags - zero or more modifiers to 'executes'
 * eflag - a single modifier to 'executes'
 */

eflags	: /* empty */
		{ $$.number = 0; }
	| eflags eflag
		{ $$.number = $1.number | $2.number; }
	;

eflag	: `updated`
		{ $$.number = EXEC_UPDATED; }
	| `together`
		{ $$.number = EXEC_TOGETHER; }
	| `ignore`
		{ $$.number = EXEC_IGNORE; }
	| `quietly`
		{ $$.number = EXEC_QUIETLY; }
	| `piecemeal`
		{ $$.number = EXEC_PIECEMEAL; }
	| `existing`
		{ $$.number = EXEC_EXISTING; }
	;


/*
 * bindlist - list of variable to bind for an action
 */

bindlist : /* empty */
		{ $$.parse = pnull(); }
	| `bind` { yymode( SCAN_PUNCT ); } list
		{ $$.parse = $3.parse; }
	;

