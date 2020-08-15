#include "table.h"

int long_compare(void *, void *);
int string_long(struct string *, long *);
int string_store(struct string *, struct store *, char **);

struct schema_markup skill_markup[] = {
    {1, schema_map, 0, NULL},
    {2, schema_list, 1, "Body"},
    {3, schema_map, 2, NULL},
    {4, schema_string, 3, "Id"},
    {4, schema_string, 4, "Name"},
    {4, schema_string, 5, "Description"},
    {4, schema_string, 6, "MaxLevel"},
    {0, 0, 0},
};

struct schema_markup constant_markup[] = {
    {1, schema_list, 0, NULL},
    {2, schema_map, 1, NULL},
    {3, schema_string, 2, "identifier"},
    {3, schema_string, 3, "value"},
    {3, schema_string, 4, "tag"},
    {3, schema_list, 5, "range"},
    {4, schema_map, 6, NULL},
    {5, schema_string, 7, "min"},
    {5, schema_string, 8, "max"},
    {3, schema_string, 9, "variable"},
    {0, 0, 0},
};

struct schema_markup constant_group_markup[] = {
    {1, schema_list, 0, NULL},
    {2, schema_map, 1, NULL},
    {3, schema_list, 2, "group"},
    {4, schema_string, 3, NULL},
    {3, schema_string, 4, "identifier"},
    {0, 0, 0},
};

struct schema_markup argument_markup[] = {
    {1, schema_list, 0, NULL},
    {2, schema_map, 1, NULL},
    {3, schema_string, 2, "identifier"},
    {3, schema_string, 3, "handler"},
    {3, schema_list | schema_string, 5, "print"},
    {4, schema_string, 6, NULL},
    {3, schema_list, 7, "range"},
    {4, schema_map, 8, NULL},
    {5, schema_string, 9, "min"},
    {5, schema_string, 10, "max"},
    {3, schema_list, 11, "array"},
    {4, schema_map, 12, NULL},
    {5, schema_string, 13, "index"},
    {5, schema_string, 14, "string"},
    {3, schema_map, 15, "integer"},
    {4, schema_string, 16, "sign"},
    {4, schema_string, 17, "string"},
    {4, schema_string, 18, "percent"},
    {4, schema_string, 19, "inverse"},
    {4, schema_string, 20, "absolute"},
    {4, schema_string, 21, "divide"},
    {3, schema_list, 22, "optional"},
    {4, schema_map, 23, NULL},
    {5, schema_string, 24, "index"},
    {5, schema_string, 25, "string"},
    {0, 0, 0},
};

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int string_long(struct string * string, long * result) {
    int status = 0;

    char * last;
    long value;

    value = strtol(string->string, &last, 0);
    if(*last) {
        status = panic("failed to strtol");
    } else {
        *result = value;
    }

    return status;
}

int string_store(struct string * string, struct store * store, char ** result) {
    int status = 0;
    char * object;

    object = store_strcpy(store, string->string, string->length);
    if(!object) {
        status = panic("failed to strcpy store object");
    } else {
        *result = object;
    }

    return status;
}

int item_create(struct item * item, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&item->store, size)) {
        status = panic("failed to create store object");
    } else if(stack_create(&item->stack, heap->stack_pool)) {
        status = panic("failed to create stack object");
        goto stack_fail;
    } else if(strbuf_create(&item->strbuf, size)) {
        status = panic("failed to create strbuf object");
        goto strbuf_fail;
    } else if(map_create(&item->id, long_compare, heap->map_pool)) {
        status = panic("failed to create map object");
        goto id_fail;
    } else if(map_create(&item->name, (map_compare_cb) strcmp, heap->map_pool)) {
        status = panic("failed to create map object");
        goto name_fail;
    }

    return status;

name_fail:
    map_destroy(&item->id);
id_fail:
    strbuf_destroy(&item->strbuf);
strbuf_fail:
    stack_destroy(&item->stack);
stack_fail:
    store_destroy(&item->store);

    return status;
}

void item_destroy(struct item * item) {
    map_destroy(&item->name);
    map_destroy(&item->id);
    strbuf_destroy(&item->strbuf);
    stack_destroy(&item->stack);
    store_destroy(&item->store);
}

