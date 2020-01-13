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
            if(strbuf_create(&script->strbuf, size)) {
                status = panic("failed to create strbuf object");
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
                if(status)
                    strbuf_destroy(&script->strbuf);
            }
        }
    }

    return status;
}
void script_destroy(struct script * script) {
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
    strbuf_destroy(&script->strbuf);
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

    if(strbuf_strcpy(&script->strbuf, string->string, string->length)) {
        status = panic("failed to strcpy strbuf object");
    } else if(strbuf_putcn(&script->strbuf, '\0', 2)) {
        status = panic("failed to putcn strbuf object");
    } else {
        string = strbuf_string(&script->strbuf);
        if(!string) {
            status = panic("failed to string strbuf object");
        } else {
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
        }
    }

    strbuf_clear(&script->strbuf);

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

struct script_node * script_node_create(struct script * script, enum script_type type, int value) {
    struct script_node * node;

    node = store_object(&script->store, sizeof(*node));
    if(node) {
        node->type = type;
        node->token = value;
        node->node = NULL;
        node->next = NULL;
    }

    return node;
}

int script_node_token(struct script * script, int value, struct script_node ** result) {
    int status = 0;
    struct script_node * node;

    node = script_node_create(script, token, value);
    if(!node) {
        status = panic("failed to create script node object");
    } else {
        *result = node;
    }

    return status;
}

int script_node_integer(struct script * script, struct string * string, struct script_node ** result) {
    int status = 0;
    struct script_node * node;

    node = script_node_create(script, integer, 0);
    if(!node) {
        status = panic("failed to create script node object");
    } else if(string_strtol(string, &node->integer)) {
        status = panic("failed to strtol string object");
    } else {
        *result = node;
    }

    return status;
}

int script_node_identifier(struct script * script, struct string * string, struct script_node ** result) {
    int status = 0;
    struct script_node * node;

    node = script_node_create(script, identifier, 0);
    if(!node) {
        status = panic("failed to create script node object");
    } else if(string_store(string, &script->store, &node->identifier)) {
        status = panic("failed to store string object");
    } else {
        *result = node;
    }

    return status;
}

void script_node_push(struct script_node * root, ...) {
    va_list args;
    struct script_node * node;

    va_start(args, root);
    node = va_arg(args, struct script_node *);
    while(node) {
        node->next = root->node;
        root->node = node;
        node = va_arg(args, struct script_node *);
    }
    va_end(args);
}

struct script_node * script_node_reverse(struct script_node * root) {
    struct script_node * list;
    struct script_node * node;

    list = NULL;
    while(root) {
        node = root;
        root = root->next;
        node->next = list;
        list = node;
    }

    return list;
}
