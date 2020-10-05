%{
#include "csv.h"
%}

%option outfile="csv_scanner.c" header-file="csv_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option reentrant pointer prefix="csv"
%option align read full
%option backup warn
%option noinput nounput
%option extra-type="struct csv *"

COMMA       \x2C
NEWLINE     \xD|\xA|\xD\xA
SPACE       [\x9\x20]
COMMENT     \x2F\x2F
CURLY       \x7B
QUOTE       \x22
TEXTDATA    [\x9\x20-\x21\x23-\x2B\x2D-\x7A\x7C\x7E]

%%

{COMMA} {
    yyextra->index++;
}

{NEWLINE} {
    if(yyextra->index) {
        if(yyextra->cb(csv_end, 0, NULL, yyextra->arg)) {
            return panic("failed to process list end event");
        } else {
            yyextra->index = 0;
        }
    }
}

{SPACE}* {
    /* ignore space */
}

{SPACE}*{COMMENT}.* {
    /* ignore comment */
}

{CURLY}.*|{TEXTDATA}* {
    struct string string = { yyleng, yytext };

    if(!yyextra->index) {
        if(yyextra->cb(csv_start, 0, NULL, yyextra->arg)) {
            return panic("failed to process list start event");
        } else {
            yyextra->index = 1;
        }
    }

    if(yyextra->cb(csv_next, yyextra->index, &string, yyextra->arg))
        return panic("failed to process scalar event");
}

{QUOTE}({TEXTDATA}|{COMMA})*{QUOTE} {
    struct string string = { yyleng - 2, yytext + 1 };

    if(!yyextra->index) {
        if(yyextra->cb(csv_start, 0, NULL, yyextra->arg)) {
            return panic("failed to process list start event");
        } else {
            yyextra->index = 1;
        }
    }

    if(yyextra->cb(csv_next, yyextra->index, &string, yyextra->arg))
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
