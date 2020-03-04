#include "unistd.h"
#include "table.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct table table;

    struct item_node * item;

    if(chdir(argv[1])) {
        status = panic("failed to change directory");
    } else if(heap_create(&heap, 4096)) {
        status = panic("failed to create heap object");
    } else {
        if(table_create(&table, 65536, &heap)) {
            status = panic("failed to create table object");
        } else {
            if(table_item_parse(&table, "item_db.txt")) {
                status = panic("failed to item parse table object");
            } else {
                if(argc < 3) {
                    item = item_start(&table);
                    while(item && !status) {

                        item = item_next(&table);
                    }
                } else {
                    item = item_id(&table, strtol(argv[2], NULL, 0));
                    if(!item)
                        status = panic("invalid item id - %s", argv[2]);
                }
            }
            table_destroy(&table);
        }
        heap_destroy(&heap);
    }

    return status;
}
