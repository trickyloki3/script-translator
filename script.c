#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_state_push(struct script *);
void script_state_pop(struct script *);

int script_parse(struct script *, struct string *, yyscan_t, scriptpstate *);

int script_create(struct script * script, size_t size, struct heap * heap, struct lookup * lookup) {
    int status = 0;

    script->heap = heap;
    if(!script->heap) {
        status = panic("invalid heap object");
    } else {
        script->lookup = lookup;
        if(!script->lookup) {
            status = panic("invalid lookup object");
        } else {
            if(scriptlex_init_extra(script, &script->scanner)) {
                status = panic("failed to create scanner object");
            } else {
                script->parser = scriptpstate_new();
                if(!script->parser) {
                    status = panic("failed to create parser object");
                } else {
                    if(store_create(&script->store, size)) {
                        status = panic("failed to create store object");
                    } else {
                        script->state = NULL;
                    }
                    if(status)
                        scriptpstate_delete(script->parser);
                }
                if(status)
                    scriptlex_destroy(script->scanner);
            }
        }
    }

    return status;
}
void script_destroy(struct script * script) {
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_state_push(struct script * script) {
    int status = 0;
    struct script_state * state;

    state = store_object(&script->store, sizeof(*state));
    if(!state) {
        status = panic("failed to object store object");
    } else {
        state->root = NULL;
        state->next = script->state;
        script->state = state;
    }

    return status;
}

void script_state_pop(struct script * script) {
    script->state = script->state->next;

    if(!script->state)
        store_clear(&script->store);
}

int script_parse(struct script * script, struct string * string, yyscan_t scanner, scriptpstate * parser) {
    int status = 0;

    YY_BUFFER_STATE buffer;
    SCRIPTSTYPE value;
    SCRIPTLTYPE location;
    int token;
    int state = YYPUSH_MORE;

    buffer = script_scan_buffer(string->string, string->length, scanner);
    if(!buffer) {
        status = panic("failed to scan buffer scanner object");
    } else {
        while(state == YYPUSH_MORE && !status) {
            token = scriptlex(&value, &location, scanner);
            if(token < 0) {
                status = panic("failed to get the next token");
            } else {
                state = scriptpush_parse(parser, token, &value, &location, script);
                if(state && state != YYPUSH_MORE)
                    status = panic("failed to parse the current token");
            }
        }
        scriptpop_buffer_state(scanner);
    }

    return status;
}

int script_translate(struct script * script, struct string * string) {
    int status = 0;

    if(script_state_push(script)) {
        status = panic("failed to state push script object");
    } else {
        if(script_parse(script, string, script->scanner, script->parser))
            status = panic("failed to parse script object");
        script_state_pop(script);
    }

    return status;
}

struct script_node * script_node_create(struct script * script, int token) {
    struct script_node * node;

    node = store_object(&script->store, sizeof(*node));
    if(node) {
        node->token = token;
        node->node = NULL;
        node->next = NULL;
    }

    return node;
}

void script_node_print(struct script_node * root) {
    struct script_node * iter;

    switch(root->token) {
        case script_curly_open:
            iter = root->node;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_integer:
            fprintf(stdout, "%ld ", root->integer);
            break;
        case script_identifier:
            fprintf(stdout, "%s ", root->identifier->string);
            if(root->node) {
                fprintf(stdout, "( ");
                iter = root->node;
                while(iter) {
                    script_node_print(iter);
                    iter = iter->next;
                }
                fprintf(stdout, ") ");
            }
            break;
        case script_for:
            fprintf(stdout, "for ");
            iter = root->node;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_if:
            fprintf(stdout, "if ");
            iter = root->node;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_else:
            fprintf(stdout, "else ");
            iter = root->node;
            while(iter) {
                script_node_print(iter);
                fprintf(stdout, "\n");
                iter = iter->next;
            }
            break;
        case script_comma:
            script_node_print(root->node);
            fprintf(stdout, ", ");
            script_node_print(root->node->next);
            break;
        case script_assign:
            script_node_print(root->node);
            fprintf(stdout, "= ");
            script_node_print(root->node->next);
            break;
        case script_plus_assign:
            script_node_print(root->node);
            fprintf(stdout, "+= ");
            script_node_print(root->node->next);
            break;
        case script_minus_assign:
            script_node_print(root->node);
            fprintf(stdout, "-= ");
            script_node_print(root->node->next);
            break;
        case script_question:
            script_node_print(root->node);
            fprintf(stdout, "? ");
            script_node_print(root->node->next);
            break;
        case script_colon:
            script_node_print(root->node);
            fprintf(stdout, ": ");
            script_node_print(root->node->next);
            break;
        case script_logic_or:
            script_node_print(root->node);
            fprintf(stdout, "|| ");
            script_node_print(root->node->next);
            break;
        case script_logic_and:
            script_node_print(root->node);
            fprintf(stdout, "&& ");
            script_node_print(root->node->next);
            break;
        case script_bit_or:
            script_node_print(root->node);
            fprintf(stdout, "| ");
            script_node_print(root->node->next);
            break;
        case script_bit_xor:
            script_node_print(root->node);
            fprintf(stdout, "^ ");
            script_node_print(root->node->next);
            break;
        case script_bit_and:
            script_node_print(root->node);
            fprintf(stdout, "& ");
            script_node_print(root->node->next);
            break;
        case script_logic_equal:
            script_node_print(root->node);
            fprintf(stdout, "== ");
            script_node_print(root->node->next);
            break;
        case script_logic_not_equal:
            script_node_print(root->node);
            fprintf(stdout, "!= ");
            script_node_print(root->node->next);
            break;
        case script_lesser:
            script_node_print(root->node);
            fprintf(stdout, "< ");
            script_node_print(root->node->next);
            break;
        case script_lesser_equal:
            script_node_print(root->node);
            fprintf(stdout, "<= ");
            script_node_print(root->node->next);
            break;
        case script_greater:
            script_node_print(root->node);
            fprintf(stdout, "> ");
            script_node_print(root->node->next);
            break;
        case script_greater_equal:
            script_node_print(root->node);
            fprintf(stdout, ">= ");
            script_node_print(root->node->next);
            break;
        case script_bit_left:
            script_node_print(root->node);
            fprintf(stdout, "<< ");
            script_node_print(root->node->next);
            break;
        case script_bit_right:
            script_node_print(root->node);
            fprintf(stdout, ">> ");
            script_node_print(root->node->next);
            break;
        case script_plus:
            script_node_print(root->node);
            fprintf(stdout, "+ ");
            script_node_print(root->node->next);
            break;
        case script_minus:
            script_node_print(root->node);
            fprintf(stdout, "- ");
            script_node_print(root->node->next);
            break;
        case script_multiply:
            script_node_print(root->node);
            fprintf(stdout, "* ");
            script_node_print(root->node->next);
            break;
        case script_divide:
            script_node_print(root->node);
            fprintf(stdout, "/ ");
            script_node_print(root->node->next);
            break;
        case script_remainder:
            script_node_print(root->node);
            fprintf(stdout, "%% ");
            script_node_print(root->node->next);
            break;
        case script_increment_prefix:
            fprintf(stdout, "++ ");
            script_node_print(root->node);
            break;
        case script_decrement_prefix:
            fprintf(stdout, "-- ");
            script_node_print(root->node);
            break;
        case script_plus_unary:
            fprintf(stdout, "+ ");
            script_node_print(root->node);
            break;
        case script_minus_unary:
            fprintf(stdout, "- ");
            script_node_print(root->node);
            break;
        case script_logic_not:
            fprintf(stdout, "! ");
            script_node_print(root->node);
            break;
        case script_bit_not:
            fprintf(stdout, "~ ");
            script_node_print(root->node);
            break;
        case script_increment_postfix:
            script_node_print(root->node);
            fprintf(stdout, "++ ");
            break;
        case script_decrement_postfix:
            script_node_print(root->node);
            fprintf(stdout, "-- ");
            break;
    }
}
