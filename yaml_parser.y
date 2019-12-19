%language "C"
%output "yaml_parser.c"
%defines "yaml_parser.h"
%verbose
%locations

%define api.prefix {yaml}
%define api.token.prefix {yaml_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction accepting
%define lr.keep-unreachable-state false

%define parse.lac full
%define parse.error verbose
%define parse.trace true

%token c_sequence_entry c_mapping_key c_mapping_value
%token c_literal c_folded
%token s_indent s_separate_in_line
%token l_empty b_break
%token nb_char ns_plain_one_line
%start yaml

%code requires {
#include "yaml.h"
}

%code provides {
#define YYSTYPE YAMLSTYPE
#define YYLTYPE YAMLLTYPE
}

%code {
void yyerror(YAMLLTYPE *, struct yaml *, char const *);
}

%define api.value.type {struct yaml_node *}
%parse-param {struct yaml * yaml}

%%

yaml : l_bare_document { if(yaml_document(yaml)) YYABORT; }
     | l_empty_r l_bare_document { if(yaml_document(yaml)) YYABORT; }

l_bare_document : s_l_block_node { $1->scope = 0; if(yaml_block(yaml, $1)) YYABORT; }
                | s_indent s_l_block_node { $2->scope = $1->scope; if(yaml_block(yaml, $2)) YYABORT; }
                | l_bare_document s_l_block_node { $2->scope = 0; if(yaml_block(yaml, $2)) YYABORT; }
                | l_bare_document s_indent s_l_block_node { $3->scope = $2->scope; if(yaml_block(yaml, $3)) YYABORT; }

s_l_block_node : ns_plain
               | l_block_scalar
               | l_block_sequence
               | l_block_mapping

ns_plain : ns_plain_one_line s_l_comments

l_block_scalar : { if(yaml_stack(yaml, yaml_c_literal)) YYABORT; } c_literal s_l_comments
               | { if(yaml_stack(yaml, yaml_c_folded)) YYABORT; } c_folded s_l_comments
               | nb_char s_l_comments { $1->child = $2; $$ = $1; }

l_block_sequence : { if(yaml_stack(yaml, yaml_c_sequence_entry)) YYABORT; else $$ = yaml->stack; } c_sequence_entry s_separate s_l_block_node {
    $1->value = $4->value;
    $$ = $2;
}

l_block_mapping : ns_plain_one_line { if(yaml_stack(yaml, yaml_c_mapping_value)) YYABORT; else $$ = yaml->stack; } c_mapping_value s_separate s_l_block_node {
    if($4->type != yaml_s_indent && ($5->type == yaml_c_sequence_entry || $5->type == yaml_c_mapping_value)) {
        YYABORT; /* map does not support compact notation */
    } else {
        $2->key = $1->value;
        $2->value = $5->value;
        $$ = $3;
    }
}

s_separate : s_separate_in_line { $$ = $1; }
           | s_l_comments s_indent { $$ = $2; }

s_l_comments : b_break
             | b_break l_empty_r { $$ = $2; $$->scope++; }

l_empty_r : l_empty
          | l_empty_r l_empty { $$ = $1; $$->scope++; }

%%

void yyerror(YAMLLTYPE * location, struct yaml * yaml, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