int item_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct item * item = context;

    switch(mark) {
        case 0:
            if(type == parser_start) {
                item->item = store_calloc(&item->store, sizeof(*item->item));
                if(!item->item)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(map_insert(&item->id, &item->item->id, item->item)) {
                    return panic("failed to insert map object");
                } else if(map_insert(&item->name, item->item->name, item->item)) {
                    return panic("failed to insert map object");
                }
            }
            break;
        case 1: return string_long(string, &item->item->id); break;
        case 3: return string_store(string, &item->store, &item->item->name); break;
        case 20:
            if(item_script_parse(item, string->string))
                return panic("failed to script parse item object");
            break;
    }

    return 0;
}

int item_script_parse(struct item * item, char * string) {
    int curly = 0;
    size_t index = 0;
    char * anchor = NULL;

    while(*string) {
        if(*string == '{') {
            if(!curly)
                anchor = string;
            curly++;
        } else if(*string == '}') {
            curly--;
            if(!curly) {
                switch(index) {
                    case 0:
                        item->item->bonus = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->bonus)
                            return panic("failed to char store object");
                        break;
                    case 1:
                        item->item->equip = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->equip)
                            return panic("failed to char store object");
                        break;
                    case 2:
                        item->item->unequip = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->unequip)
                            return panic("failed to char store object");
                        break;
                }
                index++;

                anchor = NULL;
            }
        }
        string++;
    }

    return 0;
}

int item_combo_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct item * item = context;

    char * cursor;
    long id;
    struct item_node * item_node;

    char * bonus;
    char * combo;
    struct item_combo_node * combo_node;

    switch(mark) {
        case 1:
            strbuf_clear(&item->strbuf);
            stack_clear(&item->stack);

            id = strtol(string->string, &cursor, 10);
            while(cursor[0] == ':') {
                item_node = map_search(&item->id, &id);
                if(!item_node) {
                    return panic("invalid item id - %ld", id);
                } else if(stack_push(&item->stack, item_node)) {
                    return panic("failed to push stack object");
                } else if(strbuf_printf(&item->strbuf, "%s, ", item_node->name)) {
                    return panic("failed to printf strbuf object");
                } else {
                    id = strtol(cursor + 1, &cursor, 10);
                }
            }

            item_node = map_search(&item->id, &id);
            if(!item_node) {
                return panic("invalid item id - %ld", id);
            } else if(stack_push(&item->stack, item_node)) {
                return panic("failed to push stack object");
            } else if(strbuf_printf(&item->strbuf, "%s", item_node->name)) {
                return panic("failed to printf strbuf object");
            }
            break;
        case 2:
            bonus = store_strcpy(&item->store, string->string, string->length);
            if(!bonus)
                return panic("failed to strcpy store object");

            string = strbuf_string(&item->strbuf);
            if(!string)
                return panic("failed to string strbuf object");

            combo = store_strcpy(&item->store, string->string, string->length);
            if(!combo)
                return panic("failed to strcpy store object");

            item_node = stack_start(&item->stack);
            while(item_node) {
                combo_node = store_malloc(&item->store, sizeof(*combo_node));
                if(!combo_node) {
                    return panic("failed to malloc store object");
                } else {
                    combo_node->combo = combo;
                    combo_node->bonus = bonus;
                    combo_node->next = item_node->combo;
                    item_node->combo = combo_node;
                }
                item_node = stack_next(&item->stack);
            }
            break;
    }

    return 0;
}

int skill_create(struct skill * skill, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&skill->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&skill->id, long_compare, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&skill->name, (map_compare_cb) strcmp, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                map_destroy(&skill->id);
        }
        if(status)
            store_destroy(&skill->store);
    }

    return status;
}

void skill_destroy(struct skill * skill) {
    map_destroy(&skill->name);
    map_destroy(&skill->id);
    store_destroy(&skill->store);
}

