#include <stdlib.h>
#include <stddef.h>


#define STARTING_CAPACITY 256


//macro for a generic dynamically allocated array
//this is used for both readyList and blockedList
//Author: Mika Kline-Pearson

#define DYNAMIC_ARRAY(name, type)                                 \
typedef struct {                                                  \
  type   *elems;                                                  \
  size_t  count;                                                  \
  size_t  capacity;                                               \
} name##_t;                                                       \
                                                                  \
/*Initializes the array, */                                       \
static name##_t* name##_init(void) {                              \
  name##_t *arr = malloc(sizeof *arr);                            \
  if (!arr) return NULL;                                          \
  arr->count    = 0;                                              \
  arr->capacity = STARTING_CAPACITY;                              \
  arr->elems    = malloc(arr->capacity * sizeof(type));           \
  if (!arr->elems) { free(arr); return NULL; }                    \
  return arr;                                                     \
}                                                                 \
                                                                  \
/*Reallocates space for the array, used in _push*/                \
static Boolean name##_realloc(name##_t *arr) {                    \
  arr->capacity *= 2;                                             \
  size_t reallocSize = arr->capacity * sizeof(type);              \
  type *tmp = realloc(arr->elems, reallocSize);                   \
  if  (!tmp) return FALSE;                                        \
  arr->elems = tmp;                                               \
  return TRUE;                                                    \
}                                                                 \
                                                                  \
/*Adds a new element to the end of the array*/                    \
static Boolean name##_push(name##_t *arr, type elem) {            \
  if (arr->count >= arr->capacity) {                              \
      /*Not enough space - need to realloc*/                      \
      if (!name##_realloc(arr)) return FALSE;                     \
  }                                                               \
  arr->elems[arr->count++] = elem;                                \
  return TRUE;                                                    \
}                                                                 \
                                                                  \
                                                                  \
                                                                  \
/*Removes element with a given pid*/                              \
/* this does assume that the type has a pid value*/               \
/* (which both readyList and blockedList do)*/                    \
static Boolean name##_remove_by_pid(name##_t *arr, pid_t pid) {   \
  size_t idx = 0;                                                 \
  size_t count = arr->count;                                      \
  while (idx<count && arr->elems[idx].pid != pid) idx++;          \
  /*elem didnt exist*/                                            \
  if (idx == count) return FALSE;                                 \
  /* shift left to fill the gap */                                \
  for (size_t i = idx + 1; i < arr->count; i++) {                 \
    arr->elems[i - 1] = arr->elems[i];                            \
  }                                                               \
  arr->count--;                                                   \
  return TRUE;                                                    \
}                                                                 \
                                                                  \
static Boolean name##_isEmpty(name##_t *arr) {                    \
  return arr->count == 0;                                         \
}                                                                 \
                                                                  \
static void name##_free(name##_t *arr) {                          \
  if (!arr) return;                                               \
  free(arr->elems);                                               \
  free(arr);                                                      \
}
 
// assumes element type has fields: pid, IOready

