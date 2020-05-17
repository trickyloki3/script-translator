#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

int script_map_push(struct script *, struct map *);
void script_map_pop(struct script *);

int script_logic_push(struct script *, struct logic *);
void script_logic_pop(struct script *);

int script_array_push(struct script *, struct array *);
void script_array_pop(struct script *);

struct script_range * script_range_create(struct script *, enum script_type, char *, ...);
struct script_range * script_range_argument(struct script *, struct argument_node *);
struct script_range * script_range_constant(struct script *, struct constant_node *);

enum script_flag {
    is_logic = 0x1,
    is_array = 0x2
};

int script_parse(struct script *, char *);
int script_translate(struct script *, struct script_node *);
int script_evaluate(struct script *, struct script_node *, int, struct script_range **);
struct script_range * script_execute(struct script *, struct array *, struct argument_node *);

struct script_range * function_set(struct script *, struct array *);
struct script_range * function_min(struct script *, struct array *);
struct script_range * function_max(struct script *, struct array *);
struct script_range * function_pow(struct script *, struct array *);
struct script_range * function_rand(struct script *, struct array *);

typedef struct script_range * (*function_cb) (struct script *, struct array *);

struct function_entry {
    char * identifier;
    function_cb function;
} function_list[] = {
    { "set", function_set },
    { "min", function_min },
    { "max", function_max },
    { "pow", function_pow },
    { "rand", function_rand },
    { NULL, NULL}
};

