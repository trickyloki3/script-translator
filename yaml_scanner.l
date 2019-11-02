%{
#include "yaml.h"
#include "yaml_parser.h"
#define YY_USER_ACTION yylloc->first_line = yylineno;
%}

%option outfile="yaml_scanner.c" header-file="yaml_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge bison-locations reentrant pointer prefix="yaml"
%option align read full
%option backup warn
%option noinput nounput

%%

.|\n {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