int skill_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct skill * skill = context;

    switch(mark) {
        case 2:
            if(type == parser_start) {
                skill->skill = store_calloc(&skill->store, sizeof(*skill->skill));
                if(!skill->skill)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(!skill->skill->name) {
                    return panic("invalid name");
                } else if(map_insert(&skill->id, &skill->skill->id, skill->skill)) {
                    return panic("failed to insert map object");
                } else if(map_insert(&skill->name, skill->skill->name, skill->skill)) {
                    return panic("failed to insert map object");
                }
            }
            break;
        case 3: return string_long(string, &skill->skill->id); break;
        case 4: return string_store(string, &skill->store, &skill->skill->name); break;
        case 5: return string_store(string, &skill->store, &skill->skill->description); break;
        case 6: return string_long(string, &skill->skill->level); break;
    }

    return 0;
}

int mob_create(struct mob * mob, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&mob->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&mob->id, long_compare, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&mob->sprite, (map_compare_cb) strcmp, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                map_destroy(&mob->id);
        }
        if(status)
            store_destroy(&mob->store);
    }

    return status;
}

void mob_destroy(struct mob * mob) {
    map_destroy(&mob->sprite);
    map_destroy(&mob->id);
    store_destroy(&mob->store);
}

int mob_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct mob * mob = context;

    switch(mark) {
        case 0:
            if(type == parser_start) {
                mob->mob = store_calloc(&mob->store, sizeof(*mob->mob));
                if(!mob->mob)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(map_insert(&mob->id, &mob->mob->id, mob->mob)) {
                    return panic("failed to insert map object");
                } else if(map_insert(&mob->sprite, mob->mob->sprite, mob->mob)) {
                    return panic("failed to insert map object");
                }
            }
            break;
        case 1: return string_long(string, &mob->mob->id); break;
        case 2: return string_store(string, &mob->store, &mob->mob->sprite); break;
        case 3: return string_store(string, &mob->store, &mob->mob->kro); break;
    }

    return 0;
}

int mercenary_create(struct mercenary * mercenary, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&mercenary->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&mercenary->id, long_compare, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            store_destroy(&mercenary->store);
    }

    return status;
}

void mercenary_destroy(struct mercenary * mercenary) {
    map_destroy(&mercenary->id);
    store_destroy(&mercenary->store);
}

int mercenary_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct mercenary * mercenary = context;

    switch(mark) {
        case 0:
            if(type == parser_start) {
                mercenary->mercenary = store_calloc(&mercenary->store, sizeof(*mercenary->mercenary));
                if(!mercenary->mercenary)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(map_insert(&mercenary->id, &mercenary->mercenary->id, mercenary->mercenary))
                    return panic("failed to insert map object");
            }
            break;
        case 1: return string_long(string, &mercenary->mercenary->id); break;
        case 3: return string_store(string, &mercenary->store, &mercenary->mercenary->name); break;
    }

    return 0;
}

int constant_create(struct constant * constant, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&constant->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&constant->identifier, (map_compare_cb) strcasecmp, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&constant->group, (map_compare_cb) strcasecmp, heap->map_pool)) {
                status = panic("failed to create map object");
            } else {
                constant->constant_group = NULL;
            }
            if(status)
                map_destroy(&constant->identifier);
        }
        if(status)
            store_destroy(&constant->store);
    }

    return status;
}

void constant_destroy(struct constant * constant) {
    struct constant_group_node * group;

    group = constant->constant_group;
    while(group) {
        map_destroy(&group->map_value);
        map_destroy(&group->map_identifier);
        group = group->next;
    }

    map_destroy(&constant->group);
    map_destroy(&constant->identifier);
    store_destroy(&constant->store);
}

struct constant_group_node * constant_group(struct constant * constant) {
    int status = 0;
    struct constant_group_node * group;

    group = store_calloc(&constant->store, sizeof(*group));
    if(!group) {
        status = panic("failed to calloc store object");
    } else if(map_create(&group->map_identifier, (map_compare_cb) strcasecmp, constant->identifier.pool)) {
        status = panic("failed to create map object");
    } else {
        if(map_create(&group->map_value, long_compare, constant->identifier.pool))
            status = panic("failed to create map object");
        if(status)
            map_destroy(&group->map_identifier);
    }

    return status ? NULL : group;
}

