#include "slice.h"
#include <assert.h>
#include <string.h>

int main(int argc, char *argv[]) {
  Slice slice;

  init_slice(&slice, 8);

  char *str0 = "Hello";
  char *str1 = ", World!";
  append_slice(&slice, str0);
  append_slice(&slice, str1);

  char *str = get_slice_elem(&slice, 0);
  assert(strcmp(str, str0) == 0);

  str = get_slice_elem(&slice, 1);
  assert(strcmp(str, str1) == 0);

  dispose_slice(&slice);

  return 0;
}
