#include "heap.h"

int main(int argc, char ** argv) {
    int status = 0;
    struct heap heap;

    if(heap_create(&heap, 16384, 4096, 8)) {
        status = panic("failed to create heap object");
    } else {
        heap_destroy(&heap);
    }

    return status;
}
