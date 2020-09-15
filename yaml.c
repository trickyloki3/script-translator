#include "yaml.h"

#include "yaml_scanner.h"

#define YAML_ROOT_SCOPE -1

static inline int yaml_push(struct yaml *, enum yaml_type);
static inline int yaml_pop(struct yaml *, int);

static inline void yaml_comment(struct yaml *);
static inline int yaml_newline(struct yaml *);
static inline int yaml_document(struct yaml *);
static inline int yaml_block(struct yaml *);
static inline int yaml_plain(struct yaml *);
static inline int yaml_separate(struct yaml *);
static inline int yaml_scalar(struct yaml *, int);

static inline int yaml_putc(struct yaml_buffer *, int, size_t);
static inline int yaml_puts(struct yaml_buffer *, char *, size_t);

int yaml_create(struct yaml * yaml, size_t depth, size_t size) {
    int status = 0;

    struct yaml_node * prev, * iter;

    yaml->stack = malloc(sizeof(*yaml->stack) * (depth + 1));
    if(!yaml->stack) {
        status = panic("failed to create stack object");
    } else {
        prev = yaml->stack;
        iter = yaml->stack + 1;
        while(depth--) {
            iter->prev = prev;
            prev->next = iter;
            prev = iter;
            iter++;
        }
        yaml->stack->prev = NULL;
        prev->next = NULL;

        yaml->buffer = malloc(sizeof(*yaml->buffer) + size);
        if(!yaml->buffer) {
            status = panic("failed to create buffer object");
        } else {
            yaml->buffer->base = (char *) (yaml->buffer + 1);
            yaml->buffer->size = size;

            if(yamllex_init_extra(yaml, &yaml->scanner))
                status = panic("failed to create scanner object");

            if(status)
                free(yaml->buffer);
        }
        if(status)
            free(yaml->stack);
    }

    return status;
}

void yaml_destroy(struct yaml * yaml) {
    yamllex_destroy(yaml->scanner);
    free(yaml->buffer);
    free(yaml->stack);
}

int yaml_parse(struct yaml * yaml, const char * path, yaml_cb cb, void * arg) {
    int status = 0;

    FILE * file;

    file = fopen(path, "r");
    if(!file) {
        status = panic("failed to open %s", path);
    } else {
        yamlrestart(file, yaml->scanner);
        yaml->cb = cb;
        yaml->arg = arg;
        yaml->root = yaml->stack;
        yaml->root->scope = YAML_ROOT_SCOPE;
        yaml->scope = 0;
        yaml->scalar = 0;
        yaml->token = 0;
        yaml->string = NULL;
        yaml->length = 0;
        yaml->space = 0;

        if(yaml_document(yaml)) {
            status = panic("failed to document yaml object");
        } else if(yaml_pop(yaml, YAML_ROOT_SCOPE)) {
            status = panic("failed to pop yaml object");
        }

        fclose(file);
    }

    return status;
}

static inline int yaml_push(struct yaml * yaml, enum yaml_type type) {
    if(yaml->root->scope >= yaml->scope)
        return panic("invalid scope");

    yaml->root = yaml->root->next;
    if(!yaml->root)
        return panic("out of memory");

    yaml->root->type = type;
    yaml->root->scope = yaml->scope;

    if(yaml->cb(type == yaml_sequence ? yaml_list_start : yaml_map_start, NULL, 0, yaml->arg))
        return panic("failed to start yaml object");

    return 0;
}

static inline int yaml_pop(struct yaml * yaml, int scope) {
    while(yaml->root->scope > scope) {
        if(yaml->cb(yaml->root->type == yaml_sequence ? yaml_list_end : yaml_map_end, NULL, 0, yaml->arg))
            return panic("failed to end yaml object");

        yaml->root = yaml->root->prev;
    }

    return 0;
}

static inline void yaml_comment(struct yaml * yaml) {
    yaml->token = yamllex(yaml->scanner);
    while(yaml->token == l_empty)
        yaml->token = yamllex(yaml->scanner);
}

static inline int yaml_newline(struct yaml * yaml) {
    yaml->token = yamllex(yaml->scanner);
    if(yaml->token != b_break)
        return panic("expected newline");

    yaml_comment(yaml);

    return 0;
}

static inline int yaml_document(struct yaml * yaml) {
    yaml_comment(yaml);

    while(yaml->token) {
        if(yaml->token == s_indent) {
            yaml->scope = yaml->space;
            yaml->token = yamllex(yaml->scanner);
        } else {
            yaml->scope = 0;
        }

        if(yaml_pop(yaml, yaml->scope))
            return panic("failed to pop yaml object");

        if(yaml->root->scope > YAML_ROOT_SCOPE) {
            if(yaml->root->scope != yaml->scope)
                return panic("invalid scope");

            if(yaml->root->type == yaml_sequence) {
                if(yaml->token != c_sequence_entry)
                    return panic("expected sequence entry");

                if(yaml_separate(yaml))
                    return panic("failed to container yaml object");
            } else {
                if(yaml->token != ns_key_one_line)
                    return panic("expected mapping key");

                if(yaml->cb(yaml_string, yaml->string, yaml->length, yaml->arg))
                    return panic("failed to process scalar event");

                if(yaml_separate(yaml))
                    return panic("failed to container yaml object");
            }
        } else {
            if(yaml_block(yaml))
                return panic("failed to block yaml object");
        }
    }

    return 0;
}

