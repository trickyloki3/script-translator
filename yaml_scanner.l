%{
#include "yaml.h"
%}

%option outfile="yaml_scanner.c" header-file="yaml_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option reentrant pointer prefix="yaml"
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

%x SCALAR

%%

{c-sequence-entry} {
    return c_sequence_entry;
}

{c-mapping-value} {
    return c_mapping_value;
}

{c-literal} {
    return c_literal;
}

{c-folded} {
    return c_folded;
}

{s-indent} {
    if(yyextra->scalar && yyextra->scope <= yyleng)
        BEGIN(SCALAR);
    yyextra->space = yyleng;
    return s_indent;
}

{s-separate-in-line} {
    yyextra->space = yyleng;
    return s_separate_in_line;
}

{s-indent}{c-nb-comment-text}?{b-break} {
    return l_empty;
}

({s-separate-in-line}{c-nb-comment-text}?)?{b-break} {
    return b_break;
}

<SCALAR>{ns-char}({s-space}*{nb-char})* {
    BEGIN(INITIAL);
    yyextra->string = yytext;
    yyextra->length = yyleng;
    return nb_char;
}

{ns-plain-one-line} {
    yyextra->string = yytext;
    yyextra->length = yyleng;
    return ns_plain_one_line;
}

{ns-plain-one-line}{c-mapping-value} {
    yyextra->string = yytext;
    yyextra->length = yyleng - 1;
    yytext[yyleng - 1] = 0;
    return ns_key_one_line;
}

<INITIAL,SCALAR>.|{b-break} {
    return -1;
}
