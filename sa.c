
#ifndef SA_C
#define SA_C

#include <assert.h>
#include <string.h>
#include "sa_types.h"

// headers for debugging

#include <stdio.h>

#define STACK_ALIGNMENT 16U
#define STACK_ALIGNMENT_MASK 15U

#define STACK_NODE_PTR(PTR, BYTE_OFFSET) (sa_stack_node*)((u8*)(PTR) + (BYTE_OFFSET))

#ifdef __cplusplus
extern "C" {
#endif

#define STACK_ALIGNMENT_PAD_SIZE(ALIGNMENT, SZ) (((SZ) % (ALIGNMENT)) ? ((ALIGNMENT) - (SZ) % ALIGNMENT) : 0U)

typedef struct sa_stack_head {
  size_t tail;
  size_t push_list_tail;
  size_t size;
} sa_stack_head;

typedef struct sa_stack_node {
  size_t prev_offset;
  size_t next_offset;
  size_t chunk_size;
} sa_stack_node;

typedef struct sa_stack_node_padded {
  sa_stack_node contents;

  // padding to fill up to an even 16-bytes
  u8             padding[STACK_ALIGNMENT_PAD_SIZE(STACK_ALIGNMENT, sizeof(sa_stack_node))];
} sa_stack_node_padded;

static
void sa_stack_print_node(void* a, sa_stack_node* n, const char* tag) {
  u8* byte_ptr = (u8*)a;
  size_t cur_offset = (u8*)n - byte_ptr;
  printf("Node[cur %u, prev %u, next %u] : %s\n", cur_offset, n->prev_offset, n->next_offset, tag);
}

static
size_t sa_stack_padded_offset(void* a, size_t size) {
  size_t result;
  u8* data_ptr = (u8*)a + size;

  size_t address_integer;
  memcpy(&address_integer, &data_ptr, sizeof(u8*));

  size_t alignment_error = address_integer & STACK_ALIGNMENT_MASK;

  if(alignment_error == 0U) { // good to go
    result = size;
  } else { // must add some padding
    size_t padding = STACK_ALIGNMENT - alignment_error;
    result = size + padding; 
  }

  return result;
}

void sa_stack_init(void* a, size_t size) {
  sa_stack_head* head_node = (sa_stack_head*)a;

  head_node->tail = 0U;
  head_node->size = size;
  head_node->push_list_tail = 0U;
}

void* sa_stack_alloc(void* a, size_t size) {
  u8* result;
  u8* byte_ptr = (u8*)a;
  sa_stack_head* head_node = (sa_stack_head*)a;

  size_t size_masked = size & STACK_ALIGNMENT_MASK;
  size_t size_padded;

  if(size_masked) {
    size_padded = size + (STACK_ALIGNMENT - size_masked);
  } else {
    size_padded = size;
  }

  if(head_node->tail == 0U) { // stack is empty, initialize the linked structure
    size_t first_node_offset = sa_stack_padded_offset(a, sizeof(sa_stack_head));

    size_t data_offset = first_node_offset + sizeof(sa_stack_node_padded);
    size_t tail_offset = data_offset + size_padded;

    // setup the first node

    sa_stack_node* first_node = STACK_NODE_PTR(a, first_node_offset);

    first_node->prev_offset = 0U;
    first_node->next_offset = tail_offset;
    first_node->chunk_size = size_padded;

    // setup the tail node

    sa_stack_node* tail_node = STACK_NODE_PTR(a, tail_offset);

    tail_node->prev_offset = first_node_offset;
    tail_node->next_offset = 0U;

    // setup the head node

    head_node->tail = tail_offset;

    // calculate the resulting pointer

    result = byte_ptr + data_offset;
  } else {
    size_t old_tail_offset = head_node->tail;
    size_t data_offset = old_tail_offset + sizeof(sa_stack_node_padded);
    size_t new_tail_offset = data_offset + size_padded;

    // check for stack overflow
    assert(new_tail_offset + sizeof(sa_stack_node_padded) < head_node->size);

    sa_stack_node* old_tail_node = STACK_NODE_PTR(a, old_tail_offset);
    sa_stack_node* new_tail_node = STACK_NODE_PTR(a, new_tail_offset);

    // update the previous tail node

    old_tail_node->next_offset = new_tail_offset;
    old_tail_node->chunk_size = size_padded;

    // setup the new tail node

    new_tail_node->prev_offset = old_tail_offset;
    new_tail_node->next_offset = 0U;

    // change the head node's tail offset

    head_node->tail = new_tail_offset;

    // calculate the resulting pointer

    result = byte_ptr + data_offset;
  }

  return result;
}

void sa_stack_free(void* a, void* ptr) {
  // handle freeing of null pointer gracefully
  if(ptr == 0)
    return;

  u8* byte_ptr = (u8*)a;
  sa_stack_head* head_node = (sa_stack_head*)a;

  size_t data_offset = ((u8*)ptr - byte_ptr);
  size_t node_offset = data_offset - sizeof(sa_stack_node_padded);

  sa_stack_node* cur_node = STACK_NODE_PTR(a, node_offset);
  sa_stack_node cur_node_value = *cur_node;

  sa_stack_node* next_node = STACK_NODE_PTR(a, cur_node_value.next_offset);
  sa_stack_node next_node_value = *next_node;

  // there are 4 distinct cases to handle here
  // 1: the freed element is both the only element in the list
  // 2: the freed element is the first element in the list, but not the last
  // 3: the freed element is the last element in the list, but not the first
  // 4: the freed element is neither the first, nor the last element of the list

  // *** DEBUGGING ***
  //sa_stack_print_node(a, cur_node, "free");
  // *** DEBUGGING ***

  if(cur_node_value.prev_offset == 0U) { // this is the first element
    if(next_node_value.next_offset == 0U) { // this is the last element : case 1
      
      // reset the head offsets to begin anew
      head_node->tail = 0U;

    } else { // this is NOT the last element : case 2

      next_node->prev_offset = 0U;

    }
  } else { // this is NOT the first element
    if(next_node_value.next_offset == 0U) { // this is the last element : case 3

      head_node->tail = node_offset;
      cur_node->next_offset = 0U;
      cur_node->chunk_size = 0U;

    } else { // this is NOT the last element : case 4

      sa_stack_node* prev_node = STACK_NODE_PTR(a, cur_node_value.prev_offset);
      
      prev_node->next_offset = cur_node_value.next_offset;
      next_node->prev_offset = cur_node_value.prev_offset;

    }
  }
}

void* sa_stack_realloc(void* a, void* ptr, size_t new_size) {
  void* result;
  if(ptr == 0)
    return sa_stack_alloc(a, new_size);

  size_t size_masked = new_size & STACK_ALIGNMENT_MASK;
  size_t size_padded;

  if(size_masked) {
    size_padded = new_size + (STACK_ALIGNMENT - size_masked);
  } else {
    size_padded = new_size;
  }

  sa_stack_head* head_node = (sa_stack_head*)a;
  size_t data_offset = (u8*)ptr - (u8*)a;
  size_t node_offset = data_offset - sizeof(sa_stack_node_padded);

  sa_stack_node* node = STACK_NODE_PTR(a, node_offset);

  size_t next_offset = node->next_offset;

  sa_stack_node* next_node = STACK_NODE_PTR(a, next_offset);

  if(next_node->next_offset == 0U) { // the memory chunk being reallocated is located last
    size_t new_next_offset = data_offset + size_padded;
    sa_stack_head* head_node = (sa_stack_head*)a;
    sa_stack_node* new_next_node = STACK_NODE_PTR(a, new_next_offset);

    node->next_offset = new_next_offset;
    node->chunk_size = size_padded;
    head_node->tail = new_next_offset;
    new_next_node->prev_offset = node_offset;
    new_next_node->next_offset = 0U;

    result = ptr;
  } else { // the memory chunk being reallocated is first or in the middle
    size_t old_size       = node->chunk_size;          // the current size of the reallocated chunk
    size_t push_list_tail = head_node->push_list_tail; // the offset to the current tail of the push-list
    size_t free_size      = next_offset - data_offset; // the available sequential memory in bytes
    u32 needs_relocation;

    if(push_list_tail > node_offset) { // the chunk to be reallocated is before a push list entry
      needs_relocation = 1U;
    } else if(free_size < size_padded) {
      needs_relocation = 1U;
    } else {
      needs_relocation = 0U;
    }

    if(needs_relocation) {
      void* new_ptr = sa_stack_alloc(a, size_padded);
      memcpy(new_ptr, ptr, old_size);
      sa_stack_free(a, ptr);
      result = new_ptr;
    } else {
      node->chunk_size = size_padded;
      result = ptr;
    }
  }
  
  return result;
}

void sa_stack_push(void* a) {
  sa_stack_head* head_node = (sa_stack_head*)a;

  void* push_list_new_tail_ptr = sa_stack_alloc(a, sizeof(size_t));
  
  size_t* push_list_new_tail_prev_offset = (size_t*)push_list_new_tail_ptr;

  *push_list_new_tail_prev_offset = head_node->push_list_tail;
  size_t new_tail_offset = (((size_t)push_list_new_tail_ptr - sizeof(sa_stack_node_padded)) - (size_t)a);
  
  head_node->push_list_tail = new_tail_offset;
}

void sa_stack_pop(void* a) {
  sa_stack_head *head_node = (sa_stack_head*)a;
  size_t push_list_tail_offset = head_node->push_list_tail;

  assert(push_list_tail_offset != 0U); // pre-condition: push list not empty

  size_t* data_ptr = (size_t*)((u8*)a + (push_list_tail_offset + sizeof(sa_stack_node_padded)));

  size_t prev_push_list_offset = *data_ptr;

  sa_stack_node* push_list_tail_node = STACK_NODE_PTR(a, push_list_tail_offset);

  // make the node the new tail

  push_list_tail_node->next_offset = 0U;

  // update the head node's tail offset

  head_node->tail = prev_push_list_offset;
}

// debugging routines

#define STACK_HASH_PRIME 16777619
#define STACK_HASH_OFFSET 2166136261

size_t sa_stack_meta_hash(void* a) {
  size_t result = STACK_HASH_OFFSET;

  sa_stack_head* head_node = (sa_stack_head*)a;

  result = result + (head_node->tail * STACK_HASH_PRIME);
  result = result + (head_node->size * STACK_HASH_PRIME);

  sa_stack_node* cur;

  if(head_node->tail == 0U)
    return result;

  cur = STACK_NODE_PTR(a, head_node->tail);

  while(cur) {
    result = result + (cur->prev_offset * STACK_HASH_PRIME);
    result = result + (cur->next_offset * STACK_HASH_PRIME);
    result = result + (cur->chunk_size  * STACK_HASH_PRIME);
    if(cur->prev_offset == 0U) {
      cur = 0;
    } else {
      cur = STACK_NODE_PTR(a, cur->prev_offset);
    }
  }

  return result;
}

void sa_stack_debug_print_meta(void* a) {
  u8* byte_ptr = (u8*)a;
  sa_stack_head* head_ptr = (sa_stack_head*)a;
  sa_stack_node* node_ptr;// = (sa_stack_node*)a;

  size_t memory_usage = ((head_ptr->tail == 0U) ? sa_stack_padded_offset(a, sizeof(sa_stack_head)) : (head_ptr->tail)) + sizeof(sa_stack_node_padded);

  printf("sa_STACK[size %u, used %u, hash %u]\n", head_ptr->size, memory_usage, sa_stack_meta_hash(a));

  printf("Head[tail: %u, size %u]\n", head_ptr->tail, head_ptr->size);

  node_ptr = STACK_NODE_PTR(a, head_ptr->tail);

  do {
    size_t cur_offset = (u8*)node_ptr - byte_ptr;
    printf("Node[cur %u, prev %u, next %u, size %u]\n", cur_offset, node_ptr->prev_offset, node_ptr->next_offset, node_ptr->chunk_size);

    node_ptr = (sa_stack_node*)(byte_ptr + node_ptr->prev_offset);
  } while(node_ptr != (sa_stack_node*)a);
}

void sa_stack_print_stuff() {
  printf("\n*** sa_STACK_PRINT_STUFF ***\n");
  printf("sizeof(sa_stack_head): %u\n", sizeof(sa_stack_head));
  printf("sizeof(sa_stack_node): %u\n", sizeof(sa_stack_node));
  printf("sizeof(sa_stack_node_padded): %u\n", sizeof(sa_stack_node_padded));
}

#ifdef __cplusplus
}
#endif

#endif
