#include "table.h"

int long_compare(void *, void *);
int char_compare(void *, void *);

struct schema_markup csv_markup[] = {
    {1, list, 0, NULL},
    {2, list, 1, NULL},
    {3, string, 2, NULL},
    {0, 0, 0},
};

struct schema_markup constant_markup[] = {
    {1, list, 0, NULL},
    {2, map, 1, NULL},
    {3, string, 2, "identifier"},
    {3, string, 3, "value"},
    {3, string, 4, "tag"},
    {3, list, 5, "range"},
    {4, map, 6, NULL},
    {5, string, 7, "min"},
    {5, string, 8, "max"},
    {0, 0, 0},
};

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int char_compare(void * x, void * y) {
    return strcmp(x, y);
}

int item_create(struct item * item, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&item->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&item->id, long_compare, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&item->name, char_compare, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                map_destroy(&item->id);
        }
        if(status)
            store_destroy(&item->store);
    }

    return status;
}

void item_destroy(struct item * item) {
    map_destroy(&item->name);
    map_destroy(&item->id);
    store_destroy(&item->store);
}

int item_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    char * last;
    struct item * item = context;

    switch(mark) {
        case 1:
            if(event == start) {
                item->item = store_object(&item->store, sizeof(*item->item));
                if(!item->item) {
                    status = panic("failed to object store object");
                } else {
                    memset(item->item, 0, sizeof(*item->item));
                    item->index = 0;
                }
            } else if(event == end) {
                if(item->index != 20) {
                    status = panic("invalid index");
                } else if(!item->item->name) {
                    status = panic("invalid name");
                } else if(map_insert(&item->id, &item->item->id, item->item)) {
                    status = panic("failed to insert map object");
                } else if(map_insert(&item->name, item->item->name, item->item)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 2:
            switch(item->index) {
                case 0:
                    item->item->id = strtol(string->string, &last, 10);
                    if(*last)
                        status = panic("failed to strtol string object");
                    break;
                case 2:
                    item->item->name = store_char(&item->store, string->string, string->length);
                    if(!item->item->name)
                        status = panic("failed to char store object");
                    break;
                case 19:
                    if(item_script_parse(item, string->string))
                        status = panic("failed to script parse item object");
                    break;
            }
            item->index++;
            break;
    }

    return status;
}

int item_script_parse(struct item * item, char * string) {
    int status = 0;

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
                        item->item->bonus = store_char(&item->store, anchor, string - anchor + 1);
                        if(!item->item->bonus)
                            status = panic("failed to char store object");
                        break;
                    case 1:
                        item->item->equip = store_char(&item->store, anchor, string - anchor + 1);
                        if(!item->item->equip)
                            status = panic("failed to char store object");
                        break;
                    case 2:
                        item->item->unequip = store_char(&item->store, anchor, string - anchor + 1);
                        if(!item->item->unequip)
                            status = panic("failed to char store object");
                        break;
                }
                index++;

                if(status)
                    break;

                anchor = NULL;
            }
        }
        string++;
    }

    return status;
}

int constant_create(struct constant * constant, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&constant->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&constant->identifier, char_compare, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            store_destroy(&constant->store);
    }

    return status;
}

void constant_destroy(struct constant * constant) {
    map_destroy(&constant->identifier);
    store_destroy(&constant->store);
}

int constant_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    char * last;
    struct constant * constant = context;

    switch(mark) {
        case 1:
            if(event == start) {
                constant->constant = store_object(&constant->store, sizeof(*constant->constant));
                if(!constant->constant) {
                    status = panic("failed to object store object");
                } else {
                    memset(constant->constant, 0, sizeof(*constant->constant));
                }
            } else if(event == end) {
                if(!constant->constant->identifier) {
                    status = panic("invalid string object");
                } else if(map_insert(&constant->identifier, constant->constant->identifier, constant->constant)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 2:
            constant->constant->identifier = store_char(&constant->store, string->string, string->length);
            if(!constant->constant->identifier)
                status = panic("failed to char store object");
            break;
        case 3:
            constant->constant->value = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 4:
            constant->constant->tag = store_char(&constant->store, string->string, string->length);
            if(!constant->constant->tag)
                status = panic("failed to char store object");
            break;
        case 6:
            if(event == start) {
                constant->range = store_object(&constant->store, sizeof(*constant->range));
                if(!constant->range) {
                    status = panic("failed to object store object");
                } else {
                    memset(constant->range, 0, sizeof(*constant->range));
                }
            } else if(event == end) {
                constant->range->next = constant->constant->range;
                constant->constant->range = constant->range;
            }
            break;
        case 7:
            constant->range->min = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 8:
            constant->range->max = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
    }

    return status;
}

int table_create(struct table * table, size_t size, struct heap * heap) {
    int status = 0;

    if(schema_create(&table->schema, heap)) {
        status = panic("failed to create schema object");
    } else {
        if(parser_create(&table->parser, size, heap)) {
            status = panic("failed to create parser object");
        } else {
            if(item_create(&table->item, size, heap)) {
                status = panic("failed to create item object");
            } else {
                if(constant_create(&table->constant, size, heap))
                    status = panic("failed to create constant object");
                if(status)
                    item_destroy(&table->item);
            }
            if(status)
                parser_destroy(&table->parser);
        }
        if(status)
            schema_destroy(&table->schema);
    }

    return status;
}

void table_destroy(struct table * table) {
    constant_destroy(&table->constant);
    item_destroy(&table->item);
    parser_destroy(&table->parser);
    schema_destroy(&table->schema);
}

int table_item_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, csv_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, item_parse, &table->item, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
}

int table_constant_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, constant_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, constant_parse, &table->constant, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
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

struct constant_node * constant_identifier(struct table * table, char * identifier) {
    return map_search(&table->constant.identifier, identifier);
}