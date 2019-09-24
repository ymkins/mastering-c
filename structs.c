
#include <stdbool.h>
#include <stddef.h>
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

void test_allocate_and_fill_struct_with_string_members() {
  /* https://stackoverflow.com/questions/2043871/how-can-i-get-set-a-struct-member-by-offset
   */
  StringPair pair = {"key_1", "val_1"};
  StringPair *pair01 = &pair;

  size_t size, offset;
  size = sizeof(StringPair)           /* size of StringPair struct*/
         + strlen(pair01->key) + 1    /* size of StringPair key */
         + strlen(pair01->value) + 1; /* size of StringPair value*/

  printf(
      "\n_allocate_and_fill_struct_with_string_members: %ld : %ld : %ld : %ld",
      sizeof(StringPair), strlen(pair01->key), strlen(pair01->value), size);

  printf("\n_allocate_and_fill_struct_with_string_members: %ld : %ld",
         offsetof(StringPair, key), offsetof(StringPair, value));

  StringPair *pair02 = (StringPair *)malloc(size);

  offset = sizeof(StringPair);
  pair02->key = strcpy((char *)(pair02 + offset), pair01->key);
  offset = sizeof(StringPair) + strlen(pair01->key) + 1;
  pair02->value = strcpy((char *)(pair02 + offset), pair01->value);
  printf("\n_allocate_and_fill_struct_with_string_members: %s : %s",
         pair02->key, pair02->value);

  if (strcmp(pair02->key, "key_1")) {
    fail(pair02->key);
  }
  if (strcmp(pair02->value, "val_1")) {
    fail(pair02->value);
  }
  free(pair02);
}

int main(void) {
  test_init_struct_with_array_member();
  test_allocate_and_fill_struct_with_string_members();

  fprintf(stdout, "\nall tests passed");
  return 0;
}
