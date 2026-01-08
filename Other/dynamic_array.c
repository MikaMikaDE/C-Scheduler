#include<stdio.h>
#include<stdlib.h>

typedef struct dynamicArray_struct {
  int   *elems;
  size_t count;
  size_t capacity;
} dynamicArray_t;

dynamicArray_t* darr_init(void) {
  dynamicArray_t *arr = malloc(sizeof *arr);
  if (!arr) return NULL;
  *arr = (dynamicArray_t){0};
  return arr;
}

void darr_realloc(dynamicArray_t *arr) {
  if  (arr->capacity == 0) arr->capacity = 256;
  else arr->capacity *= 2;
  arr->elems     = realloc(arr->elems, arr->capacity*sizeof(*arr->elems));
}

void darr_push(dynamicArray_t *arr, int elem) {
  if (arr->count >= arr->capacity) darr_realloc(arr);
  arr->elems[arr->count] = elem;
  arr->count++;
}

void darr_print(dynamicArray_t *arr) {
  putchar('[');
  for (size_t i = 0; i < arr->count; i++) {
      if (i) putchar(',');
      printf("%d", arr->elems[i]);
  }
  putchar(']');
}


int main(void){

  dynamicArray_t *arr = darr_init();
  for (int i=0; i<10; i++) darr_push(arr, i); 
  darr_print(arr);

  return 0;
}
