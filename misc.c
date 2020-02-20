#include "misc.h"

int long_compare(void * x, void * y) {
    long l = *((long *) x);
    long r = *((long *) y);
    return l < r ? -1 : l > r ? 1 : 0;
}

struct long_array * long_array_store(struct store * store, size_t count) {
    int status = 0;
    struct long_array * array;

    array = store_object(store, sizeof(*array));
    if(!array) {
        status = panic("failed to object store object");
    } else {
        array->count = count;
        array->array = store_object(store, sizeof(*array->array) * array->count);
        if(!array->array)
            status = panic("failed to object store object");
    }

    return status ? NULL : array;
}

int script_store(struct string * string, struct store * store, struct string ** result) {
    int status = 0;
    struct string * object;

    object = store_object(store, sizeof(*object));
    if(!object) {
        status = panic("failed to object store object");
    } else {
        /*
         * the flex scanner requires  the
         * last two characters to be zero
         */
        object->length = string->length + 2;
        object->string = store_object(store, sizeof(*string->string) * (string->length + 2));
        if(!object->string) {
            status = panic("failed to object store object");
        } else {
            memcpy(object->string, string->string, string->length);
            object->string[string->length] = 0;
            object->string[string->length + 1] = 0;

            *result = object;
        }
    }

    return status;
}

int string_store(struct string * string, struct store * store, struct string ** result) {
    int status = 0;
    struct string * object;

    object = store_object(store, sizeof(*object));
    if(!object) {
        status = panic("failed to object store object");
    } else {
        object->length = string->length;
        object->string = store_object(store, sizeof(*string->string) * (string->length + 1));
        if(!object->string) {
            status = panic("failed to object store object");
        } else {
            memcpy(object->string, string->string, string->length);
            object->string[object->length] = 0;

            *result = object;
        }
    }

    return status;
}

int string_strtol(struct string * string, long * result) {
    int status = 0;

    long value;
    char * end;

    value = strtol(string->string, &end, 0);
    if(*end) {
        status = panic("invalid '%s' in '%s'", end, string->string);
    } else {
        *result = value;
    }

    return status;
}

int string_strtol_split(struct string * string, char split, struct store * store, struct long_array ** result) {
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

    array = long_array_store(store, count);
    if(!array) {
        status = panic("failed to create long array object");
    } else {
        ptr = string->string;
        count = 0;
        while(ptr && count < array->count) {
            array->array[count++] = strtol(ptr, &end, 0);
            ptr = *end == split ? end + 1 : NULL;
            if(!ptr && *end)
                status = panic("invalid string '%s' in '%s'", end, string->string);
        }
        if(status) {
            *result = NULL;
        } else {
            *result = array;
        }
    }

    return status;
}

int string_strtol_splitv(struct string * string, int split, ...) {
    int status = 0;
    va_list args;
    long * value;
    char * ptr;
    char * end;

    va_start(args, split);
    value = va_arg(args, long *);
    ptr = string->string;
    while(value && ptr) {
        *value = strtol(ptr, &end, 0);
        value = va_arg(args, long *);
        ptr = *end == split ? end + 1 : NULL;
        if(!ptr && *end)
            status = panic("invalid string '%s' in '%s'", end, string->string);
    }
    va_end(args);

    return status;
}
