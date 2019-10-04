
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#ifndef NUL_TERM_LEN
/* Size of a NUL-termination byte. Generally useful for documenting the meaning
 * of +1 and -1 length adjustments having to do with such bytes. */
#define NUL_TERM_LEN 1 /*  sizeof('\0') */
#endif                 /* NUL_TERM_LEN */

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
  size = sizeof(StringPair);                    /* size of StringPair struct*/
  size += strlen(pair01->key) + NUL_TERM_LEN;   /* size of StringPair key */
  size += strlen(pair01->value) + NUL_TERM_LEN; /* size of StringPair value*/

  printf(
      "\n_allocate_and_fill_struct_with_string_members: %ld : %ld : %ld : %ld",
      sizeof(StringPair), strlen(pair01->key), strlen(pair01->value), size);

  printf("\n_allocate_and_fill_struct_with_string_members: %ld : %ld",
         offsetof(StringPair, key), offsetof(StringPair, value));

  StringPair *pair02 = (StringPair *)malloc(size);

  offset = sizeof(StringPair);
  pair02->key = strcpy((char *)(pair02 + offset), pair01->key);
  offset = sizeof(StringPair) + strlen(pair01->key) + NUL_TERM_LEN;
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

void test_allocate_and_fill_struct_with_array_member() {
  StringPair items[] = {
      {"key_1", "val_1"}, {"key_02", "val_02"}, {"key_003", "val_003"}};
  size_t count = sizeof(items) / sizeof(*items);
  size_t size;

  /* compute size for the target struct */
  size = sizeof(StringPairList);
  for (size_t i = 0; i < count; i++) {
    size += sizeof(StringPair);                  /* size of StringPair struct*/
    size += strlen(items[i].key) + NUL_TERM_LEN; /* size of StringPair key */
    size += strlen(items[i].value) + NUL_TERM_LEN; /* size of StringPair value*/
  }

  /* allocate and fill the target struct by pointer */
  StringPairList *dict = malloc(size);
  StringPairList *ptr = dict;
  dict->count = count;
  dict->items = (StringPair *)(ptr + offsetof(StringPairList, items));
  ptr += sizeof(StringPairList);
  for (size_t i = 0; i < count; i++) {
    ptr += sizeof(StringPair);
    dict->items[i].key = strcpy((char *)ptr, items[i].key);
    ptr += strlen(items[i].key) + NUL_TERM_LEN;
    dict->items[i].value = strcpy((char *)ptr, items[i].value);
    ptr += strlen(items[i].value) + NUL_TERM_LEN;
  }

  if (dict->count != 3) {
    fail("dict->count != 3");
  }
  if (strcmp(dict->items[0].key, "key_1")) {
    fail(dict->items[0].key);
  }
  if (strcmp(dict->items[0].value, "val_1")) {
    fail(dict->items[0].value);
  }
  if (strcmp(dict->items[1].key, "key_02")) {
    fail(dict->items[1].key);
  }
  if (strcmp(dict->items[1].value, "val_02")) {
    fail(dict->items[1].value);
  }
  if (strcmp(dict->items[2].key, "key_003")) {
    fail(dict->items[2].key);
  }
  if (strcmp(dict->items[2].value, "val_003")) {
    fail(dict->items[2].value);
  }
  free(dict);
}

int main(void) {
  test_init_struct_with_array_member();
  test_allocate_and_fill_struct_with_string_members();
  test_allocate_and_fill_struct_with_array_member();

  fprintf(stdout, "\nall tests passed");
  return 0;
}
