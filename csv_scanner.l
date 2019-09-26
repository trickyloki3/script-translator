%{
#include "csv.h"
#include "csv_parser.h"
#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="csv_scanner.c" header-file="csv_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option ansi-definitions ansi-prototypes bison-bridge bison-locations reentrant pointer prefix="csv"
%option align read full
%option backup warn

COMMA       \x2C
CR          \xD
LF          \xA
SPACE       [\x9\x20]
COMMENT     \x2F\x2F
CURLY       \x7B
DQUOTE      \x22
TEXTDATA    [\x20-\x21\x23-\x2B\x2D-\x2E\x30-\x7A\x7C\x7E]

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
    struct string * string;
    int count;
    int symbol;

    string = csv_get_string(yyextra);
    if(!string) {
        status = panic("failed to get string object");
    } else {
        count = 1;
        do {
            symbol = input(yyscanner);
            if(symbol == EOF || symbol == '\n') {
                status = panic("unmatch curly bracket");
            } else if(symbol == '{') {
                count++;
            } else if(symbol == '}') {
                count--;
            } else if(string_putc(string, symbol)) {
                status = panic("failed to putc string object");
            }
        } while(count && !status);

        if(status && csv_put_string(yyextra, string)) {
            status = panic("failed to put string object");
        } else {
            *yylval = string;
        }
    }

    return status ? -1 : CSV_ESCAPED;
}

{DQUOTE}({TEXTDATA}|{COMMA})*{DQUOTE} {
    *yylval = csv_strdup_string(yyextra, yytext, yyleng);
    return *yylval ? CSV_ESCAPED : -1;
}

{DQUOTE}({TEXTDATA}|{COMMA})* {
    panic("unmatch double quote (line %d)", yylineno);
    return -1;
}

{TEXTDATA}* {
    *yylval = csv_strdup_string(yyextra, yytext, yyleng);
    return *yylval ? CSV_NONESCAPED : -1;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
