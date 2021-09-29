# CSE-labs
labs of CSE.

### Annoying segmentation fault

+ sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) ...
  That is because some illegal operations may cause the head or tail of the allocated space to be overwritten by other data, that is, the problem of memory out of bounds. 
  For example, we may allocate a block with size of BLOCK_SIZE: char *block = (char *) malloc(sizeof(char) * BLOCK_SIZE)
  But we are trying to do something like this: memcpy(block, other, 2 * BLOCK_SIZE);
  This may cause the problem above.
  ![image](https://doc.dpdk.org/guides-1.8/_images/malloc_heap.png)