int constant_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct constant * constant = context;

    switch(mark) {
        case 1:
            if(type == parser_start) {
                constant->constant = store_calloc(&constant->store, sizeof(*constant->constant));
                if(!constant->constant)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(!constant->constant->identifier) {
                    return panic("invalid string object");
                } else if(map_insert(&constant->identifier, constant->constant->identifier, constant->constant)) {
                    return panic("failed to insert map object");
                }
            }
            break;
        case 2: return string_store(string, &constant->store, &constant->constant->identifier); break;
        case 3: return string_long(string, &constant->constant->value); break;
    }

    return 0;
}

int constant_data_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct constant * constant = context;

    switch(mark) {
        case 1:
            if(type == parser_start)
                constant->constant = NULL;
            break;
        case 2:
            constant->constant = map_search(&constant->identifier, string->string);
            if(!constant->constant)
                return panic("failed to search map object - %s", string->string);
            break;
        case 4: return string_store(string, &constant->store, &constant->constant->tag); break;
        case 6:
            if(type == parser_start) {
                constant->range = store_calloc(&constant->store, sizeof(*constant->range));
                if(!constant->range)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                constant->range->next = constant->constant->range;
                constant->constant->range = constant->range;
            }
            break;
        case 7: return string_long(string, &constant->range->min); break;
        case 8: return string_long(string, &constant->range->max); break;
        case 9:
            if(!strcmp(string->string, "true"))
                constant->constant->variable = 1;
            break;
    }

    return 0;
}

int constant_group_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct constant * constant;
    struct constant_node * node;
    struct constant_group_node * group;

    constant = context;
    group = constant->constant_group;

    switch(mark) {
        case 1:
            group = constant_group(constant);
            if(!group) {
                return panic("failed to group constant object");
            } else {
                group->next = constant->constant_group;
                constant->constant_group = group;
            }
            break;
        case 3:
            node = map_search(&constant->identifier, string->string);
            if(!node) {
                return panic("invalid constant - %s", string->string);
            } else if(!node->tag) {
                return panic("invalid tag - %s", node->identifier);
            } else if(map_insert(&group->map_identifier, node->identifier, node)) {
                return panic("failed to insert map object");
            } else if(map_insert(&group->map_value, &node->value, node)) {
                return panic("failed to insert map object");
            }
            break;
        case 4:
            group->identifier = store_strcpy(&constant->store, string->string, string->length);
            if(!group->identifier) {
                return panic("failed to strcpy store object");
            } else if(map_insert(&constant->group, group->identifier, group)) {
                return panic("failed to insert map object");
            }
            break;
    }

    return 0;
}

int argument_create(struct argument * argument, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&argument->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&argument->identifier, (map_compare_cb) strcmp, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            argument->argument = NULL;
        }
        if(status)
            store_destroy(&argument->store);
    }

    return status;
}

void argument_destroy(struct argument * argument) {
    struct argument_node * node;

    node = argument->argument;
    while(node) {
        if(node->map)
            map_destroy(node->map);
        node = node->next;
    }

    map_destroy(&argument->identifier);
    store_destroy(&argument->store);
}

