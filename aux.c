#include "aux.h"

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int string_compare(void * x, void * y) {
    return strcmp(x, y);
}

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
