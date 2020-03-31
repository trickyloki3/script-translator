#include "table.h"

int long_compare(void *, void *);

struct schema_markup csv_markup[] = {
    {1, list, 0, NULL},
    {2, list, 1, NULL},
    {3, string, 2, NULL},
    {0, 0, 0},
};

struct schema_markup skill_markup[] = {
    {1, map, 0, NULL},
    {2, list, 1, "Body"},
    {3, map, 2, NULL},
    {4, list | string, 3, "ActiveInstance"},
    {5, map, 4, NULL},
    {6, string, 5, "Level"},
    {6, string, 6, "Max"},
    {4, list | string, 7, "AfterCastActDelay"},
    {5, map, 8, NULL},
    {6, string, 9, "Level"},
    {6, string, 10, "Time"},
    {4, list | string, 11, "AfterCastWalkDelay"},
    {5, map, 12, NULL},
    {6, string, 13, "Level"},
    {6, string, 14, "Time"},
    {4, string, 15, "CastCancel"},
    {4, string, 16, "CastDefenseReduction"},
    {4, map, 17, "CastDelayFlags"},
    {5, string, 18, "IgnoreDex"},
    {5, string, 19, "IgnoreItemBonus"},
    {5, string, 20, "IgnoreStatus"},
    {4, list | string, 21, "CastTime"},
    {5, map, 22, NULL},
    {6, string, 23, "Level"},
    {6, string, 24, "Time"},
    {4, map, 25, "CastTimeFlags"},
    {5, string, 26, "IgnoreDex"},
    {5, string, 27, "IgnoreItemBonus"},
    {5, string, 28, "IgnoreStatus"},
    {4, list | string, 29, "Cooldown"},
    {5, map, 30, NULL},
    {6, string, 31, "Level"},
    {6, string, 32, "Time"},
    {4, map, 33, "CopyFlags"},
    {5, map, 34, "Skill"},
    {6, string, 35, "Plagiarism"},
    {6, string, 36, "Reproduce"},
    {4, map, 37, "DamageFlags"},
    {5, string, 38, "Critical"},
    {5, string, 39, "IgnoreAtkCard"},
    {5, string, 40, "IgnoreDefCard"},
    {5, string, 41, "IgnoreDefense"},
    {5, string, 42, "IgnoreElement"},
    {5, string, 43, "IgnoreFlee"},
    {5, string, 44, "NoDamage"},
    {5, string, 45, "Splash"},
    {5, string, 46, "SplashSplit"},
    {4, string, 47, "Description"},
    {4, list | string, 48, "Duration1"},
    {5, map, 49, NULL},
    {6, string, 50, "Level"},
    {6, string, 51, "Time"},
    {4, list | string, 52, "Duration2"},
    {5, map, 53, NULL},
    {6, string, 54, "Level"},
    {6, string, 55, "Time"},
    {4, list | string, 56, "Element"},
    {5, map, 57, NULL},
    {6, string, 58, "Element"},
    {6, string, 59, "Level"},
    {4, list | string, 60, "FixedCastTime"},
    {5, map, 61, NULL},
    {6, string, 62, "Level"},
    {6, string, 63, "Time"},
    {4, map, 64, "Flags"},
    {5, string, 65, "AllowOnMado"},
    {5, string, 66, "AllowOnWarg"},
    {5, string, 67, "AllowWhenHidden"},
    {5, string, 68, "AllowWhenPerforming"},
    {5, string, 69, "AlterRangeRadius"},
    {5, string, 70, "AlterRangeResearchTrap"},
    {5, string, 71, "AlterRangeShadowJump"},
    {5, string, 72, "AlterRangeSnakeEye"},
    {5, string, 73, "AlterRangeVulture"},
    {5, string, 74, "DisableNearNpc"},
    {5, string, 75, "GuildOnly"},
    {5, string, 76, "IgnoreAutoGuard"},
    {5, string, 77, "IgnoreBgReduction"},
    {5, string, 78, "IgnoreCicada"},
    {5, string, 79, "IgnoreGvgReduction"},
    {5, string, 80, "IgnoreHovering"},
    {5, string, 81, "IgnoreKagehumi"},
    {5, string, 82, "IgnoreLandProtector"},
    {5, string, 83, "IgnoreStasis"},
    {5, string, 84, "IgnoreWugBite"},
    {5, string, 85, "IncreaseDanceWithWugDamage"},
    {5, string, 86, "IncreaseGloomyDayDamage"},
    {5, string, 87, "IsAutoShadowSpell"},
    {5, string, 88, "IsChorus"},
    {5, string, 89, "IsEnsemble"},
    {5, string, 90, "IsGuild"},
    {5, string, 91, "IsNpc"},
    {5, string, 92, "IsQuest"},
    {5, string, 93, "IsSong"},
    {5, string, 94, "IsSpirit"},
    {5, string, 95, "IsTrap"},
    {5, string, 96, "IsWedding"},
    {5, string, 97, "NoTargetEnemy"},
    {5, string, 98, "NoTargetSelf"},
    {5, string, 99, "PartyOnly"},
    {5, string, 100, "TargetEmperium"},
    {5, string, 101, "TargetHidden"},
    {5, string, 102, "TargetManHole"},
    {5, string, 103, "TargetSelf"},
    {5, string, 104, "TargetTrap"},
    {4, string, 105, "Hit"},
    {4, list | string, 106, "HitCount"},
    {5, map, 107, NULL},
    {6, string, 108, "Count"},
    {6, string, 109, "Level"},
    {4, string, 110, "Id"},
    {4, list | string, 111, "Knockback"},
    {5, map, 112, NULL},
    {6, string, 113, "Amount"},
    {6, string, 114, "Level"},
    {4, string, 115, "MaxLevel"},
    {4, string, 116, "Name"},
    {4, map, 117, "NoNearNPC"},
    {5, map, 118, "Type"},
    {6, string, 119, "WarpPortal"},
    {4, list | string, 120, "Range"},
    {5, map, 121, NULL},
    {6, string, 122, "Level"},
    {6, string, 123, "Size"},
    {4, map, 124, "Requires"},
    {5, map, 125, "Ammo"},
    {6, string, 126, "Arrow"},
    {6, string, 127, "Bullet"},
    {6, string, 128, "Cannonball"},
    {6, string, 129, "Dagger"},
    {6, string, 130, "Grenade"},
    {6, string, 131, "Kunai"},
    {6, string, 132, "Shell"},
    {6, string, 133, "Shuriken"},
    {6, string, 134, "Throwweapon"},
    {5, string, 135, "AmmoAmount"},
    {5, map, 136, "Equipment"},
    {6, string, 137, "Accelerator"},
    {6, string, 138, "Barrier_Builder"},
    {6, string, 139, "Camouflage_Generator"},
    {6, string, 140, "Cooling_Device"},
    {6, string, 141, "High_Quality_Cooler"},
    {6, string, 142, "Hovering_Booster"},
    {6, string, 143, "Mag_Field_Generator"},
    {6, string, 144, "Pilebuncker"},
    {6, string, 145, "Pilebuncker_P"},
    {6, string, 146, "Pilebuncker_S"},
    {6, string, 147, "Pilebuncker_T"},
    {6, string, 148, "Sanctified_Bullet"},
    {6, string, 149, "Shape_Shifter"},
    {6, string, 150, "Silver_Bullet"},
    {6, string, 151, "Silver_Bullet_"},
    {6, string, 152, "Special_Cooler"},
    {6, string, 153, "Suicidal_Device"},
    {5, list | string, 154, "HpCost"},
    {6, map, 155, NULL},
    {7, string, 156, "Amount"},
    {7, string, 157, "Level"},
    {5, list | string, 158, "HpRateCost"},
    {6, map, 159, NULL},
    {7, string, 160, "Amount"},
    {7, string, 161, "Level"},
    {5, list, 162, "ItemCost"},
    {6, map, 163, NULL},
    {7, string, 164, "Amount"},
    {7, string, 165, "Item"},
    {5, list | string, 166, "SpCost"},
    {6, map, 167, NULL},
    {7, string, 168, "Amount"},
    {7, string, 169, "Level"},
    {5, list | string, 170, "SpRateCost"},
    {6, map, 171, NULL},
    {7, string, 172, "Amount"},
    {7, string, 173, "Level"},
    {5, list | string, 174, "SpiritSphereCost"},
    {6, map, 175, NULL},
    {7, string, 176, "Amount"},
    {7, string, 177, "Level"},
    {5, string, 178, "State"},
    {5, map, 179, "Status"},
    {6, string, 180, "Cartboost"},
    {6, string, 181, "Explosionspirits"},
    {6, string, 182, "Hiding"},
    {6, string, 183, "Poisoningweapon"},
    {6, string, 184, "Qd_Shot_Ready"},
    {6, string, 185, "Rollingcutter"},
    {6, string, 186, "Sight"},
    {6, string, 187, "Weaponblock_On"},
    {5, map, 188, "Weapon"},
    {6, string, 189, "1hAxe"},
    {6, string, 190, "1hSpear"},
    {6, string, 191, "1hSword"},
    {6, string, 192, "2hAxe"},
    {6, string, 193, "2hMace"},
    {6, string, 194, "2hSpear"},
    {6, string, 195, "2hStaff"},
    {6, string, 196, "2hSword"},
    {6, string, 197, "Book"},
    {6, string, 198, "Bow"},
    {6, string, 199, "Dagger"},
    {6, string, 200, "Fist"},
    {6, string, 201, "Gatling"},
    {6, string, 202, "Grenade"},
    {6, string, 203, "Huuma"},
    {6, string, 204, "Katar"},
    {6, string, 205, "Knuckle"},
    {6, string, 206, "Mace"},
    {6, string, 207, "Musical"},
    {6, string, 208, "Revolver"},
    {6, string, 209, "Rifle"},
    {6, string, 210, "Shotgun"},
    {6, string, 211, "Staff"},
    {6, string, 212, "Whip"},
    {5, list | string, 213, "ZenyCost"},
    {6, map, 214, NULL},
    {7, string, 215, "Amount"},
    {7, string, 216, "Level"},
    {4, list | string, 217, "SplashArea"},
    {5, map, 218, NULL},
    {6, string, 219, "Area"},
    {6, string, 220, "Level"},
    {4, string, 221, "TargetType"},
    {4, string, 222, "Type"},
    {4, map, 223, "Unit"},
    {5, string, 224, "AlternateId"},
    {5, map, 225, "Flag"},
    {6, string, 226, "DualMode"},
    {6, string, 227, "Ensemble"},
    {6, string, 228, "HiddenTrap"},
    {6, string, 229, "NoEnemy"},
    {6, string, 230, "NoFootSet"},
    {6, string, 231, "NoKnockback"},
    {6, string, 232, "NoMob"},
    {6, string, 233, "NoOverlap"},
    {6, string, 234, "NoPc"},
    {6, string, 235, "NoReiteration"},
    {6, string, 236, "PathCheck"},
    {6, string, 237, "RangedSingleUnit"},
    {6, string, 238, "RemovedByFireRain"},
    {6, string, 239, "Skill"},
    {5, string, 240, "Id"},
    {5, string, 241, "Interval"},
    {5, list | string, 242, "Layout"},
    {6, map, 243, NULL},
    {7, string, 244, "Level"},
    {7, string, 245, "Size"},
    {5, list | string, 246, "Range"},
    {6, map, 247, NULL},
    {7, string, 248, "Level"},
    {7, string, 249, "Size"},
    {5, string, 250, "Target"},
    {2, map, 251, "Header"},
    {3, string, 252, "Type"},
    {3, string, 253, "Version"},
    {0, 0, 0}
};

