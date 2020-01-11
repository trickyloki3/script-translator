%language "C"
%output "script_parser.c"
%defines "script_parser.h"
%verbose
%locations

%define api.prefix {script}
%define api.token.prefix {script_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction accepting
%define lr.keep-unreachable-state false

%define parse.lac full
%define parse.error verbose
%define parse.trace true

%left increment_postfix decrement_postfix round_open round_close square_open square_close
%right increment_prefix decrement_prefix plus_unary minus_unary logic_not bit_not
%left multiply divide remainder
%left plus minus
%left bit_left bit_right
%left lesser lesser_equal greater greater_equal
%left logic_equal logic_not_equal
%left bit_and
%left bit_xor
%left bit_or
%left logic_and
%left logic_or
%right question colon
%right assign plus_assign minus_assign
%left comma
%precedence for if else
%token curly_open curly_close semicolon
%precedence integer identifier
%start script

%code requires {
#include "script.h"
}

%code provides {
#define YYSTYPE SCRIPTSTYPE
#define YYLTYPE SCRIPTLTYPE
}

%code {
void yyerror(SCRIPTLTYPE *, struct script *, char const *);
}

%define api.value.type {struct script_node *}
%parse-param {struct script * script}

%%

script : statement_block
       | script comma statement_block

statement_block : statement
                | curly_open curly_close
                | curly_open statement_list curly_close

statement_list : statement
               | statement_list statement

statement : semicolon
          | if_statement
          | for_statement
          | expression_statement

if_statement : if round_open expression round_close statement_block
             | if round_open expression round_close statement_block else statement_block

for_statement : for round_open expression semicolon expression semicolon expression round_close statement_block

expression_statement : expression semicolon

expression : expression increment_prefix %prec increment_postfix
           | expression decrement_prefix %prec decrement_postfix
           | round_open round_close
           | round_open expression round_close
           | square_open expression square_close
           | increment_prefix expression
           | decrement_prefix expression
           | plus expression %prec plus_unary
           | minus expression %prec minus_unary
           | logic_not expression
           | bit_not expression
           | expression multiply expression
           | expression divide expression
           | expression remainder expression
           | expression plus expression
           | expression minus expression
           | expression bit_left expression
           | expression bit_right expression
           | expression lesser expression
           | expression lesser_equal expression
           | expression greater expression
           | expression greater_equal expression
           | expression logic_equal expression
           | expression logic_not_equal expression
           | expression bit_and expression
           | expression bit_xor expression
           | expression bit_or expression
           | expression logic_and expression
           | expression logic_or expression
           | expression question expression
           | expression colon expression
           | expression assign expression
           | expression plus_assign expression
           | expression minus_assign expression
           | expression comma expression
           | integer
           | identifier
           | identifier expression

%%

void yyerror(SCRIPTLTYPE * location, struct script * script, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
