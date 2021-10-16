# CSE-labs

labs of CSE. From lesson SE124. 

### Guide

| Labs           | Notes                                        | Remarks                                                      |
| -------------- | -------------------------------------------- | ------------------------------------------------------------ |
| [Lab1](./lab1) | [Basic File System](./notes/Lab1.md)         | A basic file system that will be used in later labs.         |
| [Lab2](./lab2) | [Word Count with MapReduce](./notes/Lab2.md) | Use MapReduce to count word frequency based on distributed fs. |

### File Structure

```
.
├── 
	└── docs
		└── lab1
        	└── ...
	├── imgs
	├── lab1
    	├── ...
	├── notes
	└── README.md
```

+ **docs**: store some materials about lab, like the design of ext3.
+ **imgs**: store the images of notes.
+ **notes**: store the notes on each lab.

### Annoying segmentation fault

+ Segmentation fault(core dumped):

  This will usually occur when we are trying to visit memory address that we haven't allocated yet. For example, if you allocate a array with SIZE.

  ```c++
  char array[SIZE];
  ```

  But you try to do this:

  ```c++
  memcpy(array, other, SIZE + n);
  ```

  It will probably cause segmentation fault.

+ sysmalloc: Assertion `(old_top == initial_top (av) && old_size == 0) ...

  That is because some illegal operations may cause the head or tail of the allocated space to be overwritten by other data, that is, the problem of memory out of bounds.
  
  For example, we may allocate a block with size of BLOCK_SIZE: 
  
  ```c++
  char *block = (char *) malloc(sizeof(char) * BLOCK_SIZE)
  ```
  
  But we are trying to do something like this: 
  
  ``` c++
  memcpy(block, other, 2 * BLOCK_SIZE);
  ```
  
  This may cause the problem above.
  
  ![image](./imgs/malloc_heap.png)