struct schema_markup constant_markup[] = {
    {1, list, 0, NULL},
    {2, map, 1, NULL},
    {3, string, 2, "identifier"},
    {3, string, 3, "value"},
    {3, string, 4, "tag"},
    {3, list, 5, "range"},
    {4, map, 6, NULL},
    {5, string, 7, "min"},
    {5, string, 8, "max"},
    {0, 0, 0},
};

struct schema_markup argument_markup[] = {
    {1, list, 0, NULL},
    {2, map, 1, NULL},
    {3, string, 2, "identifier"},
    {3, string, 3, "argument"},
    {3, list, 4, "data"},
    {4, string, 5, NULL},
    {3, list, 6, "range"},
    {4, map, 7, NULL},
    {5, string, 8, "min"},
    {5, string, 9, "max"},
    {3, string, 10, "newline"},
    {0, 0, 0},
};

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int item_create(struct item * item, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&item->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&item->id, long_compare, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&item->name, (map_compare_cb) strcmp, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                map_destroy(&item->id);
        }
        if(status)
            store_destroy(&item->store);
    }

    return status;
}

void item_destroy(struct item * item) {
    map_destroy(&item->name);
    map_destroy(&item->id);
    store_destroy(&item->store);
}

int item_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    char * last;
    struct item * item = context;

    switch(mark) {
        case 1:
            if(event == start) {
                item->item = store_calloc(&item->store, sizeof(*item->item));
                if(!item->item) {
                    status = panic("failed to object store object");
                } else {
                    item->index = 0;
                }
            } else if(event == end) {
                if(item->index != 20) {
                    status = panic("invalid index");
                } else if(!item->item->name) {
                    status = panic("invalid name");
                } else if(map_insert(&item->id, &item->item->id, item->item)) {
                    status = panic("failed to insert map object");
                } else if(map_insert(&item->name, item->item->name, item->item)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 2:
            switch(item->index) {
                case 0:
                    item->item->id = strtol(string->string, &last, 10);
                    if(*last)
                        status = panic("failed to strtol string object");
                    break;
                case 2:
                    item->item->name = store_strcpy(&item->store, string->string, string->length);
                    if(!item->item->name)
                        status = panic("failed to char store object");
                    break;
                case 19:
                    if(item_script_parse(item, string->string))
                        status = panic("failed to script parse item object");
                    break;
            }
            item->index++;
            break;
    }

    return status;
}

int item_script_parse(struct item * item, char * string) {
    int status = 0;

    int curly = 0;
    size_t index = 0;
    char * anchor = NULL;

    while(*string) {
        if(*string == '{') {
            if(!curly)
                anchor = string;
            curly++;
        } else if(*string == '}') {
            curly--;
            if(!curly) {
                switch(index) {
                    case 0:
                        item->item->bonus = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->bonus)
                            status = panic("failed to char store object");
                        break;
                    case 1:
                        item->item->equip = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->equip)
                            status = panic("failed to char store object");
                        break;
                    case 2:
                        item->item->unequip = store_strcpy(&item->store, anchor, string - anchor + 1);
                        if(!item->item->unequip)
                            status = panic("failed to char store object");
                        break;
                }
                index++;

                if(status)
                    break;

                anchor = NULL;
            }
        }
        string++;
    }

    return status;
}

