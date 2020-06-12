#include "script.h"

#include "script_parser.h"
#include "script_scanner.h"

long BF_SHORT;
long BF_LONG;
long BF_WEAPON;
long BF_MAGIC;
long BF_MISC;
long BF_NORMAL;
long BF_SKILL;

long ATF_LONG;
long ATF_MAGIC;
long ATF_MISC;
long ATF_SELF;
long ATF_SHORT;
long ATF_SKILL;
long ATF_TARGET;
long ATF_WEAPON;

int table_set_constant(struct table *, char *, long *);

int script_map_push(struct script *, struct map *);
void script_map_pop(struct script *);

int script_logic_push(struct script *, struct logic *);
void script_logic_pop(struct script *);

int script_stack_push(struct script *, struct stack *);
void script_stack_pop(struct script *);

int script_strbuf_push(struct script *, struct strbuf *);
void script_strbuf_pop(struct script *);

struct script_range * script_range_create(struct script *, enum script_type, char *, ...);

enum script_flag {
    is_logic = 0x1,
    is_array = 0x2,
    is_concat = 0x4
};

int script_compile_re(struct script *, char *, struct strbuf *);
int script_parse(struct script *, char *);
int script_translate(struct script *, struct script_node *);
int script_evaluate(struct script *, struct script_node *, int, struct script_range **);
struct script_range * script_execute(struct script *, struct stack *, struct argument_node *);
int script_default(struct script *, struct stack *, size_t);

struct script_range * function_set(struct script *, struct stack *);
struct script_range * function_min(struct script *, struct stack *);
struct script_range * function_max(struct script *, struct stack *);
struct script_range * function_pow(struct script *, struct stack *);
struct script_range * function_rand(struct script *, struct stack *);
struct script_range * function_bonus(struct script *, struct stack *);
struct script_range * function_bonus2(struct script *, struct stack *);
struct script_range * function_bonus3(struct script *, struct stack *);
struct script_range * function_bonus4(struct script *, struct stack *);
struct script_range * function_bonus5(struct script *, struct stack *);
struct script_range * function_sc_start(struct script *, struct stack *);
struct script_range * function_sc_start2(struct script *, struct stack *);
struct script_range * function_sc_start4(struct script *, struct stack *);
struct script_range * function_mercenary_sc_start(struct script *, struct stack *);
struct script_range * function_autobonus(struct script *, struct stack *);
struct script_range * function_autobonus2(struct script *, struct stack *);
struct script_range * function_getskilllv(struct script *, struct stack *);
struct script_range * function_constant(struct script *, struct stack *);

typedef struct script_range * (*function_cb) (struct script *, struct stack *);

struct function_entry {
    char * identifier;
    function_cb function;
} function_list[] = {
    { "set", function_set },
    { "min", function_min },
    { "max", function_max },
    { "pow", function_pow },
    { "rand", function_rand },
    { "bonus", function_bonus },
    { "bonus2", function_bonus2 },
    { "bonus3", function_bonus3 },
    { "bonus4", function_bonus4 },
    { "bonus5", function_bonus5 },
    { "sc_start", function_sc_start },
    { "sc_start2", function_sc_start2 },
    { "sc_start4", function_sc_start4 },
    { "mercenary_sc_start", function_mercenary_sc_start },
    { "autobonus", function_autobonus },
    { "autobonus2", function_autobonus2 },
    { "getskilllv", function_getskilllv },
    { "gettime", function_constant },
    { "readparam", function_constant },
    { "vip_status", function_constant },
    { "checkoption", function_constant },
    { NULL, NULL}
};

int entry_node_load(struct entry_node *, struct stack *, struct stack *);
int entry_node_call(struct entry_node *, struct script *, struct stack *, struct strbuf *);
int print_node_write(struct print_node *, struct script *, struct stack *, struct strbuf *);

