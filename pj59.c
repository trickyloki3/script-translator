#include "csv.h"

int process(struct list * list, void * data) {
    struct list_node * node;
    struct string * string;

    if(list->root) {
        node = list->root->prev;
        do {
            string = node->object;
            fprintf(stdout, "%s,", string->string);
            node = node->prev;
        } while(node != list->root);
    }
    fprintf(stdout, "\n");

    return 0;
}

int main(int argc, char ** argv) {
    int status = 0;
    struct pool pool;
    struct csv csv;

    if(pool_create(&pool, sizeof(struct list_node), 64)) {
        status = panic("failed to create pool object");
    } else {
        if(csv_create(&csv, &pool)) {
            status = panic("failed to create csv object");
        } else {
            if(csv_parse(&csv, "item.txt", 4096, process, NULL))
                status = panic("failed to parse csv object");
            csv_destroy(&csv);
        }
        pool_destroy(&pool);
    }

    return status;
}