int skill_create(struct skill * skill, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&skill->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&skill->id, long_compare, heap->map_pool)) {
            status = panic("failed to create map object");
        } else {
            if(map_create(&skill->name, (map_compare_cb) strcmp, heap->map_pool))
                status = panic("failed to create map object");
            if(status)
                map_destroy(&skill->id);
        }
        if(status)
            store_destroy(&skill->store);
    }

    return status;
}

void skill_destroy(struct skill * skill) {
    map_destroy(&skill->name);
    map_destroy(&skill->id);
    store_destroy(&skill->store);
}

int skill_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    char * last;
    struct skill * skill = context;

    switch(mark) {
        case 2:
            if(event == start) {
                skill->skill = store_calloc(&skill->store, sizeof(*skill->skill));
                if(!skill->skill)
                    status = panic("failed to object store object");
            } else if(event == end) {
                if(!skill->skill->name) {
                    status = panic("invalid name");
                } else if(map_insert(&skill->id, &skill->skill->id, skill->skill)) {
                    status = panic("failed to insert map object");
                } else if(map_insert(&skill->name, skill->skill->name, skill->skill)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 110:
            skill->skill->id = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 116:
            skill->skill->name = store_strcpy(&skill->store, string->string, string->length);
            if(!skill->skill->name)
                status = panic("failed to char store object");
            break;
        case 47:
            skill->skill->description = store_strcpy(&skill->store, string->string, string->length);
            if(!skill->skill->description)
                status = panic("failed to char store object");
            break;
    }

    return status;
}

int constant_create(struct constant * constant, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&constant->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&constant->identifier, (map_compare_cb) strcmp, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            store_destroy(&constant->store);
    }

    return status;
}

