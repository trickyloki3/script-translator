%language "C"
%output "csv_parser.c"
%defines "csv_parser.h"
%verbose
%locations

%define api.prefix {csv}
%define api.token.prefix {CSV_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction accepting
%define lr.keep-unreachable-state false

%define parse.lac full
%define parse.error verbose
%define parse.trace true

%token COMMA NEWLINE STRING
%start file

%code requires {
#include "csv.h"
}

%code provides {
#define YYSTYPE CSVSTYPE
#define YYLTYPE CSVLTYPE
}

%code {
void yyerror(CSVLTYPE *, struct csv *, char const *);
}

%define api.value.type {struct string *}
%parse-param {struct csv * csv}

%%

file :  record {
            if(csv_pop(csv))
                YYABORT;
        }
     |  file NEWLINE record {
            if(csv_pop(csv))
                YYABORT;
        }

record : field
       | record COMMA field

field : %empty {
            if(csv->root && csv_push(csv))
                YYABORT;
        }
      | STRING {
            if(csv_push(csv))
                YYABORT;
        }

%%

void yyerror(CSVLTYPE * location, struct csv * csv, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
