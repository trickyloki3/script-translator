%{
#include "csv.h"
%}

%option outfile="csv_scanner.c" header-file="csv_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option reentrant pointer prefix="csv"
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
    struct csv * csv = yyextra;

    csv->index++;
}

{NEWLINE} {
    struct csv * csv = yyextra;

    if(csv->index) {
        csv->index = 0;

        if(csv->callback(end, csv->index, NULL, csv->context))
            return panic("failed to process list end event");
    }
}

{SPACE}* {
    /* ignore space */
}

{SPACE}*{COMMENT}.* {
    /* ignore comment */
}

{CURLY}.*|{TEXTDATA}* {
    struct csv * csv = yyextra;
    struct string string = { yyleng, yytext };

    if(!csv->index) {
        if(csv->callback(start, csv->index, NULL, csv->context)) {
            return panic("failed to process list start event");
        } else {
            csv->index++;
        }
    }

    if(csv->callback(next, csv->index, &string, csv->context))
        return panic("failed to process scalar event");
}

{QUOTE}({TEXTDATA}|{COMMA})*{QUOTE} {
    struct csv * csv = yyextra;
    struct string string = { yyleng - 2, yytext + 1 };

    if(!csv->index) {
        if(csv->callback(start, csv->index, NULL, csv->context)) {
            return panic("failed to process list start event");
        } else {
            csv->index++;
        }
    }

    if(csv->callback(next, csv->index, &string, csv->context))
        return panic("failed to process scalar event");
}

{QUOTE}({TEXTDATA}|{COMMA})* {
    panic("unmatch double quote (line %d)", yylineno);
    return -1;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