void constant_destroy(struct constant * constant) {
    map_destroy(&constant->identifier);
    store_destroy(&constant->store);
}

int constant_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    char * last;
    struct constant * constant = context;

    switch(mark) {
        case 1:
            if(event == start) {
                constant->constant = store_calloc(&constant->store, sizeof(*constant->constant));
                if(!constant->constant)
                    status = panic("failed to object store object");
            } else if(event == end) {
                if(!constant->constant->identifier) {
                    status = panic("invalid string object");
                } else if(map_insert(&constant->identifier, constant->constant->identifier, constant->constant)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 2:
            constant->constant->identifier = store_strcpy(&constant->store, string->string, string->length);
            if(!constant->constant->identifier)
                status = panic("failed to char store object");
            break;
        case 3:
            constant->constant->value = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 4:
            constant->constant->tag = store_strcpy(&constant->store, string->string, string->length);
            if(!constant->constant->tag)
                status = panic("failed to char store object");
            break;
        case 6:
            if(event == start) {
                constant->range = store_calloc(&constant->store, sizeof(*constant->range));
                if(!constant->range)
                    status = panic("failed to object store object");
            } else if(event == end) {
                constant->range->next = constant->constant->range;
                constant->constant->range = constant->range;
            }
            break;
        case 7:
            constant->range->min = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 8:
            constant->range->max = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
    }

    return status;
}

int argument_create(struct argument * argument, size_t size, struct heap * heap) {
    int status = 0;

    if(store_create(&argument->store, size)) {
        status = panic("failed to create store object");
    } else {
        if(map_create(&argument->identifier, (map_compare_cb) strcmp, heap->map_pool))
            status = panic("failed to create map object");
        if(status)
            store_destroy(&argument->store);
    }

    return status;
}

void argument_destroy(struct argument * argument) {
    map_destroy(&argument->identifier);
    store_destroy(&argument->store);
}

int argument_parse(enum parser_event event, int mark, struct string * string, void * context) {
    int status = 0;

    struct argument * argument = context;

    char * last;
    struct data_node * root;
    struct data_node * data;

    switch(mark) {
        case 1:
            if(event == start) {
                argument->argument = store_calloc(&argument->store, sizeof(*argument->argument));
                if(!argument->argument)
                    status = panic("failed to object store object");
            } else if(event == end) {
                if(!argument->argument->identifier) {
                    status = panic("invalid string object");
                } else if(map_insert(&argument->identifier, argument->argument->identifier, argument->argument)) {
                    status = panic("failed to insert map object");
                }
            }
            break;
        case 2:
            argument->argument->identifier = store_strcpy(&argument->store, string->string, string->length);
            if(!argument->argument->identifier)
                status = panic("failed to char store object");
            break;
        case 3:
            argument->argument->argument = store_strcpy(&argument->store, string->string, string->length);
            if(!argument->argument->argument)
                status = panic("failed to char store object");
            break;
        case 4:
            if(event == end) {
                root = NULL;
                while(argument->argument->data) {
                    data = argument->argument->data;
                    argument->argument->data = argument->argument->data->next;
                    data->next = root;
                    root = data;
                }

                argument->argument->data = root;
            }
            break;
        case 5:
            data = store_malloc(&argument->store, sizeof(*data));
            if(!data) {
                status = panic("failed to object store object");
            } else {
                data->string = store_strcpy(&argument->store, string->string, string->length);
                if(!data->string) {
                    status = panic("failed to char store object");
                } else {
                    data->next = argument->argument->data;
                    argument->argument->data = data;
                }
            }
            break;
        case 7:
            if(event == start) {
                argument->range = store_calloc(&argument->store, sizeof(*argument->range));
                if(!argument->range)
                    status = panic("failed to object store object");
            } else if(event == end) {
                argument->range->next = argument->argument->range;
                argument->argument->range = argument->range;
            }
            break;
        case 8:
            argument->range->min = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 9:
            argument->range->max = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
        case 10:
            argument->argument->newline = strtol(string->string, &last, 10);
            if(*last)
                status = panic("failed to strtol string object");
            break;
    }

    return status;
}

int table_create(struct table * table, size_t size, struct heap * heap) {
    int status = 0;

    if(schema_create(&table->schema, heap)) {
        status = panic("failed to create schema object");
    } else if(parser_create(&table->parser, size, heap)) {
        status = panic("failed to create parser object");
        goto parser_fail;
    } else if(item_create(&table->item, size, heap)) {
        status = panic("failed to create item object");
        goto item_fail;
    } else if(skill_create(&table->skill, size, heap)) {
        status = panic("failed to create skill object");
        goto skill_fail;
    } else if(constant_create(&table->constant, size, heap)) {
        status = panic("failed to create constant object");
        goto constant_fail;
    } else if(argument_create(&table->argument, size, heap)) {
        status = panic("failed to create argument object");
        goto argument_fail;
    }

    return status;

argument_fail:
    constant_destroy(&table->constant);
constant_fail:
    skill_destroy(&table->skill);
skill_fail:
    item_destroy(&table->item);
item_fail:
    parser_destroy(&table->parser);
parser_fail:
    schema_destroy(&table->schema);

    return status;
}

void table_destroy(struct table * table) {
    argument_destroy(&table->argument);
    constant_destroy(&table->constant);
    skill_destroy(&table->skill);
    item_destroy(&table->item);
    parser_destroy(&table->parser);
    schema_destroy(&table->schema);
}

int table_item_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, csv_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, item_parse, &table->item, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
}

int table_skill_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, skill_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, skill_parse, &table->skill, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
}

int table_constant_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, constant_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, constant_parse, &table->constant, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
}

int table_argument_parse(struct table * table, char * path) {
    int status = 0;

    if(schema_load(&table->schema, argument_markup)) {
        status = panic("failed to load schema object");
    } else if(parser_parse(&table->parser, &table->schema, argument_parse, &table->argument, path)) {
        status = panic("failed to parse parser object");
    }

    return status;
}

struct item_node * item_start(struct table * table) {
    return map_start(&table->item.id).value;
}

struct item_node * item_next(struct table * table) {
    return map_next(&table->item.id).value;
}

struct item_node * item_id(struct table * table, long id) {
    return map_search(&table->item.id, &id);
}

struct skill_node * skill_id(struct table * table, long id) {
    return map_search(&table->skill.id, &id);
}

struct skill_node * skill_name(struct table * table, char * name) {
    return map_search(&table->skill.name, name);
}

struct constant_node * constant_identifier(struct table * table, char * identifier) {
    return map_search(&table->constant.identifier, identifier);
}

struct argument_node * argument_identifier(struct table * table, char * identifier) {
    return map_search(&table->argument.identifier, identifier);
}
