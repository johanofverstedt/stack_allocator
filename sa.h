
#ifndef SA_H
#define SA_H

#ifdef __cplusplus
extern "C" {
#endif

//
// core allocator functions
//
void sa_stack_init(void* a, size_t size);
void* sa_stack_alloc(void* a, size_t size);
void* sa_stack_realloc(void* a, void* ptr, size_t new_size);
void sa_stack_free(void* a, void* ptr);

//
// stack allocator functions
//
void sa_stack_push(void* a);
void sa_stack_pop(void* a);

// debugging routines

void sa_stack_debug_print_meta(void* a);
size_t sa_stack_meta_hash(void* a);

void sa_stack_print_stuff();

#ifdef __cplusplus
}
#endif

#endif
