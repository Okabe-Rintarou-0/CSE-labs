# CSE-labs

labs of CSE. From lesson SE124. 

### Guide

| Labs           | Notes                                        | Remarks                                                      |
| -------------- | -------------------------------------------- | ------------------------------------------------------------ |
| [Lab1](./lab1) | [Basic File System](./notes/Lab1.md)         | A basic file system that will be used in later labs.         |
| [Lab2](./lab2) | [Word Count with MapReduce](./notes/Lab2.md) | Use MapReduce to count word frequency based on distributed fs. |
| [Lab3](./lab3) | [Raft](./notes/Lab3.md) | A powerful protocol of distributed system. Easier to understand than Paxos. |
| [Lab4](./lab4) | [Shard Transactional KVS Service](./notes/Lab4.md) | A distributed KVS based on raft group. |

### Hands-on

| Hands-on                          | specification                            | solution                                       |
| --------------------------------- | ---------------------------------------- | ---------------------------------------------- |
| [Hands-on-1](./handson/Handson-1) | [spec](./handson/Handson-1/handson1.pdf) | [solution](./handson/Handson-1/Hands-on_1.pdf) |
| [Hands-on-2](./handson/Handson-2) | [spec](./handson/Handson-2/handson2.pdf) | [solution](./handson/Handson-2/Hands-on_2.pdf) |
| [Hands-on-3](./handson/Handson-3) | [spec](./handson/Handson-3/handson3.pdf) | [solution](./handson/Handson-3/Hands-on-3%20Domain%20Name%20Service%20(DNS).md) |



### Exams

| Exam        | Solution                                        | Remarks                                                      |
| -------------- | -------------------------------------------- | ------------------------------------------------------------ |
| [2020 Mid Term](./exams/2020-CSE-midterm.pdf) | [2020 Mid Term Solution](./exams/2020-CSE-midterm-sol.pdf)         | Just for reference. |

### Review Notes

For review and print.

| Note      | Remark                                        |
| -------------- | -------------------------------------------- |
| [Mid Term Review](./exams/review_notes) | A Chinese version review note.|         

### File Structure

```
.
????????? 
	????????? docs
		????????? lab1
        	????????? ...
	????????? exams
	????????? imgs
	????????? lab1
    	????????? ...
	????????? notes
	????????? README.md
```

+ **docs**: store some materials about lab, like the design of ext3.
+ **exams**: store exams and some review notes(in Chinese XD)
+ **imgs**: store the images of notes.
+ **notes**: store the notes on each lab.

### Usage of scripts

+ **Start.sh**: mount the file system
+ **Stop.sh**: unmount the file system
+ **\*.pl**: test scripts for each part

### How to test

+ Using provided **./grade.sh**
+ Manually **./stop.sh | ./start.sh | ./*.pl** to test single part.
+ Or you can directly test it **without** mounting the file system.

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
