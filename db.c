#include "db.h"

int data_create(struct schema *, enum type, int, struct data **);
void data_destroy(struct schema *, struct data *);
void data_print(struct data *, int, char *);

int parser_event(enum event_type, struct string *, void *);

int data_create(struct schema * schema, enum type type, int mark, struct data ** result) {
    int status = 0;
    struct data * data;

    data = pool_get(schema->pool);
    if(!data) {
        status = panic("out of memory");
    } else {
        data->type = type;
        data->mark = mark;
        data->data = NULL;
        data->next = NULL;
        if(map_create(&data->map, (map_compare_cb) strcmp, schema->map_pool))
            status = panic("failed to create map object");
        if(status) {
            pool_put(schema->pool, data);
        } else {
            *result = data;
        }
    }

    return status;
}

void data_destroy(struct schema * schema, struct data * data) {
    map_destroy(&data->map);
    pool_put(schema->pool, data);
}

void data_print(struct data * data, int indent, char * key) {
    int i;
    struct map_pair kv;

    for(i = 0; i < indent; i++)
        fputs("    ", stdout);

    switch(data->type) {
        case list:
            key ?
                fprintf(stdout, "[%s:list:%d]\n", key, data->mark) :
                fprintf(stdout, "[list:%d]\n", data->mark);
            data_print(data->data, indent + 1, NULL);
            break;
        case map:
            key ?
                fprintf(stdout, "[%s:map:%d]\n", key, data->mark) :
                fprintf(stdout, "[map:%d]\n", data->mark);
            kv = map_start(&data->map);
            while(kv.key) {
                data_print(kv.value, indent + 1, kv.key);
                kv = map_next(&data->map);
            }
            break;
        case string:
            key ?
                fprintf(stdout, "[%s:string:%d]\n", key, data->mark) :
                fprintf(stdout, "[string:%d]\n", data->mark);
            break;
    }
}

int schema_create(struct schema * schema, struct heap * heap) {
    int status = 0;

    schema->map_pool = heap->map_pool;

    schema->pool = heap_pool(heap, sizeof(struct data));
    if(!schema->pool) {
        status = panic("failed to pool heap object");
    } else {
        if(list_create(&schema->list, heap->list_pool)) {
            status = panic("failed to create list object");
        } else {
            if(data_create(schema, list, 0, &schema->root)) {
                status = panic("failed to create data object");
            } else {
                if(list_push(&schema->list, schema->root))
                    status = panic("failed to push list object");
                if(status)
                    data_destroy(schema, schema->root);
            }
            if(status)
                list_destroy(&schema->list);
        }
    }

    return status;
}

void schema_destroy(struct schema * schema) {
    struct data * data;

    data = list_pop(&schema->list);
    while(data) {
        data_destroy(schema, data);
        data = list_pop(&schema->list);
    }
    list_destroy(&schema->list);
}

int schema_push(struct schema * schema, enum type type, int mark, char * key) {
    int status = 0;
    struct data * data;

    if(data_create(schema, type, mark, &data)) {
        status = panic("failed to create data object");
    } else {
        if(list_push(&schema->list, data)) {
            status = panic("failed to push list object");
        } else {
            if(schema->root->type == list) {
                if(schema->root->data) {
                    status = panic("invalid data");
                } else {
                    schema->root->data = data;
                }
            } else if(schema->root->type == map) {
                if(!key) {
                    status = panic("invalid key");
                } else if(map_insert(&schema->root->map, key, data)) {
                    status = panic("failed to insert map object");
                }
            }
            if(status) {
                /* skip push on error */
            } else if(data->type == list || data->type == map) {
                data->next = schema->root;
                schema->root = data;
            }
            if(status)
                list_pop(&schema->list);
        }
        if(status)
            data_destroy(schema, data);
    }


    return status;
}

void schema_pop(struct schema * schema) {
    if(schema->root->next)
        schema->root = schema->root->next;
}

void schema_print(struct schema * schema) {
    if(schema->root->data)
        data_print(schema->root->data, 0, NULL);
}

int parser_create(struct parser * parser, size_t size, struct heap * heap) {
    int status = 0;

    parser->size = size;

    if(csv_create(&parser->csv, parser->size, heap)) {
        status = panic("failed to create csv object");
    } else {
        if(json_create(&parser->json, parser->size)) {
            status = panic("failed to create json object");
        } else {
            if(yaml_create(&parser->yaml, parser->size, heap))
                status = panic("failed to create yaml object");
            if(status)
                json_destroy(&parser->json);
        }
        if(status)
            csv_destroy(&parser->csv);
    }

    return status;
}

void parser_destroy(struct parser * parser) {
    yaml_destroy(&parser->yaml);
    json_destroy(&parser->json);
    csv_destroy(&parser->csv);
}

int parser_event(enum event_type event, struct string * string, void * context) {
    int status = 0;

    return status;
}

int parser_parse(struct parser * parser, const char * path) {
    int status = 0;
    char * ext;

    ext = strrchr(path, '.');
    if(!ext) {
        status = panic("failed to get file extension - %s", path);
    } else {
        if(!strcmp(ext, ".txt")) {
            if(csv_parse(&parser->csv, path, parser->size, parser_event, parser))
                status = panic("failed to parse csv object");
        } else if(!strcmp(ext, ".json")) {
            if(json_parse(&parser->json, path, parser->size, parser_event, parser))
                status = panic("failed to parse json object");
        } else if(!strcmp(ext, ".yaml") || !strcmp(ext, ".yml")) {
            if(yaml_parse(&parser->yaml, path, parser->size, parser_event, parser))
                status = panic("failed to parse yaml object");
        }
    }

    return status;
}
