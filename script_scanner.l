%{
#include "script.h"
#include "script_parser.h"
#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="script_scanner.c" header-file="script_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge bison-locations reentrant pointer prefix="script"
%option align read full
%option backup warn
%option noinput nounput

curly-open      \x7B
curly-close     \x7D

%%

{curly-open} {
    return script_curly_open;
}

{curly-close} {
    return script_curly_close;
}

.|\xA {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
