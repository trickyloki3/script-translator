%{
#include "json.h"
#include "json_parser.h"
#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="json_scanner.c" header-file="json_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge bison-locations reentrant pointer prefix="json"
%option align read full
%option backup warn

BEGINARRAY      \x5B
BEGINOBJECT     \x7B
ENDARRAY        \x5D
ENDOBJECT       \x7D
NAMESEPARATOR   \x3A
VALUESEPARATOR  \x2C

WS              [\x20\x9\xA\xD]

FALSE           \x66\x61\x6C\x73\x65
NULL            \x6E\x75\x6C\x6C
TRUE            \x74\x72\x75\x65

DECIMALPOINT    \x2E
DIGIT09         [\x30-\x39]
DIGIT19         [\x31-\x39]
E               [\x65\x45]
EXP             {E}({MINUS}|{PLUS})?{DIGIT09}+
FRAC            {DECIMALPOINT}{DIGIT09}+
INT             {ZERO}|{DIGIT19}{DIGIT09}*
MINUS           \x2D
PLUS            \x2B
ZERO            \x30

CHAR            {UNESCAPED}|{ESCAPE}[\x22\x5C\x2F\x62\x66\x6E\x72\x74]|{UNICODE}
ESCAPE          \x5C
QUOTATIONMARK   \x22
UNESCAPED       [\x20-\x21\x23-\x5B\x5D-\xFF]
HEXDIG          [\x30-\x39\x41-\x46\x61-\x66]
UNICODE         {ESCAPE}\x75{HEXDIG}{HEXDIG}{HEXDIG}{HEXDIG}

%%

{BEGINARRAY} {
    return json_add_node(yyextra, json_array, NULL, 0, yylval) ? -1 : JSON_BEGINARRAY;
}

{BEGINOBJECT} {
    return json_add_node(yyextra, json_object, NULL, 0, yylval) ? -1 : JSON_BEGINOBJECT;
}

{ENDARRAY} {
    return JSON_ENDARRAY;
}

{ENDOBJECT} {
    return JSON_ENDOBJECT;
}

{NAMESEPARATOR} {
    return JSON_NAMESEPARATOR;
}

{VALUESEPARATOR} {
    return JSON_VALUESEPARATOR;
}

{WS}* {

}

{FALSE} {
    return json_add_node(yyextra, json_false, NULL, 0, yylval) ? -1 : JSON_FALSE;
}

{NULL} {
    return json_add_node(yyextra, json_null, NULL, 0, yylval) ? -1 : JSON_NULL;
}

{TRUE} {
    return json_add_node(yyextra, json_true, NULL, 0, yylval) ? -1 : JSON_TRUE;
}

{MINUS}?{INT}{FRAC}?{EXP}? {
    return json_add_node(yyextra, json_number, yytext, yyleng, yylval) ? -1 : JSON_NUMBER;
}

{QUOTATIONMARK}{CHAR}*{QUOTATIONMARK} {
    return json_add_node(yyextra, json_string, yytext, yyleng, yylval) ? -1 : JSON_STRING;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
