%language "C"
%output "json_parser.c"
%defines "json_parser.h"
%verbose
%locations

%define api.prefix {json}
%define api.token.prefix {JSON_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction accepting
%define lr.keep-unreachable-state false

%define parse.lac full
%define parse.error verbose
%define parse.trace true

%token BEGINARRAY BEGINOBJECT ENDARRAY ENDOBJECT NAMESEPARATOR VALUESEPARATOR FALSE NULL TRUE NUMBER STRING
%start json

%code requires {
#include "json.h"
}

%code provides {
#define YYSTYPE JSONSTYPE
#define YYLTYPE JSONLTYPE
}

%code {
void yyerror(JSONLTYPE *, char const *);
}

%%

json : value

object : BEGINOBJECT ENDOBJECT
       | BEGINOBJECT member ENDOBJECT

member : field
       | member VALUESEPARATOR field

field : STRING NAMESEPARATOR value

array : BEGINARRAY ENDARRAY
      | BEGINARRAY element ENDARRAY

element : value
        | element VALUESEPARATOR value

value : FALSE
      | NULL
      | TRUE
      | object
      | array
      | NUMBER
      | STRING

%%

void yyerror(JSONLTYPE * location, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
