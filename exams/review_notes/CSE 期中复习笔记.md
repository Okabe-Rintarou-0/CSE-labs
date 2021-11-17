

# CSE 期中复习笔记

### Fault tolerant

+ log
+ replica
+ checksum

### Distributed

淘宝和支付宝是如何运作的？

价格这种要保证transaction正确执行的，非常重要的东西要进数据库持久化。而人气这种东西不用太精细，也不用保证每个人看到的都是一致的，所以可以用简单的k-v存储系统，这样成本会比数据库低很多。

一台服务器硬件资源有限。

于是把文件系统和数据库分离出来，形成应用服务器，文件系统服务器和数据库服务器。这样一来可以比较容易地对硬件资源进行拓展。

但是这也产生了新的问题，那就是原本直接走本地访问就行了，现在需要走一层网络才能访问（在另一台服务器上），这就提高了时延。

于是乎：添加缓存。配若干台缓存服务器，专门把数据库的数据缓存到服务器内存中（注意，虽然mysql也有缓存，但是不可控）。这样虽然还是要走网络，但是避免了大量的读硬盘的操作，要快很多。一般用Redis或memcached

缓存服务器用的是一致性hash算法。

但如果像双十一那种一下特别多http请求怎么办，一个应用服务器肯定撑不住：

于是乎：多个无状态的应用服务器形成集群：

像图片和视频这种如果每次都从很远的服务器过来就会很花时间。一般大公司都会买CDN服务器把图片和视频等缓存到服务器上。这样一个人去访问图片，图片就被缓存到服务器上，那么同一地区的下一个人再次访问就不用从远端的服务器拿了，直接就近走CDN去拿。网站上的数据不都是服务器上的，很大一部分源自CDN。

把应用服务器用到的服务抽取出来，形成一个个微服务。微服务可以给不同的应用服务器复用。抽取出来之后也会更加容易合理分配资源。更耗资源的微服务就可以多给他配点资源。

推荐算法：有个服务器在训模型，然后用户请求的时候返还的是离线的，根据训好的模型得出的数据。

处理大规模数据MapReduce

反诈骗系统（信用卡套现，A转B B转C C转A 涉及到图的计算）

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116190236778.png" alt="image-20211116190236778" style="zoom:50%;" />

+ CAP

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211117082826283.png" alt="image-20211117082826283" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211117082851742.png" alt="image-20211117082851742" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211117082909616.png" alt="image-20211117082909616" style="zoom:50%;" />

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

+ 尽量保证幂等。实在不行就保存TXID（到硬盘），删除TXID的时间如何定？

  Tradeoff！Exactly once 非常难！

  保证幂等！

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

  + 如果旧的f和新的f不共用一个inode，那么旧的f还是能够正常读取：(存在fh返回回去)
  + 共用inode，generation number +1， 再去读的时候就被告知这个被打开的文件已经被删除了。
  
  要处理这个问题服务器可能还是要存一些信息，这是个tradeoff。
  
  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114114309504.png" alt="image-20211114114309504" style="zoom:50%;" />
  
  写穿到硬盘

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114114414935.png" alt="image-20211114114414935" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114131121035.png" alt="image-20211114131121035" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211117151509533.png" alt="image-20211117151509533" style="zoom:50%;" />

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

client 传数据给最近的server，再由最近的server传输给其他server

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211114133528288.png" alt="image-20211114133528288" style="zoom:50%;" />

primary把write request，再把request发给secondary。当收集所有的ACK的时候，返回ok。

primary保证order。

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

  + **no side-effect**: a map or a reduce can simply re-execute the computation to recover from failures** ？。、

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

### Graph2Stream

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116145747617.png" alt="image-20211116145747617" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116145828933.png" alt="image-20211116145828933" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150042004.png" alt="image-20211116150042004" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150100271.png" alt="image-20211116150100271" style="zoom:50%;" />

+ Pregel 不知道周围的节点是什么，只能接收到发过来的信息。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150315256.png" alt="image-20211116150315256" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150615826.png" alt="image-20211116150615826" style="zoom:50%;" />

问题：

pregel必须要知道message内容；

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150758496.png" alt="image-20211116150758496" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150813481.png" alt="image-20211116150813481" style="zoom:50%;" />

+ GraphLab(Shared Memory)

  不需要解码消息，但是需要记住自己周围有哪些节点。(Think like a vertex)

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150844507.png" alt="image-20211116150844507" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150939347.png" alt="image-20211116150939347" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116150956998.png" alt="image-20211116150956998" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116151019017.png" alt="image-20211116151019017" style="zoom:50%;" />

