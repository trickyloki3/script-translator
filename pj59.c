#include "array.h"
#include "pool.h"
#include "map.h"
#include "list.h"
#include "sector.h"
#include "pool_map.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct pool_map pool_map;

    if(pool_map_create(&pool_map, 64)) {
        status = panic("failed to create map pool object");
    } else {
        /* object list */
        pool_map_create_pool(&pool_map, sizeof(struct array), 16);
        pool_map_create_pool(&pool_map, sizeof(struct stack), 16);
        pool_map_create_pool(&pool_map, sizeof(struct string), 16);
        pool_map_create_pool(&pool_map, sizeof(struct pool), 16);
        pool_map_create_pool(&pool_map, sizeof(struct map), 16);
        pool_map_create_pool(&pool_map, sizeof(struct list), 16);
        pool_map_create_pool(&pool_map, sizeof(struct sector), 16);

        /* object node list */
        pool_map_create_pool(&pool_map, sizeof(struct pool_node), 64);
        pool_map_create_pool(&pool_map, sizeof(struct map_node), 64);
        pool_map_create_pool(&pool_map, sizeof(struct list_node), 64);
        pool_map_create_pool(&pool_map, sizeof(struct sector_node), 64);

        pool_map_destroy(&pool_map);
    }

    return status;
}
