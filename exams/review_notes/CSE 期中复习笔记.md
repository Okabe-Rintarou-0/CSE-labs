

# CSE 期中复习笔记

### Inode FS

Inode-fs主要分为这几层：

![image-20211114085058605](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114085058605.png)

+ L1 Block Layer

  Superblock **contains**:

  + Size of the blocks
  + Number of free blocks
  + A list of free blocks 
  + Other metadata of the file system (including inode info)

  What will happen if the block size is too small? What if too big?

  太小的话会存储的文件最大大小会很小（inode中block指针数量是固定的，block小的话，最大大小也就小。）而且，会把一个文件分散到很多个block中，即使是一个很小的文件也要经过很多次磁盘读操作才能获取全部内容。

  太大的话虽然读取效率会高，但是也会造成空间浪费，一个很小的文件也占用一个较大的block的话（注意block对fs而言是最小单位，不可再分）空间利用率不高。况且一台计算机中绝大多数都是小文件。

  How to efficiently track free blocks? 当然是利用**bitmap**！

+ L2 File Layer

  注意最大文件大小的计算：

  假设block大小为4kb，有d个direct block pointer 和 i个indirect block pointer（假设是一级）

  则最大大小为d * 4 * 1024 + i * 1024 * 4 * 1024

  一个direct block pointer 对应一个4kb block，一个一级indirect block pointer对应1024个direct block pointer.

+ L3 inode Number Layer

  ![image-20211114090941458](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114090941458.png)

+ L4 File Name Layer

  Linux下的directory和windows中的folder是不同的。Directory本质上就是一个目录，记录的是文件夹下的文件名和其对应的inode id。所以directory的大小和文件的数量正相关，但是和某个文件的具体大小无关。但是windows下的folder显然不是这样的。

+ L5 Path Name Layer

  ![image-20211114091129748](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114091129748.png)

  根据path递归地搜寻，直到找到非文件夹的文件。

  Link:

  inode中记录refcnt，一旦Link refcnt+1， UNLINK refcnt -1。注意到一旦refcnt = 0， 文件就应该被删除。

  用户不能LINK 目录。 . 和 ..是两个特殊的LINK

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114091213814.png" alt="image-20211114091213814" style="zoom:50%;" />

这个例子解释了为什么用户不能LINK一个目录。注意到UNLINK a的时候已经无法从/根目录访问a了（UNLINK把a从目录中删除了。）

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114091849111.png" alt="image-20211114091849111" style="zoom:50%;" />

注意这道题：

![image-20211114093008378](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114093008378.png)

inode存储文件名显然是不合理的，文件名可变长并且有的文件名字可能很长，很容易造成空间上的浪费。

更多的是存文件名会导致无法使用hard link。

+ L6 Absolute Path Name Layer

  根目录，.和..都指向自己。

![image-20211114092441966](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114092441966.png)

查找过程：先找到根目录inode，找到根目录inode对应的data block，从目录中获取文件的inode id，根据inode id找到文件的inode，读取文件对应data block。

+ L7: Symbolic Link Layer

  Symbolic Link 又被称为soft link， 和hard link相对。 一下简称sl。 sl链接的文件不一定存在，经常用于链接外部文件。sl本身也是一个特殊的文件，里面存储着链接的文件的信息，一般就是链接的文件名。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114093348748.png" alt="image-20211114093348748" style="zoom:50%;" />

+ 总结：

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114093458607.png" alt="image-20211114093458607" style="zoom:50%;" />

  

  

  

  

  注意下面这道题：

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114093754823.png" alt="image-20211114093754823" style="zoom:50%;" />

**注意sector和block的区别，sector是磁盘物理上的单元，而block是fs管理的单元，两者还是有很大区别的。这道题block大小为4kb，一个sector 512bytes，所以读一个完整的block需要4 * 1024 / 512 = 8次磁盘读**

注意Atime默认只有在最后才会修改。

第四小问上课提过，注意不要被lab带偏了，一个block完全可以存很多个inode（lab里是一个block一个inode...）。提示把padding换成一个flag位，如果可以的话就把内容直接存在inode里。

248*4>988，够存。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114095907761.png" alt="image-20211114095907761" style="zoom:50%;" />

目录lookup是顺序查找，会涉及到很多磁盘I/O

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114100349013.png" alt="image-20211114100349013" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114100535493.png" alt="image-20211114100535493" style="zoom:50%;" />

### FS API

+ FILE vs fd

  FILE *是文件流操作，目的是为了提高程序的执行效率，相关的接口如：writev、readv，这两个函数可以读写大块数据。
  FD是文件描述符，只能读写数据到一块内存中。若通过FD读写不连续的内存块，write、read需要多次调用syscall，这样就增加了额外的系统开销，导致效率并不高；但是，通过文件流的方式，比如writev、readv，即可实现调用一次syscall，可完成多块不连续内存块的读写操作。**Time stamps**

