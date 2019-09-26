#include "sector.h"

struct sector_header {
    struct sector * sector;
    size_t min;
    size_t max;
};

int sector_node_create(struct sector *, size_t, size_t, struct sector_node **);
void sector_node_destroy(struct sector *, struct sector_node *);
inline void sector_node_attach(struct sector_node *, struct sector_node *);
inline void sector_node_detach(struct sector_node *);

inline size_t sector_min(size_t, size_t);
inline size_t sector_max(size_t, size_t);
int sector_add(struct sector *, size_t, size_t);
int sector_remove(struct sector *, size_t, size_t);

inline struct sector_header * sector_get_header(void *);
inline void * sector_get_object(struct sector_header *);

int sector_node_create(struct sector * sector, size_t min, size_t max, struct sector_node ** result) {
    int status = 0;
    struct sector_node * node;

    node = pool_get(sector->pool);
    if(!node) {
        status = panic("out of memory");
    } else {
        node->min = min;
        node->max = max;
        node->next = node;
        node->prev = node;
        *result = node;
    }

    return status;
}

void sector_node_destroy(struct sector * sector, struct sector_node * node) {
    pool_put(sector->pool, node);
}

inline void sector_node_attach(struct sector_node * x, struct sector_node * y) {
    x->next->prev = y->prev;
    y->prev->next = x->next;
    x->next = y;
    y->prev = x;
}

inline void sector_node_detach(struct sector_node * x) {
    x->prev->next = x->next;
    x->next->prev = x->prev;
    x->next = x;
    x->prev = x;
}

inline size_t sector_min(size_t x, size_t y) {
    return x < y ? x : y;
}

inline size_t sector_max(size_t x, size_t y) {
    return x < y ? y : x;
}

int sector_add(struct sector * sector, size_t x, size_t y) {
    int status = 0;
    size_t min;
    size_t max;
    struct sector_node * iter;
    struct sector_node * node;

    if(x < y) {
        min = x;
        max = y;
    } else {
        min = y;
        max = x;
    }

    if(sector->root == sector->root->next) {
        if(sector_node_create(sector, min, max, &node)) {
            status = panic("failed to create node object");
        } else {
            sector_node_attach(sector->root, node);
        }
    } else {
        iter = sector->root->next;
        while(iter->next != sector->root && iter->max != SIZE_MAX && iter->max + 1 < min)
            iter = iter->next;

        if(iter->max != SIZE_MAX && iter->max + 1 < min) {
            if(sector_node_create(sector, min, max, &node)) {
                status = panic("failed to create node object");
            } else {
                sector_node_attach(iter, node);
            }
        } else if(iter->min != 0 && iter->min - 1 > max) {
            if(sector_node_create(sector, min, max, &node)) {
                status = panic("failed to create node object");
            } else {
                sector_node_attach(node, iter);
            }
        } else {
            iter->min = sector_min(iter->min, min);
            iter->max = sector_max(iter->max, max);
            while(iter->next != sector->root && iter->max + 1 >= iter->next->min) {
                node = iter->next;
                iter->max = sector_max(iter->max, node->max);
                sector_node_detach(node);
                sector_node_destroy(sector, node);
            }
        }
    }

    return status;
}

int sector_remove(struct sector * sector, size_t x, size_t y) {
    int status = 0;
    size_t min;
    size_t max;
    struct sector_node * iter;
    struct sector_node * node;

    if(x < y) {
        min = x;
        max = y;
    } else {
        min = y;
        max = x;
    }

    if(sector->root != sector->root->next) {
        iter = sector->root->next;
        while(iter->next != sector->root && (iter->max < min || iter->min > max))
            iter = iter->next;

        while(iter != sector->root && iter->max >= min && iter->min <= max && !status) {
            if(iter->min < min) {
                if(iter->max > max) {
                    if(sector_node_create(sector, iter->min, min - 1, &node)) {
                        status = panic("failed to create node object");
                    } else {
                        sector_node_attach(node, iter);
                        iter->min = max + 1;
                    }
                } else {
                    iter->max = min - 1;
                }
            } else {
                if(iter->max > max) {
                    iter->min = max + 1;
                } else {
                    node = iter;
                    iter = iter->next;
                    sector_node_detach(node);
                    sector_node_destroy(sector, node);
                    continue;
                }
            }
            iter = iter->next;
        }
    }

    return status;
}

inline struct sector_header * sector_get_header(void * object) {
    return (void *) (char *) object - sizeof(struct sector_header);
}

inline void * sector_get_object(struct sector_header * header) {
    return (char *) header + sizeof(struct sector_header);
}

int sector_create(struct sector * sector, size_t size, struct pool * pool) {
    int status = 0;

    if(!size) {
        status = panic("size is zero");
    } else if(!pool) {
        status = panic("pool is zero");
    } else if(pool->size != sizeof(struct sector_node)) {
        status = panic("pool is invalid");
    } else {
        sector->buffer = malloc(size);
        if(!sector->buffer) {
            status = panic("out of memory");
        } else {
            sector->pool = pool;
            if(sector_node_create(sector, 0, size - 1, &sector->root)) {
                status = panic("failed to create node object");
            } else {
                if(sector_add(sector, 0, size - 1))
                    status = panic("failed to add sector object");
                if(status)
                    sector_node_destroy(sector, sector->root);
            }
            if(status)
                free(sector->buffer);
        }
    }

    return status;
}

void sector_destroy(struct sector * sector) {
    struct sector_node * node;

    if(sector->root) {
        while(sector->root != sector->root->next) {
            node = sector->root->next;
            sector_node_detach(node);
            sector_node_destroy(sector, node);
        }
        sector_node_destroy(sector, sector->root);
    }
    free(sector->buffer);
}

void * sector_malloc(struct sector * sector, size_t size) {
    void * object = NULL;
    struct sector_node * node;
    struct sector_header * header;

    size += sizeof(struct sector_header) - 1;

    node = sector->root->next;
    while(node != sector->root && size > node->max - node->min)
        node = node->next;

    if(node != sector->root) {
        header = (struct sector_header *) (sector->buffer + node->min);
        header->sector = sector;
        header->min = node->min;
        header->max = node->min + size;
        if(sector_remove(sector, header->min, header->max)) {
            panic("failed to remove sector object");
        } else {
            object = sector_get_object(header);
        }
    }

    return object;
}

void sector_free(void * object) {
    struct sector_header * header;

    header = sector_get_header(object);
    if(sector_add(header->sector, header->min, header->max)) {
        panic("failed to add sector object");
    } else {
        memset(header, 0, sizeof(*header));
    }
}

void sector_print(struct sector * sector) {
    struct sector_node * root;
    struct sector_node * node;

    root = sector->root;
    node = root->next;
    fprintf(stdout, "sector(%zu):", root->max + 1);
    if(node == root) {
        fprintf(stdout, "empty\n");
    } else {
        if(node->min > root->min)
            fprintf(stdout, "(%zu,%zu),", root->min, node->min - 1);

        while(node->next != root) {
            fprintf(stdout, "[%zu,%zu],", node->min, node->max);
            if(node->next != root)
                fprintf(stdout, "(%zu,%zu),", node->max + 1, node->next->min - 1);
            node = node->next;
        }
        fprintf(stdout, "[%zu,%zu],", node->min, node->max);

        if(node->max < root->max) {
            fprintf(stdout, "(%zu,%zu)\n", node->max + 1, root->max);
        } else {
            fprintf(stdout, "\n");
        }
    }
}
