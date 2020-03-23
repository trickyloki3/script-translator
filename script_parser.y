%language "C"
%output "script_parser.c"
%defines "script_parser.h"
%verbose

%define api.prefix {script}
%define api.token.prefix {script_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction most
%define lr.keep-unreachable-state false

%define parse.lac none
%define parse.error simple
%define parse.trace false

%token curly_open curly_close semicolon
%token integer identifier
%token for if else
%left comma
%right assign plus_assign minus_assign
%right question colon
%left or
%left and
%left bit_or
%left bit_xor
%left bit_and
%left equal not_equal
%left lesser lesser_equal greater greater_equal
%left bit_left bit_right
%left plus minus
%left multiply divide remainder
%right increment_prefix decrement_prefix plus_unary minus_unary not bit_not
%left increment_postfix decrement_postfix round_open round_close square_open square_close
%start script

%code requires {
#include "script.h"
}

%code provides {
#define YYSTYPE SCRIPTSTYPE

struct script_node * script_node_create(struct store *, int);
void script_node_push(struct script_node *, ...);
struct script_node * script_node_reverse(struct script_node *);
void script_node_print(struct script_node *);
}

%code {
void yyerror(struct script *, char const *);
}

%define api.value.type {struct script_node *}
%parse-param {struct script * script}

%%

script  : statement_block {
              script->root = $$;
          }

statement_block : statement {
                      $$ = script_node_create(&script->store, script_curly_open);
                      if(!$$) {
                          YYABORT;
                      } else {
                          $$->root = $1;
                      }
                  }
                | curly_open curly_close
                | curly_open statement_list curly_close {
                      $1->root = script_node_reverse($2);
                      $$ = $1;
                  }

statement_list  : statement
                | statement_list statement {
                      $2->next = $1;
                      $$ = $2;
                  }

statement : semicolon
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
                  $$->token = script_increment_postfix;
              }
            | identifier decrement_prefix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
                  $$->token = script_decrement_postfix;
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
                  $$->token = script_increment_postfix;
              }
            | expression decrement_prefix %prec decrement_postfix {
                  script_node_push($2, $1, NULL);
                  $$ = $2;
                  $$->token = script_decrement_postfix;
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
                  $$->token = script_plus_unary;
              }
            | minus expression %prec minus_unary {
                  script_node_push($1, $2, NULL);
                  $$ = $1;
                  $$->token = script_minus_unary;
              }
            | not expression {
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
            | expression equal expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression not_equal expression {
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
            | expression and expression {
                  script_node_push($2, $3, $1, NULL);
                  $$ = $2;
              }
            | expression or expression {
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

void yyerror(struct script * script, char const * message) {
    panic("%s", message);
}

struct script_node * script_node_create(struct store * store, int token) {
    struct script_node * node;

    node = store_malloc(store, sizeof(*node));
    if(node) {
        node->token = token;
        node->identifier = NULL;
        node->root = NULL;
        node->next = NULL;
    }

    return node;
}

void script_node_push(struct script_node * root, ...) {
    va_list args;
    struct script_node * node;

    va_start(args, root);
    node = va_arg(args, struct script_node *);
    while(node) {
        node->next = root->root;
        root->root = node;
        node = va_arg(args, struct script_node *);
    }
    va_end(args);
}

struct script_node * script_node_reverse(struct script_node * root) {
    struct script_node * list;
    struct script_node * node;

    list = NULL;
    while(root) {
        node = root;
        root = root->next;
        node->next = list;
        list = node;
    }

    return list;
}

void script_node_print(struct script_node * root) {
    struct script_node * iter;

    switch(root->token) {
        case script_curly_open:
            iter = root->root;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_integer:
            fprintf(stdout, "%ld ", root->integer);
            break;
        case script_identifier:
            fprintf(stdout, "%s ", root->identifier);
            if(root->root) {
                fprintf(stdout, "( ");
                iter = root->root;
                while(iter) {
                    script_node_print(iter);
                    iter = iter->next;
                }
                fprintf(stdout, ") ");
            }
            break;
        case script_for:
            fprintf(stdout, "for ");
            iter = root->root;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_if:
            fprintf(stdout, "if ");
            iter = root->root;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_else:
            fprintf(stdout, "else ");
            iter = root->root;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_comma:
            script_node_print(root->root);
            fprintf(stdout, ", ");
            script_node_print(root->root->next);
            break;
        case script_assign:
            script_node_print(root->root);
            fprintf(stdout, "= ");
            script_node_print(root->root->next);
            break;
        case script_plus_assign:
            script_node_print(root->root);
            fprintf(stdout, "+= ");
            script_node_print(root->root->next);
            break;
        case script_minus_assign:
            script_node_print(root->root);
            fprintf(stdout, "-= ");
            script_node_print(root->root->next);
            break;
        case script_question:
            script_node_print(root->root);
            fprintf(stdout, "? ");
            script_node_print(root->root->next);
            break;
        case script_colon:
            script_node_print(root->root);
            fprintf(stdout, ": ");
            script_node_print(root->root->next);
            break;
        case script_or:
            script_node_print(root->root);
            fprintf(stdout, "|| ");
            script_node_print(root->root->next);
            break;
        case script_and:
            script_node_print(root->root);
            fprintf(stdout, "&& ");
            script_node_print(root->root->next);
            break;
        case script_bit_or:
            script_node_print(root->root);
            fprintf(stdout, "| ");
            script_node_print(root->root->next);
            break;
        case script_bit_xor:
            script_node_print(root->root);
            fprintf(stdout, "^ ");
            script_node_print(root->root->next);
            break;
        case script_bit_and:
            script_node_print(root->root);
            fprintf(stdout, "& ");
            script_node_print(root->root->next);
            break;
        case script_equal:
            script_node_print(root->root);
            fprintf(stdout, "== ");
            script_node_print(root->root->next);
            break;
        case script_not_equal:
            script_node_print(root->root);
            fprintf(stdout, "!= ");
            script_node_print(root->root->next);
            break;
        case script_lesser:
            script_node_print(root->root);
            fprintf(stdout, "< ");
            script_node_print(root->root->next);
            break;
        case script_lesser_equal:
            script_node_print(root->root);
            fprintf(stdout, "<= ");
            script_node_print(root->root->next);
            break;
        case script_greater:
            script_node_print(root->root);
            fprintf(stdout, "> ");
            script_node_print(root->root->next);
            break;
        case script_greater_equal:
            script_node_print(root->root);
            fprintf(stdout, ">= ");
            script_node_print(root->root->next);
            break;
        case script_bit_left:
            script_node_print(root->root);
            fprintf(stdout, "<< ");
            script_node_print(root->root->next);
            break;
        case script_bit_right:
            script_node_print(root->root);
            fprintf(stdout, ">> ");
            script_node_print(root->root->next);
            break;
        case script_plus:
            script_node_print(root->root);
            fprintf(stdout, "+ ");
            script_node_print(root->root->next);
            break;
        case script_minus:
            script_node_print(root->root);
            fprintf(stdout, "- ");
            script_node_print(root->root->next);
            break;
        case script_multiply:
            script_node_print(root->root);
            fprintf(stdout, "* ");
            script_node_print(root->root->next);
            break;
        case script_divide:
            script_node_print(root->root);
            fprintf(stdout, "/ ");
            script_node_print(root->root->next);
            break;
        case script_remainder:
            script_node_print(root->root);
            fprintf(stdout, "%% ");
            script_node_print(root->root->next);
            break;
        case script_increment_prefix:
            fprintf(stdout, "++ ");
            script_node_print(root->root);
            break;
        case script_decrement_prefix:
            fprintf(stdout, "-- ");
            script_node_print(root->root);
            break;
        case script_plus_unary:
            fprintf(stdout, "+ ");
            script_node_print(root->root);
            break;
        case script_minus_unary:
            fprintf(stdout, "- ");
            script_node_print(root->root);
            break;
        case script_not:
            fprintf(stdout, "! ");
            script_node_print(root->root);
            break;
        case script_bit_not:
            fprintf(stdout, "~ ");
            script_node_print(root->root);
            break;
        case script_increment_postfix:
            script_node_print(root->root);
            fprintf(stdout, "++ ");
            break;
        case script_decrement_postfix:
            script_node_print(root->root);
            fprintf(stdout, "-- ");
            break;
    }
}