### KV

+ Naïve KVS

  比较naïve的方法 ，直接插入新的值，从后往前找，一个文件里可能有多个相同的键

  插入一个DELETE值代表删除。

+ Hash index in memory

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116152759463.png" alt="image-20211116152759463" style="zoom:50%;" />

+ Compaction

  Compaction会导致运行卡顿，compact完更新hash值 期间允许读但是不允许写 

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116152502999.png" alt="image-20211116152502999" style="zoom:50%;" />

+ Cuckoo Hash

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116152927652.png" alt="image-20211116152927652" style="zoom:50%;" />

+ B+ Tree

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116153013830.png" alt="image-20211116153013830" style="zoom:50%;" />

  对Range Query 友好，但是会涉及到很多硬盘随机读。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116153111896.png" alt="image-20211116153111896" style="zoom:50%;" />

  

+ LSMT

  Open close overhead 比较大，K-V存在同一个文件里

  顺序读写 快于 随机读写

  LSM 兼顾Range query和读写速度

  Fixed size/ Chunk

  B-Tree 虽然range query很快 但是涉及到很多硬盘随机I/O

  L0 顺序写，直接append不排序，利用磁盘顺序写速度快的性质

  WAL 

  LSM L0层 单个文件排序 但是文件之间不排序 （保证顺序写入磁盘）

  L0 层可以用SSD 速度快

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116155445128.png" alt="image-20211116155445128" style="zoom:50%;" />

  涉及到的随机I/O 交给memory（memtable 跳表排序）

  L1,L2…利用mergesort 排序并compact

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116155244328.png" alt="image-20211116155244328" style="zoom:50%;" />

### Crash

先写data

如果接着写inode 如果crash，bitmap上显示为free，可能会出现一个data块被两个inode引用的情况

所以 data=>bitmap=>inode

Bitmap 可以通过扫描恢复。

+ fsck

  Superblock 可能会有备份

  一些坏块的情况可能会导致sb的size被动态修改 如果期间crash就会导致其值不正确

  不关心对或错 关注一致性 删了就一起删了 要么就都没删，大不了再删一遍

  删文件的时候inode没删 bitmap更新为used

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116085859754.png" alt="image-20211116085859754" style="zoom:50%;" />



<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116090034941.png" alt="image-20211116090034941" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116090154873.png" alt="image-20211116090154873" style="zoom:50%;" />

几乎要扫描整个磁盘 非常耗时

进行fsck的时机？检测到不是通过reboot命令进行重启的 如果短时间内重启次数过多

+ 写文件非常慢，考虑解压的情况会怎么样？

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116105026389.png" alt="image-20211116105026389" style="zoom:50%;" />

+ 使用Write back cache

  指定一个周期，每个周期定期FLUSH

  万一还是crash了怎么办？

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116105210288.png" alt="image-20211116105210288" style="zoom:50%;" />

FLUSH的顺序问题 不一定我写的顺序和flush的顺序是一致的。

+ 使用Journal

  只写metadata到journal（默认）

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116114801454.png" alt="image-20211116114801454" style="zoom:50%;" />

  Data->JM->JC->METADATA

  Data&JM到JC之间的flush可以免去，可以用checksum检查

  用checksum检查是否JM和data是否完整
  
  如果不完整 那就是nothing
  
  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116115017469.png" alt="image-20211116115017469" style="zoom:50%;" />

利用硬件解决第二个flush

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116115313411.png" alt="image-20211116115313411" style="zoom:50%;" />

### Fault

Journal只能保证一致性，但是不能保证原子性。即使一个操作是原子的，用户也很有可能将原子操作随机组合，这样的组合不能保证使原子的。

+ Shadow copy

  临时文件。

  原子性聚焦到了rename上。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116124931766.png" alt="image-20211116124931766" style="zoom:50%;" />

+ rename的时候crash？

  Journal保证rename的原子性

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116125220673.png" alt="image-20211116125220673" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116125246490.png" alt="image-20211116125246490" style="zoom:50%;" />

  The choice of which to use is arbitrary; as shown the procedure copies the new value in sector *S1* to both sectors *S2* and *S3*. 

  一次性写三个4k

  Commit point 3-4转变的瞬间 3个sector都不一样 说明到达状态4

  为什么不是3呢？

  状态4保证了对sector S1 的写入已经完成 （S2是bad，即已经开始对S2写，S1已写入完毕）

  

  写COMMIT 保证原子 =》 4KB原子性 =》 利用电容/Section x 3

  Bootstrapping 多个操作=>若干操作+一个原子性操作=》转化

  整体思路：

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211117163418912.png" alt="image-20211117163418912" style="zoom:50%;" />

