#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_initialize(struct script *);

int script_map_push(struct script *);
void script_map_pop(struct script *);
void script_map_clear(struct script *);
int script_map_insert(struct script *, struct script_range *);
struct script_range * script_map_search(struct script *, char *);

int script_logic_push(struct script *);
void script_logic_pop(struct script *);
void script_logic_clear(struct script *);
int script_logic_top_push(struct script *, enum logic_type, void *);
int script_logic_top_pop(struct script *);

struct script_range * script_range(struct script *, enum script_type, char *, ...);
struct script_range * script_range_argument(struct script *, struct argument_node *);
struct script_range * script_range_constant(struct script *, struct constant_node *);
struct script_range * script_range_variable(struct script *, char *);
void script_range_clear(struct script *);

struct script_array * script_array(struct script *);
int script_array_add(struct script_array *, struct script_range *);
struct script_range * script_array_get(struct script_array *, size_t);

int script_array_push(struct script *);
void script_array_pop(struct script *);
void script_array_clear(struct script *);
struct script_array * script_array_top(struct script *);

char * script_store_strbuf(struct script *, struct strbuf *);

int script_parse(struct script *, char *);
int script_parse_loop(struct script *, struct string *);

enum script_flag {
    is_logic = 0x1,
    is_array = 0x2
};

int script_translate(struct script *, struct script_node *);
int script_evaluate(struct script *, struct script_node *, int, struct script_range **);
struct script_range * script_execute(struct script *, struct script_array *, struct argument_node *);

struct script_range * function_set(struct script *, struct script_array *);
struct script_range * function_min(struct script *, struct script_array *);
struct script_range * function_max(struct script *, struct script_array *);
struct script_range * function_pow(struct script *, struct script_array *);
struct script_range * function_rand(struct script *, struct script_array *);
struct script_range * function_first(struct script *, struct script_array *);

typedef struct script_range * (*function_cb) (struct script *, struct script_array *);

struct function_entry {
    char * identifier;
    function_cb function;
} function_array[] = {
    { "set", function_set },
    { "min", function_min },
    { "max", function_max },
    { "pow", function_pow },
    { "rand", function_rand },
    { "gettime", function_first },
    { "readparam", function_first },
    { "getequipid", function_first },
    { "vip_status", function_first },
    { NULL, NULL}
};

int argument_write(struct script *, struct script_array *, struct strbuf *, char *);
int argument_list(struct script *, struct script_array *, struct argument_node *, struct strbuf *);
int argument_sign(struct script *, struct script_array *, struct argument_node *, struct strbuf *);
int argument_zero(struct script *, struct script_array *, struct argument_node *, struct strbuf *);
int argument_string(struct script *, struct script_array *, struct argument_node *, struct strbuf *);
int argument_integer(struct script *, struct script_array *, struct argument_node *, struct strbuf *);
int argument_percent(struct script *, struct script_array *, struct argument_node *, struct strbuf *);

int argument_item(struct script *, struct script_array *, struct argument_node *, struct strbuf *);

typedef int (*argument_cb) (struct script *, struct script_array *, struct argument_node *, struct strbuf *);

struct argument_entry {
    char * identifier;
    argument_cb argument;
} argument_array[] = {
    { "list", argument_list },
    { "sign", argument_sign },
    { "zero", argument_zero },
    { "string", argument_string },
    { "integer", argument_integer },
    { "percent", argument_percent },
    { "item", argument_item },
    { NULL, NULL }
};

int script_buffer_create(struct script_buffer * buffer, size_t size, struct heap * heap) {
    int status = 0;

    buffer->size = size;
    if(!buffer->size) {
        status = panic("invalid size");
    } else {
        buffer->pool = heap_pool(heap, sizeof(struct strbuf));
        if(!buffer->pool) {
            status = panic("failed to pool heap object");
        } else if(stack_create(&buffer->stack, heap->stack_pool)) {
            status = panic("failed to create stack object");
        }
    }

    return status;
}

void script_buffer_destroy(struct script_buffer * buffer) {
    struct strbuf * strbuf;

    strbuf = stack_pop(&buffer->stack);
    while(strbuf) {
        strbuf_destroy(strbuf);
        pool_put(buffer->pool, strbuf);
        strbuf = stack_pop(&buffer->stack);
    }

    stack_destroy(&buffer->stack);
}

struct strbuf * script_buffer_get(struct script_buffer * buffer) {
    int status = 0;
    struct strbuf * strbuf;

