%{
#include "csv.h"
#include "csv_parser.h"
#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="csv_scanner.c" header-file="csv_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge bison-locations reentrant pointer prefix="csv"
%option align read full
%option backup warn
%option nounput

COMMA       \x2C
CR          \xD
LF          \xA
SPACE       [\x9\x20]
COMMENT     \x2F\x2F
CURLY       \x7B
DQUOTE      \x22
TEXTDATA    [\x9\x20-\x21\x23-\x2B\x2D-\x7A\x7C\x7E]

%%

{COMMA} {
    return CSV_COMMA;
}

{CR}?{LF} {
    return CSV_NEWLINE;
}

{SPACE}* {
    return CSV_SPACE;
}

{SPACE}*{COMMENT}.* {
    return CSV_COMMENT;
}

{CURLY} {
    int status = 0;
    int count;
    int symbol;

    count = 1;
    do {
        symbol = input(yyscanner);
        if(symbol == EOF || symbol == '\n') {
            status = panic("unmatch curly bracket");
        } else if(symbol == '{') {
            count++;
        } else if(symbol == '}') {
            count--;
        } else if(strbuf_putc(yyextra, symbol)) {
            status = panic("failed to putc strbuf object");
        }
    } while(count && !status);

    return status ? -1 : CSV_ESCAPED;
}

{DQUOTE}({TEXTDATA}|{COMMA})*{DQUOTE} {
    return strbuf_strcpy(yyextra, yytext, yyleng) ? -1 : CSV_ESCAPED;
}

{DQUOTE}({TEXTDATA}|{COMMA})* {
    panic("unmatch double quote (line %d)", yylineno);
    return -1;
}

{TEXTDATA}* {
    return strbuf_strcpy(yyextra, yytext, yyleng) ? -1 : CSV_NONESCAPED;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
