%{
#include "csv.h"
#include "csv_parser.h"
%}

%option outfile="csv_scanner.c" header-file="csv_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge reentrant pointer prefix="csv"
%option align read full
%option backup warn
%option noinput nounput

COMMA       \x2C
NEWLINE     \xD|\xA|\xD\xA
SPACE       [\x9\x20]
COMMENT     \x2F\x2F
CURLY       \x7B
QUOTE       \x22
TEXTDATA    [\x9\x20-\x21\x23-\x2B\x2D-\x7A\x7C\x7E]

%%

{COMMA} {
    return CSV_COMMA;
}

{NEWLINE} {
    return CSV_NEWLINE;
}

{SPACE}* {
    /* ignore space */
}

{SPACE}*{COMMENT}.* {
    /* ignore comment */
}

{CURLY}.* {
    return strbuf_strcpy(yyextra, yytext, yyleng) ? -1 : CSV_STRING;
}

{QUOTE}({TEXTDATA}|{COMMA})*{QUOTE} {
    return strbuf_strcpy(yyextra, yytext, yyleng) ? -1 : CSV_STRING;
}

{QUOTE}({TEXTDATA}|{COMMA})* {
    panic("unmatch double quote (line %d)", yylineno);
    return -1;
}

{TEXTDATA}* {
    return strbuf_strcpy(yyextra, yytext, yyleng) ? -1 : CSV_STRING;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