+ RAID

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116125714398.png" alt="image-20211116125714398" style="zoom:50%;" />

+ Redo Log Undo Log

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116130737925.png" alt="image-20211116130737925" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131007132.png" alt="image-20211116131007132" style="zoom:50%;" />

+ checkpoint

  选择在磁盘不忙的时候进行。

  checkpoint 可以删除一部分log 防止log太长。这样做可以减少系统恢复的时间。

  checkpoint之前commit的tx都不用care。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131106826.png" alt="image-20211116131106826" style="zoom:50%;" />

  T5 没有影响。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131157438.png" alt="image-20211116131157438" style="zoom:50%;" />

  使用checkpoint保证log不会太长，减少系统从错误中恢复的时间。

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131756832.png" alt="image-20211116131756832" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131747018.png" alt="image-20211116131747018" style="zoom:50%;" />

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116131902853.png" alt="image-20211116131902853" style="zoom:50%;" />

+ WAL

  Write log record to disk before modifying persistent state (e.g., replace A’s value) 

  –Write Ahead Log (WAL) protocol

  ![image-20211116130510455](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116130510455.png)

+ MTTF 

  **MTTF:** **mean** **time** **to** **failure**

  **MTTR:** **mean** **time** **to** **repair**

  **MTBF:** **mean** **time** **between** **failure**

  **MTBF** **=** **MTTF** **+** **MTTR**

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116132223387.png" alt="image-20211116132223387" style="zoom:50%;" />

### TX_LOCK

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116132854492.png" alt="image-20211116132854492" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116132925909.png" alt="image-20211116132925909" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116133042919.png" alt="image-20211116133042919" style="zoom:50%;" />\

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116133114549.png" alt="image-20211116133114549" style="zoom:50%;" />

Conflict Serializability 是最严格的。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116133137343.png" alt="image-20211116133137343" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116133243873.png" alt="image-20211116133243873" style="zoom:50%;" />

+ 2PL

  证明2PL能够产生Conflict Serializable schedule

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116141746869.png" alt="image-20211116141746869" style="zoom:50%;" />

+ 优化：在2pL放锁阶段先放读锁。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116141910080.png" alt="image-20211116141910080" style="zoom:50%;" />

+ 解决死锁：
  + 按顺序拿锁

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116142011490.png" alt="image-20211116142011490" style="zoom:50%;" />

### TX_OCC

+ 2PL会产生幻读（因为锁的粒度问题，只锁了访问的数据）

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115234327196.png" alt="image-20211115234327196" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115234402583.png" alt="image-20211115234402583" style="zoom:50%;" />

把锁的粒度集中在validation上，2,3加锁防止检查完readset 之后又被人修改。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115234438031.png" alt="image-20211115234438031" style="zoom:50%;" />

2PL 是看见一个锁一个，锁的顺序取决于数据的顺序，所以无法保证不出现死锁

OCC 给write set排个序就行，按顺序拿锁。这样就能保证锁的相对顺序都是正确的。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115234824089.png" alt="image-20211115234824089" style="zoom:50%;" />

如果thread1不abort就相当于先执行T1 再执行T2 也是可行的 这说明OCC 不是完美的，OCC会有fault abort

当read set 很大的时候很容易发生abort

利用CPU的cache coherence机制。L1缓存变化通知其他core。

RTM restricted transactoon memory：基于CPU的cache conherence机制，一旦Readset被更改就立刻abort。

不同于软件，注意到软件实现允许readset被修改到某个状态再修改回来，即只要保证validation的时候readset和一开始一致即可。

如果RTM不生效 那就用lock 如果另一个tx在用lock，那么当前tx应该abort(人家在用lock了，我凭什么用RTM呀，没有意义)

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235028224.png" alt="image-20211115235028224" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235049144.png" alt="image-20211115235049144" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235200546.png" alt="image-20211115235200546" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235217866.png" alt="image-20211115235217866" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235421420.png" alt="image-20211115235421420" style="zoom:50%;" />

有人在我之前commit了，那我就abort。

READ（X） 读到的是小于start timestamp的最大版本号的值

注意MVCC 是“多版本”

MVCC is not serializable 

Snapshot isolation

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235527870.png" alt="image-20211115235527870" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211115235503561.png" alt="image-20211115235503561" style="zoom:50%;" />

### TX_2PC

+ Multi-site Transaction

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116160841828.png" alt="image-20211116160841828" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116160909364.png" alt="image-20211116160909364" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116161041781.png" alt="image-20211116161041781" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116161409916.png" alt="image-20211116161409916" style="zoom:50%;" />

