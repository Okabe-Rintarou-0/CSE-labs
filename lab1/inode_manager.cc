#include "inode_manager.h"

#define debug   std::cout << "called" << std::endl;
// disk layer -----------------------------------------

disk::disk() {
    bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf) {
    memcpy(buf, (char *) blocks[id], BLOCK_SIZE);
}

void
disk::write_block(blockid_t id, const char *buf) {
    memcpy((char *) blocks[id], buf, BLOCK_SIZE);
}

// block layer -----------------------------------------

bool
block_manager::check_free_and_alloc(uint32_t id) {
    bool free = using_blocks[id] == 0;
    if (free) {
        using_blocks[id] = 1;
    }
    return free;
}

bool
block_manager::check_free_and_free(uint32_t id) {
    bool free = using_blocks[id] == 0;
    if (!free) {
        using_blocks[id] = 0;
    }
    return free;
}

uint32_t
block_manager::alloc_and_write(const char *buf, int size) {
    unsigned int block_id = alloc_block();
    if (block_id == 0) return 0;
    char block[BLOCK_SIZE];
    read_block(block_id, block);
    memcpy(block, buf, size);
    write_block(block_id, block);
    return block_id;
}

// Allocate a free disk block.
blockid_t
block_manager::alloc_block() {
    /*
     * your code goes here.
     * note: you should mark the corresponding bit in block bitmap when alloc.
     * you need to think about which block you can start to be allocated.
     */
    if (!free_queue.empty()) {
        unsigned int free_block_id = free_queue.front();
        free_queue.pop();
        using_blocks[free_block_id] = 1;
        return free_block_id;
    }

    for (unsigned int i = DHEAD; i <= BLOCK_NUM; ++i) {
        if (check_free_and_alloc(i)) {
//      last_allocated = i;
            return i;
        }
    }
    return 0;
}

