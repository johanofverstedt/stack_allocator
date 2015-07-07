# stack_allocator
Implementation of a fast and flexible stack based allocator. Here stack does not refer to the program stack, but to the fact that all allocations happen at the tail unlike more fine-grained allocators, such as std malloc which support random sequenced allocations and free without a lot of fragmentation. This stack based allocator performs best (no fragmentation) when all memory is freed in the reverse order of allocation. This allocator is mostly suited for thread-local working memory, and not to persistently store program data with varying lifetimes.

An important design goal for the allocator is for the data-structures to enable the common "malloc"-operations:
<ul>
  <li>allocate</li>
  <li>reallocate</li>
  <li>free</li>
</ul>

but also enable stack allocator operations:
<ul>
  <li>push</li>
  <li>pop</li>
</ul>
These operations enable the user to free all allocations since the last "push" in constant time with a single call to "pop"
