#ifndef SLICE_H 
#define SLICE_H 

#include <stdint.h>

typedef struct {
    char** data;
    uint32_t len;
    uint32_t cap;
} Slice;

void init_slice(Slice* slice, uint32_t cap);
void append_slice(Slice* slice, const char* elem);
void append_null(Slice* slice);
char* get_slice_elem(Slice* slice, uint32_t idx);
void dispose_slice(Slice* slice);
void print_slice(Slice* slice);

#endif  // SLICE_H
