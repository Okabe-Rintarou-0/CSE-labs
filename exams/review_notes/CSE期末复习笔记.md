# CSE期末复习

## 特别鸣谢

感谢[鲍神](https://github.com/Kami-code)提供的十万字笔记，书店没你的书我不去。

## Notice

+ 注意到数据库中提到的address应该是指偏移量之类的，而不是真实的物理地址。
+ 在信息安全等级保护工作中，根据信息系统的机密性（Confidentiality）、完整性（Integrity）、可用性（Availability）来划分信息系统的安全等级，三个性质简称CIA。
+ 安全的几个原则：
  + 不要给予一个服务或者软件太高的权限，尤其是不能是root权限；<img src="CSE期末复习笔记.assets/image-20211230163213587.png" alt="image-20211230163213587" style="zoom:50%;" />
  + 不要过分信赖一个东西。但是也要有一个基本的信任单元（比如硬件层面）<img src="CSE期末复习笔记.assets/image-20211230163527736.png" alt="image-20211230163527736" style="zoom:50%;" />
  + 人往往是系统中最脆弱最易错的一环（比如钓鱼攻击）我们的威胁模型不能认定用户是完美的，应该假定用户是非常容易犯错的；<img src="CSE期末复习笔记.assets/image-20211230163601598.png" alt="image-20211230163601598" style="zoom:50%;" />
  + CIA三者是很难同时实现的，本身是一个tradeoff。比如你要实现三备份虽然提升了容错，但是降低了安全性；为了提升安全性牺牲了可用性甚至导致系统不可用；要想实现完美的安全本身是很难的，也是非常耗费资源的。实际上我们不需要完美的安全，我们往往只需要提升攻击者的攻击成本，使其大于收益即可。比如我们学到的ASLR、Salting等策略。   <img src="CSE期末复习笔记.assets/image-20211230164111516.png" alt="image-20211230164111516" style="zoom:50%;" />

## Database management system and Data model

### Relational Model

+ Constrain
+ NOT NULL
+ Delete cascade
+ Primary Key + tuple 对应于key-value（**The relational model is a superset of key-value** ）

#### SQL

<img src="CSE期末复习笔记.assets/image-20211221152857994.png" alt="image-20211221152857994" style="zoom:50%;" />

+ 查询顺序对效率影响比较大；

#### Summary

<img src="CSE期末复习笔记.assets/image-20211221153516759.png" alt="image-20211221153516759" style="zoom:50%;" />

#### Many-to-one & One-to-many

+ Consistent style and spelling
+ Ambiguity (e.g., several cities with the same name)
+ Hard for updating. Suppose the city has changed its name 

+ 关系型数据库不适合one-to-many的数据结构。

### Document Model

<img src="CSE期末复习笔记.assets/image-20211221154215967.png" alt="image-20211221154215967" style="zoom:50%;" />

#### Better flexibility

<img src="CSE期末复习笔记.assets/image-20211221154354920.png" alt="image-20211221154354920" style="zoom:50%;" />

+ 如果关系型数据库需要修改schema的话，就必须要重建整张表。

#### Drawbacks

<img src="CSE期末复习笔记.assets/image-20211221154827412.png" alt="image-20211221154827412" style="zoom:50%;" />

### Contrast

<img src="CSE期末复习笔记.assets/image-20211221155001370.png" alt="image-20211221155001370" style="zoom:50%;" />

## DBMS Storage

#### Tuples

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220205006696.png" alt="image-20211220205006696" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220205219027.png" alt="image-20211220205219027" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220205650184.png" alt="image-20211220205650184" style="zoom:50%;" />

#### Page Size

如果page size太大可能无法支持原子性（还记得以前的lecture讲过用小电容实现原子性的方式。）

可以用WAL实现原子性！

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220210052600.png" alt="image-20211220210052600" style="zoom:50%;" />

#### HeapFile

DBMS使用heapfile存储pages。（用大文件组织，如果全是小文件每个都存metadata就会有很大overhead）

#### Storage of Tuples

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220210919022.png" alt="image-20211220210919022" style="zoom:50%;" />

Page is used to store B+Tree nodes

Deletion: 

\#1 may move many data to upon deletion

\#2.how to find the **empty tuple**? 

Slotted page解决了上面的问题。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220211503422.png" alt="image-20211220211503422" style="zoom:50%;" />

使用offset的话进行插入或者删除只需要修改一个元素的offset即可。

#### Denormalize tuples

拆表可以减少空间占用（应对many-to-one的情况），但是会减少locality：必须要是用join操作。

可以通过prejoin提升一部分性能：

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220212037559.png" alt="image-20211220212037559" style="zoom:50%;" />

#### Cache

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220212204178.png" alt="image-20211220212204178" style="zoom:50%;" />

## Database management system and Buffer pool

#### Structure

Tuple被存放在page中，page采用slotted的方式。

![image-20211220084354215](C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220084354215.png)

+ Header中存放tuple的数量和最后一个使用的tuple的offset。
+ Tuple往前涨，Slot array往后涨，当两者相交的时候说明该页已经存满了。
+ 为什么不用index而是用这种方式？因为page一般比较小，使用index很浪费。而log-structed segment很大，因而很适合使用index。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220084806621.png" alt="image-20211220084806621" style="zoom:50%;" />

#### Free pages

需要一个能找到free pages的数据结构：

<center class="half">
    <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220085639721.png" height="300"/>
    <img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220085643206.png" width="300"/>
</center>
page directory需要和data同步，要做到这一点可以使用log（journal），和文件系统是类似的。

#### Cache

为了加速，必须使用cache。而操作系统提供的cache是不可控的，所以：

大多数数据库会使用O_DIRECT避开OS的page cache

DB 尝试避开OS，因为cache的换页是不可控的(MMAP不可行)

DB 用自己的BUFFER POOL

> O_DIRECT (Since Linux 2.4.10)
>        Try to minimize cache effects of the I/O to and from this file.  In general this
>        will degrade performance, but it is useful in special situations, such  as  when
>        applications do their own caching.  File I/O is done directly to/from user space
>        buffers.  The I/O is synchronous, that is, at the completion  of  a  read(2)  or
>        write(2), data is guaranteed to have been transferred.  See NOTES below for 
>        further discussion.

一般如果在Linux内核中读写一个文件，其IO流程都需要经过Kernel内的page cache层次，若想要使用自己开发的缓存系统，那么就可以在打开这个文件的时候，对该文件加以O_DIRECT的标志位，这样一来就可以让程序对该文件的IO直接在磁盘上进行，从而避开了Kernel的page cache，进而对IO流程里的块数据进行拦截，让其流入到自己开发的缓存系统内。

#### BufferPool的结构

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220090731110.png" alt="image-20211220090731110" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220091252504.png" alt="image-20211220091252504" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220091441516.png" alt="image-20211220091441516" style="zoom:50%;" />

被写的page不会立刻被flush到硬盘，而是标上dirty bit表示已经被写了。只有dirty page才会被flush。使用batching来提升效率。

### Lock vs Latches

可能存在这样的情况：在evict一个frame的同时一个请求正好访问这个frame。那么这就涉及到了并发的问题。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220092034059.png" alt="image-20211220092034059" style="zoom:50%;" />

+ 数据库的latch基本对应OS的锁。比如B+树的插入，显然可能会产生分裂等等复杂的变换。如果多个插入请求过来，则必须要上锁。而这个锁又和transaction需要的逻辑上的锁不同，这是底层的数据结构需要的锁。前者我们称之为lock，后者则是latch。

+ Latch 是物理上的锁 不影响用户 logical→lock

+ Latch保证读a的时候a的frame不会被踢掉。读完就放锁
+ 对于B+tree 而言 也要latch
+ Latch是处理internal data structure。对用户没有影响
+ Latch都是in memory
+ 锁(指lock)也可以consistent 加锁这件事可能在一些数据库里面也被计入到log 以便更好地恢复

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220092759246.png" alt="image-20211220092759246" style="zoom:50%;" />

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220092844643.png" alt="image-20211220092844643" style="zoom:50%;" />

#### Multiple Buffer Pool

为什么要多个呢？不同的page可能有不同的access pattern，一个index page可能更长地被访问，index page和data page被访问的模式是不一样的，如果只有一个buffer pool，互相的pattern可能就不能很好地被利用。如果我们分开，不同的data pattern就可以做不同的优化。并且单个buffer pool可能使得竞争更加剧烈，因为buffer pool的全局唯一的latch可能就成为一个大家都在竞争的资源。

+ 一般index会被经常访问，一个buffer pool容易被覆盖。
+ 单个buffer pool容易导致latch竞争。
+ 工业级的数据库都有这一属性。
+ 使用hash或range来分配bufferpool

#### Page Prefetching

+ OS的prefetch是基于物理存储的 而DBMS维护着自己的数据结构(B+Tree) 可以更好地prefetch；
+ OS的prefetch大小和DBMS很可能不匹配。

#### Scan Sharing

–Q1: select * from Items; 

–Q2: select AVG(Price) from Items; 

像如上的两个连续的查询，实际上都会扫描整张表，两者扫描的内容都是相同的。但是显然在Q1完成查询之后，Buffer Pool的内容对Q2不起作用（根据LRU，最前面的数据会被后面的数据覆盖，但是Q2会从最前面的数据开始）

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220130240627.png" alt="image-20211220130240627" style="zoom:50%;" />

Q1会等Q2完成p0 p1 p2，避免p0被无谓地踢出的情况

上面的这种情况非常常见 30%（据统计）

**select \* from Items;**

它是顺序走一遍，但是不需要locality，整个buffer pool做完这个操作之后可能就清空了，这时候我们可能需要MRU（Most Recently Used，刚访问的就evict掉）。

如果我们在看电影的时候做一些需要locality的操作，那么看电影这件事情就会把cache的重要数据冲走。这时候我们应该把cache划分一下，把很少的一部分给电影，剩下的大部分给需要locality的操作。

**方案1：使用多个buffer pool，对于select *中就分配一个很小的buffer pool。**

**方案2：使用别的策略，LRU（k，只在k个范围内做LRU）；或者MRU。**

**方案3：不用buffer pool了。在IBM informix可以使用light scan，只使用一块小内存滑动窗口。**

### Buffer Replacement Policies

**What are the goals of buffer replacements?** 

+ **High** Accuracy

+ **Quick** Speed
+ **Low** Meta-data overhead 

#### LRU

需要额外的metadata（比如队列用于维护最近访问的page的队列）；并且每次访问的时候都需要两次队列操作，第一是把page移出队列，然后再把page加入队列。影响访问速度。

#### Clock Algorithm

Clock Algorithm是对LRU的近似算法。

顺时针转一圈，如果遇到reference bit为1的就置为0，否则就evict该page。

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220130652187.png" alt="image-20211220130652187" style="zoom:50%;" />

#### **Problem of** **sequential flooding**

+ A query performs a sequential scan that reads every page
  + E.g., select * from Items; 

**This pollutes the buffer pool with pages that are read once and then never again**

+ In some workloads the most recently used page is the most unneeded page

像select *这种扫描整张表的语句会造成flooding，污染buffer pool

#### Possible Workarounds

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220130919169.png" alt="image-20211220130919169" style="zoom:50%;" />

##### LRU-K

+ 数据第一次被访问，加入到访问历史列表；
+ 如果数据在访问历史列表里后没有达到K次访问，则按照一定规则（FIFO，LRU）淘汰；
+ 当访问历史队列中的数据访问次数达到K次后，将数据索引从历史队列删除，将数据移到缓存队列中，并缓存此数据，缓存队列重新按照时间排序；
+ 缓存数据队列中被再次访问后，重新排序；
+ 需要淘汰数据时，淘汰缓存队列中排在末尾的数据，即：淘汰“倒数第K次访问离现在最久”的数据。

LRU-K具有LRU的优点，同时能够避免LRU的缺点，实际应用中LRU-2是综合各种因素后最优的选择，LRU-3或者更大的K值命中率会高，但适应性差，需要大量的数据访问才能将历史访问记录清除掉。

### BufferPool Bypass

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220131601209.png" alt="image-20211220131601209" style="zoom:50%;" />

### Query Execution Pattern

#### Priority hints

数据库可以使用query的模式优化对应的剔除/缓存 策略

point query（点查询），index的访问相对随机，顶层节点被访问概率更高，越往下越随机

应该缓存那些位于顶层的节点。

可以给buffer pool提供这些有用的信息。



<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220131900897.png" alt="image-20211220131900897" style="zoom:50%;" />

#### **I/O cost of evicting a frame** 

+ **Problem #1. Slow** 
  + The DBMS must write back to disk to ensure that its changes are persisted 
+ **Problem #2. Consistency**
  + The log of the modified page is not persisted 

<img src="C:\Users\92304\AppData\Roaming\Typora\typora-user-images\image-20211220132031971.png" alt="image-20211220132031971" style="zoom:50%;" />

## Database management system Query execution & Summary

### Model of query plan

<img src="CSE期末复习笔记.assets/image-20211221160234005.png" alt="image-20211221160234005" style="zoom:50%;" />

#### Iterator Model

<img src="CSE期末复习笔记.assets/image-20211221160429298.png" alt="image-20211221160429298" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221160611096.png" alt="image-20211221160611096" style="zoom:50%;" />

#### Materialization Model

<img src="CSE期末复习笔记.assets/image-20211221160753423.png" alt="image-20211221160753423" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221160736464.png" alt="image-20211221160736464" style="zoom:50%;" />

##### 优缺点

+ 适合OLTP，数量有限，效率高（不会像iterator model那样频繁地调用函数。）
+ 不适合OLAP，OLAP通常需要分析所有的数据。

<img src="CSE期末复习笔记.assets/image-20211221160934966.png" alt="image-20211221160934966" style="zoom:50%;" />

#### Vectorized Model

<img src="CSE期末复习笔记.assets/image-20211221161114276.png" alt="image-20211221161114276" style="zoom:50%;" />

和iterator在逻辑上类似，但是返回的是一批数据而不是一个数据。

<img src="CSE期末复习笔记.assets/image-20211221161215798.png" alt="image-20211221161215798" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221161444498.png" alt="image-20211221161444498" style="zoom:50%;" />

### Operator implementations & optimizations

<img src="CSE期末复习笔记.assets/image-20211221161707292.png" alt="image-20211221161707292" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221161827408.png" alt="image-20211221161827408" style="zoom:50%;" />

为了支持范围搜索，适用的是B+树而不是hash。

<img src="CSE期末复习笔记.assets/image-20211221162231059.png" alt="image-20211221162231059" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221162406288.png" alt="image-20211221162406288" style="zoom:50%;" />

+ 注意到这里的multi-index应该是指多个index，而不是复合index。

#### Features

<img src="CSE期末复习笔记.assets/image-20211221213332431.png" alt="image-20211221213332431" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221213342455.png" alt="image-20211221213342455" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221213621311.png" alt="image-20211221213621311" style="zoom:50%;" />

### Tikv

<img src="CSE期末复习笔记.assets/image-20211221214129674.png" alt="image-20211221214129674" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221213952027.png" alt="image-20211221213952027" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221214351289.png" alt="image-20211221214351289" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211221214416076.png" alt="image-20211221214416076" style="zoom:50%;" />

### Summary

<img src="CSE期末复习笔记.assets/image-20211221214512090.png" alt="image-20211221214512090" style="zoom:50%;" />

## **Introduction to Network**

<img src="CSE期末复习笔记.assets/image-20211222095447954.png" alt="image-20211222095447954" style="zoom:50%;" />

| Layer             | Entities                                                     | Namespace   | Protocols                  | What to care                     |
| ----------------- | ------------------------------------------------------------ | ----------- | -------------------------- | -------------------------------- |
| Application layer | Client and server; End-to-end connection                     | URL         | HTTP, FTP, POP3, SMTP, etc | Content of data: video, text...  |
| Transport layer   | Sender and receiver; Proxy, firewall, etc; End-to-end connection | port number | TCP, UDP, etc              | TCP: retransmit data if lost     |
| Network layer     | Gateway, bridge; Router, etc                                 | IP address  | IP, ICMP(ping)             | Next hop decided by route table. |

<img src="CSE期末复习笔记.assets/image-20211222100027477.png" alt="image-20211222100027477" style="zoom: 80%;" />

+ IP datagram

![image-20211222100216796](CSE期末复习笔记.assets/image-20211222100216796.png)

<img src="CSE期末复习笔记.assets/image-20211222100315097.png" alt="image-20211222100315097" style="zoom:50%;" />

### Link Layer



<img src="CSE期末复习笔记.assets/image-20211222101539096.png" alt="image-20211222101539096" style="zoom:50%;" />![image-20211230090339676](CSE期末复习笔记.assets/image-20211230090339676.png)

+ 必须要等收到ack才能发下一条数据，至少为2△t

<img src="CSE期末复习笔记.assets/image-20211222101521619.png" alt="image-20211222101521619" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222101747219.png" alt="image-20211222101747219" style="zoom:50%;" />

#### 周期同步

+ 可以使用VCO（利用锁相环）同步信号和自身的周期。
+ 但是如果都是0或者都是1怎么办（看不出周期来了）

+ Manchester Code:

  <img src="CSE期末复习笔记.assets/image-20211222102226520.png" alt="image-20211222102226520" style="zoom:50%;" />

  ### 电话

  每个用户在每5624 bit times里面占上8个bit，依次顺序并排。



<img src="CSE期末复习笔记.assets/image-20211222102358186.png" alt="image-20211222102358186" style="zoom:50%;" />

### Network

<img src="CSE期末复习笔记.assets/image-20211222102950999.png" alt="image-20211222102950999" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222102919833.png" alt="image-20211222102919833" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222103006657.png" alt="image-20211222103006657" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222103559905.png" alt="image-20211222103559905" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222103652149.png" alt="image-20211222103652149" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222103802825.png" alt="image-20211222103802825" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222104456001.png" alt="image-20211222104456001" style="zoom:50%;" />

### IP

<img src="CSE期末复习笔记.assets/image-20211222104752976.png" alt="image-20211222104752976" style="zoom:50%;" />

现实中采用的是best-efforst这种方式。

<img src="CSE期末复习笔记.assets/image-20211222105008395.png" alt="image-20211222105008395" style="zoom:50%;" />

#### Routing

<img src="CSE期末复习笔记.assets/image-20211222105105815.png" alt="image-20211222105105815" style="zoom:50%;" />

+ Route Table

  <img src="CSE期末复习笔记.assets/image-20211222105135371.png" alt="image-20211222105135371" style="zoom:50%;" />

+ The Control-plane the table, data-plane *reads* the table

  <img src="CSE期末复习笔记.assets/image-20211222105301059.png" alt="image-20211222105301059" style="zoom:50%;" />

## Network Layer All about routing

<img src="CSE期末复习笔记.assets/image-20211222105516534.png" alt="image-20211222105516534" style="zoom:50%;" />

### Routing

<img src="CSE期末复习笔记.assets/image-20211222105602358.png" alt="image-20211222105602358" style="zoom:50%;" />

我们需要构建出这样的一张路由表，其中存储着距离目标最短的路径（比如说我要发送给dst，要保证cost最小那么我应该根据路由表转发给A）

<img src="CSE期末复习笔记.assets/image-20211222105719551.png" alt="image-20211222105719551" style="zoom:50%;" />

#### Routing protocol

+ Link-state 告诉所有的节点自己到邻居的距离，使用dijkstra算法（计算单源最短路径）

  在advertisement中放入自己到其他邻居的距离，然后通flooding的方式传递下去（一传十， 十传百...）

<img src="CSE期末复习笔记.assets/image-20211222105851775.png" alt="image-20211222105851775" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222110236094.png" alt="image-20211222110236094" style="zoom:50%;" />

下面这一块算法都是属于dijkstra的内容，即利用最短的边来松弛更多的边，把这些松弛过的边再纳入考虑范围（使用最小堆）



<img src="CSE期末复习笔记.assets/image-20211222110522796.png" alt="image-20211222110522796" style="zoom:50%;" />

+ distance-vector routing 

  link state求得的是最短路径，但是对于一个网络节点而言，只需要知道下一个传输的节点是哪个就行了，没必要掌握整体的路径。这对于一个很大的网络系统是不可能实现的。

  在advertisement中存放的是一个节点知道的节点以及当前自己到这个节点的花费，初始值为[(self, 0)] 

  只会把advertisement发给自己的邻居，而不是flooding。

<img src="CSE期末复习笔记.assets/image-20211222110956564.png" alt="image-20211222110956564" style="zoom:50%;" />

![image-20211230093023439](CSE期末复习笔记.assets/image-20211230093023439.png)

<img src="CSE期末复习笔记.assets/image-20211222111353133.png" alt="image-20211222111353133" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222111702985.png" alt="image-20211222111702985" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222122811459.png" alt="image-20211222122811459" style="zoom:50%;" />

如果用上面这种方法可能会产生无限循环的问题。如果来了一个network parition， C对B而言是inf，但是A存储了之前到C的距离（1+1=2）。这时候A把advertisement发给B会导致B认为可以通过A到达C。但是A本身是通过B到达C的，这就会导致死循环。A<=>B。

解决这种情况的方法是记住额外的信息，A没有必要把C的距离信息发给B，因为A本身就是从B获知C的存在的。

从一端收到的路由信息，不能再从原路被发送回去。

A是从B认识的C 所以没有必要把C发给B

<img src="CSE期末复习笔记.assets/image-20211222123318505.png" alt="image-20211222123318505" style="zoom:50%;" />

### Summary

| Method              | Pros                                  | Cons                                                         | Summary                          |
| ------------------- | ------------------------------------- | ------------------------------------------------------------ | -------------------------------- |
| **Link-State**      | Fast convergence                      | flooding is costly                                           | **Only good for small networks** |
| **Distance Vector** | Low overhead: 2x #Line advertisements | Convergence time is proportional to longest path; The infinity problem | **Only good for small networks** |

<img src="CSE期末复习笔记.assets/image-20211222124422038.png" alt="image-20211222124422038" style="zoom:50%;" />

![image-20211230093320760](CSE期末复习笔记.assets/image-20211230093320760.png)

### Path Vector

<img src="CSE期末复习笔记.assets/image-20211222124651383.png" alt="image-20211222124651383" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222124906190.png" alt="image-20211222124906190" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222125254516.png" alt="image-20211222125254516" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222125420998.png" alt="image-20211222125420998" style="zoom:50%;" />

类似于distance-vector，但是添加了新的结构。

<img src="CSE期末复习笔记.assets/image-20211222130209207.png" alt="image-20211222130209207" style="zoom:50%;" />

### Hierarchical Routing

<img src="CSE期末复习笔记.assets/image-20211222130253432.png" alt="image-20211222130253432" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222130625921.png" alt="image-20211222130625921" style="zoom:50%;" />

把一个个节点划分到一个region下面，这样forwarding table就不用记住所有的可到达节点，比如可以只记住R2，而不用记住R2下面所有的节点。这样就可以先通过link1到达R2，然后再转移到R2下面的某个节点。

<img src="CSE期末复习笔记.assets/image-20211222130946860.png" alt="image-20211222130946860" style="zoom:50%;" />

同时也带来一部分缺点：如果更换位置与此同时地址也要改变。可能找到的不是最短路径。

<img src="CSE期末复习笔记.assets/image-20211222131356781.png" alt="image-20211222131356781" style="zoom:50%;" />

### Topological Addressing

<img src="CSE期末复习笔记.assets/image-20211222131513279.png" alt="image-20211222131513279" style="zoom:50%;" />

同一地区的ip地址是顺序的。所以可以使用子网掩码（和ip做and位运算，这里的24代表掩码是取前24位（0xffff....ff））

这样可以减少forwarding table的entry的数量，节省空间。同时减少advertisement的大小。

![image-20211230094128050](CSE期末复习笔记.assets/image-20211230094128050.png)

### Forwarding an IP Packet

<img src="CSE期末复习笔记.assets/image-20211222133459664.png" alt="image-20211222133459664" style="zoom:50%;" />

不可以通过网卡发给自己，通过网卡的话就只能往外发。

<img src="CSE期末复习笔记.assets/image-20211222132633101.png" alt="image-20211222132633101" style="zoom:50%;" />

TTL = 64 就是这个包最多被 64 个人转发，为什么要这样呢？防止不小心出现了 loop， 这样包就一直占用着资源。还要更新 header checksum。因为我们 TTL 改了，checksum 也要 该，这就是一个写操作。整个这个操作，Linux kernel 提供了一个很好的转发机制，但是它不够快。

<img src="CSE期末复习笔记.assets/image-20211222132741783.png" alt="image-20211222132741783" style="zoom:50%;" />

转发代码位于kernel中，是否可以写一个用户态的转发？

轮询。

### NAT（Network Address Translation）

+ 为了解决网络IP不够的问题。
+ NAT网关，内网和外网连接的桥梁。

<img src="CSE期末复习笔记.assets/image-20211222134541721.png" alt="image-20211222134541721" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222134731824.png" alt="image-20211222134731824" style="zoom:50%;" />



虽然破坏了层级（port是TCP上的概念），但是这在工程上的应用十分有效。

局限性，如果不用TCP怎么办？Port还能生效吗。

容量有限，如果连接特别多，这张表就会特别大，导致卡顿。

内网的设备访问外网，NAT为设备分配一个端口，加入一个相应的entry到表中。

可以绑定端口。

### Ethernet

<img src="CSE期末复习笔记.assets/image-20211222141153126.png" alt="image-20211222141153126" style="zoom:50%;" />

当年没竞争过IP协议，沦为局域网协议。

<img src="CSE期末复习笔记.assets/image-20211222141420556.png" alt="image-20211222141420556" style="zoom:50%;" />

Hub，一堆节点连接到hub上，共享；Switch星形。

<img src="CSE期末复习笔记.assets/image-20211222141712126.png" alt="image-20211222141712126" style="zoom:50%;" />

可以broadcast一个包。收包之后往上层抛。

<img src="CSE期末复习笔记.assets/image-20211222142552026.png" alt="image-20211222142552026" style="zoom:50%;" />

#### ARP协议

<img src="CSE期末复习笔记.assets/image-20211222144301476.png" alt="image-20211222144301476" style="zoom:50%;" />

ARP ip adress to mac address

+ ARP和RARP完成 IP address和 MAC address之间的相互转换。

  <img src="CSE期末复习笔记.assets/image-20211222144416263.png" alt="image-20211222144416263" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211222144334015.png" alt="image-20211222144334015" style="zoom:50%;" />

#### ARP Spoofing

<img src="CSE期末复习笔记.assets/image-20211222145237223.png" alt="image-20211222145237223" style="zoom:50%;" />

由于ARP协议是等着别人告知自己ip地址。如果有人冒充告诉你一个假的地址怎么办？

<img src="CSE期末复习笔记.assets/image-20211222145403582.png" alt="image-20211222145403582" style="zoom:50%;" />

本来是发给B的，结果发给了Hacker。

ARP广泛使用，很难更新，很难处理上述的问题。

解决方案：

+ 静态ARP表
+ 监听是否有人恶意操作

## End-to-end Layer

<img src="CSE期末复习笔记.assets/image-20211223212706647.png" alt="image-20211223212706647" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211223211557029.png" alt="image-20211223211557029" style="zoom:50%;" />

![image-20211230125501030](CSE期末复习笔记.assets/image-20211230125501030.png)

### At least once

<img src="CSE期末复习笔记.assets/image-20211223211914821.png" alt="image-20211223211914821" style="zoom:50%;" />

+ Fixed timer不是一个好选择

<img src="CSE期末复习笔记.assets/image-20211223211958672.png" alt="image-20211223211958672" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211223213155644.png" alt="image-20211223213155644" style="zoom:50%;" />

+ **Adaptive timer**

  + E.g., adjust by currently observed RTT, set timer to 150% 

    设置为RTT的150%，但是注意到RTT在不断波动。

  + Exponential back-off: doubling from a small timer

    即1s， 2s， 4s...

![image-20211223220039320](CSE期末复习笔记.assets/image-20211223220039320.png)

+ **Linux code**

![image-20211223220247899](CSE期末复习笔记.assets/image-20211223220247899.png)

误判超时的情况非常常见。

+ **NAK (Negative AcKnowledgment)**

  + Receiver sends a message that lists missing items

  + Receiver can count arriving segments rather than timer

  + Sender can have no timer (only once per stream)

只发回没有收到的包，这样就能确定某些包没有被收到。

实际上还是要考虑很多丢包问题。

### At most once

![image-20211223220929898](CSE期末复习笔记.assets/image-20211223220929898.png)

注意，at most once需要维护额外信息（已经请求过了就不需要再做一次），更好的方法是使用幂等。

+ **Duplication Suppression**

![image-20211223221137376](CSE期末复习笔记.assets/image-20211223221137376.png)

+ 维护一个increasing number。拒绝比该number小的请求。

+ 或者使用新端口。
+ 现在老的nounce和port都不能删除，变成了tombstone。事实上要保证这一点必须要存储额外的信息。

![image-20211223222031215](CSE期末复习笔记.assets/image-20211223222031215.png)

要么存额外信息，要么约定好超过一定时间不再重试，删除相关信息。不管怎么样都会给系统带来一定的复杂度。

### Data integrity

![image-20211223222415046](CSE期末复习笔记.assets/image-20211223222415046.png)

使用checksum。

虽然网络连接层已经提供了hamming code（只能解决一位出错的情况），可以通过sender加入checksum，receiver验证checksum的方式来保证数据的完整性。但是该方法不能保证包不会被传输错地方。

### Segments and Reassembly of Long Messages

![image-20211223222933366](CSE期末复习笔记.assets/image-20211223222933366.png)

预留一个buffer，保存数据。

![	](CSE期末复习笔记.assets/image-20211223223019907.png)

![image-20211223223420552](CSE期末复习笔记.assets/image-20211223223420552.png)

Sol2可能会导致buffer特别长。

Sol3解决了如上的问题。

### Jitter Control

![image-20211223223439328](CSE期末复习笔记.assets/image-20211223223439328.png)

![image-20211223223449748](CSE期末复习笔记.assets/image-20211223223449748.png)

最长-最短/平均

### Authenticity and Privacy

![image-20211223224026000](CSE期末复习笔记.assets/image-20211223224026000.png)

用公钥私钥的非对称加密交换一把对称的秘钥，然后仅仅使用这个秘钥进行通信，对称加密。

![image-20211224201515480](CSE期末复习笔记.assets/image-20211224201515480.png)

发包速度与信息传输的稳定性是一个tradeoff。

如上的方式能够保证准确性，但是效率不是很高，大量的时间都在等待ACK。

![image-20211224201744040](CSE期末复习笔记.assets/image-20211224201744040.png)

使用流水线的方，不停地收发。发包和收包无关，这是太快的情况，会导致大量丢包。

![image-20211224201904248](CSE期末复习笔记.assets/image-20211224201904248.png)

+ Fixed window

![image-20211224201913401](CSE期末复习笔记.assets/image-20211224201913401.png)

把发单个转换为一次发多个。上面的情况是window size为3的情况。等三个包都ACK了，再发下一组。但是这种情况存在一段时间的idle，可以优化：使用**滑动窗口**！

并不是等三个都收到再发下一组，而是受到一个ACK的时候就马上发下一个包，即移动滑动窗口一位。

![image-20211224202456979](CSE期末复习笔记.assets/image-20211224202456979.png)

但是可能会出现如下的情况，因为一直没有收到2的ACK，导致窗口“卡”住了。TCP为了尽快解决这种问题，使用了duplicate ACK的方式（一直发同样的ACK，sender可以从中发现某些包发生了丢包）

![image-20211224203036699](CSE期末复习笔记.assets/image-20211224203036699.png)

#### Tradeoff

![image-20211224203801190](CSE期末复习笔记.assets/image-20211224203801190.png)

#### Sliding Window Size

![image-20211224203814652](CSE期末复习笔记.assets/image-20211224203814652.png)

保证尽可能多包在传输过程中。RTT正好是发送第一个包到第一个包ACK的时间，在此期间都可以发包，最多可以发RTT*Data rate个包。这样第一个包ACK的时候窗口就可以右移一位，发送新的一个包。

![image-20211224204118900](CSE期末复习笔记.assets/image-20211224204118900.png)

Receiver Sender相互制约。

#### Congestion

![image-20211224204613644](CSE期末复习笔记.assets/image-20211224204613644.png)

![image-20211224205038101](CSE期末复习笔记.assets/image-20211224205038101.png)

一个更大的buffer并不能解决问题，反而可能会让问题更糟。

<img src="CSE期末复习笔记.assets/image-20211230131643990.png" alt="image-20211230131643990" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211230131750219.png" alt="image-20211230131750219" style="zoom:50%;" />

#### 修改Window Size

![image-20211224205336852](CSE期末复习笔记.assets/image-20211224205336852.png)

线性增长，指数下降。

![image-20211224205458373](CSE期末复习笔记.assets/image-20211224205458373.png)

存在的问题：一开始线性增长过于缓慢。"slow start"一开始以指数级增长。

![image-20211224205607018](CSE期末复习笔记.assets/image-20211224205607018.png)

![image-20211224205837485](CSE期末复习笔记.assets/image-20211224205837485.png)

![image-20211224205842919](CSE期末复习笔记.assets/image-20211224205842919.png)

检测到duplicate ack就减少一半， timeout直接window size清零，slow start again。

数据中心会保证这个锯齿在一个水平线来回震荡（积分一下面积更大。）

对于如下的情况，如果两个sender的window size相加大于10，那么就可能会导致拥塞。下面证明TCP是公平的。

![image-20211224210453119](CSE期末复习笔记.assets/image-20211224210453119.png)

![image-20211224210422271](CSE期末复习笔记.assets/image-20211224210422271.png)

![image-20211224215532708](CSE期末复习笔记.assets/image-20211224215532708.png)

对于局域网这种场景，容易出现信号不好，这时候应该加大window size才对；对于数据中心而言并不是很友好。DCTCP

![image-20211224215858711](CSE期末复习笔记.assets/image-20211224215858711.png)

## The Design of DNS

![image-20211224221123769](CSE期末复习笔记.assets/image-20211224221123769.png)

![image-20211224221143849](CSE期末复习笔记.assets/image-20211224221143849.png)

![image-20211224221304490](CSE期末复习笔记.assets/image-20211224221304490.png)

需要有一些nameservers去提供name service。肯定不能都放在一台服务器上。

### DNS hierarchy

root只会管下面这几个domain，这些domain很少变化（不然撑不住）；

通过这种组织方式很好地进行了分工，避免了某个节点成为瓶颈。

![image-20211224221444701](CSE期末复习笔记.assets/image-20211224221444701.png)

![image-20211224221553079](CSE期末复习笔记.assets/image-20211224221553079.png)

从root开始查找。对于domain而言是反过来的。

![image-20211224222317673](CSE期末复习笔记.assets/image-20211224222317673.png)

### 优化

如果所有人的look都直接去问root， root肯定会挂。应该能够去问任何的DNS server。(拿到IP的时候会分配一个DNS)

![image-20211224223039611](CSE期末复习笔记.assets/image-20211224223039611.png)

递归搜索。DNS服务器会去做递归。DNS服务器速度会快一点，并且有缓存。

![image-20211224223455651](CSE期末复习笔记.assets/image-20211224223455651.png)

![image-20211224223504391](CSE期末复习笔记.assets/image-20211224223504391.png)

![image-20211224223744951](CSE期末复习笔记.assets/image-20211224223744951.png)

假如cache是24小时，那么如果要更换IP，必须要把旧的IP保留24小时，把访问旧IP的全部重定向到新IP。24小时后旧的IP的cache就都过期了。

![image-20211224224123177](CSE期末复习笔记.assets/image-20211224224123177.png)

![image-20211224224157012](CSE期末复习笔记.assets/image-20211224224157012.png)

内外网都需要DNS 内网映射到内网ip 外网映射外网IP

![image-20211224224209593](CSE期末复习笔记.assets/image-20211224224209593.png)

ISP 互联网服务提供商

### Behind the DNS

#### Good points

去中心化。

![image-20211224224752831](CSE期末复习笔记.assets/image-20211224224752831.png)

![image-20211224224859114](CSE期末复习笔记.assets/image-20211224224859114.png)

![image-20211224224906684](CSE期末复习笔记.assets/image-20211224224906684.png)

#### Bad points

![image-20211224224925893](CSE期末复习笔记.assets/image-20211224224925893.png)

DoS，不断cache miss（找一个不存在的东西）

![image-20211224225110378](CSE期末复习笔记.assets/image-20211224225110378.png)

attacker让大量流量发送到某台机器上。

![image-20211224225328307](CSE期末复习笔记.assets/image-20211224225328307.png)

![image-20211224225352838](CSE期末复习笔记.assets/image-20211224225352838.png)

Only part of the zones are using DNSSEC, e.g., **.*gov***, ***.org***

## Decentralized Systems: CDN & P2P

![image-20211226125614622](CSE期末复习笔记.assets/image-20211226125614622.png)

### CDN(Content Distribution Network)

![image-20211226125806261](CSE期末复习笔记.assets/image-20211226125806261.png)

CDN对用户透明，主动分发数据。

![image-20211226125917537](CSE期末复习笔记.assets/image-20211226125917537.png)

![image-20211226130633197](CSE期末复习笔记.assets/image-20211226130633197.png)

Client需要两次connect，很耗时。

![image-20211226130655552](CSE期末复习笔记.assets/image-20211226130655552.png)

![image-20211226130724513](CSE期末复习笔记.assets/image-20211226130724513.png)

DNS有cache，第二次访问基本没有overhead。

多中心 中心化的控制面 去中心化的数据面 对用户而言是去中心化的。

数据部署在cluster中

![image-20211226142955836](CSE期末复习笔记.assets/image-20211226142955836.png)

+ client去服务器要一张图片（一般大厂都会买各个地区的CDN，加快访问速度），一般不会给服务器自己的图片地址，而是会给一个cdn的地址，cachexxxx
+ client拿到这个域名，去找DNS要，DNS递归找找找，找到Akamai的DNS服务器（DNS也可以只做名字和名字的映射（即alias，别名）），一路转发到真的地址——一台靠近client的，位于Akamai cluster中的机器。
+ 所以CDN是基于DNS的。

### P2P

+ 中心化的弊端：

![image-20211226135934120](CSE期末复习笔记.assets/image-20211226135934120.png)

![image-20211226140926209](CSE期末复习笔记.assets/image-20211226140926209.png)

![image-20211226140856282](CSE期末复习笔记.assets/image-20211226140856282.png)

通过中心化的方式拿到torrent文件，之后的事情就是去中心化的了。

Tracker记录谁拥有文件的某个部分；

Seeder就是拥有文件的人；

Peer一旦拥有完整的文件就会变成seeder。

就像播种一样传播开来，seeder会越来越多。（peer从最靠近的地方下载seeder的文件，然后自己变成seeder依次循环）

Tracker是中心化的，告诉用户种子的list。（这将会成为瓶颈）

![image-20211226141541617](CSE期末复习笔记.assets/image-20211226141541617.png)

![image-20211226141807181](CSE期末复习笔记.assets/image-20211226141807181.png)

Strict：按照严格的顺序下载；

Rarest First：拥有人数少的先下载；

默认的策略：第一个随机下；之后选择rarest的下；

**torrent**文件对应于一个**tracker**，也就是说我们拿到这个种子文件之后就能找到**tracker**，再根据**tracker**提供的信息找到**seeder**，把**seeder**用有的文件都下载下来最后拼接成一个完整的文件。

#### 弊端

注意到，上面的方式依旧存在中心化的因素：Tracker成为了瓶颈！

如何解决：使用分布式哈希表。把tracker的表的存储也分布式存储。

![image-20211226142014100](CSE期末复习笔记.assets/image-20211226142014100.png)

![image-20211226144559890](CSE期末复习笔记.assets/image-20211226144559890.png)

![image-20211226144909939](CSE期末复习笔记.assets/image-20211226144909939.png)

把hash表分布式存储，避免一个节点成为bottletneck。

#### Chord

![image-20211226144917452](CSE期末复习笔记.assets/image-20211226144917452.png)

![image-20211226145013006](CSE期末复习笔记.assets/image-20211226145013006.png)

##### Finger table

![image-20211226150407099](CSE期末复习笔记.assets/image-20211226150407099.png)

增加存储空间 O(1)(1 successor) =>O(logn) 记住1/2 ¼ 1/8…… 

以空间换时间，通过logn的时间复杂度找到对应的节点。

万一fingertable上的节点fail了怎么办？保存一个successor list尽量避免这种情况下的错误。

![image-20211226151330040](CSE期末复习笔记.assets/image-20211226151330040.png)

![image-20211226151152885](CSE期末复习笔记.assets/image-20211226151152885.png)

##### 一致性hash

节点fail的问题：

最多只会有一个节点需要进行数据的迁移。

![image-20211226151631886](CSE期末复习笔记.assets/image-20211226151631886.png)

K30 分给 N36 N25的successor转而指向N36

![image-20211226151702647](CSE期末复习笔记.assets/image-20211226151702647.png)

##### 虚拟节点

![image-20211226151735893](CSE期末复习笔记.assets/image-20211226151735893.png)

+ 一台物理机 分为若干个虚拟节点

+ 虚拟节点让负载更加均匀。

+ 增加虚拟节点以分配更多的负载 动态调整负载（如果一个机器的负载一直比较小（大），那么可以动态地增加（减少）虚拟节点的数量）

+ 让hot的节点平摊到物理节点上。总而言之就是让负载更加均衡。 

### Bitcoin

![image-20211226152404574](CSE期末复习笔记.assets/image-20211226152404574.png)

+ Hash 值必须保证前100位为0（这个100是可以调节的，随着算力增长而变大，始终保证大约10分钟出一个区块） 需要花大量时间获取随机数保证这一点——Proof of work
+ 每10分钟出一个block

![image-20211226152515581](CSE期末复习笔记.assets/image-20211226152515581.png)

必须要做到不可篡改。

![image-20211226152541673](CSE期末复习笔记.assets/image-20211226152541673.png)

矿工把鉴定为合法的交易记录积累到block里，当block到达一定的规模就把block连到区块链里（存一个地址（把前面的block算一个hash值）链接上一个block），全球就一个chain。一旦篡改，hash就会变，下一个block就不会指向自己，所以保证了不会被篡改。

如果block太多了，就会导致一台机器存不下block chain。这样的话掌握全貌的人就会是一台data center！注意，那可是中心化的，和bitcoin的理念背道而驰。

![image-20211226153516447](CSE期末复习笔记.assets/image-20211226153516447.png)

<img src="CSE期末复习笔记.assets/image-20211226153541895.png" alt="image-20211226153541895" style="zoom: 50%;" />

只有最长的链才会“赢”

<img src="CSE期末复习笔记.assets/image-20211226154505797.png" alt="image-20211226154505797" style="zoom: 80%;" />

只要修改一个就会导致后面的全部都需要修改，仿佛多米诺骨牌一般，这就大大加大了篡改的难度。

![image-20211226154604087](CSE期末复习笔记.assets/image-20211226154604087.png)

由掌握51%的人创建一个新的链？删掉一个区块然后迅速算一个chain，这样还是能够赢过其他人。根据比特币的协议，只会选择最长的链（同一时刻有很多的链（因为很多人在挖矿））

![image-20211226161314073](CSE期末复习笔记.assets/image-20211226161314073.png)

![image-20211226213914718](CSE期末复习笔记.assets/image-20211226213914718.png)

+ 比特币持有者拥有私钥，其他人必须拿到私钥才能进行交易；
+ 如果私钥丢了，那就没办法了；
+ 不会通货膨胀；而会通货紧缩。总有人会丢弃比特币，只会越来越少。
+ 是否是decentralized？不能算是完全去中心化。75%算力在中国（2019）
+ 是否需要PoW？浪费能源。但是用其他方法可能会带来风险。
+ 国内ban了，别想了

![image-20211226214515308](CSE期末复习笔记.assets/image-20211226214515308.png)

区块里存储着一块空间，可以存一段脚本。

智能合约，不需要中心的法规合同？

![image-20211226214928220](CSE期末复习笔记.assets/image-20211226214928220.png)

![image-20211226215235385](CSE期末复习笔记.assets/image-20211226215235385.png)

![image-20211226215242084](CSE期末复习笔记.assets/image-20211226215242084.png)

![image-20211226215248893](CSE期末复习笔记.assets/image-20211226215248893.png)

相信则有价值；需要一个分布式的分类账簿。

![image-20211226215255878](CSE期末复习笔记.assets/image-20211226215255878.png)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           

中国拥有75%的算力，让bitcoin失信？

## Security Intro

**Buffer** **overflow** **attack** **(stack/heap)**

**ROP** **attack** **(Return-Oriented** **Programming)**

**Password** **attack**

**Phishing** **attack** 

**XSS** **attack** **(Cross** **Site** **Script)**

**SQL** **injection** **attack**

**Integer overflow attack** 

**Social** **engineering** **attack** 

**Side-channel attack**

<img src="CSE期末复习笔记.assets/image-20211230162358260.png" alt="image-20211230162358260" style="zoom:50%;" />

安全是一个negative goal，对于positive goal，比如我是否能够读某个文件是很容易的，只需判定权限即可；而对于某人不能读取某个文件这样的negative goal是很难的，因为绕过authentic读取文件的方法是无穷无尽的，只有我们想不到，没有hacker们做不到。

#### Thread Model

<img src="CSE期末复习笔记.assets/image-20211227223527660.png" alt="image-20211227223527660" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211227224054228.png" alt="image-20211227224054228" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211227224102186.png" alt="image-20211227224102186" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211227224114199.png" alt="image-20211227224114199" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211227224153210.png" alt="image-20211227224153210" style="zoom:50%;" />

它有两个基本的要素，你要有一扇门，门口要站一个保镖。我们要求这是一个complete mediation，换句话说，旁边不能有一扇窗导致能够绕过这个保镖。以之前的访问.txt文件为例，我们能不能把访问它变成一种方法，有一扇门我们就可以更好地保护它。

+ 认证：判断身份，这个人是谁？
+ 授权：根据身份判断其权限，并给某个principal赋予某个身份的能力。

几个Guard Model的例子

![image-20211228214200714](CSE期末复习笔记.assets/image-20211228214200714.png)

+ 文件系统：Kernel会为每一个进程保留一个userid，process的userid是怎么得到的呢？在登录的时候我们会输入账号密码，然后OS会判断账号密码，给你这个shell。后面的所有进程都是从登录的shell进程fork出来的，所以都有这个user id。

  通过对进程的userid的检查，在加上文件inode中记录的权限位，就完成了安全的访问控制。

  ![image-20211228214302521](CSE期末复习笔记.assets/image-20211228214302521.png)

![image-20211228214324912](CSE期末复习笔记.assets/image-20211228214324912.png)

+ 防火墙建立在公司外网出口的地方，client就是所有在外网的地方，它有一个端口的表来表明哪些端口是可以通过的。

问题：

+ **软件bug导致有些数据可以绕开complete mediation**

+ 有些内存可以绕开OS防御

+ **User会犯错，我们一定要考虑人在环路（human in the loop，人是系统中的一部分，而最容易出错的地方就是human）**

+ 安全等级

几个措施：

+ 内核代码越长，潜在的bug就越多。应该让内核的体量尽可能的小，这就是所谓的微内核的概念。事实上业界没有很好的衡量代码安全性的指标，代码数往往是迫于现实的一种相对有效的衡量方式。—— **Reduce lines of codes**
+ 永远不要使用root权限登录。 —— **"The principle of least privilege"**
+ 网页URL隐含的安全问题：**https://my.paymaxx.com/get-w2.cgi?id=1234** 像这样的URL存在着安全隐患：如果用户把id改成别人的，那么就能获得其他人的账单。这是05年发生的真实事情，其本质上就是没有做安全认证鉴定用户的权限。
+ SQL注入：一种防范方式，使用一个很长的正则表达式将危险的单双引号过滤。字符串解析是和安全最相关的地方，哈哈哈，log4j那些用爱发电的程序员好惨啊，log4shell。。。

### 认证

+ **timing attack**(Guess one character at a time)：

  ![image-20211228220043941](CSE期末复习笔记.assets/image-20211228220043941.png)

  即使是上面这段看起来平常无奇没什么问题的代码也可能存在着被攻击的风险。如果攻击者掌握了内存的布局，那么他就能让密码的第一位位于4K页面的边界，那么一旦第一位匹配成功就会去匹配下一个字符，这就会因为缺页而触发一次page fault。page fault大概需要几百个周期，这对于攻击者而言是可以察觉到的。通过上述的方式，攻击者能够一位一位地拿到密码。攻击者只需要把密码放在文件中，mmap到内存中。攻击者也可以去malloc一块内存，拼命使用其他内存把这两块先swap到硬盘上，再把一块swap到内存里。

  ![image-20211228220442286](CSE期末复习笔记.assets/image-20211228220442286.png)

  timing attack越来越多。

+ 解决方案：不存储密码的明文，而是存储密码的hash。这样一来，即使攻击者偷取了密码，也只是获得了一个加密过的值，这种hash往往是不可逆的，所以很难通过逆向破解获取代码。但是。。。彩虹表解决了这样的问题。事实证明，大多数人的密码都是比较简单的，简单到可以用一张彩虹表来记录。

  用户并不擅长设置密码，攻击者可以预先计算这些常用密码的hash值，然后根据攻击获得的hash值在这张表上查表就能获得真实的密码。

  ![image-20211228220905300](CSE期末复习笔记.assets/image-20211228220905300.png)

+ 解决方案x2：加盐（Salting）！（用很小的代价大大增加攻击难度）

  <img src="CSE期末复习笔记.assets/image-20211228221541802.png" alt="image-20211228221541802" style="zoom:50%;" />

  

  <img src="CSE期末复习笔记.assets/image-20211228221552585.png" alt="image-20211228221552585" style="zoom:50%;" />

攻击者要想拿到真正的密码，他必须：先用timing attack获取加过密的明文（注意这是加过盐的，即和salt异或的，而salt是不公开的），这时候原先的彩虹表并不适用了。攻击者还必须要窃取salt，并构建一张新的彩虹表，这样的攻击成本未免也太高了。

+ Session Cookie：

  由于攻击者没有Cookie中的server_key，所以无法伪造cookie。

  

  ![image-20211228222034803](CSE期末复习笔记.assets/image-20211228222034803.png)

  这也存在一些问题：Ben22-May-2012的问题，如下所示。所以应该设定一个统一的准确的格式。

  ![image-20211228222136584](CSE期末复习笔记.assets/image-20211228222136584.png)

+ 钓鱼攻击 Phising

  把用户引导到一个钓鱼网站上，诱导用户输入密码。虽然hash有一定的防范作用，但是由于用户还是要把密码的明文发给服务器进行验证。这就是问题所在：如何不发送密码的明文？

+ 解决方式：使用**challenge-response scheme**

  ![image-20211228222822823](CSE期末复习笔记.assets/image-20211228222822823.png)

  服务器给客户端一个随机数，客户端根据这个随机数和自己的密码算一个hash，Server把这个hash值和自己算出来的值进行比较，如果相等的话就身份验证成功。注意到这种情况下传递的是一个hash值而不是密码的明文。

  ![image-20211228223110688](CSE期末复习笔记.assets/image-20211228223110688.png)

**Adversary only learns H(valarMorghul1s|458643); can not recover the password from that** 

+ 用户验证服务器的身份（只有真正的服务器拥有密码！个人感觉这有点像那种正版书籍提供的刮刮乐一样的防伪标签）

  ![image-20211228223313602](CSE期末复习笔记.assets/image-20211228223313602.png)

  ![image-20211228223355503](CSE期末复习笔记.assets/image-20211228223355503.png)

  但是在这个情况下，存在如下的一种攻击：

  + 尝试登录服务器，得到服务器的challenge R。

  + 再和服务器说想测试服务器，把服务器返回的R发给服务器让它去做验证。

  + 把服务器返回的H(R+password)返回给服务器作为challenge-response scheme的结果。

  这就等于左手倒右手，利用服务器来破解密码，所以这两个机制不能在一起同时使用。

  <img src="CSE期末复习笔记.assets/image-20211228224118399.png" alt="image-20211228224118399" style="zoom:50%;" />

+ ![image-20211228224226367](CSE期末复习笔记.assets/image-20211228224226367.png)

  使用不同的密码

+ 一次性密码

  ![image-20211228224308084](CSE期末复习笔记.assets/image-20211228224308084.png)

  把时间因素加入请求中。

  ![image-20211228224320791](CSE期末复习笔记.assets/image-20211228224320791.png)

+ 把认证和请求权限绑定在一起。

  现实中一般不使用密码，而是使用session token。可以和一次性密码相结合。

  <img src="CSE期末复习笔记.assets/image-20211228224711531.png" alt="image-20211228224711531" style="zoom:50%;" />



+ FIDO：**Replace the Password**

![image-20211228225242362](CSE期末复习笔记.assets/image-20211228225242362.png)

这个设备保存了你的私钥，当我们登录一个网站的时候，它如果支持FIDO协议，它就会检查你电脑上是否插入了这个小设备。

![image-20211228225541553](CSE期末复习笔记.assets/image-20211228225541553.png)

通过这个小设备实现指纹和私钥的绑定，而在server端实现用户名和公钥的绑定。通常设备端的安全性会更加重要一些。我们今天的手机在识别指纹和人脸的时候，数据不是放在IOS和Android里的，我们今天的手机里有一个额外的OS，叫做TrustZone，只有当我们需要识别身份的时候，它才会启动。支付宝会放入一个trust application，要求TrustZone做一个认证检查。如果我们不用TrustZone，而是直接把数据放在IOS/Android，这就意味着只要有一个应用有root权限，它就可以无限模拟支付的流程。

密码修改其实是一个非常危险的事情，今天修改密码已经比以前安全很多了，比如给邮箱发送一个URL，再点进去，但是如果我们邮箱被人攻击了，那就没有办法了。服务器在生成reset password url的过程中，到底是怎么生成这个url呢？它里面一定要包含用户名+多长时间过期，如果攻击者知道生成url的算法，那它一个自己生成这个url。所以url里面也要放一个随机数，专门有一个论文调查各大网站的重置密码url是怎么生成的。里面用到了/dev/random里的一个随机数。键盘按了多少次，鼠标产生了多少次中断，有多少个网络包。Cloudflare甚至使用熔岩灯产生随机数。

Cost往往会和其他属性形成tradeoff，比如fault tolerant，3备份提升容错性的同时降低了安全性。

## 攻击者如何偷走我们的数据

### KeyLogger

![image-20211228230013952](CSE期末复习笔记.assets/image-20211228230013952.png)

输入法监听、键盘输入监听。通过钓鱼网站获取密码。

### MemScan

获取内存信息。注意在交换密钥的时候，需要用到大质数。那么我们在扫描的时候比较容易可以发现这个模式。可能这个质数就是用来做密钥生成的。如果我们还知道用户名等，我们一下子可以在内存中找到用户名字符串所在的位置，紧跟着的这块区域可能就是我们的密码。

因为应用程序开发的时候，**默认内存是安全**的，过于依赖OS提供的进程间的隔离机制，而进程间隔离依赖的是虚拟内存。**一旦我们的物理内存泄露了，那么虚拟内存建立的隔离机制就不复存在了。**

### SceenCapture

在输入密码的时候最后一位往往会显示一会然后再变成一个圆点，如果有个app具有截图的权限，就有可能获取用户的密码。

![image-20211228230557736](CSE期末复习笔记.assets/image-20211228230557736.png)

### Cold-Boot

冷冻的时候内存会过几秒上面的bit才会完全掉电。

### Side-Channel

根据陀螺仪猜出密码。按每个键的时候陀螺仪有个大概的模式。

### TaintTracking

它的思路是说，既然要做安全，我们要定义要保护什么数据，并不是所有的数据都不是一样重要的。比如我们装了一个网上下载的App，需要200M空间。可以公开获得的数据和代码是没有保护价值的。而在App使用过程中使用的账号密码就是需要保护的。

对于需要保护的**sensitive data**，它的**生命周期应当最小化**。因为它在内存中的时间越长，它就越容易被攻击。因为在内存中可能发生Swap，因为OS并不知道我们的数据是重要的还是不重要的数据，所以OS在不知道数据关键等级的情况下，把一个包含密码的内存交换到了磁盘，那么我们只要把磁盘偷走，我们就得到了密钥。除了swap之外，还有hibernation（休眠），当我们休眠的时候，一部分数据会被写到磁盘上，防止断电以后数据就没了。

休眠分为两种模式：

+ 比较快，合上笔记本再打开立马显示出来。这个时候的数据还在内存中。

+ 慢一点，需要等一会儿才能进入到我们合上笔记本之前的状态，此时的数据就需要从硬盘上恢复进内存中。

如果我们可以把关键数据**染上颜色**，告诉OS当我们写数据的时候，看到有颜色的数据就不要swap或者加密再写磁盘，这样我们的数据就不会泄露。

比如我们可以把密码染色，当我们在发网络包的时候，我们看到了尝试发送一个染色的sensitive data，那么这个时候我们就可以禁止住。我们也可以把信用卡号打上标签，禁止一个应用程序把信用卡号发给另一个应用程序。

Q：现在假设我们是写一个恶意应用程序的人，我们也知道了拿到的信用卡号打上了一个标签，我们怎么样才能把这个标签删掉呢？因为颜色本身也是一种数据，肯定有一种方式把颜色编码进来，这个数据就叫做**Taint**。data和taint之间就会联系在一起。我们作为攻击者来说，怎么样才能把这个数据安全地发给别人呢？

A：把有标签的数据，打一个压缩包，再拆成十份发送，尝试破坏原先data+taint的格式。

但是taint之所以叫做data flow tracking，就是因为当我们打上压缩包并且拆分成10个包以后，这十个压缩包都会被染上颜色。任何一个压缩包出去的时候都会触发警报。所以那么我们怎么样在这个过程中让颜色数据和原先的数据非常紧密地联系在一起，让任何操作都不能拆分开它们。



### Dynamic Taint Analysis

<img src="CSE期末复习笔记.assets/image-20211228230815751.png" alt="image-20211228230815751" style="zoom:50%;" />

假如我们现在有一个关键数据i，为了跟踪i在程序中是怎么流动的，我们需要维护一个表格。在运行第一行的时候，我们就会记录下来第一行：i = 6, taint = True。taint = True代表这是一个关键数据被我们记录下来了。

所以通过这个最终，我们就会发现ijl都是关键数据，当它要做一些发送关键数据的时候，我们是可以最终判断出来。

但是这个方法并没有考虑到控制流的问题，如果i出现在if语句的条件中，每次至少都可以判断出关于i的1 bit的信息。

不过正常代码的数据流是比较正常的，可以对绝大部分的应用使用taint的方案。

Q：那么到底是谁来做表格的维护呢？当我们去做j = i + two;的时候，为什么会有表格的更新呢？

A：OS吗？难道每次运行一行指令的时候要触发一个syscall吗？

其实taint的维护和更新是在编译器从源代码生成汇编代码的时候维护的。拿“j = i + two;”为例，此时编译器产生汇编指令的时候就不仅仅是要把两个寄存器相加，还需要取出i和two各自的taint做一个or操作，再放进j的taint中。并且为了实现这个算法，必须在变量附近的位置再空出64 bit来存放变量对应的taint。所以可想而知生成的汇编代码的效率是很低的，在实际过程中可能会慢10倍~50倍，因为除去额外的汇编行数不说，还可能把原先寄存器中的计算操作变成访存操作。

![image-20211228230920098](CSE期末复习笔记.assets/image-20211228230920098.png)

有一个程序员为了做这件事情修改了安卓的OS，使其产生的汇编指令支持taint tracing（污点追踪）。比如用户在使用APP时打入的账号密码、GPS产生的经纬度数据、照片、IMEI（手机序列号），获取这些数据的API都会给对应的关键数据染色。

那么还有一个问题，应用程序下载的程序是怎么样也让它支持染色的呢？如果我们通过反汇编+汇编的方式，成功率可能不到20%。这个就涉及到安卓的历史了，最开始的安卓程序都是以Java字节码发布的，下载到本机后，会通过本机上的JVM（java虚拟机）转变成汇编代码。所以其实我们只需要魔改JVM的实现，也使其支持染色即可。

![image-20211228231009295](CSE期末复习笔记.assets/image-20211228231009295.png)

我们application有一个taint source，当java虚拟机里面同一个变量copy到另一个变量的时候，jvm就会负责帮我们把taint一起传递过去。当我们要从一个应用程序通过IPC发到另一个应用程序的时候，它把发送的消息做了一个简化，比如有一个1K的消息，这个1K的消息，只要有一个byte是taint的，那么它就认为整个消息是taint的。然后进到虚拟机里面继续使用比较细粒度的做一个拆分。**并且一个文件只要有一个byte是taint的，那么它就认为整个文件是taint的，这样就可以减少taint的数量。这样子虽然会导致精度下降，但也足够使用。**

这样去做了以后，比如我们的一个应用程序拿到了一个GPS数据，它把GPS数据通过各种加密和压缩放到另一个地方，再通过IPC的形式发到另一个应用程序。但是taint source还是可以追踪到有应用程序在做坏事。

具体taint的流动我们可以简单看一下binary operation的例子，

![image-20211228231143648](CSE期末复习笔记.assets/image-20211228231143648.png)

其实就是除了本身要做的事情，字节码在翻译的时候还要把taint的对应操作加上。最终TaintDroid的额外性能花销是30%。就是因为它把大量的taint的传递放到了文件级和消息级，这样我们就**不用维护Byte级别的taint改动**。

现在大量有native code，这个编译出来的二进制就已经不是字节码了，怎么办。TaintDriod想的办法就是它把.so文件里用到的所有函数列了一个大表，举个例子 strcpy : <img src="CSE期末复习笔记.assets/clip_image002.gif" alt="img" style="zoom: 150%;" />，这样我们就不需要跟踪strcpy里的每一行，只需要找到里面的dst和src即可。最终这个表格是380多个函数，一个个手动标记。

## 攻击者如何找到bug

**FFmpeg**

+ 攻击者选择了FFmpeg支持的比较小众化的一个格式（这种用的人少的东西往往bug会很多）
+ 攻击者使用常见的int 和unsigned int问题使得一个分支的判断永远为假，从而跳过了该判断。而该分支涉及到对内存空间的初始化，跳过了这一步攻击者就可以使用buffer overflow进行攻击。

![image-20211229104835161](CSE期末复习笔记.assets/image-20211229104835161.png)

## TaintCheck Detection Module

我们能否使用Taint Tracking技术来找之前的这些Bug？

我们可以想一个策略，当我们在写一个内存地址的时候，这个地址不能是和传入的数据相关的，否则我们就认为是一个非法的。包括说我们跳过去的一个地址不能是taint的数据，否则攻击者就可以根据输入的数据来修改我们的控制流。

​	

![image-20211229105140486](CSE期末复习笔记.assets/image-20211229105140486.png)

这是一个工具，不需要源码，传入的就是2进制，在每一位上去做taint check。这个过程分成三步：

+ 标记Taint，比如说来自网络的不可信的数据。

+ 跟踪Taint，在做加减法/copy的时候。

+ Assert Taint，当我们把Taint数据作为函数的跳转地址的时候，就会报警。

![image-20211229105805746](CSE期末复习笔记.assets/image-20211229105805746.png)

用效率来换取安全性，大约慢37倍，适用于那种对安全要求特别高的场景

![image-20211229110014902](CSE期末复习笔记.assets/image-20211229110014902.png)

对于网络IO而言，数据量大的情况下其开销占比比较低。

### No data to protect

<img src="CSE期末复习笔记.assets/image-20211229110151600.png" alt="image-20211229110151600" style="zoom:50%;" />

应用在后台的时候，GC会识别出sensitive data，并且逐步加密，当我们拿起手机再准备用的时候，数据会解密。这样哪怕有人在我们不用的时候复制出了我们的内存，他也不能破解。在手机进入休眠的时候，手机上的内容会慢慢加密存入硬盘，当解锁手机的时候会从硬盘中读取并解密到内存（原始情况下，在休眠时内存会被swap到磁盘上）。虽然解锁的时候可能会有一定的性能消耗，但是这样一来就不怕信息被偷了。

![image-20211229110320238](CSE期末复习笔记.assets/image-20211229110320238.png)

另一种方法是，当我们要登录应用的时候，把我们的QQ migrate到家里，登录完了再migrate到手机上。所以我们手机丢了都无所谓，因为手机上从来就没有密钥。

### 安全信道

![image-20211229122444211](CSE期末复习笔记.assets/image-20211229122444211.png)

上面那个就相当于是信息摘要（可以认为是checksum or hash），为了防止内容被篡改并创建一个新的信息摘要，可以使用MAC（就是对这个摘要进行加密，避免摘要被人修改）

![image-20211229122700052](CSE期末复习笔记.assets/image-20211229122700052.png)

但是中间人可以截获Alice的信息并发送给Bob进行一系列操作；解决方式是引入seq（序列号）。如果Eve尝试把这个请求回放多次，Bob会发现seq一直是相同的，可以驳回这一请求。

![image-20211229122837270](CSE期末复习笔记.assets/image-20211229122837270.png)

但是Eve可以把截获的Alice发送的消息还给Alice，从而得到递增的sequence number的消息序列。Alice可以把消息标记为from Alice。另一种方式是让Alice和Bob各自维护一个Key，当Alice向Bob发送的时候使用Keya，而Bob向Alice发的时候就用Keyb。这样我们就解决了发和收可能被混在一起的问题。

## DH秘钥交换算法

![image-20211229123310013](CSE期末复习笔记.assets/image-20211229123310013.png)

![image-20211229123412524](CSE期末复习笔记.assets/image-20211229123412524.png)

![image-20211229123418203](CSE期末复习笔记.assets/image-20211229123418203.png)

但是DH算法不能抵御中间人攻击，因为Eve可以骗到Alice说他是Bob，并且骗到Bob说他是Alice。中间人攻击没有别的办法，解决方案只有RSA（非对称加密）。

## RSA非对称加密算法

![image-20211229125055348](CSE期末复习笔记.assets/image-20211229125055348.png)

Q：Alice怎么知道Bob的公钥是什么？Alice拿到一个公钥以后，怎么判断这是Bob的合法公钥呢？

A：需要找一个第三方，把Bob和Bob的public key绑定在一起以后，用机构的私钥做一个签名，这就是所谓的签名。Alice收到这个以后，用机构的public key来验证一下机构的签名。这个机构就是CA（certificate authority，证书授权中心）。这个机构在全世界非常少，它们的公钥被预装在每一个游览器中。我们会无条件相信这些机构所签发的所有证书。这就是一切信任的根基。

如果CA一不小心签了一个不该签的证书，我们需要有一个把证书回收的方式，使用超时机制就会产生各种各样的问题，所以这不是一个很好的方法。所以我们应该定期地检查一些被召回的证书。

![image-20211229125202035](CSE期末复习笔记.assets/image-20211229125202035.png)

## **Trust and Security**

代码、甚至编译器都可能存在被植入的漏洞；甚至CPU都可能不是完全可信的（比如在制造过程中被植入一些东西）

![image-20211229130425385](CSE期末复习笔记.assets/image-20211229130425385.png)

### TPM

Trust Computing Base，最小需要相信的base软件

TPM是装在主板上的很小的元器件，里面包含了一个私钥，它固化在硬件里，是不能偷出来的，如果使用物理来强行取的话，它里面就会自动破坏关键数据。TPM有对应的公钥放在CA里。如果我们想让电脑运行一个OS，那就必须要找到这个TPM做一个合法的签名，这样TPM就会判断这个是否是一个合法的OS，如果不合法就去拒绝。

Win11用了TPM2.0，导致理论上win11的电脑上是不能装其他OS的。所以我们只需要相信TPM，这样后面启动的所有程序都放在了一条信任链里，哪里出了问题我们就可以找到。

我们相信TPM是可信的，也就是TPM的私钥没有被人获取过，如果这个假设被打破了，那么整个信任体系就没了。如果我们想验证一台服务器上的OS，那么我们只需要问服务器上的TPM这个OS有没有注册过就可以了。TPM不会被人冒充，因为我们可以通过公钥和私钥的形式来验证TPM。TPM也存在一些可能被定位的问题，所以现在一些TPM service并不对外提供服务。

![image-20211229130709813](CSE期末复习笔记.assets/image-20211229130709813.png)

## ROP and CFI

![image-20211229131542878](CSE期末复习笔记.assets/image-20211229131542878.png)

nt 0x80的意思就是主动触发一个系统调用，CPU就会跳转到0x80编号所对应的exception handler。（注意这里是一个软件触发的同步的exception，而不是硬件触发的异步的interrupt）。Interrupt handler就会看你为什么会进入到kernel里来，也就是看寄存器里的参数。对应的就是sys_execve，调用这个的目的是调用到execv(“/bin/sh”)，push的两个字符串其实就是/bin/sh。

有很多操作和指令都会短暂地提权，必须修改密码，会短暂地把权限提高然后在修改完之后变为正常，攻击者就有可能乘虚而入。

解决这种buffer overflow attack的方式主要有：ASLR（地址空间布局随机化）和Canary（金丝雀），以及限定代码的权限，即栈上的空间为可读写但是不可执行，或者把二进制结果存起来，再或者在编译的时候增加扰动或者定时重新编译。

## ROP

![image-20211229131808061](CSE期末复习笔记.assets/image-20211229131808061.png)

为了获取特定的数值，攻击者需要找到包含pop的特殊gadget，并在栈上存储特定的值，然后执行pop之后会把栈上的特定值赋值给某个寄存器。

![image-20211229131948617](CSE期末复习笔记.assets/image-20211229131948617.png)

![image-20211229132117752](CSE期末复习笔记.assets/image-20211229132117752.png)

每次return之前，就检查一下canary有没有被修改掉。如果被修改了说明有return address attack。每次在调用的时候，就要把canary放到return address的后面，返回之前就要检查，约增加8%的overhead。

其实每一个程序写完以后编译成二进制，function call都是有规律的，而return-oriented programming是在乱跳。这就是我们的control flow graph，如果我们在编译的时候去构造control flow graph，在运行的时候去enforce control flow graph，这样就可以保证控制流的完整性。一旦我们发现控制流不一样了，我们就会发现有人在攻击，但是传统的控制流图并不被包含在bin中。

如果我们可以把cfg提取出来和bin组合在一起运行，这样它就保证了控制流不会乱走，不会出现return-oriented programming乱跳的情况。

### CFI Control-Flow Integrity

![image-20211229132358254](CSE期末复习笔记.assets/image-20211229132358254.png)

![image-20211229132538202](CSE期末复习笔记.assets/image-20211229132538202.png)

binary里大部分是direct jump，而runtime的时候大多数是indirect jump。对于indirect call来说，大概率只会跳到1~2个地方。CFI在call的时候检查一下我们要call的地方是不是我们需要去的地方，它改写了二进制。

<img src="CSE期末复习笔记.assets/image-20211229133123527.png" alt="image-20211229133123527" style="zoom:50%;" />

![image-20211229133443829](CSE期末复习笔记.assets/image-20211229133443829.png)

而CFI就可以通过这种方式做一个最简单的实现，但是它有一个缺点，在sort可以调用lt和gt，所以lt和gt的内部的label都是17。所以如果一个函数可以调用100个地方，那么这100个地方的label都是一样的。

<img src="CSE期末复习笔记.assets/image-20211229133300363.png" alt="image-20211229133300363" style="zoom:50%;" />

**Insert binary code that at runtime will check whether the bit pattern of the target instruction matches the pattern of possible destinations**

![image-20211229133453448](CSE期末复习笔记.assets/image-20211229133453448.png)

每次都会跳到这个label而不是跳转地点。代码会对比ecx里存储的地址和12345678h这样的label的地址是否相等，如果不相等那么就报错，否则就[ecx+4]跳到正文部分。

注意右边; data这行汇编就是在指定一个位于跳转位置头部的标签。

但是这种方式可能存在一个问题，如果一个库打了CFI patch而另一个库没有的话会怎么样。

![image-20211229134023034](CSE期末复习笔记.assets/image-20211229134023034.png)

**Prefetchnta**: prefetch memory to cache. Become a *nop* if not available

Prefetch是一条出错以后也没问题的指令。如果这个地址是一个错误的地址，那么它就是一个nop。所以它能够提供一个兼容性，使得没有打过patch的代码可以去调用这个打过patch的库。

假如函数A要调用C，而B既可以调用C也可以调用D。那么CFI就必须对C和D使用同样的tag，比如说是12。那么就等于A也可以调用D了。一种可能的方法就是multiple tag，对于B来说调用C和调用D，必须是2个call。

还有一个问题就是，假设有一个F。先被A调用，再被B调用，被B调用的时候返回到A也被CFI认为是合法的，要解决这个问题就需要一个shadow stack，它和正常的stack很像，但是它只存return address。

<img src="CSE期末复习笔记.assets/image-20211229135241398.png" alt="image-20211229135241398" style="zoom:50%;" />

### 提升CFI精确度

对于如下的情况可能会导致问题：

如果A能够到达C， B能够到达C或D，那么根据CFI的算法，C和D都会被标上同一个标签。那么在这种情况下A到D也会被认为是合法的。

解决方案：

+ 使用多重标签，认为B到C和B到D是不等价的

![image-20211229135617458](CSE期末复习笔记.assets/image-20211229135617458.png)

事实上，我们会被ROP攻击的本质是因为我们把控制流和数据混合了，一同存放在栈上。使得攻击者能够在栈上放上一些不正常的数据，通过ret的方式破坏控制流。为程序专门构建了一个shadow stack，当我们调用函数的时候，我们同时压栈shadow stack和原先的stack。所以有了这之后，A调用F一定会返回A。所以有了shadow call stack就很大程序上解决了这个问题。

![image-20211229140633103](CSE期末复习笔记.assets/image-20211229140633103.png)

x86加上CFI之后，只允许跳转到label对应的位置。所以CFI就是保证了我们的控制流不会来回地跳。

## Example

攻击者没有nginx的二进制，所以不知道要写什么地址，并且nginx部署了金丝雀和ASLR。

dup2把sock重定向到stdin和stdout，使得攻击者能够查看结果并输入指令。攻击者的最终目标是打开nginx服务器的shell。

![image-20211229140926727](CSE期末复习笔记.assets/image-20211229140926727.png)

虽然使用了**ASLR**机制，但是对于web请求而言，每次请求都会fork出一个进程。这些进程的内存布局都是一致的。

<img src="CSE期末复习笔记.assets/image-20211229142608088.png" alt="image-20211229142608088" style="zoom:50%;" />

一位一位地找到返回地址。如果crash就代表猜的这一位猜错了。

通过不断修改返回地址找到三种gadget：

<img src="CSE期末复习笔记.assets/image-20211229142746091.png" alt="image-20211229142746091" style="zoom:50%;" />

![image-20211229142804760](CSE期末复习笔记.assets/image-20211229142804760.png)

+ Stop Gadget：从来不会crash，只会暂停之类的；
+ Crash Gadget：无论如何一直都会Crash；
+ Useful Gadget：是否crash依赖于返回地址；

![image-20211229143102875](CSE期末复习笔记.assets/image-20211229143102875.png)

如果other是已知的crash gadget，执行401170这段代码返回之后导致了crash；而换成not crash的那种就不再crash了，那么就可以认为401170这段代码是useful gadget，即这段代码包含ret语句。

一旦找到了stop gadget，我们就可以逐步验证哪些是useful gadget（即包含ret）

![image-20211229143450842](CSE期末复习笔记.assets/image-20211229143450842.png)

接下来我们要找write buffer到socket的系统调用，这是为了把二进制偷到手。

它需要去设置%rdi, %rsi,%rdx这三个参数，最后去调用int 0x80. 从callee-saved相关的汇编代码找（函数返回的时候会有大量的pop）

<img src="CSE期末复习笔记.assets/image-20211229144537412.png" alt="image-20211229144537412" style="zoom: 80%;" />

![image-20211229144650323](CSE期末复习笔记.assets/image-20211229144650323.png)![image-20211229144657359](CSE期末复习笔记.assets/image-20211229144657359.png)

如果跳转到了这段代码，我们就会把栈上的6个地址都扔掉。我们跳过去，如果最后这个系统没有crash并且hang住了，那么它一定pop了6次。所以攻击者只需要一次次试去看有没有hang住。

接下来通过strcmp找到rdx（存储string 长度）

如果我们构造一个栈使得我们的栈+4个byte又可以运行，不crash，说明我们找到了PLT表所在的位置。那么我们只需要试几次，就知道哪个是write和strcmp了。strcmp有两个参数，第一个参数可读，第二个参数为null，就会crash，只有两个参数都可读才不会crash，所以攻击者就利用这个特征找到了strcmp。攻击者怎么找write呢？其实他只要跳转到一个PLT函数之后，使得攻击者这一端收到数据了，那么我们就知道了write函数。

## Data Privacy

常见攻击

+ 通过密文获取信息。密文可能存在一定的规律，比如税单大小的密文和收入有关；
+ 通过预先知道的数据分布推断信息的大概内容；
+ 通过访问的次数、模式等推算个人信息。

![image-20211229151631400](CSE期末复习笔记.assets/image-20211229151631400.png)

## Zero-Knowledge Proof(ZKP)

<img src="CSE期末复习笔记.assets/image-20211229153158042.png" alt="image-20211229153158042" style="zoom:50%;" />

Alice用Bob的公钥加密，Bob用私钥解密，然后返回给A，那么A就能够知道对方的身份（是否是Bob）

<img src="CSE期末复习笔记.assets/image-20211229153230711.png" alt="image-20211229153230711" style="zoom: 50%;" />

![image-20211229153316957](CSE期末复习笔记.assets/image-20211229153316957.png)

P把原问题L变换为其他问题L‘就（L的答案x也变换为x'），并且尝试用大于1轮证明他拥有答案

Verifier每次都去求证变换过的问题L’，如果每次都能得到正确结果x‘（不会泄露x）就认为P真的有x。

ZKP的原理就是要构造出一个证明。

**例子：**

**Q：怎么证明自己是一个十年的豆瓣老粉。**

**A：把自己的豆瓣账号发给对方，对方看一下你确实有十年前的动态，然后对方再给你发一个私信，然后你把私信内容回答它。**

ZKP还是会多多少少泄露一部分隐私。

## PIR 

<img src="CSE期末复习笔记.assets/image-20211229154650769.png" alt="image-20211229154650769" style="zoom:50%;" />

用户想要搜索的内容往往暗示了用户的私人信息： （比如查询一些病情相关的内容）

匿名有难度：可以使用IP进行定位（可以使用代理，但是如果要你登录再查询，那么代理也没有用了）

假设现在有2个server，用户想要去查询i这个信息，它对于第一个server发出一个query。里面包含了1~n的一个子集，这个子集里面并不包含它要查询的i。（虚张声势，不包含查询的癌症）。它再去问server2，问的是原先的子集再加上她要查询的内容。她只需要做一个集合差就可以知道她要查询的真正内容了

前提：Server是不能串通的，不能知道Alice查了对方的什么数据。

![image-20211229154753448](CSE期末复习笔记.assets/image-20211229154753448.png)

对于S2里面，知道Q2里面一定有一个是你问的关键信息。但是S2并不知道自己是S2，它有可能是S1。所以对用户来说，要间接地交换S1和S2的身份，所以server就不知道自己扮演的角色。

相对不可行，且overhead比较高。

<img src="CSE期末复习笔记.assets/image-20211229154956332.png" alt="image-20211229154956332" style="zoom:50%;" />

### OT Oblivious Transfer

<img src="CSE期末复习笔记.assets/image-20211229155058891.png" alt="image-20211229155058891" style="zoom:50%;" />

Alice给Bob两者其一，但是Alice自己并不知道给了Bob具体哪个。

<img src="CSE期末复习笔记.assets/image-20211229155302407.png" alt="image-20211229155302407" style="zoom:50%;" />

### 1-out-of-2 OT

+ 解决方法1，使用RSA

  ![image-20211229155356186](CSE期末复习笔记.assets/image-20211229155356186.png)

  + Alice生成两个公钥，k0pub和k1pub（公钥），把这两个公钥发送给Bob。
  + Bob随机选择一个公钥，生成一个随机数r，用这个公钥加密r得到c
  + Alice用两个私钥解密c，得到k0和k1（注意到k0和k1其中有一个就是随机数r，但是Alice并不会知道Bob生成的随机数是哪个，也就无法推断Bob选择了哪个公钥）
  + 把k0和信息m0，k1和信息m1进行异或计算得到e0和e1。把两者发送给Bob。
  + Bob把msigma异或出来。

+ 解决方法2

  这种方法同样基于RSA，但是可以只用一对公私钥

  ![image-20211229160428709](CSE期末复习笔记.assets/image-20211229160428709.png)

  + Alice生成一对公私钥，然后选择两个随机数r0和r1，把公钥和两个随机数一起发给Bob。
  + Bob生成一个key **k** k， 用k把公钥加密并和自己随机选的一个随机数rsigma异或得到V，把V发给Alice
  + Alice先用私钥依次解开V异或r0和V异或r1，注意到这两个异或实际上会抵消rsigma的异或，这两个异或的结果中的一个就是Bob用自己的k加密过的值。解开过后得到k_0和k_1，两者其中一个就是Bob生成的k，但是Alice显然无法知道是哪个。
  + Alice用k0和k1加密，发给Bob，Bob解密。

  这两种方案本质上都是请求数据的那一方做了一定的随机化，让发送者无从了解具体哪个被接收了。

Q：OT在什么场景下有用什么场景下没用？PIR和OT有什么区别呢？

A：在PIR的场景中，Server的数据量是非常大的。对于OT来说，Alice也就知道个位数的信息，如果知道**太多了**，那么在做transfer的时候，生成了一大堆的无用操作。如果知道1w个信息，那么就要生成1万个c0和c1。

### DP Differential Privacy

虽然每次问的时候都是没有泄露个人隐私的内容，但是通过差分我们显然能够获知Tom的信息。

![image-20211229162131468](CSE期末复习笔记.assets/image-20211229162131468.png)

![image-20211229204157678](CSE期末复习笔记.assets/image-20211229204157678.png)

对于每个人有个budget设定用户能够询问的内容和询问的数量（防止多次查询获知更多有关噪音的信息，以推测出大概的私人信息。）

对询问的信息添加噪声，让用户查询不那么准确？

### Secret Sharing

![image-20211229204339624](CSE期末复习笔记.assets/image-20211229204339624.png)

假设有一个secret owner知道一个很重要的数字，比如说是银行密码等。他把这个数字拆成了n份，它能够做到的是，只有当n中的k个人把碎片拼在一起了，就可以还原出这个数字，但是如果只有k-1个人是不能还原出数字的。这个特点就很有用。

![image-20211229210418410](CSE期末复习笔记.assets/image-20211229210418410.png)

所以新应用场景就是用来做容错，也就是n个服务器允许挂掉n-k个。对于part1到partn来说，最简单的备份方法就是copy一份，现在每个人手里拿到的只是一个part，最终n个人只要有k个就可以恢复，并且这个k比起k replica用的空间会更少、但是达到了k replica的效果。

## Secure MPC（安全多方计算）

你有数据、我有数据，但是不能合在一起。

<img src="CSE期末复习笔记.assets/image-20211229210740349.png" alt="image-20211229210740349" style="zoom:50%;" />

<img src="CSE期末复习笔记.assets/image-20211229210717402.png" alt="image-20211229210717402" style="zoom:50%;" />

C+1 …. C+10中总有一个是K

用自己的私钥解密 得到y_i 其中一定有一个是x，但是并不知道是哪个

Bob只对i后面的数据++，所以如果dj = x mod p 就说明 dj没有被++

### Homomorphic Encryption（同态加密技术）

![image-20211229212304306](CSE期末复习笔记.assets/image-20211229212304306.png)

训练一个加密的数据 然后获得一个加密的模型？

加密搜索，把搜索关键字加密获取加密的文档。

<img src="CSE期末复习笔记.assets/image-20211229212312516.png" alt="image-20211229212312516" style="zoom:50%;" />

加密运算的结果就是没有加密的结果经过加密获取的结果

RSA 具有加同态和乘同态

SWHE 在某种特定运算上是同态的

FHE 所有运算和操作都是同态的

有一类是我们不能排序。排序非常重要，因为range query都需要顺序。但是一个数字1加密之后得到的数字不一定比2加密以后得到的数字要小。

所以有专门的保序加密算法，也就是小的数字加密以后还是小。但是这几个特性合在一块就很危险了，假如我们有保序和加同态的特点，我们很有可能可以反推出这个算法。我们在数据库中想要推测出3个数字，我们可以通过保序性来二分插入新数字来推算出3个数字分别是什么。

怎么样又要做又要保证安全呢？我们可以对每次运算从头加一个密。比如我们要做一个加法，那么我们对整个数据库做一个加同态，做完以后删掉。要做range query，对整个数据库用保序算法做一个range query，做完以后再删掉。这个在很需要安全、不需要性能的场景是很有用的。

全同态加密：谁能找到加减乘除的同态算法，这就是理论上的完美。很可惜还没有很完美的全同态加密的算法。

# Hardware Enclave 可信执行环境

![image-20211229213031094](CSE期末复习笔记.assets/image-20211229213031094.png)

我们能不能实现计算的时候在很大情况下都是密文的情况呢？Bob创建一个黑箱。Enclave就是飞地的意思。创业小公司没有GPU，就可以在字节跳动的云服务器里创建一个飞地，创建以后属于小公司。作为飞地以后，字节跳动就不能访问，就等于领事馆属于国家的领土。

我们在cloud里创建了一个enclave，我们先不去管它是怎么实现的。

它有两个特点：

+ 隔离执行，当enclave在运行的时候，没有任何人（云管理员、硬件维修人员）能够干扰程序的执行。（安全的3个goal：C，I，availability）

+ Remote attestation，我们作为用户是通过网络连到这个云，我们不知道你有没有骗我们。需要飞地证明自己的飞地，不会被别人通过各种各样的手段偷到数据。

![image-20211229214446534](CSE期末复习笔记.assets/image-20211229214446534.png)

在user process里，OS在地址空间的头部。他在地址空间中内部分成了enclave data和enclave code，当OS要访问enclave的时候，就会被CPU禁止。访问的时候有一个CPU reserved memory，这样就可以在MMU的时候就禁止住。

为了防止暴力复制内存（低温等环境），我们还需要对内存做加密。所以内存进cache的时候做解密，cache进内存的时候就做加密。那么cache要是被偷了怎么办？需要在CPU上电的情况下把CPU外层磨掉，再用特质的针脚来偷到这个数据。

只要我们在CPU之外都是密文，这样对于绝大部分的数据加密需求都够了。

![img](CSE期末复习笔记.assets/clip_image002-16407856973284.gif)

Remote attestation就不一样了，它的核心就来源于Intel。要证明你是你，那就需要用到私钥，如果存在云服务器上，那就很容易被云厂商偷走。私钥在CPU里，由Intel来验证这个私钥。当我们要核实真假的时候，直接给飞地发随机数要求签名。它就会把加载的代码算一个哈希，和刚才发的随机数写在一起签一个名发回来。因为我们是owner，我们知道code的哈希和随机数是多少。但是我们还是要让intel帮忙看一下。

Q：Intel可信吗？

A：我们暂时还不能断掉Intel。

对于阿里云/Amazon来说。华为云用的是华为芯片怎么办呢？这个问题目前还没有什么很好的方案。

数据安全是一个不断演进的过程。

