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

script :  statement_block {
              struct script_node * node;

              node = script_node_create(script, block);
              if(!node) {
                  YYABORT;
              } else {
                  script_node_push(node, $1, NULL);
                  $$ = script->state->root = node;
              }
          }
       |  script comma statement_block {
              struct script_node * node;

              node = script_node_create(script, block);
              if(!node) {
                  YYABORT;
              } else {
                  script_node_push(node, $3, NULL);
                  $$ = $1->next = node;
              }
          }

statement_block : statement
                | curly_open curly_close {
                      $$ = script_node_create(script, null);
                      if(!$$)
                          YYABORT;
                  }
                | curly_open statement_list curly_close {
                      $$ = script_node_reverse($2);
                  }

statement_list :  statement
               |  statement_list statement {
                      $2->next = $1;
                      $$ = $2;
                  }

statement : semicolon {
                $$ = script_node_create(script, null);
                if(!$$)
                    YYABORT;
            }
          | if_statement
          | for_statement
          | expression semicolon

if_statement :  if round_open expression round_close statement_block {
                    script_node_push($1, $5, $3, NULL);
                    $$ = $1;
                }
             |  if round_open expression round_close statement_block else statement_block {
                    script_node_push($6, $7, $5, $3, NULL);
                    $$ = $6;
                }

for_statement : for round_open expression semicolon expression semicolon expression round_close statement_block {
                    script_node_push($1, $9, $7, $5, $3, NULL);
                    $$ = $1;
                }

expression :  expression increment_prefix %prec increment_postfix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
           |  expression decrement_prefix %prec decrement_postfix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
           |  round_open round_close {
                  $$ = script_node_create(script, null);
                  if(!$$)
                      YYABORT;
              }
           |  round_open expression round_close {
                  $$ = $2;
              }
           |  square_open expression square_close {
                  $$ = $2;
              }
           |  increment_prefix expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  decrement_prefix expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  plus expression %prec plus_unary {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  minus expression %prec minus_unary {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  logic_not expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  bit_not expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
           |  expression multiply expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression divide expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression remainder expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression plus expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression minus expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression bit_left expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression bit_right expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression lesser expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression lesser_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression greater expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression greater_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression logic_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression logic_not_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression bit_and expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression bit_xor expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression bit_or expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression logic_and expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression logic_or expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression question expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression colon expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression plus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression minus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  expression comma expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
           |  integer
           |  identifier
           |  identifier expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }

%%

void yyerror(SCRIPTLTYPE * location, struct script * script, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
