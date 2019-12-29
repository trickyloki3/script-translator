#include "lookup.h"

int string_store(struct store *, struct string *, struct string **);
int string_strtol(struct string *, int, long *);

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

int string_store(struct store * store, struct string * string, struct string ** result) {
    int status = 0;

    string = store_string(store, string);
    if(!string) {
        status = panic("failed to string store object");
    } else {
        *result = string;
    }

    return status;
}

int string_strtol(struct string * string, int base, long * result) {
    int status = 0;

    long number;
    char * end = NULL;

    if(!string->length) {
        *result = 0;
    } else {
        number = strtol(string->string, &end, base);
        if(string->string + string->length != end) {
            status = panic("invalid '%s' in '%s'", end, string->string);
        } else {
            *result = number;
        }
    }

    return status;
}

int pet_db_create(struct pet_db * pet_db, size_t size, struct heap * heap) {
    int status = 0;

    if(map_create(&pet_db->map, (map_compare_cb) strcmp, heap->map_pool)) {
        status = panic("failed to create map object");
    } else {
        if(store_create(&pet_db->store, size)) {
            status = panic("failed to create store object");
        } else {
            pet_db->pet = NULL;
            pet_db->evo = NULL;
            pet_db->req = NULL;
        }
        if(status)
            map_destroy(&pet_db->map);
    }

    return status;
}

void pet_db_destroy(struct pet_db * pet_db) {
    store_destroy(&pet_db->store);
    map_destroy(&pet_db->map);
}

int pet_db_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;
    struct pet_db * pet_db = context;

    switch(mark) {
        case 5:
            if(event == start) {
                pet_db->pet = store_object(&pet_db->store, sizeof(*pet_db->pet));
                if(!pet_db->pet)
                    status = panic("failed to object store object");
            } else if(event == end) {
                if(!pet_db->pet->mob) {
                    status = panic("invalid string object");
                } else if(map_insert(&pet_db->map, pet_db->pet->mob->string, pet_db->pet)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 6: status = string_store(&pet_db->store, string, &pet_db->pet->mob); break;
        case 7: status = string_store(&pet_db->store, string, &pet_db->pet->tame_item); break;
        case 8: status = string_store(&pet_db->store, string, &pet_db->pet->egg_item); break;
        case 9: status = string_store(&pet_db->store, string, &pet_db->pet->equip_item); break;
        case 10: status = string_store(&pet_db->store, string, &pet_db->pet->food_item); break;
        case 11: status = string_strtol(string, 10, &pet_db->pet->fullness); break;
        case 12: status = string_strtol(string, 10, &pet_db->pet->hungry_delay); break;
        case 13: status = string_strtol(string, 10, &pet_db->pet->hunger_increase); break;
        case 14: status = string_strtol(string, 10, &pet_db->pet->intimacy_start); break;
        case 15: status = string_strtol(string, 10, &pet_db->pet->intimacy_fed); break;
        case 16: status = string_strtol(string, 10, &pet_db->pet->intimacy_overfed); break;
        case 17: status = string_strtol(string, 10, &pet_db->pet->intimacy_hungry); break;
        case 18: status = string_strtol(string, 10, &pet_db->pet->intimacy_owner_die); break;
        case 19: status = string_strtol(string, 10, &pet_db->pet->capture_rate); break;
        case 20: status = string_store(&pet_db->store, string, &pet_db->pet->special_performance); break;
        case 21: status = string_strtol(string, 10, &pet_db->pet->attack_rate); break;
        case 22: status = string_strtol(string, 10, &pet_db->pet->retaliate_rate); break;
        case 23: status = string_strtol(string, 10, &pet_db->pet->change_target_rate); break;
        case 24: status = string_store(&pet_db->store, string, &pet_db->pet->allow_auto_feed); break;
        case 25: status = string_store(&pet_db->store, string, &pet_db->pet->script); break;
        case 26: status = string_store(&pet_db->store, string, &pet_db->pet->support_script); break;
        case 28:
            if(event == start) {
                pet_db->evo = store_object(&pet_db->store, sizeof(*pet_db->evo));
                if(!pet_db->evo)
                    status = panic("failed to object store object");
            } else if(event == end) {
                pet_db->evo->next = pet_db->pet->evolution;
                pet_db->pet->evolution = pet_db->evo;
            }
            break;
        case 29: status = string_store(&pet_db->store, string, &pet_db->evo->target); break;
        case 31:
            if(event == start) {
                pet_db->req = store_object(&pet_db->store, sizeof(*pet_db->req));
                if(!pet_db->req)
                    status = panic("failed to object store object");
            } else if(event == end) {
                pet_db->req->next = pet_db->evo->requirement;
                pet_db->evo->requirement = pet_db->req;
            }
            break;
        case 32: status = string_store(&pet_db->store, string, &pet_db->req->item); break;
        case 33: status = string_strtol(string, 10, &pet_db->req->amount); break;
    }

    return status;
}

int lookup_create(struct lookup * lookup, size_t size, struct heap * heap) {
    int status = 0;

    if(schema_create(&lookup->schema, heap)) {
        status = panic("failed to create schema object");
    } else {
        if(parser_create(&lookup->parser, size, heap)) {
            status = panic("failed to create parser object");
        } else {
            if(pet_db_create(&lookup->pet_db, size, heap))
                status = panic("failed to create pet db object");
            if(status)
                parser_destroy(&lookup->parser);
        }
        if(status)
            schema_destroy(&lookup->schema);
    }

    return status;
}

void lookup_destroy(struct lookup * lookup) {
    pet_db_destroy(&lookup->pet_db);
    parser_destroy(&lookup->parser);
    schema_destroy(&lookup->schema);
}

int lookup_pet_db_parse(struct lookup * lookup, char * path) {
    int status = 0;
    struct schema_data * pet_db_schema;

    pet_db_schema = schema_load(&lookup->schema, pet_db_markup);
    if(!pet_db_schema) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&lookup->parser, path, pet_db_schema, pet_db_parse, &lookup->pet_db)) {
        status = panic("failed to parse parser object");
    }

    return status;
}
