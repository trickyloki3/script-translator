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

yaml : s_indent s_l_block_node
     | yaml s_indent s_l_block_node

s_l_block_node : ns_plain_one_line s_l_comments
               | c_literal s_separate nb_char s_l_comments
               | c_folded s_separate nb_char s_l_comments
               | nb_char s_l_comments
               | l_block_sequence
               | l_block_mapping

l_block_sequence : c_sequence_entry s_l_block_indented
                 | l_block_sequence c_sequence_entry s_l_block_indented

s_l_block_indented : s_separate_in_line l_block_mapping
                   | s_l_block_node

l_block_mapping : ns_l_block_map_implicit_entry
                | l_block_mapping ns_l_block_map_implicit_entry

ns_l_block_map_implicit_entry : ns_plain_one_line c_mapping_value s_separate s_l_block_node

s_separate : s_separate_in_line
           | s_l_comments s_indent

s_l_comments : b_break
             | b_break l_empty_r

l_empty_r : l_empty
          | l_empty_r l_empty

%%

void yyerror(YAMLLTYPE * location, struct yaml * yaml, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
