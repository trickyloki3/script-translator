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
%token double_quote_start double_quote_next double_quote_end
%token s_flow_line_prefix
%token nb_single_one_line
%token single_quote_start single_quote_next single_quote_end
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

yaml : l_directive_document c_double_quoted m_l_empty
     | l_directive_document c_single_quoted m_l_empty


l_directive_document  : l_directive
                      | l_directive_document l_directive

l_directive : ns_yaml_directive b_break
            | ns_tag_directive b_break
            | ns_reserve_directive b_break

c_double_quoted : nb_double_one_line b_break
                | nb_double_multi_line b_break

nb_double_multi_line    :   double_quote_start double_quote_end
                        |   double_quote_start s_separate_in_line double_quote_end
                        |   double_quote_start s_double_next_line double_quote_end

s_double_next_line  : s_double_break
                    | s_double_break double_quote_next
                    | s_double_break double_quote_next s_separate_in_line
                    | s_double_break double_quote_next s_double_next_line

c_single_quoted :   nb_single_one_line b_break
                |   nb_single_multi_line b_break

nb_single_multi_line :  single_quote_start single_quote_end
                     |  single_quote_start s_separate_in_line single_quote_end
                     |  single_quote_start s_single_next_line single_quote_end

s_single_next_line  :   s_double_break
                    |   s_double_break single_quote_next
                    |   s_double_break single_quote_next s_separate_in_line
                    |   s_double_break single_quote_next s_single_next_line

s_double_break :    b_break m_l_empty
               |    b_break m_l_empty s_flow_line_prefix

m_l_empty   :   %empty
            |   l_empty
            |   m_l_empty l_empty

%%

void yyerror(YAMLLTYPE * location, struct yaml * yaml, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
