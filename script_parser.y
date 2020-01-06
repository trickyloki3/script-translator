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

script : %empty

%%

void yyerror(SCRIPTLTYPE * location, struct script * script, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