static inline int yaml_block(struct yaml * yaml) {
    if(yaml->token == ns_key_one_line) {
        if(yaml_push(yaml, yaml_map))
            return panic("failed to start yaml object");

        if(yaml->cb(yaml_string, yaml->string, yaml->length, yaml->arg))
            return panic("failed to process scalar event");

        if(yaml_separate(yaml))
            return panic("failed to container yaml object");
    } else if(yaml->token == c_sequence_entry) {
        if(yaml_push(yaml, yaml_sequence))
            return panic("failed ot start yaml object");

        if(yaml_separate(yaml))
            return panic("failed to container yaml object");
    } else {
        if(yaml_plain(yaml))
            return panic("failed to plain yaml object");
    }

    return 0;
}

static inline int yaml_plain(struct yaml * yaml) {
    if(yaml->token == ns_plain_one_line) {
        if(yaml->cb(yaml_string, yaml->string, yaml->length, yaml->arg))
            return panic("failed to process scalar event");

        if(yaml_newline(yaml))
            return panic("failed to parse newline");
    } else if(yaml->token == c_literal) {
        if(yaml_scalar(yaml, 1))
            return panic("failed to scalar yaml object");
    } else if(yaml->token == c_folded) {
        if(yaml_scalar(yaml, 0))
            return panic("failed to scalar yaml object");
    } else {
        return panic("invalid token - %d", yaml->token);
    }

    return 0;
}

static inline int yaml_separate(struct yaml * yaml) {
    yaml->token = yamllex(yaml->scanner);
    if(yaml->token == s_separate_in_line) {
        yaml->scope += yaml->space + 1;
        yaml->token = yamllex(yaml->scanner);

        if(yaml->root->type == yaml_sequence ? yaml_block(yaml) : yaml_plain(yaml))
            return panic("failed to block yaml object");
    } else if(yaml->token == b_break) {
        yaml_comment(yaml);

        if(yaml->token != s_indent)
            return panic("expected space");

        yaml->scope = yaml->space;
        yaml->token = yamllex(yaml->scanner);

        if(yaml_block(yaml))
            return panic("failed to block yaml object");
    } else {
        return panic("expected space or newline");
    }

    return 0;
}

static inline int yaml_scalar(struct yaml * yaml, int flag) {
    int space = 0;
    int newline = 0;
    struct yaml_buffer cursor;

    yaml->scalar = 1;

    if(yaml_newline(yaml))
        return panic("failed to parse newline");

    if(yaml->token != s_indent)
        return panic("expected space");

    yaml->scope = yaml->space;

    if(yaml->root->scope >= yaml->scope)
        return panic("invalid scope");

    memcpy(&cursor, yaml->buffer, sizeof(cursor));

    while(yaml->token == s_indent && yaml->scope <= yaml->space) {
        if(flag) {
            if(yaml_putc(&cursor, '\n', newline))
                return panic("failed to putc buffer object");
        } else {
            if(newline == 1) {
                if(yaml_putc(&cursor, space || yaml->scope < yaml->space ? '\n' : ' ', 1))
                    return panic("failed to putc buffer object");
            } else if(newline > 1) {
                if(yaml_putc(&cursor, '\n', newline - 1))
                    return panic("failed to putc buffer object");
            }
        }

        space = yaml->space - yaml->scope;

        if(yaml_putc(&cursor, ' ', space))
            return panic("failed to putcn buffer object");

        yaml->token = yamllex(yaml->scanner);
        if(yaml->token != nb_char)
            return panic("expected scalar");

        if(yaml_puts(&cursor, yaml->string, yaml->length))
            return panic("failed to strcpy buffer object");

        yaml->token = yamllex(yaml->scanner);
        if(yaml->token != b_break)
            return panic("expected newline");

        newline = 1;
        yaml->token = yamllex(yaml->scanner);
        while(yaml->token == l_empty) {
            newline++;
            yaml->token = yamllex(yaml->scanner);
        }
    }

    if(yaml_putc(&cursor, '\0', 1))
        return panic("failed to putc buffer object");

    if(yaml->cb(yaml_string, yaml->buffer->base, yaml->buffer->size - cursor.size - 1, yaml->arg))
        return panic("failed to process scalar event");

    yaml->scalar = 0;

    return 0;
}

static inline int yaml_putc(struct yaml_buffer * buffer, int c, size_t n) {
    if(buffer->size < n)
        return panic("out of memory");

    memset(buffer->base, c, n);
    buffer->base += n;
    buffer->size -= n;

    return 0;
}

static inline int yaml_puts(struct yaml_buffer * buffer, char * string, size_t length) {
    if(buffer->size < length)
        return panic("out of memory");

    memcpy(buffer->base, string, length);
    buffer->base += length;
    buffer->size -= length;

    return 0;
}
