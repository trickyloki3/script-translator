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
%option nounput

alpha               [\x41-\x5A\x61-\x7A]
digit               [\x30-\x39]
prefix              \x40|\x24|\x24\x40|\x2E|\x2E\x40|\x27|\x23|\x23\x23
postfix             \x24

/*increment_postfix*/
/*decrement_postfix*/
round_open          \x28
round_close         \x29
square_open         \x5B
square_close        \x5D
increment_prefix    \x2B\x2B
decrement_prefix    \x2D\x2D
/*plus_unary*/
/*minus_unary*/
logic_not           \x21
bit_not             \x7E
multiply            \x2A
divide              \x2F
remainder           \x25
plus                \x2B
minus               \x2D
bit_left            \x3C\x3C
bit_right           \x3E\x3E
lesser              \x3C
lesser_equal        \x3C\x3D
greater             \x3E
greater_equal       \x3E\x3D
logic_equal         \x3D\x3D
logic_not_equal     \x21\x3D
bit_and             \x26
bit_xor             \x5E
bit_or              \x7C
logic_and           \x26\x26
logic_or            \x7C\x7C
question            \x3F
colon               \x3A
assign              \x3D
plus_assign         \x2B\x3D
minus_assign        \x2D\x3D
comma               \x2C
for                 \x66\x6F\x72
if                  \x69\x66
else                \x65\x6C\x73\x65
curly_open          \x7B
curly_close         \x7D
semicolon           \x3B
decimal             [\x30-\x39]+
hexadecimal         (\x30\x58|\x30\x78)[\x30-\x39\x41-\x46\x61-\x66]+
string              \x22([\x9\x20-\x21\x23-\x7E]|\x5C\x22)*\x22
identifier          {prefix}?{alpha}({alpha}|{digit}|\x5F)*{postfix}?

whitespace          [\x20\x9\xA\xD]|\xD\xA
line_comment        \x2F\x2F.*
block_comment       \x2F\x2A

%%

{round_open} {
    return script_round_open;
}

{round_close} {
    return script_round_close;
}

{square_open} {
    return script_square_open;
}

{square_close} {
    return script_square_close;
}

{increment_prefix} {
    return script_node_token(yyextra, script_increment_prefix, yylval) ? -1 : script_increment_prefix;
}

{decrement_prefix} {
    return script_node_token(yyextra, script_decrement_prefix, yylval) ? -1 : script_decrement_prefix;
}

{logic_not} {
    return script_node_token(yyextra, script_logic_not, yylval) ? -1 : script_logic_not;
}

{bit_not} {
    return script_node_token(yyextra, script_bit_not, yylval) ? -1 : script_bit_not;
}

{multiply} {
    return script_node_token(yyextra, script_multiply, yylval) ? -1 : script_multiply;
}

{divide} {
    return script_node_token(yyextra, script_divide, yylval) ? -1 : script_divide;
}

{remainder} {
    return script_node_token(yyextra, script_remainder, yylval) ? -1 : script_remainder;
}

{plus} {
    return script_node_token(yyextra, script_plus, yylval) ? -1 : script_plus;
}

{minus} {
    return script_node_token(yyextra, script_minus, yylval) ? -1 : script_minus;
}

{bit_left} {
    return script_node_token(yyextra, script_bit_left, yylval) ? -1 : script_bit_left;
}

{bit_right} {
    return script_node_token(yyextra, script_bit_right, yylval) ? -1 : script_bit_right;
}

{lesser} {
    return script_node_token(yyextra, script_lesser, yylval) ? -1 : script_lesser;
}

{lesser_equal} {
    return script_node_token(yyextra, script_lesser_equal, yylval) ? -1 : script_lesser_equal;
}

{greater} {
    return script_node_token(yyextra, script_greater, yylval) ? -1 : script_greater;
}

{greater_equal} {
    return script_node_token(yyextra, script_greater_equal, yylval) ? -1 : script_greater_equal;
}

{logic_equal} {
    return script_node_token(yyextra, script_logic_equal, yylval) ? -1 : script_logic_equal;
}

{logic_not_equal} {
    return script_node_token(yyextra, script_logic_not_equal, yylval) ? -1 : script_logic_not_equal;
}

{bit_and} {
    return script_node_token(yyextra, script_bit_and, yylval) ? -1 : script_bit_and;
}

{bit_xor} {
    return script_node_token(yyextra, script_bit_xor, yylval) ? -1 : script_bit_xor;
}

{bit_or} {
    return script_node_token(yyextra, script_bit_or, yylval) ? -1 : script_bit_or;
}

{logic_and} {
    return script_node_token(yyextra, script_logic_and, yylval) ? -1 : script_logic_and;
}

{logic_or} {
    return script_node_token(yyextra, script_logic_or, yylval) ? -1 : script_logic_or;
}

{question} {
    return script_node_token(yyextra, script_question, yylval) ? -1 : script_question;
}

{colon} {
    return script_node_token(yyextra, script_colon, yylval) ? -1 : script_colon;
}

{assign} {
    return script_node_token(yyextra, script_assign, yylval) ? -1 : script_assign;
}

{plus_assign} {
    return script_node_token(yyextra, script_plus_assign, yylval) ? -1 : script_plus_assign;
}

{minus_assign} {
    return script_node_token(yyextra, script_minus_assign, yylval) ? -1 : script_minus_assign;
}

{comma} {
    return script_node_token(yyextra, script_comma, yylval) ? -1 : script_comma;
}

{for} {
    return script_node_token(yyextra, script_for, yylval) ? -1 : script_for;
}

{if} {
    return script_node_token(yyextra, script_if, yylval) ? -1 : script_if;
}

{else} {
    return script_node_token(yyextra, script_else, yylval) ? -1 : script_else;
}

{curly_open} {
    return script_curly_open;
}

{curly_close} {
    return script_curly_close;
}

{semicolon} {
    return script_semicolon;
}

{decimal}|{hexadecimal} {
    struct string string = { yyleng, yytext };
    return script_node_integer(yyextra, &string, yylval) ? -1 : script_integer;
}

{string} {
    struct string string = { yyleng - 1, yytext + 1};
    return script_node_identifier(yyextra, &string, yylval) ? -1 : script_identifier;
}

{identifier} {
    struct string string = { yyleng, yytext };
    return script_node_identifier(yyextra, &string, yylval) ? -1 : script_identifier;
}

{whitespace} {
    /* ignore whitespace */
}

{line_comment} {
    /* ignore line comment */
}

{block_comment} {
    char last = 0;
    char next = input(yyscanner);

    while(last != '*' || next != '/') {
        last = next;
        next = input(yyscanner);
    }
}

. {
    panic("invalid character (line %d)", yylineno);
    return -1;
}
