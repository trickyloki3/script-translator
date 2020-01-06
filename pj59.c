#include "lookup.h"
#include "unistd.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct lookup lookup;

    struct item * item;

    if(chdir(argv[1])) {
        status = panic("failed to change directory");
    } else {
        if(heap_create(&heap, 4096)) {
            status = panic("failed to create heap object");
        } else {
            if(lookup_create(&lookup, 65536, &heap)) {
                status = panic("failed to create lookup object");
            } else {
                if(lookup_pet_db_parse(&lookup, "pet_db.yml")) {
                    status = panic("failed to pet db lookup object");
                } else if(lookup_item_db_parse(&lookup, "item_db.txt")) {
                    status = panic("failed to item db lookup object");
                } else if(lookup_item_combo_db_parse(&lookup, "item_combo_db.txt")) {
                    status = panic("failed to item combo db lookup object");
                } else if(lookup_skill_db_parse(&lookup, "skill_db.txt")) {
                    status = panic("failed to skill db lookup object");
                } else if(lookup_mob_db_parse(&lookup, "mob_db.txt")) {
                    status = panic("failed to mob db lookup object");
                } else if(lookup_mob_race_db_parse(&lookup, "mob_race2_db.txt")) {
                    status = panic("failed to mob race db lookup object");
                } else if(lookup_mercenary_db_parse(&lookup, "mercenary_db.txt")) {
                    status = panic("failed to mercenary db lookup object");
                } else if(lookup_produce_db_parse(&lookup, "produce_db.txt")) {
                    status = panic("failed to produce db lookup object");
                } else if(lookup_constant_db_parse(&lookup, "constant_db.yml")) {
                    status = panic("failed to constant db lookup object");
                } else if(lookup_constant_group_parse(&lookup, "constant_group.yml")) {
                    status = panic("failed to constant group lookup object");
                } else if(lookup_data_group_parse(&lookup, "data_group.yml")) {
                    status = panic("failed to data group lookup object");
                } else if(lookup_prototype_group_parse(&lookup, "prototype_group.yml")) {
                    status = panic("failed to prototype group lookup object");
                } else {
                    if(argc > 2) {
                        item = lookup_item_db_by_id(&lookup, strtol(argv[2], NULL, 10));
                        if(!item) {
                            status = panic("failed to item db by id lookup object");
                        } else {

                        }
                    } else {
                        item = lookup_item_db_start(&lookup);
                        while(item) {

                            item = lookup_item_db_next(&lookup);
                        }
                    }
                }
                lookup_destroy(&lookup);
            }
            heap_destroy(&heap);
        }
    }

    return status;
}
