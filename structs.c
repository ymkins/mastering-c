
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct {
  char *key, *value;
} StringPair;

typedef struct {
  size_t count;
  StringPair *items;
} StringPairList;

void test_init_struct_with_array_member() {
  StringPair items[] = {
      {"key_1", "val_1"}, {"key_02", "val_02"}, {"key_003", "val_003"}};

  StringPairList dict01 = {3, items};
  StringPairList dict02 = {
      3, (StringPair[]){
             {"key_1", "val_1"}, {"key_02", "val_02"}, {"key_003", "val_003"}}};

  if (strcmp(dict01.items[1].value, "val_02")) {
    fail(dict01.items[1].value);
  }
  if (strcmp(dict02.items[1].value, "val_02")) {
    fail(dict02.items[1].value);
  }
}

int main(void) {
  test_init_struct_with_array_member();

  fprintf(stdout, "\nall tests passed");
  return 0;
}
