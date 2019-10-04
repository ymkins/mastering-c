
#include <jansson.h>
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

/* https://stackoverflow.com/a/10966395 */
#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define FOREACH_CARD_TYPE(CARD_TYPE) \
  CARD_TYPE(CARD_TYPE_A)             \
  CARD_TYPE(CARD_TYPE_B)             \
  CARD_TYPE(CARD_TYPE_C)

static const char *CARD_TYPE_STRING[] = {FOREACH_CARD_TYPE(GENERATE_STRING)};

typedef enum { FOREACH_CARD_TYPE(GENERATE_ENUM) } CARD_TYPE_ENUM;

typedef struct {
  char *key, *value;
} StringPair;

typedef struct {
  size_t count;
  StringPair *items;
} StringPairList;

typedef struct {
  char *name;          /* json:"name" */
  CARD_TYPE_ENUM type; /* json:"type" */
  StringPairList dict; /* json:"dict,omitempty" */
} Card;

int indexOf(const char **arr, size_t len, const char *target) {
  for (size_t i = 0; i < len; i++) {
    if (strncmp(arr[i], target, strlen(target)) == 0) {
      return i;
    }
  }
  return -1;
}

/* Returns new reference or NULL on error.
 The return value must be freed by the caller using free(). */
Card *decodeCard(const char *str) {
  /* https://stackoverflow.com/questions/2043871/how-can-i-get-set-a-struct-member-by-offset
   * https://en.wikipedia.org/wiki/Offsetof */

  Card *card = NULL;
  size_t size = 0;
  json_error_t error;
  json_t *json, *json_name, *json_type, *json_dict;
  json = json_name = json_type = json_dict = NULL;

  json = json_loads(str, 0, &error);
  if (json) {
    json_name = json_object_get(json, "name");
    json_type = json_object_get(json, "type");
    json_dict = json_object_get(json, "dict");

    /* compute size for the target struct */
    size = sizeof(Card);
    size += json_string_length(json_name) + NUL_TERM_LEN;
    if (json_dict) {
      const char *key;
      json_t *value;
      size += sizeof(StringPairList);
      json_object_foreach(json_dict, key, value) {
        size += sizeof(StringPair);
        size += strlen(key) + NUL_TERM_LEN;
        size += json_string_length(value) + NUL_TERM_LEN;
      }
    }

    /* allocate and fill the target struct by pointer */
    card = (Card *)malloc(size);
    Card *ptr = card;

    ptr += sizeof(Card);
    card->name = strcpy((char *)ptr, json_string_value(json_name));
    ptr += json_string_length(json_name) + NUL_TERM_LEN;
    card->type = indexOf(CARD_TYPE_STRING,
                         sizeof(CARD_TYPE_STRING) / sizeof(*CARD_TYPE_STRING),
                         json_string_value(json_type));
    if (json_dict) {
      card->dict.count = json_object_size(json_dict);
      card->dict.items = (StringPair *)(ptr + offsetof(StringPairList, items));
      ptr += sizeof(StringPairList);

      const char *key;
      json_t *value;
      size_t i = 0;
      json_object_foreach(json_dict, key, value) {
        ptr += sizeof(StringPair);
        card->dict.items[i].key = strcpy((char *)ptr, key);
        ptr += strlen(key) + NUL_TERM_LEN;
        card->dict.items[i].value =
            strcpy((char *)ptr, json_string_value(value));
        ptr += json_string_length(value) + NUL_TERM_LEN;
        i++;
      }
      json_decref(value);
    }
  } else {
    fprintf(stderr, "decodeCard error: %d: %s\n", error.line, error.text);
  }

  json_decref(json);
  json_decref(json_name);
  json_decref(json_type);
  json_decref(json_dict);
  return card;
}

/* Returns the JSON representation of json as a string, or NULL on error.
The return value must be freed by the caller using free().
flags is described in `char *json_dumps(const json_t *json, size_t flags)`
The JSON_SORT_KEYS is used by default. */
char *encodeCard(Card *card, size_t flags) {
  char *result;
  json_t *json = json_object();
  json_t *json_dict = json_object();

  json_object_set_new(json, "name", json_string(card->name));
  json_object_set_new(json, "type", json_string(CARD_TYPE_STRING[card->type]));
  if (card->dict.count) {
    json_object_set_new(json, "dict", json_dict);

    for (size_t i = 0; i < card->dict.count; i++) {
      json_object_set_new(json_dict, card->dict.items[i].key,
                          json_string(card->dict.items[i].value));
    }
  }

  if (!flags) {
    flags = JSON_SORT_KEYS;
  }
  result = json_dumps(json, flags);
  json_decref(json);
  json_decref(json_dict);
  return result;
}

void test_decodeCard() {
  char *card_str01 = "{\"name\": \"name-01\", \"type\": \"CARD_TYPE_A\"}";
  char *card_str02 =
      "{\"dict\": {\"key_003\": \"val_003\", \"key_02\": \"val_02\", "
      "\"key_1\": \"val_1\"}, \"name\": \"Card Name 2\", \"type\": "
      "\"CARD_TYPE_B\"}";

  Card *card = decodeCard(card_str01);

  if (!card) {
    fail("!card");
  };
  if (strcmp(card->name, "name-01")) {
    fail(card->name);
  }
  if (card->type != CARD_TYPE_A) {
    fail("card->type != CARD_TYPE_A");
  }

  free(card);
  card = decodeCard(card_str02);

  if (!card) {
    fail("!card");
  };
  if (card->dict.count != 3) {
    fail("card->dict.count");
  }
  if (strcmp(card->dict.items[0].key, "key_003")) {
    fail(card->dict.items[0].key);
  }
  if (strcmp(card->dict.items[1].key, "key_02")) {
    fail(card->dict.items[1].key);
  }
  if (strcmp(card->dict.items[2].key, "key_1")) {
    fail(card->dict.items[2].key);
  }
  if (strcmp(card->dict.items[0].value, "val_003")) {
    fail(card->dict.items[0].value);
  }
  if (strcmp(card->dict.items[1].value, "val_02")) {
    fail(card->dict.items[1].value);
  }
  if (strcmp(card->dict.items[2].value, "val_1")) {
    fail(card->dict.items[2].value);
  }
  if (strcmp(card->name, "Card Name 2")) {
    fail(card->name);
  }
  if (card->type != CARD_TYPE_B) {
    fail("card->type != CARD_TYPE_B");
  }

  free(card);
}

void test_encodeCard() {
  Card card01 = {"name-01", CARD_TYPE_A};

  StringPair items[] = {
      {"key_1", "val_1"}, {"key_02", "val_02"}, {"key_003", "val_003"}};
  size_t count = sizeof(items) / sizeof(*items);
  Card card02 = {"Card Name 2", CARD_TYPE_B, {count, items}};

  char *result = NULL;
  result = encodeCard(&card01, 0);
  if (!result ||
      strcmp(result, "{\"name\": \"name-01\", \"type\": \"CARD_TYPE_A\"}")) {
    fail(result);
  }

  free(result);
  result = encodeCard(&card02, 0);

  char *expected =
      "{\"dict\": {\"key_003\": \"val_003\", \"key_02\": \"val_02\", "
      "\"key_1\": \"val_1\"}, \"name\": \"Card Name 2\", \"type\": "
      "\"CARD_TYPE_B\"}";

  //   printf("\n_encodeCard: %s", result);
  if (!result || strcmp(result, expected)) {
    fail(result);
  }

  free(result);
}

int main(void) {
  test_encodeCard();
  test_decodeCard();

  fprintf(stdout, "\nall tests passed");
  return 0;
}
