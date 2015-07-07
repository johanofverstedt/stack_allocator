
#include <stdlib.h>
#include <stdio.h>
#include "sa.h"

void debug_printouts(void* arena) {
  printf("--- META ---\n");

  sa_stack_debug_print_meta(arena);

  //printf("--- FULL ---\n");

  //sa_stack_debug_print_all(arena);

  printf("\n");  
}

void debug_print_pointer(void* ptr) {
  printf("%x\n", (unsigned int)ptr);
}

int main(int argc, char** argv) {
  sa_stack_print_stuff();

  void* arena = malloc(1024U);

  sa_stack_init(arena, 1024U);

  debug_printouts(arena);


  void* first_allocation = sa_stack_alloc(arena, 32);

  void* second_allocation = sa_stack_alloc(arena, 12);

  debug_print_pointer(first_allocation);
  debug_print_pointer(second_allocation);
  
  debug_printouts(arena);

  sa_stack_free(arena, second_allocation);

  debug_printouts(arena);

  second_allocation = sa_stack_alloc(arena, 12);

  debug_print_pointer(second_allocation);

  debug_printouts(arena);

  sa_stack_free(arena, first_allocation);

  debug_printouts(arena);

  sa_stack_free(arena, second_allocation);

  debug_printouts(arena);

  printf("\n*** REALLOC TESTS ***\n\n");

  // test realloc

  first_allocation = sa_stack_alloc(arena, 32);

  second_allocation = sa_stack_alloc(arena, 16);

  debug_printouts(arena);

  // realloc the last allocation

  // grow the chunk

  sa_stack_realloc(arena, second_allocation, 64);

  debug_printouts(arena);

  // shrink the chunk

  sa_stack_realloc(arena, second_allocation, 32);

  debug_printouts(arena);

  // alloc a third allocation from scratch using realloc

  void* third_allocation = sa_stack_realloc(arena, 0, 128);

  debug_printouts(arena);

  sa_stack_free(arena, third_allocation);

  debug_printouts(arena);

  return 0;
}
