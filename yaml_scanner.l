%{
#include "yaml.h"
%}

%option outfile="yaml_scanner.c" header-file="yaml_scanner.h"
%option batch 8bit nodefault never-interactive noyywrap
%option reentrant pointer prefix="yaml"
%option align read full
%option backup warn
%option noinput nounput
%option extra-type="struct yaml *"

c-sequence-entry        \x2D
c-mapping-value         \x3A
c-literal               \x7C
c-folded                \x3E
s-separate-in-line      [\x20]*
b-break                 \xD\xA|\xD|\xA
nb-char                 [\x9\x20-\x7E]
c-nb-comment-text       [\x23]{nb-char}*
ns-char                 [\x21-\x7E]
ns-plain-first          [\x21-\x7E]{-}[\x2D\x3A\x23\x7C\x3E]|[\x2D]{ns-char}
ns-plain-char           [\x20-\x7E]{-}[\x3A\x23]
ns-plain-one-line       {ns-plain-first}{ns-plain-char}*

%x SCALAR

%%

{c-sequence-entry} {
    return c_sequence_entry;
}

{c-literal} {
    return c_literal;
}

{c-folded} {
    return c_folded;
}

{s-separate-in-line} {
    if(yyextra->scalar && yyextra->scope <= yyleng)
        BEGIN(SCALAR);

    yyextra->space = yyleng;
    return s_separate_in_line;
}

{s-separate-in-line}{c-nb-comment-text}? {
    return b_break;
}

{s-separate-in-line}{c-nb-comment-text}?{b-break} {
    return b_break;
}

<SCALAR>{ns-char}{nb-char}* {
    BEGIN(INITIAL);
    yyextra->string = yytext;
    yyextra->length = yyleng;
    return nb_char;
}

{ns-plain-one-line} {
    /* remove trailing whitespace */
    while(yytext[yyleng - 1] == ' ')
        yyleng--;

    yytext[yyleng] = 0;

    yyextra->string = yytext;
    yyextra->length = yyleng;
    return ns_plain_one_line;
}

{ns-plain-one-line}{c-mapping-value} {
    /* remove delimiter and trailing whitespace */
    yyleng--;
    while(yytext[yyleng - 1] == ' ')
        yyleng--;

    yytext[yyleng] = 0;

    yyextra->string = yytext;
    yyextra->length = yyleng;
    return c_mapping_value;
}

<INITIAL,SCALAR>.|{b-break} {
    return -1;
}
