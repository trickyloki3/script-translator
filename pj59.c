#include "unistd.h"
#include "script.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct table table;
    struct script script;
    struct strbuf strbuf;

    struct item_node * item;

    if(chdir(argv[1])) {
        status = panic("failed to change directory");
    } else if(heap_create(&heap, 4096)) {
        status = panic("failed to create heap object");
    } else {
        if(table_create(&table, 4096, &heap)) {
            status = panic("failed to create table object");
        } else {
            if(table_item_parse(&table, "item_db.txt")) {
                status = panic("failed to item parse table object");
            } else if(table_item_combo_parse(&table, "item_combo_db.txt")) {
                status = panic("failed to item combo parse table object");
            } else if (table_skill_parse(&table, "skill_db.yml")) {
                status = panic("failed to skill parse table object");
            } else if (table_mob_parse(&table, "mob_db.txt")) {
                status = panic("failed to mob parse table object");
            } else if (table_mercenary_parse(&table, "mercenary_db.txt")) {
                status = panic("failed to mercenary parse table object");
            } else if (table_constant_parse(&table, "constant.yml")) {
                status = panic("failed to constant parse table object");
            } else if(table_constant_group_parse(&table, "constant_group.yml")) {
                status = panic("failed to constant group parse table object");
            } else if(table_argument_parse(&table, "argument.yml")) {
                status = panic("failed to argument parse table object");
            } else if(table_bonus_parse(&table, "bonus.yml")) {
                status = panic("failed to bonus parse table object");
            } else if(table_bonus2_parse(&table, "bonus2.yml")) {
                status = panic("failed to bonus2 parse table object");
            } else if(table_bonus3_parse(&table, "bonus3.yml")) {
                status = panic("failed to bonus3 parse table object");
            } else if(table_bonus4_parse(&table, "bonus4.yml")) {
                status = panic("failed to bonus4 parse table object");
            } else if(table_bonus5_parse(&table, "bonus5.yml")) {
                status = panic("failed to bonus5 parse table object");
            } else if(table_sc_start_parse(&table, "sc_start.yml")) {
                status = panic("failed to sc_start parse table object");
            } else if(table_sc_start2_parse(&table, "sc_start2.yml")) {
                status = panic("failed to sc_start2 parse table object");
            } else if(table_sc_start4_parse(&table, "sc_start4.yml")) {
                status = panic("failed to sc_start4 parse table object");
            } else if(table_statement_parse(&table, "statement.yml")) {
                status = panic("failed to statement parse table object");
            } else {
                if(script_setup(&table)) {
                    status = panic("failed to setup script object");
                } else if(script_create(&script, 4096, &heap, &table)) {
                    status = panic("failed to create script object");
                } else {
                    if(strbuf_create(&strbuf, 4096)) {
                        status = panic("failed to create strbuf object");
                    } else {
                        if(argc < 3) {
                            item = item_start(&table);
                            while(item && !status) {
                                if(script_compile(&script, item->bonus, &strbuf)) {
                                    status = panic("failed to translate script object - %ld", item->id);
                                } else {
                                    item = item_next(&table);
                                }
                            }
                        } else {
                            item = item_id(&table, strtol(argv[2], NULL, 0));
                            if(!item) {
                                status = panic("invalid item id - %s", argv[2]);
                            } else if(script_compile(&script, item->bonus, &strbuf)) {
                                status = panic("failed to translate script object - %ld", item->id);
                            }
                        }

                        undefined_print(&script.undefined);

                        strbuf_destroy(&strbuf);
                    }
                    script_destroy(&script);
                }
            }
            table_destroy(&table);
        }
        heap_destroy(&heap);
    }

    return status;
}