+ Timestamp

  atime: Last access (by READ)

  mtime: Last modification (by WRITE)

  ctime: Last change of inode (by LINK)

+ 为什么需要FD 而不是 inode id

  Security: user can never access kernel's data structure

  Non-bypassability: all file operations are done by the kernel

  拿着inode id 不用open file也能对文件进行操作。

+ fd table 和file table

  fork出去的子进程会复制父进程的fd。

  Fd_table是进程独立的，file_table是进程共享的。

  一个 v-node 表的表项对应于一个文件。它记录了文件的元数据信息。包括文件权限，文件大小， 文件类型等信息。

  v-node 中的一个表项可能被多个文件表表项指向。当对同一个文件调用多次 open() 函数，那么会产生多个文件表表项，但是只有一个 v-node 表表项。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114101808457.png" alt="image-20211114101808457" style="zoom:50%;" />

+ 读一个文件的过程

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114102142489.png" alt="image-20211114102142489" style="zoom:50%;" />

  注意到涉及到了很多次write，这个write是在修改atime。

  Write->atime, 影响性能 noatime 选项（默认选项） close的时候才会更新atime 减少磁盘写

  Read before write 把一个block读出来，修改其中一部分，再write进去。粒度如此，必须读一整块。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114102323320.png" alt="image-20211114102323320" style="zoom:50%;" />

+ When writing, which **order** is preferred?

  Allocate new blocks, write new data, update size

  如果断电的话，只是修改了block但是没有更新大小。用户大不了再写一次。（脏了也没关系，反正只会读size大小）

  反观如果先修改大小再写，那么断电之后大小变了，万一内容没有写完怎么办？

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114102927173.png" alt="image-20211114102927173" style="zoom:50%;" />

+ FAT

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114103055826.png" alt="image-20211114103055826" style="zoom:50%;" />

  FAT 不支持hardlink。hardlink两个文件等价，试想一下删除一个文件或者更新一个文件会发生什么？

  FAT 最大4GB：size为int类型2^32 -> 4GB

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114103403564.png" alt="image-20211114103403564" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114104423316.png" alt="image-20211114104423316" style="zoom:50%;" />

+ **这块需要去听一下网课**

### RPC

RPC stub 帮程序员完成了封装，免去了一些不必要的工作。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114105410688.png" alt="image-20211114105410688" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114105533356.png" alt="image-20211114105533356" style="zoom:50%;" />

+ RPC message的组成：

  + Service ID (e.g., function ID)

  + Service parameter (e.g., function parameter)

  + Using marshal / unmarshal 即序列化和反序列化

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114105702810.png" alt="image-20211114105702810" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114105717627.png" alt="image-20211114105717627" style="zoom:50%;" />

​	Accepted 和rpc 有关 failure -> rpc version goes wrong, etc.

​	Success 和server有关

​	基于Socket完成server和client的binding。

+ 消息传递

  分布式系统存在不兼容问题：在单台机器上不存在

  例如，远程机器可能有不同的：

  + 字节顺序、整数和其他类型的大小
  + 浮点表示
  + 字符集
  + 对齐要求等。

+ Encoding

  使用JSON/XML...：

  + 好处：易读，人和机器都读得懂；
  + 坏处：
    + 占空间（xml）
    + 存在歧义：12怎么解释？unit32?uint16?int64? 
    + 依赖语言

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114110440610.png" alt="image-20211114110440610" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114110501957.png" alt="image-20211114110501957" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114110537771.png" alt="image-20211114110537771" style="zoom:50%;" />

+ 如何实现exactly once

  要么：

  + 幂等 多做几次无所谓

  + 服务器记住状态（唯一的XID） tradeoff => stateless => stateful

    –Server remember the requests it has seen and replies to executed RPCs (across reboots)

    –Detect duplicates, reqs need unique IDs (XIDs) 

### Distributed FS

##### NFS

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114113008704.png" alt="image-20211114113008704" style="zoom:50%;" />



把文件系统API变成了RPC形式

\+ LOOKUP

\- OPEN CLOSE

+ Mount

  Mount 把外部文件挂在到本地fs。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114113046818.png" alt="image-20211114113046818" style="zoom:50%;" />

+ NFS 流程：

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114113252667.png" alt="image-20211114113252667" style="zoom:50%;" />

  lookup 不像open，返回的是服务器端维护的文件的元数据。client是有状态的，server是无状态的（无状态好啊，一视同仁高效）。

  Application 调用传统接口之后经过位于kernel的NFS Client层，该层发送RPC执行操作。

  打开前和打开后没有任何区别。（因为server端不保存状态，server并不知道一个app打开了文件。）

  READ RPC 幂等（用的是offset）

  NFS client存储状态，cursor位于哪里？这样就可以保证读是幂等的。Application读n个byte，就从NFS client的cursor往后n个offset读数据。

