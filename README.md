# stack_allocator
Implementation of a fast and flexible stack based allocator.

An important design goal for the allocator is for the data-structures to enable the common "malloc"-operations:
<ul>
  <li>allocate</li>
  <li>reallocate</li>
  <li>free</li>
</ul>

but also enable stack allocator operations that enable the user to free all allocations since the last "push" in constant time:
<ul>
  <li>push</li>
  <li>pop</li>
</ul>
