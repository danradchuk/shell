#include "slice.h"
#include <_string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void init_slice(Slice *slice, uint32_t cap) {
  slice->data = (char **)malloc(cap * sizeof(char *));
  if (!slice->data) {
    fprintf(stderr, "Fatal: failed to allocate %zu bytes.\n",
            cap * sizeof(char *));
    abort();
    return;
  }

  slice->len = 0;
  slice->cap = cap;
}

void append_slice(Slice *slice, const char *elem) {
  if (slice == NULL) {
    return;
  }

  if (slice->len == slice->cap) { // resize
    slice->cap *= 2;
    slice->data = realloc(slice->data, slice->cap * sizeof(const char *));
  }

  slice->data[slice->len++] = strdup(elem);
}

void append_null(Slice *slice) {
  if (slice == NULL) {
    return;
  }

  if (slice->len == slice->cap) { // resize
    slice->cap *= 2;
    slice->data = realloc(slice->data, slice->cap * sizeof(const char *));
  }

  slice->data[slice->len++] = NULL;
}

char *get_slice_elem(Slice *slice, uint32_t idx) {
  if (slice == NULL) {
    return NULL;
  }

  return slice->data[idx];
}

void dispose_slice(Slice *slice) {
  if (slice == NULL) {
    return;
  }

  // free elements
  for (size_t i = 0; i < slice->len; i++) {
    if (slice->data[i]) {
      free(slice->data[i]);
    }
  }

  // free slice
  free(slice->data);

  slice->data = NULL;
  slice->len = 0;
  slice->cap = 0;
}

void print_slice(Slice *slice) {
  if (slice == NULL || slice->data == NULL) {
    return;
  }

  for (size_t i = 0; i < slice->len; i++) {
    char *s = slice->data[i];
    if (s) {
      printf("%s\n", s);
    }
  }
}