int argument_parse(enum parser_type type, int mark, struct string * string, void * context) {
    struct argument_node * node;
    struct print_node * print;
    struct map * map;
    struct argument * argument = context;

    switch(mark) {
        case 1:
            if(type == parser_start) {
                node = store_calloc(&argument->store, sizeof(*node));
                if(!node) {
                    return panic("failed to calloc store object");
                } else {
                    node->next = argument->argument;
                    argument->argument = node;
                }
            } else if(type == parser_end) {
                if(!argument->argument->identifier) {
                    return panic("invalid string object");
                } else if(map_insert(&argument->identifier, argument->argument->identifier, argument->argument)) {
                    return panic("failed to insert map object");
                }
            }
            break;
        case 2: return string_store(string, &argument->store, &argument->argument->identifier); break;
        case 3: return string_store(string, &argument->store, &argument->argument->handler); break;
        case 5:
            argument->print = NULL;
            if(type == parser_next)
                return argument_parse(type, 6, string, context);
            break;
        case 6:
            print = store_calloc(&argument->store, sizeof(*print));
            if(!print) {
                return panic("failed to calloc store object");
            } else {
                if(argument->print) {
                    argument->print->next = print;
                } else {
                    argument->argument->print = print;
                }
                argument->print = print;

                if(argument_entry_parse(argument, string->string, string->length))
                    return panic("failed toi entry push argument object");
            }
            break;
        case 8:
            if(type == parser_start) {
                argument->range = store_calloc(&argument->store, sizeof(*argument->range));
                if(!argument->range)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                argument->range->next = argument->argument->range;
                argument->argument->range = argument->range;
            }
            break;
        case 9:  return string_long(string, &argument->range->min); break;
        case 10: return string_long(string, &argument->range->max); break;
        case 11:
            if(type == parser_start) {
                map = store_malloc(&argument->store, sizeof(*map));
                if(!map) {
                    return panic("failed to malloc store object");
                } else if(map_create(map, long_compare, argument->identifier.pool)) {
                    return panic("failed to create map object");
                } else {
                    argument->argument->map = map;
                }
            }
            break;
        case 12:
            if(type == parser_start) {
                argument->array = store_calloc(&argument->store, sizeof(*argument->array));
                if(!argument->array)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                if(map_insert(argument->argument->map, &argument->array->index, argument->array->string))
                    return panic("failed to insert map object");
            }
            break;
        case 13: return string_long(string, &argument->array->index); break;
        case 14: return string_store(string, &argument->store, &argument->array->string); break;
        case 15:
            if(type == parser_start) {
                argument->integer = store_calloc(&argument->store, sizeof(*argument->integer));
                if(!argument->integer)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                argument->argument->integer = argument->integer;
            }
            break;
        case 16:
            if(!strcmp("true", string->string))
                argument->integer->flag |= integer_sign;
            break;
        case 17:
            if(!strcmp("true", string->string))
                argument->integer->flag |= integer_string;
            break;
        case 18:
            if(!strcmp("true", string->string))
                argument->integer->flag |= integer_percent;
            break;
        case 19:
            if(!strcmp("true", string->string))
                argument->integer->flag |= integer_inverse;
            break;
        case 20:
            if(!strcmp("true", string->string))
                argument->integer->flag |= integer_absolute;
            break;
        case 21: return string_long(string, &argument->integer->divide); break;
        case 23:
            if(type == parser_start) {
                argument->optional = store_calloc(&argument->store, sizeof(*argument->optional));
                if(!argument->optional)
                    return panic("failed to calloc store object");
            } else if(type == parser_end) {
                argument->optional->next = argument->argument->optional;
                argument->argument->optional = argument->optional;
            }
            break;
        case 24: return string_long(string, &argument->optional->index); break;
        case 25: return string_store(string, &argument->store, &argument->optional->string); break;
    }

    return 0;
}

int argument_entry_parse(struct argument * argument, char * string, size_t length) {
    char * anchor;
    char * cursor;
    struct entry_node * entry;

    argument->entry = NULL;

    anchor = string;
    cursor = strchr(anchor, '{');
    while(cursor) {
        if(argument_entry_create(argument, anchor, cursor - anchor))
            return panic("failed to entry push argument object");

        entry = argument->entry;

        if(cursor[1] == '*') {
            cursor += 2;
        } else {
            entry->array[entry->count++] = strtol(cursor + 1, &cursor, 10);
            while(cursor[0] == ',' && entry->count < ENTRY_MAX)
                entry->array[entry->count++] = strtol(cursor + 1, &cursor, 10);
        }

        if(cursor[0] != '|')
            return panic("expected vertical bar");

        anchor = cursor + 1;
        cursor = strchr(anchor, '}');
        if(!cursor)
            return panic("expected curly close");

        entry->identifier = store_strcpy(&argument->store, anchor, cursor - anchor);
        if(!entry->identifier)
            return panic("failed to strcpy store object");

        anchor = cursor + 1;
        cursor = strchr(anchor, '{');
    }

    if(argument_entry_create(argument, anchor, (string + length) - anchor))
        return panic("failed to entry push argument object");

    return 0;
}

