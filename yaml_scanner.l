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
s-space                 \x20
s-indent                ^{s-space}*
s-white                 [\x20\x9]
s-separate-in-line      {s-white}+
c-comment               \x23
nb-char                 [\x9\x20-\x7E]
c-nb-comment-text       {c-comment}{nb-char}*
b-break                 \xD\xA|\xD|\xA
c-directive             \x25
ns-char                 [\x21-\x7E]
ns-dec-digit            [\x30-\x39]
ns-hex-digit            [\x30-\x39\x41-\x46\x61-\x66]
ns-ascii-letter         [\x41-\x5A\x61-\x7A]
ns-word-char            {ns-dec-digit}|{ns-ascii-letter}|\x2D
ns-uri-char             \x25{ns-hex-digit}{ns-hex-digit}|{ns-word-char}|[\x23\x3B\x2F\x3F\x3A\x40\x26\x3D\x2B\x24\x2C\x5F\x2E\x21\x7E\x2A\x27\x28\x29\x5B\x5D]
ns-tag-char             \x25{ns-hex-digit}{ns-hex-digit}|{ns-word-char}|[\x23\x3B\x2F\x3F\x3A\x40\x26\x3D\x2B\x24\x5F\x2E\x7E\x2A\x27\x28\x29]
c-tag                   \x21
c-primary-tag-handle    {c-tag}
c-secondary-tag-handle  {c-tag}{c-tag}
c-named-tag-handle      {c-tag}{ns-word-char}+{c-tag}
c-tag-handle            {c-primary-tag-handle}|{c-secondary-tag-handle}|{c-named-tag-handle}
c-ns-local-tag-prefix   {c-tag}{ns-uri-char}*
ns-global-tag-prefix    {ns-tag-char}{ns-uri-char}*
ns-tag-prefix           {c-ns-local-tag-prefix}|{ns-global-tag-prefix}
c-verbatim-tag          {c-tag}\x3C{ns-uri-char}+\x3E
c-ns-shorthand-tag      {c-tag-handle}{ns-tag-char}+
c-non-specific-tag      {c-tag}
c-ns-tag-property       {c-verbatim-tag}|{c-ns-shorthand-tag}|{c-non-specific-tag}
c-flow-indicator        [\x2C\x5B\x5D\x7B\x7D]
c-anchor                \x26
/* ns-anchor-char       {ns-char}{-}{c-flow-indicator} */
ns-anchor-char          [\x21-\x7E]{-}[\x2C\x5B\x5D\x7B\x7D]
c-ns-anchor-property    {c-anchor}{ns-anchor-char}+
c-alias                 \x2A
c-ns-alias-node         {c-alias}{ns-anchor-char}+
c-double-quote          \x22
c-ns-esc-char           \x5C[\x30\x61\x62\x74\x9\x6E\x76\x66\x72\x65\x20\x22\x2F\x5C]
nb-double-char          {c-ns-esc-char}|[\x9\x20-\x7E]{-}[\x5C\x22]
nb-double-one-line      {c-double-quote}{nb-double-char}*{c-double-quote}

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

{c-ns-tag-property} {
    return yaml_c_ns_tag_property;
}

{c-ns-anchor-property} {
    return yaml_c_ns_anchor_property;
}

{c-ns-alias-node} {
    return yaml_c_ns_alias_node;
}

{nb-double-one-line} {
    return yaml_nb_double_one_line;
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

^{c-directive}\x59\x41\x4D\x4C{s-separate-in-line}{ns-dec-digit}+\x2E{ns-dec-digit}+ {
    return yaml_ns_yaml_directive;
}

^{c-directive}\x54\x41\x47{s-separate-in-line}{c-tag-handle}{s-separate-in-line}{ns-tag-prefix} {
    return yaml_ns_tag_directive;
}

^{c-directive}{ns-char}+({s-separate-in-line}{ns-char}+)* {
    return yaml_ns_reserve_directive;
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
