
#ifndef SA_H
#define SA_H

#ifdef __cplusplus
extern "C" {
#endif

//
// core allocator functions
//
void bft_stack_init(void* a, size_t size);
void* bft_stack_alloc(void* a, size_t size);
void* bft_stack_realloc(void* a, void* ptr, size_t new_size);
void bft_stack_free(void* a, void* ptr);

//
// stack allocator functions
//
void bft_stack_push(void* a);
void bft_stack_pop(void* a);

// debugging routines

void bft_stack_debug_print_meta(void* a);
size_t bft_stack_meta_hash(void* a);

void bft_stack_print_stuff();

#ifdef __cplusplus
}
#endif

#endif
