%{
#include "json.h"
#include "json_parser.h"

int json_next(struct json *, enum event_type, int, struct string *);
%}

%option outfile="json_scanner.c" header-file="json_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option reentrant pointer prefix="json"
%option align read full
%option backup warn
%option noinput nounput
%option extra-type="struct json *"

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
    if(json_next(yyextra, event_list_start, JSON_BEGINARRAY, NULL))
        return panic("failed to next json object");
}

{BEGINOBJECT} {
    if(json_next(yyextra, event_map_start, JSON_BEGINOBJECT, NULL))
        return panic("failed to next json object");
}

{ENDARRAY} {
    if(json_next(yyextra, event_list_end, JSON_ENDARRAY, NULL))
        return panic("failed to next json object");
}

{ENDOBJECT} {
    if(json_next(yyextra, event_map_end, JSON_ENDOBJECT, NULL))
        return panic("failed to next json object");
}

{NAMESEPARATOR} {
    if(YYPUSH_MORE != jsonpush_parse(yyextra->parser, JSON_NAMESEPARATOR, NULL))
        return panic("failed to push parse json");
}

{VALUESEPARATOR} {
    if(YYPUSH_MORE != jsonpush_parse(yyextra->parser, JSON_VALUESEPARATOR, NULL))
        return panic("failed to push parse json");
}

{WS}* {

}

{FALSE} {
    struct string string = { yyleng, yytext };

    if(json_next(yyextra, event_scalar, JSON_FALSE, &string))
        return panic("failed to next json object");
}

{NULL} {
    struct string string = { yyleng, yytext };

    if(json_next(yyextra, event_scalar, JSON_NULL, &string))
        return panic("failed to next json object");
}

{TRUE} {
    struct string string = { yyleng, yytext };

    if(json_next(yyextra, event_scalar, JSON_TRUE, &string))
        return panic("failed to next json object");
}

{MINUS}?{INT}{FRAC}?{EXP}? {
    struct string string = { yyleng, yytext };

    if(json_next(yyextra, event_scalar, JSON_NUMBER, &string))
        return panic("failed to next json object");
}

{QUOTATIONMARK}{CHAR}*{QUOTATIONMARK} {
    struct string string = { yyleng - 2, yytext + 1 };

    if(json_next(yyextra, event_scalar, JSON_STRING, &string))
        return panic("failed to next json object");
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}

%%

int json_next(struct json * json, enum event_type type, int token, struct string * string) {
    int status = 0;

    if(YYPUSH_MORE != jsonpush_parse(json->parser, token, NULL)) {
        status = panic("failed to push parse json object");
    } else if(json->callback(type, string, json->context)) {
        status = panic("failed to process event");
    }

    return status;
}