coordinator在返回OK之前写COMMIT

Coordinate是事务的协调者。Server通过回应prepare来告知Coorinator是不是具备提交的能力。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116162953354.png" alt="image-20211116162953354" style="zoom:50%;" />

+ 丢失prepare：

  重发即可

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116163448615.png" alt="image-20211116163448615" style="zoom:50%;" />

+ 丢失ACK： 

  重发，prepare是幂等的，没关系

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116163712966.png" alt="image-20211116163712966" style="zoom:50%;" />

+ commit point之前crash

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116164230538.png" alt="image-20211116164230538" style="zoom:50%;" />

+ worker commit 恢复之后可以重新commit（发现自己是prepared去询问coordinator 或者 coordinator重新发commit）

  coordinator通过心跳判断是否还活着

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116165548613.png" alt="image-20211116165548613" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116165822496.png" alt="image-20211116165822496" style="zoom:50%;" />

第二个OK之前commit了，coordinate可以根据log redo（重新commit）

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116165838280.png" alt="image-20211116165838280" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116165753216.png" alt="image-20211116165753216" style="zoom:50%;" />

+ Clock synchronization

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116182810600.png" alt="image-20211116182810600" style="zoom:50%;" />

  假设时间快了，然后t_begin和t_end之间回调了时间，那么t_end-t_begin就会小于0

  + 破坏顺序
  + 导致负的interval

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116183633804.png" alt="image-20211116183633804" style="zoom:50%;" />

  根据时钟频率缓慢调整时间：

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116183915790.png" alt="image-20211116183915790" style="zoom:50%;" />

  

+ File Reconciliation

  Vector timestamp

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116184214339.png" alt="image-20211116184214339" style="zoom:50%;" />

时间戳向量记录每台机器上的timestamp

+ Quorum

  <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116184845238.png" alt="image-20211116184845238" style="zoom:50%;" />

写的时候是都写，只不过只要保证Qw个成功就算写成功了。

一开始全是3，后来写2，但是只有3个写成功。

读的话读出2,3,3 只能重读。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116190825741.png" alt="image-20211116190825741" style="zoom:50%;" />

**Majority holds the truth.**

### RSM & PAXOS

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116191028479.png" alt="image-20211116191028479" style="zoom:50%;" />

备份的初始状态一样，输入一样，顺序一样，那么最终结果就是一样的。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116191622210.png" alt="image-20211116191622210" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116191839964.png" alt="image-20211116191839964" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116191900648.png" alt="image-20211116191900648" style="zoom:50%;" />

Split brain：

S2 访问不到S1的 C2把S2当做primary 出现分脑现象。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116192235668.png" alt="image-20211116192235668" style="zoom:50%;" />

primary只有收到backup的ACK才会返回ok，否则reject。

backup会reject 所有来自用户的请求。只有primary接受。

有了View Server，primary就能知道自己有哪些backup，对于backup则反过来。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116192132024.png" alt="image-20211116192132024" style="zoom:50%;" />

更换primary

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116192503938.png" alt="image-20211116192503938" style="zoom:50%;" />

VS和S1断开。



<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116192642261.png" alt="image-20211116192642261" style="zoom:50%;" />

S2知道自己没有backup

所以不会reject

Primary必须得到backup的ack才能算是完成了请求的任务，否则将会拒绝request

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116192751573.png" alt="image-20211116192751573" style="zoom:50%;" />

VS挂了怎么办？PAXOS!

+ Paxos

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116193605601.png" alt="image-20211116193605601" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116193629819.png" alt="image-20211116193629819" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194218439.png" alt="image-20211116194218439" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194229350.png" alt="image-20211116194229350" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194243597.png" alt="image-20211116194243597" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194348875.png" alt="image-20211116194348875" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194423564.png" alt="image-20211116194423564" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194505773.png" alt="image-20211116194505773" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194527744.png" alt="image-20211116194527744" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116194821161.png" alt="image-20211116194821161" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116195208406.png" alt="image-20211116195208406" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116195605092.png" alt="image-20211116195605092" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116195713187.png" alt="image-20211116195713187" style="zoom:50%;" />

Commit point：majority accept V。

如果已经有majority accept， 那么accept的value就不会被修改。

比如又来一个leader 他得到majority promise一定包含这个accept value，再次accept就还是这个值。

leader会从promise里挑proposal号最大的value。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116215525169.png" alt="image-20211116215525169" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116220847423.png" alt="image-20211116220847423" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211116221246172.png" alt="image-20211116221246172" style="zoom:50%;" />