int argument_entry_create(struct argument * argument, char * string, size_t length) {
    int status = 0;
    struct entry_node * entry;

    entry = store_calloc(&argument->store, sizeof(*entry));
    if(!entry) {
        status = panic("failed to calloc store object");
    } else {
        entry->length = length;
        entry->string = store_strcpy(&argument->store, string, length);
        if(!entry->string) {
            status = panic("failed to strcpy store object");
        } else {
            if(argument->entry) {
                argument->entry->next = entry;
            } else {
                argument->print->entry = entry;
            }
            argument->entry = entry;
        }
    }

    return status;
}

int table_create(struct table * table, size_t size, struct heap * heap) {
    int status = 0;

    if(parser_create(&table->parser, size)) {
        status = panic("failed to create parser object");
    } else if(item_create(&table->item, size, heap)) {
        status = panic("failed to create item object");
        goto item_fail;
    } else if(skill_create(&table->skill, size, heap)) {
        status = panic("failed to create skill object");
        goto skill_fail;
    } else if(mob_create(&table->mob, size, heap)) {
        status = panic("failed to create mob object");
        goto mob_fail;
    } else if(mercenary_create(&table->mercenary, size, heap)) {
        status = panic("failed to create mercenary object");
        goto mercenary_fail;
    } else if(constant_create(&table->constant, size, heap)) {
        status = panic("failed to create constant object");
        goto constant_fail;
    } else if(argument_create(&table->argument, size, heap)) {
        status = panic("failed to create argument object");
        goto argument_fail;
    } else if(argument_create(&table->bonus, size, heap)) {
        status = panic("failed to create argument object");
        goto bonus_fail;
    } else if(argument_create(&table->bonus2, size, heap)) {
        status = panic("failed to create argument object");
        goto bonus2_fail;
    } else if(argument_create(&table->bonus3, size, heap)) {
        status = panic("failed to create argument object");
        goto bonus3_fail;
    } else if(argument_create(&table->bonus4, size, heap)) {
        status = panic("failed to create argument object");
        goto bonus4_fail;
    } else if(argument_create(&table->bonus5, size, heap)) {
        status = panic("failed to create argument object");
        goto bonus5_fail;
    } else if(argument_create(&table->sc_start, size, heap)) {
        status = panic("failed to create argument object");
        goto sc_start_fail;
    } else if(argument_create(&table->sc_start2, size, heap)) {
        status = panic("failed to create argument object");
        goto sc_start2_fail;
    } else if(argument_create(&table->sc_start4, size, heap)) {
        status = panic("failed to create argument object");
        goto sc_start4_fail;
    } else if(argument_create(&table->statement, size, heap)) {
        status = panic("failed to create argument object");
        goto statement_fail;
    }

    return status;

statement_fail:
    argument_destroy(&table->sc_start4);
sc_start4_fail:
    argument_destroy(&table->sc_start2);
sc_start2_fail:
    argument_destroy(&table->sc_start);
sc_start_fail:
    argument_destroy(&table->bonus5);
bonus5_fail:
    argument_destroy(&table->bonus4);
bonus4_fail:
    argument_destroy(&table->bonus3);
bonus3_fail:
    argument_destroy(&table->bonus2);
bonus2_fail:
    argument_destroy(&table->bonus);
bonus_fail:
    argument_destroy(&table->argument);
argument_fail:
    constant_destroy(&table->constant);
constant_fail:
    mercenary_destroy(&table->mercenary);
mercenary_fail:
    mob_destroy(&table->mob);
mob_fail:
    skill_destroy(&table->skill);
skill_fail:
    item_destroy(&table->item);
item_fail:
    parser_destroy(&table->parser);

    return status;
}

void table_destroy(struct table * table) {
    argument_destroy(&table->statement);
    argument_destroy(&table->sc_start4);
    argument_destroy(&table->sc_start2);
    argument_destroy(&table->sc_start);
    argument_destroy(&table->bonus5);
    argument_destroy(&table->bonus4);
    argument_destroy(&table->bonus3);
    argument_destroy(&table->bonus2);
    argument_destroy(&table->bonus);
    argument_destroy(&table->argument);
    constant_destroy(&table->constant);
    mercenary_destroy(&table->mercenary);
    mob_destroy(&table->mob);
    skill_destroy(&table->skill);
    item_destroy(&table->item);
    parser_destroy(&table->parser);
}

