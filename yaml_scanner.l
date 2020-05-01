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
%option extra-type="struct yaml *"

c-sequence-entry        \x2D
c-mapping-value         \x3A
c-comment               \x23
c-literal               \x7C
c-folded                \x3E
s-space                 \x20
s-indent                ^{s-space}*
s-separate-in-line      {s-space}+
b-break                 \xD\xA|\xD|\xA
nb-char                 [\x9\x20-\x7E]
c-nb-comment-text       {c-comment}{nb-char}*
ns-char                 [\x21-\x7E]
ns-plain-first          [\x21-\x7E]{-}[\x2D\x3F\x3A\x23\x7C\x3E]|(\x2D|\x3F|\x3A){ns-char}
ns-plain-char           [\x21-\x7E]{-}[\x3A\x23]|{ns-char}{c-comment}|{c-mapping-value}{ns-char}
ns-plain-one-line       {ns-plain-first}({s-space}*{ns-plain-char})*

%x literal

%%

{c-sequence-entry} {
    return yaml_token(yyextra, yaml_c_sequence_entry, 0, NULL, yylval) ? -1 : yaml_c_sequence_entry;
}

{c-mapping-value} {
    return yaml_token(yyextra, yaml_c_mapping_value, 0, NULL, yylval) ? -1 : yaml_c_mapping_value;
}

{c-literal} {
    return yaml_token(yyextra, yaml_c_literal, 0, NULL, yylval) ? -1 : yaml_c_literal;
}

{c-folded} {
    return yaml_token(yyextra, yaml_c_folded, 0, NULL, yylval) ? -1 : yaml_c_folded;
}

{s-indent} {
    if(yaml_literal(yyextra, yyleng))
        BEGIN(literal);
    return yaml_token(yyextra, yaml_s_indent, yyleng, NULL, yylval) ? -1 : yaml_s_indent;
}

{s-separate-in-line} {
    return yaml_token(yyextra, yaml_s_separate_in_line, yyleng, NULL, yylval) ? -1 : yaml_s_separate_in_line;
}

{s-indent}{c-nb-comment-text}?{b-break} {
    return yaml_token(yyextra, yaml_l_empty, 1, NULL, yylval) ? -1 : yaml_l_empty;
}

({s-separate-in-line}{c-nb-comment-text}?)?{b-break} {
    return yaml_token(yyextra, yaml_b_break, 1, NULL, yylval) ? -1 : yaml_b_break;
}

<literal>{ns-char}({s-space}*{nb-char})* {
    BEGIN(INITIAL);
    return yaml_string(yyextra, yaml_nb_char, 0, yytext, yyleng, yylval) ? -1 : yaml_nb_char;
}

{ns-plain-one-line} {
    return yaml_string(yyextra, yaml_ns_plain_one_line, 0, yytext, yyleng, yylval) ? -1 : yaml_ns_plain_one_line;
}

<INITIAL,literal>.|{b-break} {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
