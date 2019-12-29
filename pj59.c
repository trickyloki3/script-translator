#include "db.h"

struct schema_markup pet_db_markup[] = {
    {1, map, 0, NULL},
    {2, map, 1, "Header"},
    {3, string, 2, "Type"},
    {3, string, 3, "Version"},
    {2, list, 4, "Body"},
    {3, map, 5, NULL},
    {4, string, 6, "Mob"},
    {4, string, 7, "TameItem"},
    {4, string, 8, "EggItem"},
    {4, string, 9, "EquipItem"},
    {4, string, 10, "FoodItem"},
    {4, string, 11, "Fullness"},
    {4, string, 12, "HungryDelay"},
    {4, string, 13, "HungerIncrease"},
    {4, string, 14, "IntimacyStart"},
    {4, string, 15, "IntimacyFed"},
    {4, string, 16, "IntimacyOverfed"},
    {4, string, 17, "IntimacyHungry"},
    {4, string, 18, "IntimacyOwnerDie"},
    {4, string, 19, "CaptureRate"},
    {4, string, 20, "SpecialPerformance"},
    {4, string, 21, "AttackRate"},
    {4, string, 22, "RetaliateRate"},
    {4, string, 23, "ChangeTargetRate"},
    {4, string, 24, "AllowAutoFeed"},
    {4, string, 25, "Script"},
    {4, string, 26, "SupportScript"},
    {4, list, 27, "Evolution"},
    {5, map, 28, NULL},
    {6, string, 29, "Target"},
    {6, list, 30, "ItemRequirements"},
    {7, map, 31, NULL},
    {8, string, 32, "Item"},
    {8, string, 33, "Amount"},
    {0, 0, 0},
};

int pet_db_parse(enum parser_event event, int mark, struct string * string, void * context) {
    switch(mark) {
        case 1:  event == start ? fprintf(stdout, "Start Header\n") : fprintf(stdout, "End Header\n"); break;
        case 2:  fprintf(stdout, "Type: %s\n", string->string); break;
        case 3:  fprintf(stdout, "Version: %s\n", string->string); break;
        case 4:  break;
        case 5:  event == start ? fprintf(stdout, "Start Body\n") : fprintf(stdout, "End Body\n"); break;
        case 6:  fprintf(stdout, "Mob: %s\n", string->string); break;
        case 7:  fprintf(stdout, "TameItem: %s\n", string->string); break;
        case 8:  fprintf(stdout, "EggItem: %s\n", string->string); break;
        case 9:  fprintf(stdout, "EquipItem: %s\n", string->string); break;
        case 10: fprintf(stdout, "FoodItem: %s\n", string->string); break;
        case 11: fprintf(stdout, "Fullness: %s\n", string->string); break;
        case 12: fprintf(stdout, "HungryDelay: %s\n", string->string); break;
        case 13: fprintf(stdout, "HungerIncrease: %s\n", string->string); break;
        case 14: fprintf(stdout, "IntimacyStart: %s\n", string->string); break;
        case 15: fprintf(stdout, "IntimacyFed: %s\n", string->string); break;
        case 16: fprintf(stdout, "IntimacyOverfed: %s\n", string->string); break;
        case 17: fprintf(stdout, "IntimacyHungry: %s\n", string->string); break;
        case 18: fprintf(stdout, "IntimacyOwnerDie: %s\n", string->string); break;
        case 19: fprintf(stdout, "CaptureRate: %s\n", string->string); break;
        case 20: fprintf(stdout, "SpecialPerformance: %s\n", string->string); break;
        case 21: fprintf(stdout, "AttackRate: %s\n", string->string); break;
        case 22: fprintf(stdout, "RetaliateRate: %s\n", string->string); break;
        case 23: fprintf(stdout, "ChangeTargetRate: %s\n", string->string); break;
        case 24: fprintf(stdout, "AllowAutoFeed: %s\n", string->string); break;
        case 25: fprintf(stdout, "Script: %s\n", string->string); break;
        case 26: fprintf(stdout, "SupportScript: %s\n", string->string); break;
        case 27: break;
        case 28: event == start ? fprintf(stdout, "Start Evolution\n") : fprintf(stdout, "End Evolution\n"); break;
        case 29: fprintf(stdout, "Target: %s\n", string->string); break;
        case 30: break;
        case 31: event == start ? fprintf(stdout, "Start item Requirements\n") : fprintf(stdout, "End item Requirements\n"); break;
        case 32: fprintf(stdout, "Item: %s\n", string->string); break;
        case 33: fprintf(stdout, "Amount: %s\n", string->string); break;
    }
    return 0;
}

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;
    struct schema schema;
    struct parser parser;
    struct schema_data * pet_db_schema;

    if(heap_create(&heap, 4096)) {
        status = panic("failed to create heap object");
    } else {
        if(schema_create(&schema, &heap)) {
            status = panic("failed to markup schema object");
        } else {
            pet_db_schema = schema_load(&schema, pet_db_markup);
            if(!pet_db_schema) {
                status = panic("failed to load schema object");
            } else if(parser_create(&parser, 4096, &heap)) {
                status = panic("failed to create parser object");
            } else {
                if(parser_parse(&parser, "pet_db.yml", pet_db_schema, pet_db_parse, NULL))
                    status = panic("failed to parse parser object");
                parser_destroy(&parser);
            }
            schema_destroy(&schema);
        }
        heap_destroy(&heap);
    }

    return status;
}
