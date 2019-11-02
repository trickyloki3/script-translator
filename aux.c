#include "aux.h"

int sstring_create(sstring * result, char * string, size_t length, struct sector_list * sector_list) {
    int status = 0;
    sstring object;

    object = sector_list_malloc(sector_list, length + 1);
    if(!object) {
        status = panic("out of memory");
    } else {
        object[length] = 0;
        *result = memcpy(object, string, length);
    }

    return status;
}

int sstring_create2(sstring * result, struct string * string, struct sector_list * sector_list) {
    return sstring_create(result, string->string, string->offset, sector_list);
}

void sstring_destroy(sstring string) {
    sector_list_free(string);
}

size_t sstring_size(sstring string) {
    return sector_size(string) - 1;
}
