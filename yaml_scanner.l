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

c-sequence-entry    \x2D
c-mapping-key       \x3F
c-mapping-value     \x3A
s-space             \x20
s-indent            ^{s-space}*
s-white             [\x20\x9]
s-separate-in-line  {s-white}+
c-comment           \x23
nb-char             [\x9\x20-\x7E]
c-nb-comment-text   {c-comment}{nb-char}*
b-break             \xD\xA|\xD|\xA

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

{s-indent} {
    return yaml_s_indent;
}

{s-separate-in-line} {
    return yaml_s_separate_in_line;
}

({s-separate-in-line}{c-nb-comment-text}?)?{b-break} {
    return yaml_b_break;
}

{s-indent}{s-separate-in-line}?{c-nb-comment-text}?{b-break} {
    return yaml_l_empty;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
