#include "misc.h"

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

int string_store(struct string * string, struct store * store, struct string ** result) {
    int status = 0;
    struct string * object;

    object = store_object(store, sizeof(*string) + string->length + 1);
    if(!object) {
        status = panic("failed to object store object");
    } else {
        object->string = (char *) object + sizeof(*string);
        object->length = string->length;

        memcpy(object->string, string->string, string->length);
        object->string[object->length] = 0;

        *result = object;
    }

    return status;
}

int string_strtol(struct string * string, int base, long * result) {
    int status = 0;

    long number;
    char * last;

    if(!string->length) {
        *result = 0;
    } else {
        number = strtol(string->string, &last, base);
        if(string->string + string->length != last) {
            status = panic("invalid '%s' in '%s'", last, string->string);
        } else {
            *result = number;
        }
    }

    return status;
}

int string_strtoul(struct string * string, int base, unsigned long * result) {
    int status = 0;

    unsigned long number;
    char * last;

    if(!string->length) {
        *result = 0;
    } else {
        number = strtoul(string->string, &last, base);
        if(string->string + string->length != last) {
            status = panic("invalid '%s' in '%s'", last, string->string);
        } else {
            *result = number;
        }
    }

    return status;
}

int string_strtol_split(struct string * string, int base, char split, struct store * store, struct long_array ** result) {
    int status = 0;
    struct long_array * array;

    char * ptr;
    size_t count;
    char * end;

    ptr = string->string;
    count = 0;
    while(ptr) {
        count++;
        ptr = strchr(ptr, split);
        if(ptr)
            ptr++;
    }

    array = store_object(store, sizeof(*array));
    if(!array) {
        status = panic("failed to object store object");
    } else {
        array->count = count;
        array->array = store_object(store, sizeof(*array->array) * array->count);
        if(!array->array) {
            status = panic("failed to object store object");
        } else {
            ptr = string->string;
            count = 0;
            while(ptr && count < array->count) {
                array->array[count++] = strtol(ptr, &end, base);
                ptr = *end == split ? end + 1 : NULL;
                if(!ptr && *end)
                    status = panic("invalid string '%s' in '%s'", end, string->string);
            }

            if(status) {

            } else {
                *result = array;
            }
        }
    }

    return status;
}

int string_strtol_splitv(struct string * string, int base, int split, ...) {
    int status = 0;
    va_list args;
    long * value;
    char * ptr;
    char * end;

    va_start(args, split);
    value = va_arg(args, long *);
    ptr = string->string;
    while(value && ptr) {
        *value = strtol(ptr, &end, base);
        ptr = *end == split ? end + 1 : NULL;
        if(!ptr && *end) {
            status = panic("invalid string '%s' in '%s'", end, string->string);
        } else {
            value = va_arg(args, long *);
        }
    }
    va_end(args);

    return status;
}
