#include "csv.h"
#include "json.h"
#include "yaml.h"
#include "db.h"
#include "data.h"

int load_test(struct pool_map *, struct sector_list *);
int yaml_test(struct pool_map *, struct sector_list *);

int main(int argc, char ** argv) {
    int status = 0;
    struct pool list_node_pool;
    struct pool map_node_pool;
    struct pool sector_node_pool;
    struct pool_map pool_map;
    struct sector_list sector_list;

    if(argc < 2) {
        status = panic("missing working directory argument");
    } else if(chdir(argv[1])) {
        status = panic("failed to change directory - %s", argv[1]);
    } else if(pool_create(&list_node_pool, sizeof(struct list_node), 64)) {
        status = panic("failed to create pool object");
    } else {
        if(pool_create(&map_node_pool, sizeof(struct map_node), 64)) {
            status = panic("failed to create pool object");
        } else {
            if(pool_create(&sector_node_pool, sizeof(struct sector_node), 64)) {
                status = panic("failed to create pool object");
            } else {
                if(pool_map_create(&pool_map, 524288, &list_node_pool, &map_node_pool)) {
                    status = panic("failed to create pool map object");
                } else {
                    if(sector_list_create(&sector_list, 524288, &sector_node_pool, &list_node_pool)) {
                        status = panic("failed to create sector list object");
                    } else {
                        status = yaml_test(&pool_map, &sector_list);

                        sector_list_destroy(&sector_list);
                    }
                    pool_map_destroy(&pool_map);
                }
                pool_destroy(&sector_node_pool);
            }
            pool_destroy(&map_node_pool);
        }
        pool_destroy(&list_node_pool);
    }

    return status;
}

int load_test(struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;
    struct csv csv;
    struct json json;
    struct db db;
    struct data data;

    if(csv_create(&csv, 524288, pool_map_get(pool_map, sizeof(struct list_node)))) {
        status = panic("failed to create csv object");
    } else {
        if(json_create(&json, pool_map, sector_list)) {
            status = panic("failed to create json object");
        } else {
            if(db_create(&db, pool_map, sector_list, &csv, &json)) {
                status = panic("failed to create db object");
            } else {
                if(data_create(&data, pool_map, sector_list, &json)) {
                    status = panic("failed to create data object");
                } else {
                    data_destroy(&data);
                }
                db_destroy(&db);
            }
            json_destroy(&json);
        }
        csv_destroy(&csv);
    }

    return status;
}

int yaml_test(struct pool_map * pool_map, struct sector_list * sector_list) {
    int status = 0;
    struct yaml yaml;

    if(yaml_create(&yaml, pool_map, sector_list)) {
        status = panic("failed to create yaml object");
    } else {
        if(yaml_parse(&yaml, "yaml.yml"))
            status = panic("failed to parse yaml object");
        yaml_destroy(&yaml);
    }

    return status;
}