    strbuf = stack_pop(&buffer->stack);
    if(!strbuf) {
        strbuf = pool_get(buffer->pool);
        if(!strbuf) {
            status = panic("out of memory");
        } else {
            if(strbuf_create(strbuf, buffer->size))
                status = panic("failed to create strbuf object");
            if(status)
                pool_put(buffer->pool, strbuf);
        }
    }

    return status ? NULL : strbuf;
}

void script_buffer_put(struct script_buffer * buffer, struct strbuf * strbuf) {
    strbuf_clear(strbuf);

    if(stack_push(&buffer->stack, strbuf)) {
        strbuf_destroy(strbuf);
        pool_put(buffer->pool, strbuf);
    }
}

int script_undef_create(struct script_undef * undef, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&undef->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&undef->map, (map_compare_cb) strcmp, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            store_destroy(&undef->store);
    }

    return status;
}

void script_undef_destroy(struct script_undef * undef) {
    map_destroy(&undef->map);
    store_destroy(&undef->store);
}

int script_undef_add(struct script_undef * undef, char * identifier) {
    int status = 0;
    char * string;

    if(!map_search(&undef->map, identifier)) {
        string = store_printf(&undef->store, "%s", identifier);
        if(!string) {
            status = panic("failed to printf store object");
        } else if(map_insert(&undef->map, string, string)) {
            status = panic("failed to insert map object");
        }
    }

    return status;
}

void script_undef_print(struct script_undef * undef) {
    struct map_kv kv;

    kv = map_start(&undef->map);
    while(kv.key) {
        fprintf(stdout, "%s ", (char *) kv.key);
        kv = map_next(&undef->map);
    }
}