int argument_print(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_prefix(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_zero(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_array(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_integer(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_string(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_second(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_millisecond(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_constant(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_item(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_skill(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_mob(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_mercenary(struct script *, struct stack *, struct argument_node *, struct strbuf *);

int argument_group(struct script *, struct stack *, struct strbuf *, char *);
int argument_element(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_job(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_size(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_race(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_mob_race(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_effect(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_class(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_splash(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_bf(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_atf_target(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_atf_trigger(struct script *, struct stack *, struct argument_node *, struct strbuf *);
int argument_script(struct script *, struct stack *, struct argument_node *, struct strbuf *);

typedef int (*argument_cb) (struct script *, struct stack *, struct argument_node *, struct strbuf *);

struct argument_entry {
    char * identifier;
    argument_cb argument;
} argument_list[] = {
    { "print", argument_print },
    { "prefix", argument_prefix },
    { "zero", argument_zero },
    { "array", argument_array },
    { "integer", argument_integer },
    { "string", argument_string },
    { "second", argument_second },
    { "millisecond", argument_millisecond },
    { "constant", argument_constant },
    { "item", argument_item },
    { "skill", argument_skill },
    { "mob", argument_mob },
    { "mercenary", argument_mercenary },
    { "element", argument_element },
    { "job", argument_job },
    { "size", argument_size },
    { "race", argument_race },
    { "mob_race", argument_mob_race },
    { "effect", argument_effect },
    { "class", argument_class },
    { "splash", argument_splash },
    { "bf", argument_bf },
    { "atf_target", argument_atf_target },
    { "atf_trigger", argument_atf_trigger },
    { "script", argument_script },
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
    if(kv.key) {
        fprintf(stdout, "undefined: ");
        while(kv.key) {
            fprintf(stdout, "%s ", (char *) kv.key);
            kv = map_next(&undef->map);
        }
        fprintf(stdout, "\n");
    }
}

int script_setup(struct table * table) {
    int status = 0;

    if( table_set_constant(table, "BF_SHORT", &BF_SHORT) ||
        table_set_constant(table, "BF_LONG", &BF_LONG) ||
        table_set_constant(table, "BF_WEAPON", &BF_WEAPON) ||
        table_set_constant(table, "BF_MAGIC", &BF_MAGIC) ||
        table_set_constant(table, "BF_MISC", &BF_MISC) ||
        table_set_constant(table, "BF_NORMAL", &BF_NORMAL) ||
        table_set_constant(table, "BF_SKILL", &BF_SKILL) ||
        table_set_constant(table, "ATF_LONG", &ATF_LONG) ||
        table_set_constant(table, "ATF_MAGIC", &ATF_MAGIC) ||
        table_set_constant(table, "ATF_MISC", &ATF_MISC) ||
        table_set_constant(table, "ATF_SELF", &ATF_SELF) ||
        table_set_constant(table, "ATF_SHORT", &ATF_SHORT) ||
        table_set_constant(table, "ATF_SKILL", &ATF_SKILL) ||
        table_set_constant(table, "ATF_TARGET", &ATF_TARGET) ||
        table_set_constant(table, "ATF_WEAPON", &ATF_WEAPON) )
        status = panic("failed to set constant table object");

    return status;
}

int script_create(struct script * script, size_t size, struct heap * heap, struct table * table) {
    int status = 0;

    struct function_entry * function;
    struct argument_entry * argument;

    script->heap = heap;
    script->table = table;

    if(!script->heap) {
        status = panic("invalid heap object");
    } else if(!script->table) {
        status = panic("invalid table object");
    } else if(scriptlex_init_extra(&script->store, &script->scanner)) {
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
        } else if(stack_create(&script->stack_stack, heap->stack_pool)) {
            status = panic("failed to create stack object");
            goto stack_fail;
        } else if(stack_create(&script->strbuf_stack, heap->stack_pool)) {
            status = panic("failed to create stack object");
            goto strbuf_fail;
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
    stack_destroy(&script->strbuf_stack);
strbuf_fail:
    stack_destroy(&script->stack_stack);
stack_fail:
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
    stack_destroy(&script->strbuf_stack);
    stack_destroy(&script->stack_stack);
    stack_destroy(&script->logic_stack);
    stack_destroy(&script->map_stack);
    store_destroy(&script->store);
    scriptpstate_delete(script->parser);
    scriptlex_destroy(script->scanner);
}

int script_compile(struct script * script, char * string, struct strbuf * strbuf) {
    int status = 0;

    strbuf_clear(strbuf);

    script->root = NULL;
    script->map = NULL;
    script->logic = NULL;
    script->stack = NULL;
    script->strbuf = NULL;
    script->range = NULL;

    if(script_compile_re(script, string, strbuf))
        status = panic("failed to compile script object");

    while(script->range) {
        range_destroy(script->range->range);
        script->range = script->range->next;
    }

    store_clear(&script->store);

    return status;
}

int table_set_constant(struct table * table, char * identifier, long * result) {
    struct constant_node * constant;

    constant = constant_identifier(table, identifier);
    if(!constant)
        return panic("invalid constant - %s", identifier);

    *result = constant->value;

    return 0;
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

int script_stack_push(struct script * script, struct stack * stack) {
    int status = 0;

    if(stack_create(stack, script->heap->stack_pool)) {
        status = panic("failed to create stack object");
    } else {
        if(script->stack) {
            if(stack_push(&script->stack_stack, script->stack)) {
                status = panic("failed to push stack object");
            } else {
                script->stack = stack;
            }
        } else {
            script->stack = stack;
        }
        if(status)
            stack_destroy(stack);
    }

    return status;
}

void script_stack_pop(struct script * script) {
    stack_destroy(script->stack);

    script->stack = stack_pop(&script->stack_stack);
}

int script_strbuf_push(struct script * script, struct strbuf * strbuf) {
    int status = 0;

    if(script->strbuf) {
        if(stack_push(&script->strbuf_stack, script->strbuf)) {
            status = panic("failed to push stack object");
        } else {
            script->strbuf = strbuf;
        }
    } else {
        script->strbuf = strbuf;
    }

    return status;
}

void script_strbuf_pop(struct script * script) {
    script->strbuf = stack_pop(&script->strbuf_stack);
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

int script_compile_re(struct script * script, char * string, struct strbuf * strbuf) {
    int status = 0;

    struct map map;

    if(script_strbuf_push(script, strbuf)) {
        status = panic("failed to strbuf push script object");
    } else {
        if(script_map_push(script, &map)) {
            status = panic("failed to map push script object");
        } else {
            if(script_parse(script, string)) {
                status = panic("failed to parse script object");
            } else if(script_translate(script, script->root)) {
                status = panic("failed to translate script object");
            }
            strbuf_trim(strbuf);

            script_map_pop(script);
        }
        script_strbuf_pop(script);
    }

    return status;
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

    struct logic logic;

    struct script_node * node;
    struct script_range * range;

    switch(root->token) {
        case script_curly_open:
            node = root->root;
            while(node && !status) {
                if(script_translate(script, node)) {
                    status = panic("failed to statement script object");
                } else {
                    node = node->next;
                }
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
                } else if(strbuf_printf(script->strbuf, "[%s]\n", range->string)) {
                    status = panic("failed to printf strbuf object");
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
                } else if(logic_push(script->logic, or, NULL)) {
                    status = panic("failed to logic top push script object");
                } else {
                    if(script_evaluate(script, root->root, is_logic, &range)) {
                        status = panic("failed to expression script object");
                    } else if(strbuf_printf(script->strbuf, "[%s]\n", range->string)) {
                        status = panic("failed to printf strbuf object");
                    } else if(script_translate(script, root->root->next)) {
                        status = panic("failed to statement script object");
                    } else if(logic_pop(script->logic)) {
                        status = panic("failed to logic top pop script object");
                    } else if(strbuf_printf(script->strbuf, "[else]\n")) {
                        status = panic("failed to printf strbuf object");
                    } else if(script_translate(script, root->root->next->next)) {
                        status = panic("failed to statement script object");
                    }
                }
                script_logic_pop(script);
            }
            break;
        default:
            if(script_evaluate(script, root, 0, &range)) {
                status = panic("failed to expression script object");
            } else if(range->type == identifier) {
                if(strbuf_printf(script->strbuf, "%s\n", range->string))
                    status = panic("failed to printf strbuf object");
            }
            break;
    }

    return status;
}

int script_evaluate(struct script * script, struct script_node * root, int flag, struct script_range ** result) {
    int status = 0;

    struct logic logic;
    struct stack stack;

    struct script_range * x;
    struct script_range * y;
    struct script_range * z;

    struct script_range * range;
    struct range_node * node;

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
                if(script_stack_push(script, &stack)) {
                    status = panic("failed to stack push script object");
                } else {
                    if(script_evaluate(script, root->root, flag | is_array, &x)) {
                        status = panic("failed to evaluate script object");
                    } else if(!stack_top(script->stack) && stack_push(script->stack, x)) {
                        status = panic("failed to push stack object");
                    } else {
                        function = map_search(&script->function, root->identifier);
                        if(function) {
                            range = function(script, script->stack);
                            if(!range) {
                                status = panic("failed to function range script object");
                            } else {
                                *result = range;
                            }
                        } else {
                            argument = statement_identifier(script->table, root->identifier);
                            if(argument) {
                                range = script_execute(script, script->stack, argument);
                                if(!range) {
                                    status = panic("failed to execute script object");
                                } else {
                                    *result = range;
                                }
                            } else {
                                if(undefined_add(&script->undefined, "statement.%s", root->identifier)) {
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
                    script_stack_pop(script);
                }
            } else {
                argument = statement_identifier(script->table, root->identifier);
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
                        range = script_range_create(script, constant->variable ? identifier : integer, "%s", constant->identifier);
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
                            } else if(range_add(range->range, constant->value, constant->value)) {
                                status = panic("failed to add range object");
                            }

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

                        /*
                         * use variable identifier when concatenating a with string
                         */
                        if(flag & is_concat) {
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
        case script_string:
            range = script_range_create(script, string, "%s", root->identifier);
            if(!range) {
                status = panic("failed to range script object");
            } else {
                *result = range;
            }
            break;
        case script_comma:
            if( script_evaluate(script, root->root, flag, &x) ||
                script_evaluate(script, root->root->next, flag, &y) ) {
                status = panic("failed to evaluate script object");
            } else {
                if(flag & is_array) {
                    if(root->root->token == script_comma) {
                        if(stack_push(script->stack, y))
                            status = panic("failed to push stack object");
                    } else if(root->root->next->token == script_comma) {
                        if(stack_push(script->stack, x))
                            status = panic("failed to push stack object");
                    } else {
                        if(stack_push(script->stack, x) || stack_push(script->stack, y))
                            status = panic("failed to push stack object");
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
                range = script_range_create(script, integer, "%s", y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_assign(range->range, y->range)) {
                    status = panic("failed to assign range object");
                } else if(map_insert(script->map, x->string, range)) {
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
                range = script_range_create(script, integer, "%s", y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_plus(range->range, x->range, y->range)) {
                    status = panic("failed to plus range object");
                } else if(map_insert(script->map, x->string, range)) {
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
                range = script_range_create(script, integer, "%s", y->string);
                if(!range) {
                    status = panic("failed to range script object");
                } else if(range_minus(range->range, x->range, y->range)) {
                    status = panic("failed to minus range object");
                } else if(map_insert(script->map, x->string, range)) {
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
                if(x->type == string) {
                    if(script_evaluate(script, root->root->next, flag | is_concat, &z)) {
                        status = panic("failed to evaluate script object");
                    } else {
                        range = script_range_create(script, integer, "%s%s", x->string, z->string);
                    }
                } else if(y->type == string) {
                    if(script_evaluate(script, root->root, flag | is_concat, &z)) {
                        status = panic("failed to evaluate script object");
                    } else {
                        range = script_range_create(script, integer, "%s%s", z->string, y->string);
                    }
                } else {
                    range = script_range_create(script, integer, "%s + %s", x->string, y->string);
                }

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

struct script_range * script_execute(struct script * script, struct stack * stack, struct argument_node * argument) {
    int status = 0;
    argument_cb handler;
    struct strbuf * strbuf;
    struct string * string;
    struct script_range * range;
    struct range_node * node;

    handler = argument->handler ? map_search(&script->argument, argument->handler) : (void *) argument_print;
    if(!handler) {
        status = panic("invalid argument - %s", argument->handler);
    } else {
        strbuf = script_buffer_get(&script->buffer);
        if(!strbuf) {
            status = panic("failed to get script buffer object");
        } else {
            if(handler(script, stack, argument, strbuf)) {
                status = panic("failed to execute argument object");
            } else {
                string = strbuf_string(strbuf);
                if(!string) {
                    status = panic("failed to string strbuf object");
                } else {
                    range = script_range_create(script, identifier, "%s", string->string);
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
                }
            }
            script_buffer_put(&script->buffer, strbuf);
        }
    }

    return status ? NULL : range;
}

int script_default(struct script * script, struct stack * stack, size_t index) {
    struct script_range * range;

    range = stack_get(stack, index);
    if(!range) {
        range = script_range_create(script, integer, "%ld", 0);
        if(!range) {
            return panic("failed to create script range object");
        } else if(range_add(range->range, 0, 0)) {
            return panic("failed to add range object");
        } else if(stack_push(stack, range)) {
            return panic("failed to push stack object");
        }
    }

    return 0;
}

struct script_range * function_set(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = stack_get(stack, 0);
    if(!x) {
        status = panic("invalid indentifier");
    } else {
        y = stack_get(stack, 1);
        if(!y) {
            status = panic("invalid expression");
        } else {
            range = script_range_create(script, identifier, "%s", y->string);
            if(!range) {
                status = panic("failed to range script object");
            } else if(range_assign(range->range, y->range)) {
                status = panic("failed to assign range object");
            }  else if(map_insert(script->map, x->string, range)) {
                status = panic("failed to map insert script object");
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_min(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = stack_get(stack, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = stack_get(stack, 1);
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

struct script_range * function_max(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = stack_get(stack, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = stack_get(stack, 1);
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

struct script_range * function_pow(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = stack_get(stack, 0);
    if(!x) {
        status = panic("invalid base");
    } else {
        y = stack_get(stack, 1);
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

struct script_range * function_rand(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * x;
    struct script_range * y;
    struct script_range * range;

    x = stack_get(stack, 0);
    if(!x) {
        status = panic("invalid min");
    } else {
        y = stack_get(stack, 1);
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

struct script_range * function_bonus(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = bonus_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "bonus.%s", range->string))
                status = panic("failed to add undefined object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_bonus2(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = bonus2_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "bonus2.%s", range->string))
                status = panic("failed to add undefined object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_bonus3(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = bonus3_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "bonus3.%s", range->string))
                status = panic("failed to add undefined object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_bonus4(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = bonus4_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "bonus4.%s", range->string))
                status = panic("failed to add undefined object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_bonus5(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = bonus5_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "bonus5.%s", range->string))
                status = panic("failed to add undefined object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_sc_start(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = sc_start_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "sc_start.%s", range->string))
                status = panic("failed to add undefined object");
        } else if(script_default(script, stack, 3)) {
            status = panic("failed to default script object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_sc_start2(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = sc_start2_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "sc_start2.%s", range->string))
                status = panic("failed to add undefined object");
        } else if(script_default(script, stack, 4)) {
            status = panic("failed to default script object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_sc_start4(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = sc_start4_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "sc_start4.%s", range->string))
                status = panic("failed to add undefined object");
        } else if(script_default(script, stack, 6)) {
            status = panic("failed to default script object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_mercenary_sc_start(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("invalid bonus");
    } else {
        argument = sc_start_identifier(script->table, range->string);
        if(!argument) {
            if(undefined_add(&script->undefined, "sc_start.%s", range->string))
                status = panic("failed to add undefined object");
        } else if(script_default(script, stack, 3)) {
            status = panic("failed to default script object");
        } else {
            range = script_execute(script, stack, argument);
            if(!range)
                status = panic("failed to execute script object");
        }
    }

    return status ? NULL : range;
}

struct script_range * function_autobonus(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    argument = statement_identifier(script->table, "autobonus");
    if(!argument) {
        if(undefined_add(&script->undefined, "statement.autobonus"))
            status = panic("failed to add undefined object");
    } else if(script_default(script, stack, 3)) {
        status = panic("failed to default script object");
    } else {
        range = script_execute(script, stack, argument);
        if(!range)
            status = panic("failed to execute script object");
    }

    return status ? NULL : range;
}

struct script_range * function_autobonus2(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct argument_node * argument;

    argument = statement_identifier(script->table, "autobonus2");
    if(!argument) {
        if(undefined_add(&script->undefined, "statement.autobonus2"))
            status = panic("failed to add undefined object");
    } else if(script_default(script, stack, 3)) {
        status = panic("failed to default script object");
    } else {
        range = script_execute(script, stack, argument);
        if(!range)
            status = panic("failed to execute script object");
    }

    return status ? NULL : range;
}

struct script_range * function_getskilllv(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * index;
    struct script_range * range;
    struct argument_node * argument;
    struct skill_node * skill;

    argument = statement_identifier(script->table, "getskilllv");
    if(!argument) {
        if(undefined_add(&script->undefined, "statement.getskilllv"))
            status = panic("failed to add undefined object");
    } else {
        range = script_execute(script, stack, argument);
        if(!range) {
            status = panic("failed to execute script object");
        } else {
            index = stack_get(stack, 0);
            if(!index) {
                status = panic("failed to get stack object");
            } else {
                skill = skill_id(script->table, index->range->min);
                if(skill) {
                    if(range_add(range->range, 0, skill->level))
                        status = panic("failed to add range object");
                } else {
                    skill = skill_name(script->table, index->string);
                    if(skill) {
                        if(range_add(range->range, 0, skill->level))
                            status = panic("failed to add range object");
                    } else{
                        status = panic("invalid skill - %s", index->string);
                    }
                }
            }
        }
    }

    return status ? NULL : range;
}

struct script_range * function_constant(struct script * script, struct stack * stack) {
    int status = 0;
    struct script_range * range;
    struct constant_node * constant;
    struct range_node * node;

    range = stack_get(stack, 0);
    if(!range) {
        status = panic("failed to get stack object");
    } else {
        constant = constant_identifier(script->table, range->string);
        if(!constant) {
            status = panic("invalid constant - %s", range->string);
        } else if(!constant->tag) {
            status = panic("invalid constant tag - %s", range->string);
        } else {
            range = script_range_create(script, constant->variable ? identifier : integer, "%s", constant->tag);
            if(!range) {
                status = panic("failed to create script range object");
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
                } else if(range_add(range->range, constant->value, constant->value)) {
                    status = panic("failed to add range object");
                }
            }
        }
    }

    return status ? NULL : range;
}

int entry_node_load(struct entry_node * entry, struct stack * result, struct stack * source) {
    size_t i;
    struct script_range * range;

    if(!entry->count) {
        range = stack_start(source);
        while(range) {
            if(stack_push(result, range))
                return panic("failed to push stack object");
            range = stack_next(source);
        }
    } else {
        for(i = 0; i < entry->count; i++) {
            range = stack_get(source, entry->array[i]);
            if(!range) {
                return panic("failed to get stack object");
            } else if(stack_push(result, range)) {
                return panic("failed to push stack object");
            }
        }
    }

    return 0;
}

int entry_node_call(struct entry_node * entry, struct script * script, struct stack * stack, struct strbuf * strbuf) {
    argument_cb handler;
    struct argument_node * argument;
    struct script_range * range;

    handler = map_search(&script->argument, entry->identifier);
    if(handler) {
        if(handler(script, stack, NULL, strbuf))
            return panic("failed to execute argument object");
    } else {
        argument = argument_identifier(script->table, entry->identifier);
        if(argument) {
            range = script_execute(script, stack, argument);
            if(!range) {
                return panic("failed to execute script object");
            } else if(strbuf_printf(strbuf, "%s", range->string)) {
                return panic("failed to printf strbuf object");
            }
        } else {
            return panic("undefined argument - %s", entry->identifier);
        }
    }

    return 0;
}

int print_node_write(struct print_node * print, struct script * script, struct stack * stack, struct strbuf * strbuf) {
    int status = 0;
    struct stack subset;
    struct entry_node * entry;

    if(stack_create(&subset, script->heap->stack_pool)) {
        status = panic("failed to create stack object");
    } else {
        entry = print->entry;
        while(entry && !status) {
            if(strbuf_strcpy(strbuf, entry->string, entry->length)) {
                status = panic("failed to strcpy strbuf object");
            } else if(entry->identifier) {
                if(entry_node_load(entry, &subset, stack)) {
                    status = panic("failed to load entry node object");
                } else if(entry_node_call(entry, script, &subset, strbuf)) {
                    status = panic("failed to call entry node object");
                } else {
                    stack_clear(&subset);
                }
            }
            entry = entry->next;
        }
        stack_destroy(&subset);
    }

    return status;
}

int argument_print(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    char * anchor;
    struct print_node * print;

    anchor = strbuf->pos;
    print = argument->print;
    while(print) {
        if(anchor != strbuf->pos)
            if(strbuf_putc(strbuf, '\n'))
                return panic("failed to putc strbuf object");
        anchor = strbuf->pos;

        if(print_node_write(print, script, stack, strbuf))
            return panic("failed to parse argument object");
        print = print->next;
    }

    strbuf_trim(strbuf);

    return 0;
}

int argument_prefix(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct print_node * print;
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else {
        print = argument->print;
        if(range->range->max < 0)
            print = print->next;

        if(print_node_write(print, script, stack, strbuf))
            return panic("failed to write argument object");
    }

    return 0;
}

int argument_zero(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else if(range->range->min || range->range->max) {
        if(print_node_write(argument->print, script, stack, strbuf))
            return panic("failed to write argument object");
    }

    return 0;
}

int argument_array(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long i;
    struct script_range * range;
    struct range_node * node;
    char * string = NULL;

    range = stack_start(stack);
    while(range) {
        node = range->range->root;
        while(node) {
            for(i = node->min; i <= node->max; i++) {
                string = map_search(argument->map, &i);
                if(!string) {
                    return panic("invalid index - %ld", i);
                } else if(strbuf_printf(strbuf, "%s, ", string)) {
                    return panic("failed to printf strbuf object");
                }
            }
            node = node->next;
        }
        range = stack_next(stack);
    }

    if(string && strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_integer(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long min;
    long max;
    long tmp;

    long flag;
    long divide;

    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
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
            if(min < 0 && max < 0) {
                min *= -1;
                max *= -1;

                tmp = min;
                min = max;
                max = tmp;
            }
        }

        if(flag & integer_inverse) {
            min *= -1;
            max *= -1;

            tmp = min;
            min = max;
            max = tmp;
        }

        if(flag & integer_sign)
            if(min >= 0 && strbuf_putc(strbuf, '+'))
                return panic("failed to putc strbuf object");

        if(strbuf_printf(strbuf, "%ld", min))
            return panic("failed to printf strbuf object");

        if(flag & integer_percent)
            if(strbuf_putc(strbuf, '%'))
                return panic("failed to putc strbuf object");

        if(min != max) {
            if(strbuf_printf(strbuf, " ~ "))
                return panic("failed to printf strbuf object");

            if(flag & integer_sign)
                if(max >= 0 && strbuf_putc(strbuf, '+'))
                    return panic("failed to putc strbuf object");

            if(strbuf_printf(strbuf, "%ld", max))
                return panic("failed to printf strbuf object");

            if(flag & integer_percent)
                if(strbuf_putc(strbuf, '%'))
                    return panic("failed to putc strbuf object");

            if(flag & integer_string)
                if(strbuf_printf(strbuf, " (%s)", range->string))
                    return panic("failed to printf strbuf object");
        }
    }

    return 0;
}

int argument_string(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else if(strbuf_printf(strbuf, "%s", range->string)) {
        return panic("failed to printf strbuf object");
    }

    return 0;
}

int argument_second(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    long min;
    long max;
    long tmp;
    struct print_node * print;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else {
        min = range->range->min;
        max = range->range->max;
        if(min < 0 && max < 0) {
            min *= -1;
            max *= -1;

            tmp = min;
            min = max;
            max = tmp;
        }

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
            if(strbuf_printf(strbuf, "%ld ", min)) {
                return panic("failed to printf strbuf object");
            } else if(print_node_write(print, script, stack, strbuf)) {
                return panic("failed to write argument object");
            }
        } else {
            if(strbuf_printf(strbuf, "%ld ~ %ld ", min, max)) {
                return panic("failed to printf strbuf object");
            } else if(print_node_write(print, script, stack, strbuf)) {
                return panic("failed to write argument object");
            }
        }
    }

    return 0;
}

int argument_millisecond(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    long min;
    long max;
    long tmp;
    struct print_node * print;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else {
        min = range->range->min;
        max = range->range->max;
        if(min < 0 && max < 0) {
            min *= -1;
            max *= -1;

            tmp = min;
            min = max;
            max = tmp;
        }

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
            if(strbuf_printf(strbuf, "%ld ", min)) {
                return panic("failed to printf strbuf object");
            } else if(print_node_write(print, script, stack, strbuf)) {
                return panic("failed to write argument object");
            }
        } else {
            if(strbuf_printf(strbuf, "%ld ~ %ld ", min, max)) {
                return panic("failed to printf strbuf object");
            } else if(print_node_write(print, script, stack, strbuf)) {
                return panic("failed to write argument object");
            }
        }
    }

    return 0;
}

int argument_constant(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;
    struct constant_node * constant;

    range = stack_get(stack, 0);
    if(!range) {
        return panic("failed to get stack object");
    } else {
        constant = constant_identifier(script->table, range->string);
        if(!constant) {
            return panic("invalid constant - %s", range->string);
        } else if(!constant->tag) {
            return panic("invalid constant tag - %s", range->string);
        } else if(strbuf_printf(strbuf, "%s", constant->tag)) {
            return panic("failed to printf strbuf object");
        }
    }

    return 0;
}

int argument_item(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long i;
    struct script_range * range;
    struct range_node * node;
    struct item_node * item = NULL;

    range = stack_start(stack);
    while(range) {
        item = item_name(script->table, range->string);
        if(item) {
            if(strbuf_printf(strbuf, "%s, ", item->name))
                return panic("failed to printf strbuf object");
        } else {
            node = range->range->root;
            while(node) {
                for(i = node->min; i <= node->max; i++) {
                    item = item_id(script->table, i);
                    if(!item) {
                        return panic("invalid item id - %ld", i);
                    } else if(strbuf_printf(strbuf, "%s, ", item->name)) {
                        return panic("failed to printf strbuf object");
                    }
                }
                node = node->next;
            }
        }
        range = stack_next(stack);
    }

    if(item && strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_skill(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    size_t i;
    struct script_range * range;
    struct range_node * node;
    struct skill_node * skill = NULL;

    range = stack_start(stack);
    while(range) {
        skill = skill_name(script->table, range->string);
        if(skill) {
            if(strbuf_printf(strbuf, "%s, ", skill->description))
                return panic("failed to printf strbuf object");
        } else {
            node = range->range->root;
            while(node) {
                for(i = node->min; i <= node->max; i++) {
                    skill = skill_id(script->table, i);
                    if(!skill) {
                        return panic("invalid skill id - %ld", i);
                    } else if(strbuf_printf(strbuf, "%s, ", skill->description)) {
                        return panic("failed to printf strbuf object");
                    }
                }
                node = node->next;
            }
        }
        range = stack_next(stack);
    }

    if(skill && strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_mob(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long i;
    struct script_range * range;
    struct range_node * node;
    struct mob_node * mob = NULL;

    range = stack_start(stack);
    while(range) {
        mob = mob_sprite(script->table, range->string);
        if(mob) {
            if(strbuf_printf(strbuf, "%s, ", mob->kro))
                return panic("failed to printf strbuf object");
        } else {
            node = range->range->root;
            while(node) {
                for(i = node->min; i <= node->max; i++) {
                    mob = mob_id(script->table, i);
                    if(!mob) {
                        return panic("invalid mob id - %ld", i);
                    } else if(strbuf_printf(strbuf, "%s, ", mob->kro)) {
                        return panic("failed to printf strbuf object");
                    }
                }
                node = node->next;
            }
        }
        range = stack_next(stack);
    }

    if(mob && strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_mercenary(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long i;
    struct script_range * range;
    struct range_node * node;
    struct mercenary_node * mercenary = NULL;

    range = stack_start(stack);
    while(range) {
        node = range->range->root;
        while(node) {
            for(i = node->min; i <= node->max; i++) {
                mercenary = mercenary_id(script->table, i);
                if(!mercenary) {
                    return panic("invalid mercenary id - %ld", i);
                } else if(strbuf_printf(strbuf, "%s, ", mercenary->name)) {
                    return panic("failed to printf strbuf object");
                }
            }
            node = node->next;
        }
        range = stack_next(stack);
    }

    if(mercenary && strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_group(struct script * script, struct stack * stack, struct strbuf * strbuf, char * group) {
    struct map * map;
    struct script_range * range;
    struct constant_node * constant;

    map = constant_group_identifier(script->table, group);
    if(!map)
        return panic("failed to get constant group - %s", group);

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    constant = map_search(map, range->string);
    if(!constant)
        return panic("invalid constant - %s", range->string);

    if(strbuf_printf(strbuf, "%s", constant->tag))
        return panic("failed to printf strbuf object");

    return 0;
}

int argument_element(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "element");
}

int argument_job(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "job");
}

int argument_size(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "size");
}

int argument_race(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "race");
}

int argument_mob_race(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "mob_race");
}

int argument_effect(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "effect");
}

int argument_class(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    return argument_group(script, stack, strbuf, "class");
}

int argument_splash(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    long min;
    long max;

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    min = range->range->min * 2 + 1;
    max = range->range->max * 2 + 1;

    if(strbuf_printf(strbuf, "[%ld x %ld]", min, min))
        return panic("failed to printf strbuf object");

    if(min != max && strbuf_printf(strbuf, " ~ [%ld x %ld]", max, max))
        return panic("failed to printf strbuf object");

    return 0;
}

int argument_bf(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long flag;
    struct print_node * print;
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    flag = range->range->min | range->range->max;

    if(!(flag & (BF_SHORT | BF_LONG)))
        flag |= BF_SHORT | BF_LONG;

    if(!(flag & (BF_WEAPON | BF_MAGIC | BF_MISC)))
        flag |= BF_WEAPON;

    print = argument->print;

    if(flag & BF_MAGIC) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    print = print->next;

    if(flag & BF_MISC) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    print = print->next;

    if(flag & BF_WEAPON) {
        flag = flag & (BF_SHORT | BF_LONG);

        if(flag == BF_SHORT)
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        print = print->next;

        if(flag == BF_LONG)
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        print = print->next;

        if(flag == (BF_SHORT | BF_LONG))
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        if(strbuf_printf(strbuf, ", "))
            return panic("failed to printf strbuf object");
    }

    if(strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_atf_target(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long flag;
    struct print_node * print;
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    flag = range->range->min | range->range->max;

    if(!(flag & (ATF_SELF | ATF_TARGET)))
        flag |= ATF_TARGET;

    print = argument->print;

    if(flag & ATF_SELF) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    print = print->next;

    if(flag & ATF_TARGET) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    if(strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_atf_trigger(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    long flag;
    struct print_node * print;
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    flag = range->range->min | range->range->max;

    if(!(flag & (ATF_SHORT | ATF_LONG)))
        flag |= ATF_SHORT | ATF_LONG;

    if(!(flag & (ATF_SKILL | ATF_WEAPON | ATF_MAGIC | ATF_MISC)))
        flag |= ATF_WEAPON;

    if(flag & ATF_SKILL)
        flag |= ATF_MAGIC | ATF_MISC;

    print = argument->print;

    if(flag & ATF_MAGIC) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    print = print->next;

    if(flag & ATF_MISC) {
        if(print_node_write(print, script, stack, strbuf)) {
            return panic("failed to write print node object");
        } else if(strbuf_printf(strbuf, ", ")) {
            return panic("failed to printf strbuf object");
        }
    }

    print = print->next;

    if(flag & ATF_WEAPON) {
        flag = flag & (ATF_SHORT | ATF_LONG);

        if(flag == ATF_SHORT)
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        print = print->next;

        if(flag == ATF_LONG)
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        print = print->next;

        if(flag == (ATF_SHORT | ATF_LONG))
            if(print_node_write(print, script, stack, strbuf))
                return panic("failed to write print node object");

        if(strbuf_printf(strbuf, ", "))
            return panic("failed to printf strbuf object");
    }

    if(strbuf_unputn(strbuf, 2))
        return panic("failed to unputn strbuf object");

    return 0;
}

int argument_script(struct script * script, struct stack * stack, struct argument_node * argument, struct strbuf * strbuf) {
    struct script_range * range;

    range = stack_get(stack, 0);
    if(!range)
        return panic("failed to get stack object");

    if(script_compile_re(script, range->string, strbuf))
        return panic("failed to compile script object");

    return 0;
}
