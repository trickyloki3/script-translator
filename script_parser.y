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

%token curly_open curly_close semicolon
%token integer identifier
%token for if else
%left comma
%right assign plus_assign minus_assign
%right question colon
%left logic_or
%left logic_and
%left bit_or
%left bit_xor
%left bit_and
%left logic_equal logic_not_equal
%left lesser lesser_equal greater greater_equal
%left bit_left bit_right
%left plus minus
%left multiply divide remainder
%right increment_prefix decrement_prefix plus_unary minus_unary logic_not bit_not
%left increment_postfix decrement_postfix round_open round_close square_open square_close
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

script  : statement_block {
              script->state->root = $1;
              $$ = $1;
          }
        | script comma statement_block {
              $1->next = $3;
              $$ = $3;
          }

statement_block : statement {
                      if(script_node_block(script, &$$)) {
                          YYABORT;
                      } else {
                          $$->node = $1;
                      }
                  }
                | curly_open curly_close {
                      if(script_node_token(script, 0, &$$))
                          YYABORT;
                  }
                | curly_open statement_list curly_close {
                      if(script_node_block(script, &$$)) {
                          YYABORT;
                      } else {
                          $$->node = script_node_flip($2);
                      }
                  }

statement_list  : statement
                | statement_list statement {
                      $2->next = $1;
                      $$ = $2;
                  }

statement : semicolon {
                if(script_node_token(script, 0, &$$))
                    YYABORT;
            }
          | if_statement
          | for_statement
          | assignment semicolon

if_statement  : if round_open expression round_close statement_block {
                    script_node_push($1, $5, $3, NULL);
                    $$ = $1;
                }
              | if round_open expression round_close statement_block else statement_block {
                    script_node_push($6, $7, $5, $3, NULL);
                    $$ = $6;
                }

for_statement : for round_open expression semicolon expression semicolon expression round_close statement_block {
                    script_node_push($1, $9, $7, $5, $3, NULL);
                    $$ = $1;
                }

assignment  : identifier
            | identifier round_open round_close
            | identifier increment_prefix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
            | identifier decrement_prefix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
            | identifier assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | identifier plus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | identifier minus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | identifier expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }

expression  : expression increment_prefix %prec increment_postfix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
            | expression decrement_prefix %prec decrement_postfix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
              }
            | identifier round_open round_close {
                  $$ = $1;
              }
            | round_open expression round_close {
                  $$ = $2;
              }
            | identifier round_open expression round_close {
                  script_node_push($1, $3, NULL);
                  $$ = $1;
              }
            | identifier square_open expression square_close {
                  script_node_push($1, $3, NULL);
                  $$ = $1;
              }
            | increment_prefix expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | decrement_prefix expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | plus expression %prec plus_unary {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | minus expression %prec minus_unary {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | logic_not expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | bit_not expression {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
              }
            | expression multiply expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression divide expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression remainder expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression plus expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression minus expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression bit_left expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression bit_right expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression lesser expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression lesser_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression greater expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression greater_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression logic_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression logic_not_equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression bit_and expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression bit_xor expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression bit_or expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression logic_and expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression logic_or expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression question expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression colon expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression plus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression minus_assign expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression comma expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | integer
            | identifier

%%

void yyerror(SCRIPTLTYPE * location, struct script * script, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
