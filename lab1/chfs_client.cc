// chfs client.  implements FS operations using extent and lock server
#include "chfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ROUND_UP(x) (x + 3) & ~3

#define INVARIABLE_SIZE (sizeof(char) + sizeof(inum) + sizeof(uint16_t))

chfs_client::chfs_client() {
    ec = new extent_client();
}

chfs_client::chfs_client(std::string extent_dst, std::string lock_dst) {
    ec = new extent_client();
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
}

void
chfs_client::write_entry(char *buf, dirent *entry) {
    memset(buf, 0, entry->entry_len);

    memcpy(buf, &(entry->inum), sizeof(inum));
    buf += sizeof(inum);

    memcpy(buf, &(entry->entry_len), sizeof(uint16_t));
    buf += sizeof(uint16_t);

    char name_len = entry->name.size();
    memcpy(buf, &name_len, sizeof(char));
    buf += sizeof(char);

    memcpy(buf, entry->name.c_str(), name_len);
}

void chfs_client::construct_entry(inum i, dirent &entry, const char *name) {
    unsigned int name_len = strlen(name);
    const unsigned int supplemented_size = ROUND_UP(name_len);
    entry.inum = i;
    char _name[supplemented_size];
    memset(_name, 0, supplemented_size);
    strncpy(_name, name, name_len);
    entry.name.assign(_name);
    entry.entry_len = INVARIABLE_SIZE + supplemented_size;
}

void chfs_client::modify_dire_content(dirent *entry, std::string &content) {
    unsigned int total_size = content.size();
    unsigned int cursor = 0;
    bool inserted = false;
    while (cursor < total_size) {
        dirent cur_entry = *((dirent *) &content[cursor]);
        unsigned int real_size = INVARIABLE_SIZE + cur_entry.name.size();
        unsigned int entry_size = cur_entry.entry_len;
        unsigned int rest_size = entry_size - real_size;
        if (rest_size >= entry->entry_len) {
            write_entry((char *) &content[cursor + real_size], entry);
            inserted = true;
            break;
        }
        cursor += entry_size;
    }

    // if not inserted, append
    if (!inserted) {
        content.append(std::string(entry->entry_len, 0));
        write_entry((char *) &content[total_size], entry);
    }
}

chfs_client::inum
chfs_client::n2i(std::string n) {
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
chfs_client::filename(inum inum) {
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
chfs_client::isfile(inum inum) {
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    }
    printf("isfile: %lld is a dir\n", inum);
    return false;
}

/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool
chfs_client::isdir(inum inum) {
    // Oops! is this still correct when you implement symlink?
    return !isfile(inum);
}

int
chfs_client::getfile(inum inum, fileinfo &fin) {
    int r = OK;

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

    release:
    return r;
}

int
chfs_client::getdir(inum inum, dirinfo &din) {
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

    release:
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
chfs_client::setattr(inum ino, size_t size) {
    int r = OK;

    /*
     * your code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */

    return r;
}

int
chfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out) {
    int r = OK;

    /*
     * your code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent information.
     */

    bool found = false;
    lookup(parent, name, found, ino_out);
    // if exists, then return EXIST
    if (found) return EXIST;
    // if not found, create new file.
    inum i;
    ec->create(extent_protocol::T_FILE, i);

    // construct the entry
    dirent entry;
    construct_entry(i, entry, name);

    // get and modify the content of parent directory
    std::string content;
    ec->get(i, content);
    modify_dire_content(&entry, content);

    // write back.
    ec->put(i, content);

    return r;
}

int
chfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out) {
    int r = OK;

    /*
     * your code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent information.
     */

    bool found = false;
    lookup(parent, name, found, ino_out);
    // if exists, then return EXIST
    if (found) return EXIST;
    // if not found, create new directory.
    inum i;
    ec->create(extent_protocol::T_DIR, i);

    // construct the entry
    dirent entry;
    construct_entry(i, entry, name);

    // get and modify the content of parent directory
    std::string content;
    ec->get(i, content);
    modify_dire_content(&entry, content);

    // write back.
    ec->put(i, content);

    return r;
}

int
chfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out) {
    int r = OK;

    /*
     * your code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    std::string content;
    ec->get(parent, content);
    int total_size = content.size();
    int cursor = 0;
    while (cursor < total_size) {
        dirent entry = *((dirent *) &content[cursor]);

        if (!strncmp(entry.name.c_str(), name, entry.name.size())) {
            found = true;
            ino_out = entry.inum;
            std::cout << "Find file name: " << name << std::endl;
            break;
        }

        unsigned int size = entry.entry_len;
        cursor += size;
    }
    return r;
}

int
chfs_client::readdir(inum dir, std::list <dirent> &list) {
    int r = OK;

    /*
     * your code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */

    return r;
}

int
chfs_client::read(inum ino, size_t size, off_t off, std::string &data) {
    int r = OK;

    /*
     * your code goes here.
     * note: read using ec->get().
     */

    return r;
}

int
chfs_client::write(inum ino, size_t size, off_t off, const char *data,
                   size_t &bytes_written) {
    int r = OK;

    /*
     * your code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */

    return r;
}

int chfs_client::unlink(inum parent, const char *name) {
    int r = OK;

    /*
     * your code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */

    return r;
}

