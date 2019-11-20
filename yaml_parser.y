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
%token s_indent s_separate_in_line
%token b_break l_empty
%token ns_yaml_directive ns_tag_directive ns_reserve_directive
%token c_ns_tag_property c_ns_anchor_property c_ns_alias_node
%token nb_double_one_line
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

yaml : l_directive_document

l_directive_document  : l_directive
                      | l_directive_document l_directive

l_directive : ns_yaml_directive b_break
            | ns_tag_directive b_break
            | ns_reserve_directive b_break

%%

void yyerror(YAMLLTYPE * location, struct yaml * yaml, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