int argument_write(struct script *, struct array *, struct strbuf *, char *);
int argument_description(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_sign(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_zero(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_array(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_integer(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_string(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_second(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_millisecond(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_item(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_skill(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_mob(struct script *, struct array *, struct argument_node *, struct strbuf *);
int argument_mercenary(struct script *, struct array *, struct argument_node *, struct strbuf *);

typedef int (*argument_cb) (struct script *, struct array *, struct argument_node *, struct strbuf *);

struct argument_entry {
    char * identifier;
    argument_cb argument;
} argument_list[] = {
    { "description", argument_description },
    { "sign", argument_sign },
    { "zero", argument_zero },
    { "array", argument_array },
    { "integer", argument_integer },
    { "string", argument_string },
    { "second", argument_second },
    { "millisecond", argument_millisecond },
    { "item", argument_item },
    { "skill", argument_skill },
    { "mob", argument_mob },
    { "mercenary", argument_mercenary },
    { NULL, NULL }
};

int array_add(struct array * array, struct script_range * range) {
    int status = 0;

    if(array->count >= ARRAY_TOTAL) {
        status = panic("out of memory");
    } else {
        array->array[array->count] = range;
        array->count++;
    }

    return status;
}

struct script_range * array_get(struct array * array, size_t index) {
    return index < array->count ? array->array[index] : NULL;
}

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

int undefined_create(struct undefined * undef, size_t size, struct heap * heap) {
    int status = 0;

    if(strbuf_create(&undef->strbuf, size)) {
        status = panic("failed to create strbuf object");
    } else {
        if(store_create(&undef->store, size)) {
            status = panic("failed to create store object");
        } else {
            if(map_create(&undef->map, (map_compare_cb) strcmp, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                store_destroy(&undef->store);
        }
        if(status)
            strbuf_destroy(&undef->strbuf);
    }

    return status;
}

void undefined_destroy(struct undefined * undef) {
    map_destroy(&undef->map);
    store_destroy(&undef->store);
    strbuf_destroy(&undef->strbuf);
}

int undefined_add(struct undefined * undef, char * format, ...) {
    int status = 0;

    va_list vararg;
    va_list varcpy;
    char * string;

    va_start(vararg, format);
    va_copy(varcpy, vararg);

    if(strbuf_vprintf(&undef->strbuf, format, vararg)) {
        status = panic("failed to vprintf strbuf object");
    } else {
        string = strbuf_array(&undef->strbuf);
        if(!string) {
            status = panic("failed to array strbuf object");
        } else {
            if(!map_search(&undef->map, string)) {
                string = store_vprintf(&undef->store, format, varcpy);
                if(!string) {
                    status = panic("failed to printf store object");
                } else if(map_insert(&undef->map, string, string)) {
                    status = panic("failed to insert map object");
                }
            }
        }
        strbuf_clear(&undef->strbuf);
    }

    va_end(varcpy);
    va_end(vararg);

    return status;
}

void undefined_print(struct undefined * undef) {
    struct map_kv kv;

    kv = map_start(&undef->map);
    while(kv.key) {
        fprintf(stdout, "%s ", (char *) kv.key);
        kv = map_next(&undef->map);
    }
}

int script_create(struct script * script, size_t size, struct heap * heap, struct table * table) {
    int status = 0;

    struct function_entry * function;
    struct argument_entry * argument;

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
                } else if(stack_create(&script->map_stack, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto map_fail;
                } else if(stack_create(&script->logic_stack, heap->stack_pool)) {
                    status = panic("failed to create stack object");
                    goto logic_fail;
                } else if(stack_create(&script->array_stack, heap->stack_pool)) {
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
                } else if(undefined_create(&script->undefined, size, heap)) {
                    status = panic("failed to create undefined object");
                    goto undef_fail;
                } else {
                    function = function_list;
                    while(function->identifier && !status) {
                        if(map_insert(&script->function, function->identifier, function->function)) {
                            status = panic("failed to insert map object");
                        } else {
                            function++;
                        }
                    }

                    argument = argument_list;
                    while(argument->identifier && !status) {
                        if(map_insert(&script->argument, argument->identifier, argument->argument)) {
                            status = panic("failed to insert map object");
                        } else {
                            argument++;
                        }
                    }

                    if(status)
                        goto script_fail;
                }
            }
        }
    }

    return status;

script_fail:
    undefined_destroy(&script->undefined);
undef_fail:
    script_buffer_destroy(&script->buffer);
buffer_fail:
    map_destroy(&script->argument);
argument_fail:
    map_destroy(&script->function);
function_fail:
    stack_destroy(&script->array_stack);
array_fail:
    stack_destroy(&script->logic_stack);
logic_fail:
    stack_destroy(&script->map_stack);
map_fail:
    store_destroy(&script->store);
store_fail:
    scriptpstate_delete(script->parser);
parser_fail:
    scriptlex_destroy(script->scanner);

    return status;
}

void script_destroy(struct script * script) {
    undefined_destroy(&script->undefined);
    script_buffer_destroy(&script->buffer);
    map_destroy(&script->argument);
    map_destroy(&script->function);
    stack_destroy(&script->array_stack);
    stack_destroy(&script->logic_stack);
    stack_destroy(&script->map_stack);
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_compile(struct script * script, char * string) {
    int status = 0;

    script->root = NULL;
    script->map = NULL;
    script->logic = NULL;
    script->array = NULL;
    script->range = NULL;

    if(script_parse(script, string)) {
        status = panic("failed to parse script object");
    } else if(script_translate(script, script->root)) {
        status = panic("failed to translate script object");
    }

    while(script->range) {
        range_destroy(script->range->range);
        script->range = script->range->next;
    }

    store_clear(&script->store);

    return status;
}

int script_map_push(struct script * script, struct map * map) {
    int status = 0;

    if(script->map) {
        if(stack_push(&script->map_stack, script->map)) {
            status = panic("failed to push stack object");
        } else if(map_copy(map, script->map)) {
            status = panic("failed to copy map object");
        } else {
            script->map = map;
        }
    } else {
        if(map_create(map, (map_compare_cb) strcmp, script->heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            script->map = map;
        }
    }

    return status;
}

void script_map_pop(struct script * script) {
    map_destroy(script->map);

    script->map = stack_pop(&script->map_stack);
}

int script_logic_push(struct script * script, struct logic * logic) {
    int status = 0;

    if(script->logic) {
        if(stack_push(&script->logic_stack, script->logic)) {
            status = panic("failed to push stack object");
        } else if(logic_copy(logic, script->logic)) {
            status = panic("failed to copy logic object");
        } else {
            script->logic = logic;
        }
    } else {
        if(logic_create(logic, script->heap->logic_pool)) {
            status = panic("failed to create logic object");
        } else {
            if(logic_push(logic, or, NULL)) {
                status = panic("failed to push logic object");
            } else {
                script->logic = logic;
            }

            if(status)
                logic_destroy(logic);
        }
    }

    return status;
}

void script_logic_pop(struct script * script) {
    logic_destroy(script->logic);

    script->logic = stack_pop(&script->logic_stack);
}

int script_array_push(struct script * script, struct array * array) {
    int status = 0;

    array->count = 0;

    if(script->array) {
        if(stack_push(&script->array_stack, script->array)) {
            status = panic("failed to push stack object");
        } else {
            script->array = array;
        }
    } else {
        script->array = array;
    }

    return status;
}

void script_array_pop(struct script * script) {
    script->array = stack_pop(&script->array_stack);
}

struct script_range * script_range_create(struct script * script, enum script_type type, char * format, ...) {
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
                } else {
                    range->next = script->range;
                    script->range = range;
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
    struct range_node * node;

    range = script_range_create(script, identifier, "%s", argument->identifier);
    if(!range) {
        status = panic("failed to range script object");
    } else if(argument->range) {
        node = argument->range;
        while(node && !status) {
            if(range_add(range->range, node->min, node->max)) {
                status = panic("failed to add range object");
            } else {
                node = node->next;
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * script_range_constant(struct script * script, struct constant_node * constant) {
    int status = 0;
    struct script_range * range;
    struct range_node * node;

    range = script_range_create(script, integer, "%s", constant->tag ? constant->tag : constant->identifier);
    if(!range) {
        status = panic("failed to range script object");
    } else {
        if(constant->range) {
            node = constant->range;
            while(node && !status) {
                if(range_add(range->range, node->min, node->max)) {
                    status = panic("failed to add range object");
                } else {
                    node = node->next;
                }
            }
        } else {
            if(range_add(range->range, constant->value, constant->value))
                status = panic("failed to add range object");
        }
    }

    return status ? NULL : range;
}

int script_parse(struct script * script, char * string) {
    int status = 0;

    size_t length;
    char * source;

    SCRIPTSTYPE value;
    int token;
    int state = YYPUSH_MORE;

    length = strlen(string);
    source = store_malloc(&script->store, length + 2);
    if(!source) {
        status = panic("failed to malloc store object");
    } else {
        memcpy(source, string, length);
        source[length] = 0;
        source[length + 1] = 0;

        if(!script_scan_buffer(source, length + 2, script->scanner)) {
            status = panic("failed to scan buffer scanner object");
        } else {
            while(state == YYPUSH_MORE && !status) {
                token = scriptlex(&value, script->scanner);
                if(token < 0) {
                    status = panic("failed to get the next token");
                } else {
                    state = scriptpush_parse(script->parser, token, &value, script);
                    if(state && state != YYPUSH_MORE)
                        status = panic("failed to parse the current token");
                }
            }
            scriptpop_buffer_state(script->scanner);
        }
    }

    return status;
}

int script_translate(struct script * script, struct script_node * root) {
    int status = 0;

    struct map map;
    struct logic logic;

    struct script_node * node;
    struct script_range * range;

    switch(root->token) {
        case script_curly_open:
            if(script_map_push(script, &map)) {
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
            if(script_logic_push(script, &logic)) {
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
            if(script_logic_push(script, &logic)) {
                status = panic("failed to logic push script object");
            } else {
                if(logic_push(script->logic, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if(script_evaluate(script, root->root, is_logic, &range)) {
                        status = panic("failed to expression script object");
                    } else if(script_translate(script, root->root->next)) {
                        status = panic("failed to statement script object");
                    } else if(logic_pop(script->logic)) {
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

    struct logic logic;
    struct array array;

    struct script_range * x;
    struct script_range * y;

    struct script_range * range;

    function_cb function;
    struct argument_node * argument;
    struct constant_node * constant;

    switch(root->token) {
        case script_integer:
            range = script_range_create(script, integer, "%ld", root->integer);
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
                if(script_array_push(script, &array)) {
                    status = panic("failed to array push script object");
                } else {
                    if(script_evaluate(script, root->root, flag | is_array, &x)) {
                        status = panic("failed to evaluate script object");
                    } else if(!script->array->count && array_add(script->array, x)) {
                        status = panic("failed to add array object");
                    } else {
                        function = map_search(&script->function, root->identifier);
                        if(function) {
                            range = function(script, script->array);
                            if(!range) {
                                status = panic("failed to function range script object");
                            } else {
                                *result = range;
                            }
                        } else {
                            argument = argument_identifier(script->table, root->identifier);
                            if(argument) {
                                range = script_execute(script, script->array, argument);
                                if(!range) {
                                    status = panic("failed to execute script object");
                                } else {
                                    *result = range;
                                }
                            } else {
                                if(undefined_add(&script->undefined, "%s", root->identifier)) {
                                    status = panic("failed to add undefined object");
                                } else {
                                    range = script_range_create(script, identifier, "%s", root->identifier);
                                    if(!range) {
                                        status = panic("failed to range script object");
                                    } else {
                                        *result = range;
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
                    range = script_execute(script, NULL, argument);
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
                        range = map_search(script->map, root->identifier);
                        if(range) {
                            *result = range;
                        } else {
                            range = script_range_create(script, identifier, "%s", root->identifier);
                            if(!range) {
                                status = panic("failed to range script object");
                            } else {
                                *result = range;
                            }
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
                    if(root->root->token == script_comma) {
                        if(array_add(script->array, y))
                            status = panic("failed to add array object");
                    } else if(root->root->next->token == script_comma) {
                        if(array_add(script->array, x))
                            status = panic("failed to add array object");
                    } else {
                        if(array_add(script->array, x) || array_add(script->array, y))
                            status = panic("failed to add array object");
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s", y->string);
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
                range = script_range_create(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_assign(range->range, y->range)) {
                    status = panic("failed to assign range object");
                } else if(map_insert(script->map, range->string, range)) {
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
                range = script_range_create(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else if(map_insert(script->map, range->string, range)) {
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
                range = script_range_create(script, integer, "%s", x->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else if(map_insert(script->map, range->string, range)) {
                    status = panic("failed to map insert script object");
                } else {
                    *result = range;
                }
            }
            break;
        case script_question:
            if(script_logic_push(script, &logic)) {
                status = panic("failed to logic push script object");
            } else {
                if(logic_push(script->logic, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag | is_logic, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else {
                        range = script_range_create(script, integer, "%s ? %s", x->string, y->string);
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
            } else if(logic_pop(script->logic)) {
                status = panic("failed to logic top pop script object");
            } else if(script_evaluate(script, root->root->next, flag, &y)) {
                status = panic("failed to evaluate script object");
            } else {
                range = script_range_create(script, integer, "%s : %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s | %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s ^ %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s & %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s << %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s >> %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s + %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s - %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s * %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s / %s", x->string, y->string);
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
                range = script_range_create(script, integer, "%s %% %s", x->string, y->string);
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
                range = script_range_create(script, integer, "+ %s", x->string);
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
                range = script_range_create(script, integer, "- %s", x->string);
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
                range = script_range_create(script, integer, "~ %s", x->string);
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
                range = script_range_create(script, integer, "%s ++", x->string);
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
                range = script_range_create(script, integer, "%s --", x->string);
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
                if(logic_push(script->logic, or, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(logic_pop(script->logic)) {
                        status = panic("failed to logic top pop script object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range_create(script, integer, "%s || %s", x->string, y->string);
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
                if(logic_push(script->logic, and, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if( script_evaluate(script, root->root, flag, &x) ||
                        script_evaluate(script, root->root->next, flag, &y) ) {
                        status = panic("failed to evaluate script object");
                    } else if(logic_pop(script->logic)) {
                        status = panic("failed to logic top pop script object");
                    }
                }
            } else {
                if( script_evaluate(script, root->root, flag, &x) ||
                    script_evaluate(script, root->root->next, flag, &y) )
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range_create(script, integer, "%s && %s", x->string, y->string);
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
                if(logic_push(script->logic, not, NULL)) {
                    status = panic("failed to logic top push script object");
                } else if(script_evaluate(script, root->root, flag, &x)) {
                    status = panic("failed to evaluate script object");
                } else if(logic_pop(script->logic)) {
                    status = panic("failed to logic top pop script object");
                }
            } else {
                if(script_evaluate(script, root->root, flag, &x))
                    status = panic("failed to evaluate script object");
            }

            if(!status) {
                range = script_range_create(script, integer, "! %s", x->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_equal(range->range, x->range, y->range)) {
                            status = panic("failed to equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_equal(range->range, y->range, x->range)) {
                            status = panic("failed to equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s == %s", x->string, y->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_not_equal(range->range, x->range, y->range)) {
                            status = panic("failed to not equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_not_equal(range->range, y->range, x->range)) {
                            status = panic("failed to not equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s != %s", x->string, y->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser(range->range, x->range, y->range)) {
                            status = panic("failed to lesser range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater(range->range, y->range, x->range)) {
                            status = panic("failed to greater range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s < %s", x->string, y->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser_equal(range->range, x->range, y->range)) {
                            status = panic("failed to lesser equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater_equal(range->range, y->range, x->range)) {
                            status = panic("failed to greater equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s <= %s", x->string, y->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater(range->range, x->range, y->range)) {
                            status = panic("failed to greater range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser(range->range, y->range, x->range)) {
                            status = panic("failed to lesser range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s > %s", x->string, y->string);
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
                        range = script_range_create(script, identifier, "%s", x->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_greater_equal(range->range, x->range, y->range)) {
                            status = panic("failed to greater equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }

                    if(y->type == identifier && !status) {
                        range = script_range_create(script, identifier, "%s", y->string);
                        if(!range) {
                            status = panic("failed to range script object");
                        } else if(range_lesser_equal(range->range, y->range, x->range)) {
                            status = panic("failed to lesser equal range object");
                        } else if(logic_push(script->logic, cond, range)) {
                            status = panic("failed to logic top push script object");
                        }
                    }
                }

                if(!status) {
                    range = script_range_create(script, integer, "%s >= %s", x->string, y->string);
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

struct script_range * script_execute(struct script * script, struct array * array, struct argument_node * argument) {
    int status = 0;
    argument_cb handler;
    struct strbuf * strbuf;
    struct string * string;
    struct script_range * range;

    if(!argument->handler) {
        range = array_get(array, argument->index);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            range->type = identifier;
        }
    } else {
        handler = map_search(&script->argument, argument->handler);
        if(!handler) {
            status = panic("invalid argument - %s", argument->handler);
        } else {
            range = script_range_argument(script, argument);
            if(!range) {
                status = panic("failed to range argument script object");
            } else {
                strbuf = script_buffer_get(&script->buffer);
                if(!strbuf) {
                    status = panic("failed to get script buffer object");
                } else {
                    if(handler(script, array, argument, strbuf)) {
                        status = panic("failed to execute argument object");
                    } else {
                        string = strbuf_string(strbuf);
                        if(!string) {
                            status = panic("failed to string strbuf object");
                        } else {
                            range->string = store_strcpy(&script->store, string->string, string->length);
                            if(!range->string)
                                status = panic("failed to strcpy store object");
                        }
                    }
                    script_buffer_put(&script->buffer, strbuf);
                }
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_set(struct script * script, struct array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = array_get(array, 0);
    if(!x) {
        status = panic("invalid indentifier");
    } else {
        y = array_get(array, 1);
        if(!y) {
            status = panic("invalid expression");
        } else {
            range = script_range_create(script, identifier, "%s", x->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_assign(range->range, y->range)) {
                status = panic("failed to assign range object");
            }  else if(map_insert(script->map, range->string, range)) {
                status = panic("failed to map insert script object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_min(struct script * script, struct array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = array_get(array, 1);
        if(!y) {
            status = panic("invalid max");
        } else {
            range = script_range_create(script, integer, "min(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_min(range->range, x->range, y->range)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_max(struct script * script, struct array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = array_get(array, 1);
        if(!y) {
            status = panic("invalid max");
        } else {
            range = script_range_create(script, integer, "max(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_max(range->range, x->range, y->range)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_pow(struct script * script, struct array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = array_get(array, 0);
    if(!x) {
        status = panic("invalid base");
    } else {
        y = array_get(array, 1);
        if(!y) {
            status = panic("invalid power");
        } else {
            range = script_range_create(script, integer, "pow(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_pow(range->range, x->range, y->range)) {
                status = panic("failed to pow range object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_rand(struct script * script, struct array * array) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = array_get(array, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = array_get(array, 1);
        if(!y) {
            range = script_range_create(script, integer, "rand(%s)", x->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, 0, x->range->max - 1)) {
                status = panic("failed to add range object");
            }
        } else {
            range = script_range_create(script, integer, "rand(%s,%s)", x->string, y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_add(range->range, x->range->min, y->range->max)) {
                status = panic("failed to add range object");
            }
        }
    }

    return status ? NULL : range;
}

int argument_write(struct script * script, struct array * array, struct strbuf * strbuf, char * string) {
    int status = 0;

    size_t i;
    struct array subset;

    char * anchor;
    struct script_range * range;
    struct argument_node * argument;
    argument_cb handler;

    anchor = string;
    while(*string && !status) {
        if(*string == '{') {
            if(strbuf_strcpy(strbuf, anchor, string - anchor)) {
                status = panic("failed to strcpy strbuf object");
            } else {
                subset.count = 0;

                if(*(string + 1) == '*') {
                    for(i = 0; i < array->count; i++) {
                        range = array_get(array, i);
                        if(!range) {
                            status = panic("failed to get array object");
                        } else if(array_add(&subset, range)) {
                            status = panic("failed to add array object");
                        }
                    }

                    string += 2;
                } else {
                    do {
                        range = array_get(array, strtol(string + 1, &string, 10));
                        if(!range) {
                            status = panic("failed to get array object");
                        } else if(array_add(&subset, range)) {
                            status = panic("failed to add array object");
                        }
                    } while(*string == ',' && !status);
                }

                if(status) {
                    /* error */
                } else if(*string != '|') {
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
                            handler = map_search(&script->argument, anchor);
                            if(!handler) {
                                argument = argument_identifier(script->table, anchor);
                                if(argument) {
                                    range = script_execute(script, &subset, argument);
                                    if(!range) {
                                        status = panic("failed to execute script object");
                                    } else if(strbuf_printf(strbuf, "%s", range->string)) {
                                        status = panic("failed to printf strbuf object");
                                    }
                                }
                            } else if(handler(script, &subset, argument, strbuf)) {
                                status = panic("failed to execute argument object");
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

int argument_description(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct print_node * print;

    print = argument->print;
    while(print && !status) {
        if(argument_write(script, array, strbuf, print->string)) {
            status = panic("failed to parse argument object");
        } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
            status = panic("failed to putcn strbuf object");
        } else {
            print = print->next;
        }
    }

    return status;
}

int argument_sign(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct print_node * print;
    struct script_range * range;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else {
        print = argument->print;
        if(range->range->max < 0)
            print = print->next;

        if(argument_write(script, array, strbuf, print->string)) {
            status = panic("failed to write argument object");
        } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
            status = panic("failed to putcn strbuf object");
        }
    }

    return status;
}

int argument_zero(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else if(range->range->min && range->range->max) {
        if(argument_write(script, array, strbuf, argument->print->string)) {
            status = panic("failed to write argument object");
        } else if(argument->newline && strbuf_putcn(strbuf, '\n', argument->newline)) {
            status = panic("failed to putcn strbuf object");
        }
    }

    return status;
}

int argument_array(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    size_t i;
    struct script_range * range;

    long j;
    struct range_node * node;
    char * string;

    for(i = 0; i < array->count && !status; i++) {
        range = array_get(array, i);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            node = range->range->root;
            while(node && !status) {
                for(j = node->min; j <= node->max && !status; j++) {
                    string = map_search(argument->array, &j);
                    if(!string) {
                        status = panic("invalid index - %ld", j);
                    } else {
                        if(!i && j == node->min) {
                            if(strbuf_printf(strbuf, "%s", string))
                                status = panic("failed to printf strbuf object");
                        } else {
                            if(strbuf_printf(strbuf, ", %s", string))
                                status = panic("failed to printf strbuf object");
                        }
                    }
                }
                node = node->next;
            }
        }
    }

    return status;
}

int argument_integer(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    long min;
    long max;

    long flag;
    long divide;

    struct script_range * range;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else {
        if(argument->integer) {
            flag = argument->integer->flag;
            divide = argument->integer->divide;
        } else {
            flag = 0;
            divide = 0;
        }

        if(divide) {
            min = range->range->min / divide;
            max = range->range->max / divide;
        } else {
            min = range->range->min;
            max = range->range->max;
        }

        if(flag & integer_absolute) {
            min = labs(min);
            max = labs(max);
        }

        if(flag & integer_sign)
            if(min < 0 ? strbuf_putc(strbuf, '-') : strbuf_putc(strbuf, '+'))
                status = panic("failed to putc strbuf object");

        if(strbuf_printf(strbuf, "%ld", min))
            status = panic("failed to printf strbuf object");

        if(flag & integer_percent)
            if(strbuf_putc(strbuf, '%'))
                status = panic("failed to putc strbuf object");

        if(min != max) {
            if(strbuf_printf(strbuf, " ~ "))
                status = panic("failed to printf strbuf object");

            if(flag & integer_sign)
                if(max < 0 ? strbuf_putc(strbuf, '-') : strbuf_putc(strbuf, '+'))
                    status = panic("failed to putc strbuf object");

            if(strbuf_printf(strbuf, "%ld", max))
                status = panic("failed to printf strbuf object");

            if(flag & integer_percent)
                if(strbuf_putc(strbuf, '%'))
                    status = panic("failed to putc strbuf object");

            if(flag & integer_string)
                if(strbuf_printf(strbuf, "(%s)", range->string))
                    status = panic("failed to printf strbuf object");
        }
    }

    return status;
}

int argument_string(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else if(strbuf_printf(strbuf, "%s", range->string)) {
        status = panic("failed to printf strbuf object");
    }

    return status;
}

int argument_second(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    long min;
    long max;
    struct print_node * print;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else {
        min = labs(range->range->min);
        max = labs(range->range->max);
        print = argument->print;
        if(min / 86400 > 0) {
            min /= 86400;
            max /= 86400;
        } else {
            print = print->next;
            if(min / 3600 > 0) {
                min /= 3600;
                max /= 3600;
            } else {
                print = print->next;
                if(min / 60 > 0) {
                    min /= 60;
                    max /= 60;
                } else {
                    print = print->next;
                }
            }
        }

        if(min == max) {
            if(strbuf_printf(strbuf, "%ld %s", min, print->string))
                status = panic("failed to printf strbuf object");
        } else {
            if(strbuf_printf(strbuf, "%ld ~ %ld %s", min, max, print->string))
                status = panic("failed to printf strbuf object");
        }
    }

    return status;
}

int argument_millisecond(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;
    struct script_range * range;

    long min;
    long max;
    struct print_node * print;

    range = array_get(array, 0);
    if(!range) {
        status = panic("failed to get array object");
    } else {
        min = labs(range->range->min);
        max = labs(range->range->max);
        print = argument->print;
        if(min / 86400000 > 0) {
            min /= 86400000;
            max /= 86400000;
        } else {
            print = print->next;
            if(min / 3600000 > 0) {
                min /= 3600000;
                max /= 3600000;
            } else {
                print = print->next;
                if(min / 60000 > 0) {
                    min /= 60000;
                    max /= 60000;
                } else {
                    print = print->next;
                    if(min / 1000 > 0) {
                        min /= 1000;
                        max /= 1000;
                    } else {
                        print = print->next;
                    }
                }
            }
        }

        if(min == max) {
            if(strbuf_printf(strbuf, "%ld %s", min, print->string))
                status = panic("failed to printf strbuf object");
        } else {
            if(strbuf_printf(strbuf, "%ld ~ %ld %s", min, max, print->string))
                status = panic("failed to printf strbuf object");
        }
    }

    return status;
}

int argument_item(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    size_t i;
    struct script_range * range;

    long j;
    struct range_node * node;
    struct item_node * item;

    for(i = 0; i < array->count && !status; i++) {
        range = array_get(array, i);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            item = item_name(script->table, range->string);
            if(item) {
                if(!i) {
                    if(strbuf_printf(strbuf, "%s", item->name))
                        status = panic("failed to printf strbuf object");
                } else {
                    if(strbuf_printf(strbuf, ", %s", item->name))
                        status = panic("failed to printf strbuf object");
                }
            } else {
                node = range->range->root;
                while(node && !status) {
                    for(j = node->min; j <= node->max && !status; j++) {
                        item = item_id(script->table, j);
                        if(!item) {
                            status = panic("invalid item id - %ld", j);
                        } else {
                            if(!i && j == node->min) {
                                if(strbuf_printf(strbuf, "%s", item->name))
                                    status = panic("failed to printf strbuf object");
                            } else {
                                if(strbuf_printf(strbuf, ", %s", item->name))
                                    status = panic("failed to printf strbuf object");
                            }
                        }
                    }
                    node = node->next;
                }
            }
        }
    }

    return status;
}

int argument_skill(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    size_t i;
    struct script_range * range;

    long j;
    struct range_node * node;
    struct skill_node * skill;

    for(i = 0; i < array->count && !status; i++) {
        range = array_get(array, i);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            skill = skill_name(script->table, range->string);
            if(skill) {
                if(!i) {
                    if(strbuf_printf(strbuf, "%s", skill->description))
                        status = panic("failed to printf strbuf object");
                } else {
                    if(strbuf_printf(strbuf, ", %s", skill->description))
                        status = panic("failed to printf strbuf object");
                }
            } else {
                node = range->range->root;
                while(node && !status) {
                    for(j = node->min; j <= node->max && !status; j++) {
                        skill = skill_id(script->table, j);
                        if(!skill) {
                            status = panic("invalid skill id - %ld", j);
                        } else {
                            if(!i && j == node->min) {
                                if(strbuf_printf(strbuf, "%s", skill->description))
                                    status = panic("failed to printf strbuf object");
                            } else {
                                if(strbuf_printf(strbuf, ", %s", skill->description))
                                    status = panic("failed to printf strbuf object");
                            }
                        }
                    }
                    node = node->next;
                }
            }
        }
    }

    return status;
}

int argument_mob(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    size_t i;
    struct script_range * range;

    long j;
    struct range_node * node;
    struct mob_node * mob;

    for(i = 0; i < array->count && !status; i++) {
        range = array_get(array, i);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            mob = mob_sprite(script->table, range->string);
            if(mob) {
                if(!i) {
                    if(strbuf_printf(strbuf, "%s", mob->kro))
                        status = panic("failed to printf strbuf object");
                } else {
                    if(strbuf_printf(strbuf, ", %s", mob->kro))
                        status = panic("failed to printf strbuf object");
                }
            } else {
                node = range->range->root;
                while(node && !status) {
                    for(j = node->min; j <= node->max && !status; j++) {
                        mob = mob_id(script->table, j);
                        if(!mob) {
                            status = panic("invalid mob id - %ld", j);
                        } else {
                            if(!i && j == node->min) {
                                if(strbuf_printf(strbuf, "%s", mob->kro))
                                    status = panic("failed to printf strbuf object");
                            } else {
                                if(strbuf_printf(strbuf, ", %s", mob->kro))
                                    status = panic("failed to printf strbuf object");
                            }
                        }
                    }
                    node = node->next;
                }
            }
        }
    }

    return status;
}

int argument_mercenary(struct script * script, struct array * array, struct argument_node * argument, struct strbuf * strbuf) {
    int status = 0;

    size_t i;
    struct script_range * range;

    long j;
    struct range_node * node;
    struct mercenary_node * mercenary;

    for(i = 0; i < array->count && !status; i++) {
        range = array_get(array, i);
        if(!range) {
            status = panic("failed to get array object");
        } else {
            node = range->range->root;
            while(node && !status) {
                for(j = node->min; j <= node->max && !status; j++) {
                    mercenary = mercenary_id(script->table, j);
                    if(!mercenary) {
                        status = panic("invalid mercenary id - %ld", j);
                    } else {
                        if(!i && j == node->min) {
                            if(strbuf_printf(strbuf, "%s", mercenary->name))
                                status = panic("failed to printf strbuf object");
                        } else {
                            if(strbuf_printf(strbuf, ", %s", mercenary->name))
                                status = panic("failed to printf strbuf object");
                        }
                    }
                }
                node = node->next;
            }
        }
    }

    return status;
}