+ File handler保存的是Inode number

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114113913835.png" alt="image-20211114113913835" style="zoom:50%;" />

​	为什么不能是pathname？rename之后怎么办！

+ Delete after open

  要处理这个问题服务器可能还是要存一些信息，这是个tradeoff。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114114309504.png" alt="image-20211114114309504" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114114414935.png" alt="image-20211114114414935" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114131121035.png" alt="image-20211114131121035" style="zoom:50%;" />

+ Vnode

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114131241043.png" alt="image-20211114131241043" style="zoom:50%;" />

+ Validation

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114131412210.png" alt="image-20211114131412210" style="zoom:50%;" />

+ Improving Read Performance

  **Transfer data in large chunks**

  + 8KB default

  **Read-ahead**

  + Optimize for sequential file access
  + Send requests to read disk blocks before they are requested by the applications

+ File consistency

  **Assumes synchronized clock (a global clock)**

  + We have mentioned in lecture02, its challenging 

  **Locking cannot work**

  + Separate lock manager added (stateful)

  **No reference counting of open files** **(stateless** **server)**

  + You can delete a file opened by yourself/others

##### GFS

+ 大多数文件都是大型文件，而且主要操作都是append。主要是顺序读写，而不是随机读写；

+ 在一个分布式系统里错误很容易出现，默认三备份容错；

+ GFS里是简单的键值对。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132305173.png" alt="image-20211114132305173" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132330991.png" alt="image-20211114132330991" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132441144.png" alt="image-20211114132441144" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132722804.png" alt="image-20211114132722804" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132921078.png" alt="image-20211114132921078" style="zoom:50%;" />

Why no caching? “Client caches offer little benefit because most applications stream through huge files or have working sets too large to be cached.” 

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114132942296.png" alt="image-20211114132942296" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133202665.png" alt="image-20211114133202665" style="zoom:50%;" />

+ Read and Write

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133329336.png" alt="image-20211114133329336" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133338415.png" alt="image-20211114133338415" style="zoom:50%;" />

Master给replica一个租约 让replica完成对chunk 的修改，其他replica不能进行修改。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133528288.png" alt="image-20211114133528288" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133545172.png" alt="image-20211114133545172" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133909299.png" alt="image-20211114133909299" style="zoom:50%;" />

### Batch Processing

+ 整体结构

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114141717032.png" alt="image-20211114141717032" style="zoom:50%;" />

IF Intermediate File

具体到实现，以文件作为input

Shard/IF 均存储于GFS上 公共访问

+ 步骤

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114141823059.png" alt="image-20211114141823059" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114141836046.png" alt="image-20211114141836046" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114141850070.png" alt="image-20211114141850070" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142023796.png" alt="image-20211114142023796" style="zoom:50%;" />

通过Hash保证同一个key都发给同一个reducer处理 Hash到一个文件投喂给一个reducer

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142051547.png" alt="image-20211114142051547" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142118001.png" alt="image-20211114142118001" style="zoom:50%;" />

为什么mapreduce需要sort key？加速reduce worker的merge key过程

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142238668.png" alt="image-20211114142238668" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142257509.png" alt="image-20211114142257509" style="zoom:50%;" />

+ Fault Tolerance

  + **no side-effect**: a map or a reduce can simply re-execute the computation to recover from failures** 

  + Builds on **a reliable service** (i.e., GFS)  

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142559191.png" alt="image-20211114142559191" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142625038.png" alt="image-20211114142625038" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142724417.png" alt="image-20211114142724417" style="zoom:50%;" />

为了尽可能利用资源 mapreduce和存储都放在Chunkserver上 （但是逻辑上是分离的）

MR 的master本质上就是一个fork出来的process，可以有多个。

GFS的master就是一台机器。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114142839674.png" alt="image-20211114142839674" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143135140.png" alt="image-20211114143135140" style="zoom:50%;" />

+ MapReduce存在的问题
  + 无法高效处理像PageRank那样需要迭代的任务。

  + Partition不均匀的问题

    <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143223829.png" alt="image-20211114143223829" style="zoom:50%;" />

+ Dryad

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143846143.png" alt="image-20211114143846143" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143901806.png" alt="image-20211114143901806" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143911642.png" alt="image-20211114143911642" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143933137.png" alt="image-20211114143933137" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114143628695.png" alt="image-20211114143628695" style="zoom:50%;" />



<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114144002971.png" alt="image-20211114144002971" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114144016355.png" alt="image-20211114144016355" style="zoom:50%;" />

总而言之，Dryad牺牲了一部分系统的简单性，增加了可用性。