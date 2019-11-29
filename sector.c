#include "sector.h"

struct header {
    struct sector * sector;
    long min;
    long max;
};

#define header_size sizeof(struct header)

static inline struct header * get_header(void *);
static inline void * get_object(struct header *);

static inline struct header * get_header(void * object) {
    return (void *) (char *) object - header_size;
}

static inline void * get_object(struct header * header) {
    return (char *) header + header_size;
}

int sector_create(struct sector * sector, long size) {
    int status = 0;

    if(size <= 0) {
        status = panic("invalid size");
    } else {
        sector->buffer = malloc(size + header_size);
        if(!sector->buffer) {
            status = panic("out of memory");
        } else {
            if(pool_create(&sector->pool, sizeof(struct range_node), 16)) {
                status = panic("failed to create pool object");
            } else {
                if(range_create_add(&sector->range, &sector->pool, 0, size + header_size - 1))
                    status = panic("failed to create range object");
                if(status)
                    pool_destroy(&sector->pool);
            }
            if(status)
                free(sector->buffer);
        }
    }

    return status;
}

void sector_destroy(struct sector * sector) {
    range_destroy(&sector->range);
    pool_destroy(&sector->pool);
    free(sector->buffer);
}

void * sector_malloc(struct sector * sector, long size) {
    void * object = NULL;
    long min;
    long max;
    struct header * header;

    size += header_size - 1;

    if(!range_search(&sector->range, size, &min, &max)) {
        header = (struct header *) (sector->buffer + min);
        header->sector = sector;
        header->min = min;
        header->max = min + size;
        if(range_remove(&sector->range, header->min, header->max)) {
            panic("failed to remove sector object");
        } else {
            object = get_object(header);
        }
    }

    return object;
}

void sector_free(void * object) {
    struct header * header;

    header = get_header(object);
    if(range_add(&header->sector->range, header->min, header->max))
        panic("failed to add sector object");
}

long sector_size(void * object) {
    struct header * header = get_header(object);
    return header->max - header->min - header_size + 1;
}

void sector_print(struct sector * sector) {
    struct range_node * root;
    struct range_node * node;

    root = sector->range.root;
    node = root->next;
    fprintf(stdout, "sector(%ld):", root->max + 1);
    if(node == root) {
        fprintf(stdout, "empty\n");
    } else {
        if(node->min > root->min)
            fprintf(stdout, "(%ld,%ld),", root->min, node->min - 1);

        while(node->next != root) {
            fprintf(stdout, "[%ld,%ld],", node->min, node->max);
            if(node->next != root)
                fprintf(stdout, "(%ld,%ld),", node->max + 1, node->next->min - 1);
            node = node->next;
        }
        fprintf(stdout, "[%ld,%ld],", node->min, node->max);

        if(node->max < root->max) {
            fprintf(stdout, "(%ld,%ld)\n", node->max + 1, root->max);
        } else {
            fprintf(stdout, "\n");
        }
    }
}
