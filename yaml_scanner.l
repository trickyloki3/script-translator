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

c-sequence-entry        \x2D
c-mapping-key           \x3F
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
    return yaml_c_sequence_entry;
}

{c-mapping-key} {
    return yaml_c_mapping_key;
}

{c-mapping-value} {
    return yaml_c_mapping_value;
}

{c-literal} {
    BEGIN(literal);
    return yaml_c_literal;
}

{c-folded} {
    BEGIN(literal);
    return yaml_c_folded;
}

<literal>{ns-char}({s-space}*{nb-char})* {
    return yaml_nb_char;
}

<literal>{s-indent} {
    struct yaml * yaml = yyextra;

    if(!yaml->indent) {
        yaml->indent = yyleng;
    } else if(yaml->indent > yyleng) {
        yaml->indent = 0;
        BEGIN(INITIAL);
    }

    return yaml_s_indent;
}

{s-indent} {
    return yaml_s_indent;
}

{s-separate-in-line} {
    return yaml_s_separate_in_line;
}

<INITIAL,literal>{s-indent}{c-nb-comment-text}?{b-break} {
    return yaml_l_empty;
}

<INITIAL,literal>({s-separate-in-line}{c-nb-comment-text}?)?{b-break} {
    return yaml_b_break;
}

{ns-plain-one-line} {
    return yaml_ns_plain_one_line;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