int table_item_parse(struct table * table, char * path) {
    return csv_parse(path, item_parse, &table->item);
}

int table_item_combo_parse(struct table * table, char * path) {
    return csv_parse(path, item_combo_parse, &table->item);
}

int table_skill_parse(struct table * table, char * path) {
    return parser_file2(&table->parser, skill_markup, path, skill_parse, &table->skill);
}

int table_mob_parse(struct table * table, char * path) {
    return csv_parse(path, mob_parse, &table->mob);
}

int table_mercenary_parse(struct table * table, char * path) {
    return csv_parse(path, mercenary_parse, &table->mercenary);
}

int table_constant_parse(struct table * table, char * path) {
    return parser_file(&table->parser, constant_markup, path, constant_parse, &table->constant);
}

int table_constant_data_parse(struct table * table, char * path) {
    return parser_file(&table->parser, constant_markup, path, constant_data_parse, &table->constant);
}

int table_constant_group_parse(struct table * table, char * path) {
    return parser_file(&table->parser, constant_group_markup, path, constant_group_parse, &table->constant);
}

int table_argument_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->argument);
}

int table_bonus_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->bonus);
}

int table_bonus2_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->bonus2);
}

int table_bonus3_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->bonus3);
}

int table_bonus4_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->bonus4);
}

int table_bonus5_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->bonus5);
}

int table_sc_start_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->sc_start);
}

int table_sc_start2_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->sc_start2);
}

int table_sc_start4_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->sc_start4);
}

int table_statement_parse(struct table * table, char * path) {
    return parser_file(&table->parser, argument_markup, path, argument_parse, &table->statement);
}

struct item_node * item_start(struct table * table) {
    return map_start(&table->item.id).value;
}

struct item_node * item_next(struct table * table) {
    return map_next(&table->item.id).value;
}

struct item_node * item_id(struct table * table, long id) {
    return map_search(&table->item.id, &id);
}

struct item_node * item_name(struct table * table, char * name) {
    return map_search(&table->item.name, name);
}

struct skill_node * skill_id(struct table * table, long id) {
    return map_search(&table->skill.id, &id);
}

struct skill_node * skill_name(struct table * table, char * name) {
    return map_search(&table->skill.name, name);
}

struct mob_node * mob_id(struct table * table, long id) {
    return map_search(&table->mob.id, &id);
}

struct mob_node * mob_sprite(struct table * table, char * sprite) {
    return map_search(&table->mob.sprite, sprite);
}

struct mercenary_node * mercenary_id(struct table * table, long id) {
    return map_search(&table->mercenary.id, &id);
}

struct constant_node * constant_identifier(struct table * table, char * identifier) {
    return map_search(&table->constant.identifier, identifier);
}

struct constant_group_node * constant_group_identifier(struct table * table, char * identifier) {
    return map_search(&table->constant.group, identifier);
}

struct argument_node * argument_identifier(struct table * table, char * identifier) {
    return map_search(&table->argument.identifier, identifier);
}

struct argument_node * bonus_identifier(struct table * table, char * identifier) {
    return map_search(&table->bonus.identifier, identifier);
}

struct argument_node * bonus2_identifier(struct table * table, char * identifier) {
    return map_search(&table->bonus2.identifier, identifier);
}

struct argument_node * bonus3_identifier(struct table * table, char * identifier) {
    return map_search(&table->bonus3.identifier, identifier);
}

struct argument_node * bonus4_identifier(struct table * table, char * identifier) {
    return map_search(&table->bonus4.identifier, identifier);
}

struct argument_node * bonus5_identifier(struct table * table, char * identifier) {
    return map_search(&table->bonus5.identifier, identifier);
}

struct argument_node * sc_start_identifier(struct table * table, char * identifier) {
    return map_search(&table->sc_start.identifier, identifier);
}

struct argument_node * sc_start2_identifier(struct table * table, char * identifier) {
    return map_search(&table->sc_start2.identifier, identifier);
}

struct argument_node * sc_start4_identifier(struct table * table, char * identifier) {
    return map_search(&table->sc_start4.identifier, identifier);
}

struct argument_node * statement_identifier(struct table * table, char * identifier) {
    return map_search(&table->statement.identifier, identifier);
}
