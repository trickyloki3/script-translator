%{
#include "script.h"
#include "script_parser.h"

int script_node_token(struct store *, int, struct script_node **);
int script_node_integer(struct store *, char *, int, struct script_node **);
int script_node_identifier(struct store *, char *, size_t, struct script_node **);
%}

%option outfile="script_scanner.c" header-file="script_scanner.h"
%option batch 8bit nodefault never-interactive yylineno noyywrap
%option bison-bridge reentrant pointer prefix="script"
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
not                 \x21
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
equal               \x3D\x3D
not_equal           \x21\x3D
bit_and             \x26
bit_xor             \x5E
bit_or              \x7C
and                 \x26\x26
or                  \x7C\x7C
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
    return script_node_token(yyextra, script_increment_prefix, yylval);
}

{decrement_prefix} {
    return script_node_token(yyextra, script_decrement_prefix, yylval);
}

{not} {
    return script_node_token(yyextra, script_not, yylval);
}

{bit_not} {
    return script_node_token(yyextra, script_bit_not, yylval);
}

{multiply} {
    return script_node_token(yyextra, script_multiply, yylval);
}

{divide} {
    return script_node_token(yyextra, script_divide, yylval);
}

{remainder} {
    return script_node_token(yyextra, script_remainder, yylval);
}

{plus} {
    return script_node_token(yyextra, script_plus, yylval);
}

{minus} {
    return script_node_token(yyextra, script_minus, yylval);
}

{bit_left} {
    return script_node_token(yyextra, script_bit_left, yylval);
}

{bit_right} {
    return script_node_token(yyextra, script_bit_right, yylval);
}

{lesser} {
    return script_node_token(yyextra, script_lesser, yylval);
}

{lesser_equal} {
    return script_node_token(yyextra, script_lesser_equal, yylval);
}

{greater} {
    return script_node_token(yyextra, script_greater, yylval);
}

{greater_equal} {
    return script_node_token(yyextra, script_greater_equal, yylval);
}

{equal} {
    return script_node_token(yyextra, script_equal, yylval);
}

{not_equal} {
    return script_node_token(yyextra, script_not_equal, yylval);
}

{bit_and} {
    return script_node_token(yyextra, script_bit_and, yylval);
}

{bit_xor} {
    return script_node_token(yyextra, script_bit_xor, yylval);
}

{bit_or} {
    return script_node_token(yyextra, script_bit_or, yylval);
}

{and} {
    return script_node_token(yyextra, script_and, yylval);
}

{or} {
    return script_node_token(yyextra, script_or, yylval);
}

{question} {
    return script_node_token(yyextra, script_question, yylval);
}

{colon} {
    return script_node_token(yyextra, script_colon, yylval);
}

{assign} {
    return script_node_token(yyextra, script_assign, yylval);
}

{plus_assign} {
    return script_node_token(yyextra, script_plus_assign, yylval);
}

{minus_assign} {
    return script_node_token(yyextra, script_minus_assign, yylval);
}

{comma} {
    return script_node_token(yyextra, script_comma, yylval);
}

{for} {
    return script_node_token(yyextra, script_for, yylval);
}

{if} {
    return script_node_token(yyextra, script_if, yylval);
}

{else} {
    return script_node_token(yyextra, script_else, yylval);
}

{curly_open} {
    return script_node_token(yyextra, script_curly_open, yylval);
}

{curly_close} {
    return script_curly_close;
}

{semicolon} {
    return script_node_token(yyextra, script_semicolon, yylval);
}

{decimal} {
    return script_node_integer(yyextra, yytext, 10, yylval);
}

{hexadecimal} {
    return script_node_integer(yyextra, yytext, 16, yylval);
}

{string} {
    return script_node_identifier(yyextra, yytext + 1, yyleng - 2, yylval);
}

{identifier} {
    return script_node_identifier(yyextra, yytext, yyleng, yylval);
}

{whitespace}+ {
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
    panic("invalid character %s", yytext);
    return -1;
}

%%

int script_node_token(struct store * store, int token, struct script_node ** result) {
    int status = 0;
    struct script_node * node;

    node = script_node_create(store, token);
    if(!node) {
        status = panic("failed to create script node object");
    } else {
        *result = node;
    }

    return status ? -1 : node->token;
}

int script_node_integer(struct store * store, char * string, int base, struct script_node ** result) {
    int status = 0;
    char * last = NULL;
    struct script_node * node;

    node = script_node_create(store, script_integer);
    if(!node) {
        status = panic("failed to create script node object");
    } else {
        node->integer = strtol(string, &last, base);
        if(*last) {
            status = panic("failed to strtol string object");
        } else {
            *result = node;
        }
    }

    return status ? -1 : node->token;
}

int script_node_identifier(struct store * store, char * string, size_t length, struct script_node ** result) {
    int status = 0;
    struct script_node * node;

    node = script_node_create(store, script_identifier);
    if(!node) {
        status = panic("failed to create script node object");
    } else {
        node->identifier = store_strcpy(store, string, length);
        if(!node->identifier) {
            status = panic("failed to char store object");
        } else {
            *result = node;
        }
    }

    return status ? -1 : node->token;
}
