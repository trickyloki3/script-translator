%language "C"
%output "json_parser.c"
%defines "json_parser.h"
%verbose

%define api.prefix {json}
%define api.token.prefix {JSON_}
%define api.pure full
%define api.push-pull push

%define lr.type lalr
%define lr.default-reduction most
%define lr.keep-unreachable-state false

%define parse.lac none
%define parse.error simple
%define parse.trace false

%token BEGINARRAY BEGINOBJECT ENDARRAY ENDOBJECT NAMESEPARATOR VALUESEPARATOR FALSE NULL TRUE NUMBER STRING
%start json

%code requires {
#include "json.h"
}

%code provides {
#define YYSTYPE JSONSTYPE
}

%code {
void yyerror(char const *);
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

void yyerror(char const * message) {
    panic("%s", message);
}