int script_create(struct script * script, size_t size, struct heap * heap, struct table * table) {
    int status = 0;

    script->heap = heap;
    if(!script->heap) {
        status = panic("invalid heap object");
    } else {
        script->table = table;
        if(!script->table) {
            status = panic("invalid table object");
        } else {
            if(scriptlex_init_extra(&script->store, &script->scanner)) {
                status = panic("failed to create scanner object");
            } else {
                script->parser = scriptpstate_new();
                if(!script->parser) {
                    status = panic("failed to create parser object");
                    goto parser_fail;
                } else if(store_create(&script->store, size)) {
                    status = panic("failed to create store object");
                    goto store_fail;
                } else if(stack_create(&script->map, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto map_fail;
                } else if(stack_create(&script->logic, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto logic_fail;
                } else if(stack_create(&script->range, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto range_fail;
                } else if(stack_create(&script->array, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto array_fail;
                } else if(map_create(&script->function, (map_compare_cb) strcmp, heap->map_pool)) {
                    status = panic("failed to create map object");
                    goto function_fail;
                } else if(map_create(&script->argument, (map_compare_cb) strcmp, heap->map_pool)) {
                    status = panic("failed to create map object");
                    goto argument_fail;
                } else if(script_buffer_create(&script->buffer, size, heap)) {
                    status = panic("failed to create script buffer object");
                    goto buffer_fail;
                } else if(script_undef_create(&script->undef, size, heap)) {
                    status = panic("failed to create script undef object");
                    goto undef_fail;
                } else if(script_initialize(script)) {
                    status = panic("failed to initialize script object");
                    goto initialize_fail;
                }
            }
        }
    }

    return status;

initialize_fail:
    script_undef_destroy(&script->undef);
undef_fail:
    script_buffer_destroy(&script->buffer);
buffer_fail:
    map_destroy(&script->argument);
argument_fail:
    map_destroy(&script->function);
function_fail:
    stack_destroy(&script->array);
array_fail:
    stack_destroy(&script->range);
range_fail:
    stack_destroy(&script->logic);
logic_fail:
    stack_destroy(&script->map);
map_fail:
    store_destroy(&script->store);
store_fail:
    scriptpstate_delete(script->parser);
parser_fail:
    scriptlex_destroy(script->scanner);

    return status;
}

void script_destroy(struct script * script) {
    script_undef_destroy(&script->undef);
    script_buffer_destroy(&script->buffer);
    map_destroy(&script->argument);
    map_destroy(&script->function);
    stack_destroy(&script->array);
    stack_destroy(&script->range);
    stack_destroy(&script->logic);
    stack_destroy(&script->map);
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_compile(struct script * script, char * string) {
    int status = 0;

    if(script_parse(script, string)) {
        status = panic("failed to parse script object");
    } else if(script_translate(script, script->root)) {
        status = panic("failed to translate script object");
    }

    script_array_clear(script);
    script_range_clear(script);
    script_logic_clear(script);
    script_map_clear(script);
    store_clear(&script->store);

    return status;
}

int script_initialize(struct script * script) {
    int status = 0;
    struct function_entry * function;
    struct argument_entry * argument;

    function = function_array;
    while(function->identifier) {
        if(map_insert(&script->function, function->identifier, function->function)) {
            status = panic("failed to insert map object");
        } else {
            function++;
        }
    }

    argument = argument_array;
    while(argument->identifier) {
        if(map_insert(&script->argument, argument->identifier, argument->argument)) {
            status = panic("failed to insert map object");
        } else {
            argument++;
        }
    }

    return status;
}

int script_map_push(struct script * script) {
    int status = 0;
    struct map * map;
    struct map * top;

    map = store_malloc(&script->store, sizeof(*map));
    if(!map) {
        status = panic("failed to object store object");
    } else {
        top = stack_top(&script->map);
        if(top ? map_copy(map, top) : map_create(map, (map_compare_cb) strcmp, script->heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(stack_push(&script->map, map))
                status = panic("failed to push stack object");
            if(status)
                map_destroy(map);
        }
    }

    return status;
}

void script_map_pop(struct script * script) {
    struct map * map;

    map = stack_pop(&script->map);
    if(map)
        map_destroy(map);
}

void script_map_clear(struct script * script) {
    struct map * map;

    map = stack_pop(&script->map);
    while(map) {
        map_destroy(map);
        map = stack_pop(&script->map);
    }
}

int script_map_insert(struct script * script, struct script_range * range) {
    int status = 0;
    struct map * map;

    map = stack_top(&script->map);
    if(!map) {
        status = panic("invalid map");
    } else if(map_insert(map, range->string, range)) {
        status = panic("failed to insert map object");
    }

    return status;
}

struct script_range * script_map_search(struct script * script, char * identifier) {
    struct map * map;

    map = stack_top(&script->map);

    return map ? map_search(map, identifier) : NULL;
}

int script_logic_push(struct script * script) {
    int status = 0;
    struct logic * logic;
    struct logic * top;

    logic = store_malloc(&script->store, sizeof(*logic));
    if(!logic) {
        status = panic("failed to object store object");
    } else {
        top = stack_top(&script->logic);
        if(top ? logic_copy(logic, top) : logic_create(logic, script->heap->logic_pool)) {
            status = panic("failed to create logic object");
        } else {
            if(stack_push(&script->logic, logic))
                status = panic("failed to push stack object");
            if(status)
                logic_destroy(logic);
        }
    }

    return status;
}

void script_logic_pop(struct script * script) {
    struct logic * logic;

    logic = stack_pop(&script->logic);
    if(logic)
        logic_destroy(logic);
}

void script_logic_clear(struct script * script) {
    struct logic * logic;

    logic = stack_pop(&script->logic);
    while(logic) {
        logic_destroy(logic);
        logic = stack_pop(&script->logic);
    }
}

int script_logic_top_push(struct script * script, enum logic_type type, void * data) {
    int status = 0;
    struct logic * logic;

    logic = stack_top(&script->logic);
    if(!logic) {
        status = panic("invalid logic");
    } else if(logic_push(logic, type, data)) {
        status = panic("failed to push logic object");
    }

    return status;
}

int script_logic_top_pop(struct script * script) {
    int status = 0;
    struct logic * logic;

    logic = stack_top(&script->logic);
    if(!logic) {
        status = panic("invalid logic");
    } else if(logic_pop(logic)) {
        status = panic("failed to pop logic object");
    }

    return status;
}

struct script_range * script_range(struct script * script, enum script_type type, char * format, ...) {
    int status = 0;
    va_list vararg;
    struct script_range * range;

    va_start(vararg, format);

    range = store_malloc(&script->store, sizeof(*range));
    if(!range) {
        status = panic("failed to object store object");
    } else {
        range->type = type;
        range->range = store_malloc(&script->store, sizeof(*range->range));
        if(!range->range) {
            status = panic("failed to object store object");
        } else {
            if(range_create(range->range, script->heap->range_pool)) {
                status = panic("failed to create range object");
            } else {
                range->string = store_vprintf(&script->store, format, vararg);
                if(!range->string) {
                    status = panic("failed to vprintf store object");
                } else if(stack_push(&script->range, range)) {
                    status = panic("failed to push stack object");
                }
                if(status)
                    range_destroy(range->range);
            }
        }
    }

    va_end(vararg);

    return status ? NULL : range;
}

struct script_range * script_range_argument(struct script * script, struct argument_node * argument) {
    int status = 0;
    struct script_range * range;

    range = script_range(script, identifier, "%s", argument->identifier);
    if(!range) {
        status = panic("failed to range script object");
    } else if(argument->range) {
        if(range_add_list(range->range, argument->range))
            status = panic("failed to add list range object");
    }

    return status ? NULL : range;
}

struct script_range * script_range_constant(struct script * script, struct constant_node * constant) {
    int status = 0;
    struct script_range * range;

    range = script_range(script, integer, "%s", constant->tag ? constant->tag : constant->identifier);
    if(!range) {
        status = panic("failed to range script object");
    } else {
        if(constant->range) {
            if(range_add_list(range->range, constant->range))
                status = panic("failed to add list range object");
        } else {
            if(range_add(range->range, constant->value, constant->value))
                status = panic("failed to add range object");
        }
    }

    return status ? NULL : range;
}

struct script_range * script_range_variable(struct script * script, char * variable) {
    int status = 0;
    struct script_range * range;

    range = script_map_search(script, variable);
    if(!range) {
        range = script_range(script, identifier, "%s", variable);
        if(!range)
            status = panic("failed to range script object");
    }

    return status ? NULL : range;
}

void script_range_clear(struct script * script) {
    struct script_range * range;

    range = stack_pop(&script->range);
    while(range) {
        range_destroy(range->range);
        range = stack_pop(&script->range);
    }
}

struct script_array * script_array(struct script * script) {
    int status = 0;
    struct script_array * array;

    array = store_malloc(&script->store, sizeof(*array) + sizeof(*array->array) * ARRAY_TOTAL);
    if(!array) {
        status = panic("failed to object store object");
    } else {
        array->array = (void **) (array + 1);
        array->count = 0;
        array->total = ARRAY_TOTAL;
    }

    return status ? NULL : array;
}

int script_array_add(struct script_array * array, struct script_range * range) {
    int status = 0;

    if(array->count >= array->total) {
        status = panic("out of memory");
    } else {
        array->array[array->count] = range;
        array->count++;
    }

    return status;
}

struct script_range * script_array_get(struct script_array * array, size_t index) {
    return index < array->count ? array->array[index] : NULL;
}

int script_array_push(struct script * script) {
    int status = 0;
    struct script_array * array;

    array = script_array(script);
    if(!array) {
        status = panic("failed to array script object");
    } else if(stack_push(&script->array, array)) {
        status = panic("failed to push stack object");
    }

    return status;
}

void script_array_pop(struct script * script) {
    stack_pop(&script->array);
}

void script_array_clear(struct script * script) {
    stack_clear(&script->array);
}

struct script_array * script_array_top(struct script * script) {
    return stack_top(&script->array);
}

char * script_store_strbuf(struct script * script, struct strbuf * strbuf) {
    int status = 0;

    char * result;
    struct string * string;

    string = strbuf_string(strbuf);
    if(!string) {
        status = panic("failed to string strbuf object");
    } else {
        result = store_strcpy(&script->store, string->string, string->length);
        if(!result)
            status = panic("failed to strcpy store object");
    }

    return status ? NULL : result;
}

int script_parse(struct script * script, char * string) {
    int status = 0;
    struct strbuf * strbuf;
    struct string * buffer;

    strbuf = script_buffer_get(&script->buffer);
    if(!strbuf) {
        status = panic("failed to get script buffer object");
    } else {
        if(strbuf_printf(strbuf, "%s", string)) {
            status = panic("failed to printf strbuf object");
        } else if(strbuf_putcn(strbuf, '\0', 2)) {
            status = panic("failed to putcn strbuf object");
        } else {
            buffer = strbuf_string(strbuf);
            if(!buffer) {
                status = panic("failed to string strbuf object");
            } else if(script_parse_loop(script, buffer)) {
                status = panic("failed to parse loop script object");
            }
        }
        script_buffer_put(&script->buffer, strbuf);
    }

    return status;
}

int script_parse_loop(struct script * script, struct string * string) {
    int status = 0;

    yyscan_t scanner = script->scanner;
    scriptpstate * parser = script->parser;

    YY_BUFFER_STATE buffer;
    SCRIPTSTYPE value;
    int token;
    int state = YYPUSH_MORE;

    buffer = script_scan_buffer(string->string, string->length, scanner);
    if(!buffer) {
        status = panic("failed to scan buffer scanner object");
    } else {
        while(state == YYPUSH_MORE && !status) {
            token = scriptlex(&value, scanner);
            if(token < 0) {
                status = panic("failed to get the next token");
            } else {
                state = scriptpush_parse(parser, token, &value, script);
                if(state && state != YYPUSH_MORE)
                    status = panic("failed to parse the current token");
            }
        }
        scriptpop_buffer_state(scanner);
    }

    return status;
}

int script_translate(struct script * script, struct script_node * root) {
    int status = 0;
    struct script_node * node;
    struct script_range * range;

    switch(root->token) {
        case script_curly_open:
            if(script_map_push(script)) {
                status = panic("failed to map push script object");
            } else {
                node = root->root;
                while(node && !status) {
                    if(script_translate(script, node)) {
                        status = panic("failed to statement script object");
                    } else {
                        node = node->next;
                    }
                }
                script_map_pop(script);
            }
            break;
        case script_semicolon:
            /* empty statement */
            break;
        case script_for:
            /* unsupport loop */
            break;
        case script_if:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                if(script_evaluate(script, root->root, is_logic, &range)) {
                    status = panic("failed to expression script object");
                } else if(script_translate(script, root->root->next)) {
                    status = panic("failed to statement script object");
                }
                script_logic_pop(script);
            }
            break;
        case script_else:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                if(script_logic_top_push(script, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if(script_evaluate(script, root->root, is_logic, &range)) {
                        status = panic("failed to expression script object");
                    } else if(script_translate(script, root->root->next)) {
                        status = panic("failed to statement script object");
                    } else if(script_logic_top_pop(script)) {
                        status = panic("failed to logic top pop script object");
                    } else if(script_translate(script, root->root->next->next)) {
                        status = panic("failed to statement script object");
                    }
                }
                script_logic_pop(script);
            }
            break;
        default:
            if(script_evaluate(script, root, 0, &range))
                status = panic("failed to expression script object");
            break;
    }

    return status;
}

int script_evaluate(struct script * script, struct script_node * root, int flag, struct script_range ** result) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;

    struct script_range * range;
    struct script_array * array;

    function_cb function;
    struct argument_node * argument;
    struct constant_node * constant;

    switch(root->token) {
        case script_integer:
            range = script_range(script, integer, "%ld", root->integer);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, root->integer, root->integer)) {
                status = panic("failed to add range object");
            } else {
                *result = range;
            }
            break;
        case script_identifier:
            if(root->root) {
                if(script_array_push(script)) {
                    status = panic("failed to array push script object");
                } else {
                    if(script_evaluate(script, root->root, flag | is_array, &x)) {
                        status = panic("failed to evaluate script object");
                    } else {
                        array = script_array_top(script);
                        if(!array) {
                            status = panic("invalid array");
                        } else if(!script_array_get(array, 0) && script_array_add(array, x)) {
                            status = panic("failed to add script array object");
                        } else {
                            function = map_search(&script->function, root->identifier);
                            if(function) {
                                range = function(script, array);
                                if(!range) {
                                    status = panic("failed to function range script object");
                                } else {
                                    *result = range;
                                }
                            } else {
                                argument = argument_identifier(script->table, root->identifier);
                                if(argument) {
                                    range = script_execute(script, array, argument);
                                    if(!range) {
                                        status = panic("failed to execute script object");
                                    } else {
                                        *result = range;
                                    }
                                } else {
                                    if(script_undef_add(&script->undef, root->identifier)) {
                                        status = panic("failed to add script undef object");
                                    } else {
                                        range = script_range(script, identifier, "%s", root->identifier);
                                        if(!range) {
                                            status = panic("failed to range script object");
                                        } else {
                                            *result = range;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    script_array_pop(script);
                }
            } else {
                argument = argument_identifier(script->table, root->identifier);
                if(argument) {
                    range = script_execute(script, array, argument);
                    if(!range) {
                        status = panic("failed to execute script object");
                    } else {
                        *result = range;
                    }
                } else {
                    constant = constant_identifier(script->table, root->identifier);
                    if(constant) {
                        range = script_range_constant(script, constant);
                        if(!range) {
                            status = panic("failed to range constant script object");
                        } else {
                            *result = range;
                        }
                    } else {
                        range = script_range_variable(script, root->identifier);
                        if(!range) {
                            status = panic("failed to range variable script object");
                        } else {
                            *result = range;
                        }
                    }
                }
            }
            break;
        case script_comma:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_array) {
                    array = script_array_top(script);
                    if(!array) {
                        status = panic("invalid array");
                    } else {
                        if(root->root->token == script_comma) {
                            if(script_array_add(array, y))
                                status = panic("failed to add script array object");
                        } else if(root->root->next->token == script_comma) {
                            if(script_array_add(array, x))
                                status = panic("failed to add script array object");
                        } else {
                            if(script_array_add(array, x) || script_array_add(array, y))
                                status = panic("failed to add script array object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s", y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_assign(range->range, y->range)) {
                        status = panic("failed to assign range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_assign(range->range, y->range)) {
                    status = panic("failed to assign range object");
                } else if(script_map_insert(script, range)) {
                    status = panic("failed to map insert script object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_plus_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else if(script_map_insert(script, range)) {
                    status = panic("failed to map insert script object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_minus_assign:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else if(script_map_insert(script, range)) {
                    status = panic("failed to map insert script object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_question:
            if(script_logic_push(script)) {
                status = panic("failed to logic push script object");
            } else {
                if(script_logic_top_push(script, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag | is_logic, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else {
                        range = script_range(script, integer, "%s ? %s", x->string, y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_assign(range->range, y->range)) {
                            status = panic("failed to assign range object");
                        } else {
                            *result = range;
                        }
                    }
                }
                script_logic_pop(script);
            }
            break;
        case script_colon:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else if(script_logic_top_pop(script)) {
                status = panic("failed to logic top pop script object");
            } else if(script_evaluate(script, root->root->next, flag, &y)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s : %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_or(range->range, x->range, y->range)) {
                    status = panic("failed to or range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_or:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s | %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_or(range->range, x->range, y->range)) {
                    status = panic("failed to bit or range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_xor:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s ^ %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_xor(range->range, x->range, y->range)) {
                    status = panic("failed to bit xor range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_and:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s & %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_and(range->range, x->range, y->range)) {
                    status = panic("failed to bit and range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_left:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s << %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_left(range->range, x->range, y->range)) {
                    status = panic("failed to bit left range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_right:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s >> %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_right(range->range, x->range, y->range)) {
                    status = panic("failed to bit right range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_plus:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s + %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_minus:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s - %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_multiply:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s * %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_multiply(range->range, x->range, y->range)) {
                    status = panic("failed to multiply range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_divide:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s / %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_divide(range->range, x->range, y->range)) {
                    status = panic("failed to divide range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_remainder:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s %% %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_remainder(range->range, x->range, y->range)) {
                    status = panic("failed to remainder range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_plus_unary:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "+ %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus_unary(range->range, x->range)) {
                    status = panic("failed to plus unary range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_minus_unary:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "- %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus_unary(range->range, x->range)) {
                    status = panic("failed to minus unary range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_bit_not:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "~ %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_bit_not(range->range, x->range)) {
                    status = panic("failed to bit not range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_increment_prefix:
        case script_increment_postfix:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s ++", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_increment(range->range, x->range)) {
                    status = panic("failed to increment range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_decrement_prefix:
        case script_decrement_postfix:
            if(script_evaluate(script, root->root, flag, &x)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range(script, integer, "%s --", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_decrement(range->range, x->range)) {
                    status = panic("failed to decrement range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_or:
            if(flag & is_logic) {
                if(script_logic_top_push(script, or, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(script_logic_top_pop(script)) {
                        status = panic("failed to logic top pop script object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, integer, "%s || %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_and:
            if(flag & is_logic) {
                if(script_logic_top_push(script, and, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(script_logic_top_pop(script)) {
                        status = panic("failed to logic top pop script object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, integer, "%s && %s", x->string, y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_not:
            if(flag & is_logic) {
                if(script_logic_top_push(script, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else if(script_evaluate(script, root->root, flag, &x)) {
                    status = panic("failed to evaluate script object");
                } else if(script_logic_top_pop(script)) {
                    status = panic("failed to logic top pop script object");
                }
            } else {
                if(script_evaluate(script, root->root, flag, &x))
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range(script, integer, "! %s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_add(range->range, 0, 1)) {
                    status = panic("failed to add range object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_equal(range->range, x->range, y->range)) {
                            status = panic("failed to equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_equal(range->range, y->range, x->range)) {
                            status = panic("failed to equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s == %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_not_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_not_equal(range->range, x->range, y->range)) {
                            status = panic("failed to not equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_not_equal(range->range, y->range, x->range)) {
                            status = panic("failed to not equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s != %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_lesser:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser(range->range, x->range, y->range)) {
                            status = panic("failed to lesser range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater(range->range, y->range, x->range)) {
                            status = panic("failed to greater range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s < %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_lesser_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser_equal(range->range, x->range, y->range)) {
                            status = panic("failed to lesser equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater_equal(range->range, y->range, x->range)) {
                            status = panic("failed to greater equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s <= %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_greater:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater(range->range, x->range, y->range)) {
                            status = panic("failed to greater range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser(range->range, y->range, x->range)) {
                            status = panic("failed to lesser range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s > %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        case script_greater_equal:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_logic) {
                    if(x->type == identifier) {
                        range = script_range(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater_equal(range->range, x->range, y->range)) {
                            status = panic("failed to greater equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser_equal(range->range, y->range, x->range)) {
                            status = panic("failed to lesser equal range object");
                        } else if(script_logic_top_push(script, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range(script, integer, "%s >= %s", x->string, y->string);
                    if(!range) {
                        status = panic("failed to range script object");
                    } else if(range_add(range->range, 0, 1)) {
                        status = panic("failed to add range object");
                    } else {
                        *result = range;
                    }
                }
            }
            break;
        default:
            status = panic("invalid token - %d", root->token);
            break;
    }

    return status;
}

struct script_range * script_execute(struct script * script, struct script_array * array, struct argument_node * argument) {
    int status = 0;
    argument_cb callback;
    struct strbuf * strbuf;
    struct script_range * range;

    callback = map_search(&script->argument, argument->argument);
    if(!callback) {
        status = panic("invalid argument - %s", argument->argument);
    } else {
        range = script_range_argument(script, argument);
        if(!range) {
            status = panic("failed to range argument script object");
        } else {
            strbuf = script_buffer_get(&script->buffer);
            if(!strbuf) {
                status = panic("failed to get script buffer object");
            } else {
                if(callback(script, array, argument, strbuf)) {
                    status = panic("failed to execute argument object");
                } else {
                    range->string = script_store_strbuf(script, strbuf);
                    if(!range->string)
                        status = panic("failed to range string script object");
                }
                script_buffer_put(&script->buffer, strbuf);
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_set(struct script * script, struct script_array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = script_array_get(array, 0);
    if(!x) {
        status = panic("invalid indentifier");
    } else {
        y = script_array_get(array, 1);
        if(!y) {
            status = panic("invalid expression");
        } else {
            range = script_range(script, identifier, "%s", x->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_assign(range->range, y->range)) {
                status = panic("failed to assign range object");
            }  else if(script_map_insert(script, range)) {
                status = panic("failed to map insert script object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_min(struct script * script, struct script_array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = script_array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = script_array_get(array, 1);
        if(!y) {
            status = panic("invalid max");
        } else {
            range = script_range(script, integer, "min(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_min(range->range, x->range, y->range)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_max(struct script * script, struct script_array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = script_array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = script_array_get(array, 1);
        if(!y) {
            status = panic("invalid max");
        } else {
            range = script_range(script, integer, "max(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_max(range->range, x->range, y->range)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_pow(struct script * script, struct script_array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = script_array_get(array, 0);
    if(!x) {
        status = panic("invalid base");
    } else {
        y = script_array_get(array, 1);
        if(!y) {
            status = panic("invalid power");
        } else {
            range = script_range(script, integer, "pow(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_pow(range->range, x->range, y->range)) {
                status = panic("failed to pow range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_rand(struct script * script, struct script_array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = script_array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = script_array_get(array, 1);
        if(!y) {
            range = script_range(script, integer, "rand(%s)", x->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, 0, x->range->max - 1)) {
                status = panic("failed to add range object");
            }
        } else {
            range = script_range(script, integer, "rand(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, x->range->min, y->range->max)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_first(struct script * script, struct script_array * array) {
    return script_array_get(array, 0);
}

int argument_write(struct script * script, struct script_array * array, struct strbuf * strbuf, char * string) {
    int status = 0;

    char * anchor;
    struct script_range * range;
    struct script_array * subset;
    struct argument_node * argument;
    argument_cb callback;

    anchor = string;
    while(*string && !status) {
        if(*string == '{') {
            subset = script_array(script);
            if(!subset) {
                status = panic("failed to array script object");
            } else if(strbuf_strcpy(strbuf, anchor, string - anchor)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                do {
                    range = script_array_get(array, strtol(string + 1, &string, 10));
                    if(!range) {
                        status = panic("failed to get script array object");
                    } else if(script_array_add(subset, range)) {
                        status = panic("failed to add script array object");
                    }
                } while(*string == ',' && !status);

                if(*string != '|') {
                    status = panic("expected vertical bar");
                } else {
                    anchor = string + 1;
                    string = strchr(string, '}');
                    if(!string) {
                        status = panic("expected curly close");
                    } else {
                        anchor = store_strcpy(&script->store, anchor, string - anchor);
                        if(!anchor) {
                            status = panic("failed to strcpy store object");
                        } else {
                            argument = argument_identifier(script->table, anchor);
                            if(argument) {
                                range = script_execute(script, subset, argument);
                                if(!range) {
                                    status = panic("failed to execute script object");
                                } else if(strbuf_printf(strbuf, "%s", range->string)) {
                                    status = panic("failed to printf strbuf object");
                                }
                            } else {
                                callback = map_search(&script->argument, anchor);
                                if(!callback) {
                                    status = panic("invalid argument - %s", anchor);
                                } else if(callback(script, subset, argument, strbuf)) {
                                    status = panic("failed to execute argument object");
                                }
                            }
                            anchor = ++string;
                        }
                    }
                }
            }
        } else {
            string++;
        }
    }

    if(!status)
        if(strbuf_strcpy(strbuf, anchor, string - anchor))
            status = panic("failed to strcpy strbuf object");

    return status;
}

int argument_list(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct data_node * data;

    data = argument->data;
    while(data && !status) {
        if(argument_write(script, array, strbuf, data->string)) {
            status = panic("failed to parse argument object");
        } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
            status = panic("failed to putcn strbuf object");
        } else {
            data = data->next;
        }
    }

    return status;
}

int argument_sign(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct data_node * data;
    struct script_range * range;

    range = script_array_get(array, 0);
    if(!range) {
        status = panic("failed to get script array object");
    } else {
        if(!argument->data) {
            status = panic("invalid data");
        } else {
            data = argument->data;
            if(range->range->max < 0)
                data = data->next;

            if(!data) {
                status = panic("invalid data");
            } else if(argument_write(script, array, strbuf, data->string)) {
                status = panic("failed to write argument object");
            } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
                status = panic("failed to putcn strbuf object");
            }
        }
    }

    return status;
}

int argument_zero(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    if(!argument->data) {
        status = panic("invalid data");
    } else {
        range = script_array_get(array, 0);
        if(!range) {
            status = panic("failed to get script array object");
        } else if(range->range->min && range->range->max) {
            if(argument_write(script, array, strbuf, argument->data->string)) {
                status = panic("failed to write argument object");
            } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
                status = panic("failed to putcn strbuf object");
            }
        }
    }

    return status;
}

int argument_string(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    range = script_array_get(array, 0);
    if(!range) {
        status = panic("failed to get script array object");
    } else if(strbuf_printf(strbuf, "%s", range->string)) {
        status = panic("failed to printf strbuf object");
    }

    return status;
}

int argument_integer(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    range = script_array_get(array, 0);
    if(!range) {
        status = panic("failed to get script array object");
    } else {
        if(range->range->min == range->range->max) {
            if(strbuf_printf(strbuf, "%ld", range->range->min))
                status = panic("failed to printf strbuf object");
        } else {
            if(strbuf_printf(strbuf, "%ld ~ %ld", range->range->min, range->range->max))
                status = panic("failed to printf strbuf object");
        }
    }

    return status;
}

int argument_percent(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    range = script_array_get(array, 0);
    if(!range) {
        status = panic("failed to get script array object");
    } else {
        if(range->range->min == range->range->max) {
            if(strbuf_printf(strbuf, "%ld%%", range->range->min))
                status = panic("failed to printf strbuf object");
        } else {
            if(strbuf_printf(strbuf, "%ld%% ~ %ld%%", range->range->min, range->range->max))
                status = panic("failed to printf strbuf object");
        }
    }

    return status;
}

int argument_item(struct script * script, struct script_array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;
    struct range_node * node;

    long i;
    struct item_node * item;

    range = script_array_get(array, 0);
    if(!range) {
        status = panic("failed to get script array object");
    } else {
        node = range->range->root;
        while(node && !status) {
            for(i = node->min; i <= node->max && !status; i++) {
                item = item_id(script->table, i);
                if(!item) {
                    status = panic("invalid item id - %ld", i);
                } else if(strbuf_printf(strbuf, "%s", item->name)) {
                    status = panic("failed to printf strbuf object");
                }
            }
            node = node->next;
        }
    }

    return status;
}