void
block_manager::free_block(uint32_t id) {
    /*
     * your code goes here.
     * note: you should unmark the corresponding bit in the block bitmap when free.
     */
    if (id >= DHEAD && id <= BLOCK_NUM) {
        char block[BLOCK_SIZE];
        if (!check_free_and_free(id)) {
            memset(block, 0, BLOCK_SIZE);
            write_block(id, block);
            free_queue.push(id);
        }
    }
    return;
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager() {
    d = new disk();

    // format the disk
    sb.size = BLOCK_SIZE * BLOCK_NUM;
    sb.nblocks = BLOCK_NUM;
    sb.ninodes = INODE_NUM;

}

void
block_manager::read_block(uint32_t id, char *buf) {
    d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf) {
    d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager() {
    bm = new block_manager();
    uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
    if (root_dir != 1) {
        printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
        exit(0);
    }
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type) {
    /*
     * your code goes here.
     * note: the normal inode block should begin from the 2nd inode block.
     * the 1st is used for root_dir, see inode_manager::inode_manager().
     */
    for (int i = 1; i <= INODE_NUM; ++i) {
//    std::cout << "iid: " << iid << std::endl;
        inode *_inode = get_inode(i);
        if (_inode == nullptr) {
            _inode = (inode *) malloc(sizeof(inode));
            _inode->type = type;
            _inode->size = 0;
            put_inode(i, _inode);
            free(_inode);
            return i;
        }
    }
    return 1;
}

void
inode_manager::free_inode(uint32_t inum) {
    /*
     * your code goes here.
     * note: you need to check if the inode is already a freed one;
     * if not, clear it, and remember to write back to disk.
     */
    unsigned int inode_block_id = IBLOCK(inum, this->bm->sb.nblocks);
    inode *i = get_inode(inum);
    if (i == nullptr) return;
    if (i->type != 0) {
        char empty[BLOCK_SIZE];
        memset(empty, 0, BLOCK_SIZE);
        this->bm->write_block(inode_block_id, empty);
    }
    free(i);
    return;
}


/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode *
inode_manager::get_inode(uint32_t inum) {
    struct inode *ino, *ino_disk;
    char buf[BLOCK_SIZE];

//  printf("\tim: get_inode %d\n", inum);

    if (inum < 0 || inum >= INODE_NUM) {
        printf("\tim: inum out of range\n");
        return nullptr;
    }

    bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
    // printf("%s:%d\n", __FILE__, __LINE__);

    ino_disk = (struct inode *) buf + inum % IPB;
    if (ino_disk->type == 0) {
        printf("\tim: inode not exist\n");
        return nullptr;
    }
    ino = (struct inode *) malloc(sizeof(struct inode));
    *ino = *ino_disk;
    return ino;
}

void
inode_manager::put_inode(uint32_t inum, struct inode *ino) {
    char buf[BLOCK_SIZE];
    struct inode *ino_disk;

//    printf("\tim: put_inode %d\n", inum);
    if (ino == NULL)
        return;

    bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
    ino_disk = (struct inode *) buf + inum % IPB;
    *ino_disk = *ino;
    bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a, b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf_out, int *size) {
    /*
     * your code goes here.
     * note: read blocks related to inode number inum,
     * and copy them to buf_out
     */
    //get inode
    inode *_inode = get_inode(inum);
    if (_inode == nullptr) return;
    *size = _inode->size;
    std::cout << "Read file size: " << *size << std::endl;
    // calc direct block size
    unsigned int direct_block_size = *size <= NDIRECT * BLOCK_SIZE ? *size : NDIRECT * BLOCK_SIZE;
    unsigned int n_direct_blocks =
            direct_block_size % BLOCK_SIZE ? direct_block_size / BLOCK_SIZE + 1 : direct_block_size / BLOCK_SIZE;

    // calc total bytes
    unsigned int n_total = *size % BLOCK_SIZE ? *size / BLOCK_SIZE + 1 : *size / BLOCK_SIZE;
    *buf_out = (char *) malloc(*size);

    unsigned int i;
    unsigned int rest_size = direct_block_size;
    // read direct blocks
    char block[BLOCK_SIZE];
    for (i = 0; i < n_direct_blocks; ++i) {
        // direct block, so directly get the block id
        unsigned int block_id = _inode->blocks[i];
        // read block to buf_out[i]
        unsigned int written_size = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
        this->bm->read_block(block_id, block);
        memcpy(*buf_out + i * BLOCK_SIZE, block, written_size);
        rest_size -= written_size;
//    std::cout << "Read: " << *buf_out << std::endl;
    }
    std::cout << "Read n_total: " << n_total << std::endl;
    // if exists indirect block
    if (n_total > n_direct_blocks) {
        // get the indirect block
        unsigned int indirect_block_id = _inode->blocks[NDIRECT];
//    std::cout << "Read indirect_block_id: " << indirect_block_id << std::endl;
        char indirect_block[BLOCK_SIZE];
        this->bm->read_block(indirect_block_id, indirect_block);

        // calc how many direct blocks
        unsigned int indirect_block_size = *size - direct_block_size;
        rest_size = indirect_block_size;
        unsigned int n_temp = n_direct_blocks;
        n_direct_blocks =
                indirect_block_size % BLOCK_SIZE ? indirect_block_size / BLOCK_SIZE + 1 : indirect_block_size /
                                                                                          BLOCK_SIZE;

        // alloc direct blocks that the indirect block links to
        for (; i < n_total; ++i) {
            // read 4 bytes each, transform to block id.
            // then read from the corresponding block.
            unsigned int written_size = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
            unsigned int block_id = read_block_id(&indirect_block[4 * (i - n_temp)]);
            std::cout << "Read block id: " << block_id << std::endl;
            this->bm->read_block(block_id, block);
            memcpy(*buf_out + i * BLOCK_SIZE, block, written_size);
            rest_size -= written_size;
        }
    }
//  std::cout << "Read: " << *buf_out << std::endl;
    free(_inode);
    /// TODO: do remember that the caller of this method should delete allocated objects.
    return;
}

void
inode_manager::write_block_id(char *x, unsigned int id) {
    for (int i = 0; i < 4; ++i) {
        *(x + i) = *((char *) &id + i);
    }
}

unsigned int
inode_manager::read_block_id(char *x) {
//  std::cout << "try read: " << *((int *)x) << std::endl;
    unsigned int id;
    for (int i = 0; i < 4; ++i) {
        *((char *) &id + i) = *(x + i);
    }
    return id;
}

void
inode_manager::free_indirect(unsigned int indirect_block_id) {
    char indirect_block[BLOCK_SIZE];
    this->bm->read_block(indirect_block_id, indirect_block);

    unsigned int direct_block_id;
    int idx = 0;
    while ((direct_block_id = read_block_id(indirect_block + 4 * idx))) {
        this->bm->free_block(direct_block_id);
        ++idx;
    }

    this->bm->free_block(indirect_block_id);
}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size) {
    /*
     * your code goes here.
     * note: write buf to blocks of inode inum.
     * you need to consider the situation when the size of buf
     * is larger or smaller than the size of original inode
     */
    inode *_inode = get_inode(inum);
    // check validation
    if (_inode == nullptr)
        return;
    if ((unsigned int) size > (MAXFILE * BLOCK_SIZE) || size <= 0) {
        free(_inode);
        return;
    }

    // calc the size of direct blocks
    unsigned int direct_size = size <= NDIRECT * BLOCK_SIZE ? size : NDIRECT * BLOCK_SIZE;

    // the number of direct blocks
    unsigned int n_direct_blocks = direct_size % BLOCK_SIZE ? direct_size / BLOCK_SIZE + 1 : direct_size / BLOCK_SIZE;

    unsigned int rest_size = direct_size;
    unsigned int written_size;
    unsigned int i;
    unsigned int direct_block_id;
    char block[BLOCK_SIZE];

    // alloc direct blocks
    for (i = 0; i < n_direct_blocks; ++i) {
        written_size = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
        // make use of the allocated blocks
        if ((direct_block_id = _inode->blocks[i]) > 0) {
            memset(block, 0, BLOCK_SIZE);
            memcpy(block, buf + i * BLOCK_SIZE, written_size);
            this->bm->write_block(direct_block_id, block);
        } else
            _inode->blocks[i] = this->bm->alloc_and_write(buf + i * BLOCK_SIZE, written_size);
        rest_size -= written_size;
    }

    // calc the size of indirect blocks
    // if > 0, means that need store block in indirect way
    unsigned int indirect_size = size - direct_size;
    if (indirect_size > 0) {
        unsigned int indirect_block_id = _inode->blocks[NDIRECT];
        if (indirect_block_id) free_indirect(indirect_block_id);
        // indirect block => many direct blocks
        indirect_block_id = this->bm->alloc_block();
        _inode->blocks[NDIRECT] = indirect_block_id;

        // calc the number of direct blocks that the indirect block links to
        n_direct_blocks = indirect_size % BLOCK_SIZE ? indirect_size / BLOCK_SIZE + 1 : indirect_size / BLOCK_SIZE;
//    std::cout << "wt n_direct_blocks: " << n_direct_blocks << std::endl;
        char indirect_block[BLOCK_SIZE];
        this->bm->read_block(indirect_block_id, indirect_block);
        rest_size = indirect_size;
        for (unsigned int i = 0; i < n_direct_blocks; ++i) {
            unsigned int written_size = rest_size > BLOCK_SIZE ? BLOCK_SIZE : rest_size;
            unsigned int block_id = this->bm->alloc_and_write(buf + (NDIRECT + i) * BLOCK_SIZE, written_size);
            // write block id into the indirect block
            // four byte each. so i * 4
            write_block_id(&indirect_block[i * 4], block_id);
//      std::cout << "after write: " << *((int *)&indirect_block[i * 4]) << std::endl;
        }
        this->bm->write_block(indirect_block_id, indirect_block);
    }
    // put inode
    _inode->size = size;
    put_inode(inum, _inode);
    free(_inode);
    return;
}

void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a) {
    /*
     * your code goes here.
     * note: get the attributes of inode inum.
     * you can refer to "struct attr" in extent_protocol.h
     */
    inode *i = get_inode(inum);
    if (i == nullptr) return;
    a.size = i->size;
    a.type = i->type;
    a.atime = i->atime;
    a.ctime = i->ctime;
    a.mtime = i->mtime;
    free(i);
    return;
}

void
inode_manager::remove_file(uint32_t inum) {
    /*
     * your code goes here
     * note: you need to consider about both the data block and inode of the file
     */
    inode *_inode = get_inode(inum);
    if (_inode == nullptr) return;
    for (int i = 0; i < NDIRECT; ++i) {
        if (_inode->blocks[i])
            this->bm->free_block(_inode->blocks[i]);
        else
            break;
    }

    unsigned int indirect_block_id;
    if ((indirect_block_id = _inode->blocks[NDIRECT])) {
        free_indirect(indirect_block_id);
    }
    free(_inode);
    free_inode(inum);
    return;
}
