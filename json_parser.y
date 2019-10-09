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
void yyerror(JSONLTYPE *, struct json *, char const *);
}

%define api.value.type {struct json_node *}
%parse-param {struct json * json}

%%

json : value { json->root = $1; }

object : BEGINOBJECT ENDOBJECT { if(json_pop_node(json, json_object)) YYABORT; }
       | BEGINOBJECT member ENDOBJECT { if(json_pop_node(json, json_object)) YYABORT; }

member : field
       | member VALUESEPARATOR field

field : STRING NAMESEPARATOR value { if(json_insert_object(json, $1, $3)) YYABORT; }

array : BEGINARRAY ENDARRAY { if(json_pop_node(json, json_array)) YYABORT; }
      | BEGINARRAY element ENDARRAY { if(json_pop_node(json, json_array)) YYABORT; }

element : value { if(json_insert_array(json, $1)) YYABORT; }
        | element VALUESEPARATOR value { if(json_insert_array(json, $3)) YYABORT; }

value : FALSE
      | NULL
      | TRUE
      | object
      | array
      | NUMBER
      | STRING

%%

void yyerror(JSONLTYPE * location, struct json * json, char const * message) {
    panic("%s (line %d)", message, location->first_line);
}
