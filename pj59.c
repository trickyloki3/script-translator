#include "csv.h"

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
            if(csv_parse(&csv, "item.txt", 4096))
                status = panic("failed to parse csv object");
            csv_destroy(&csv);
        }
        pool_destroy(&pool);
    }

    return status;
}
