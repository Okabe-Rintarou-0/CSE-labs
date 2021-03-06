// chfs client.  implements FS operations using extent server
#include "chfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ROUND_UP(x) ((x + 3) & ~3)

#define INVARIABLE_SIZE (sizeof(char) + sizeof(inum) + sizeof(uint16_t))

std::ostream &operator<<(std::ostream &os, const chfs_client::dirent &entry) {
    return os
            << "----------------------------------------" << std::endl
            << "Entry info: " << std::endl
            << "inum: " << entry.inum << std::endl
            << "entry_len: " << entry.entry_len << std::endl
            << "name_len: " << (int) entry.name_len << std::endl
            << "name: " << entry.name << std::endl
            << "----------------------------------------";
}

chfs_client::chfs_client(std::string extent_dst) {
    ec = new extent_client(extent_dst);
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
}

void
chfs_client::parse_content(std::string &content, std::list <dirent> &entries) {
    int total_size = content.size();
    int cursor = 0;
    while (cursor < total_size) {
        dirent entry;
        read_entry(&content[cursor], &entry);
//        std::cout << "Read entry from content: " << std::endl
//                  << entry << std::endl;

        entries.push_back(entry);
        cursor += entry.entry_len;
    }
}

void
chfs_client::write_entry(char *buf, dirent *entry) {
    memset(buf, 0, entry->entry_len);

    memcpy(buf, &(entry->inum), sizeof(inum));
    buf += sizeof(inum);

    memcpy(buf, &(entry->entry_len), sizeof(uint16_t));
    buf += sizeof(uint16_t);

    memcpy(buf, &(entry->name_len), sizeof(char));
    buf += sizeof(char);

    memcpy(buf, entry->name.c_str(), entry->name_len);
}

void
chfs_client::read_entry(char *buf, dirent *entry) {
    memcpy(&(entry->inum), buf, sizeof(inum));
    buf += sizeof(inum);

    memcpy(&(entry->entry_len), buf, sizeof(uint16_t));
    buf += sizeof(uint16_t);

    memcpy(&(entry->name_len), buf, sizeof(char));
    buf += sizeof(char);

    char name[entry->name_len];
    memcpy(name, buf, entry->name_len);
    entry->name.assign(name, entry->name_len);
}

void chfs_client::construct_entry(inum i, dirent &entry, const char *name) {
    unsigned int name_len = strlen(name);
//    std::cout << "name = " << name << std::endl;
//    std::cout << "name_len = " << name_len << std::endl;

    const unsigned int supplemented_size = ROUND_UP(name_len);
//    std::cout << "supplemented_size = " << supplemented_size << std::endl;
//    std::cout << "invariable_size = " << INVARIABLE_SIZE << std::endl;
    char _name[supplemented_size];
    memset(_name, 0, supplemented_size);
    strncpy(_name, name, name_len);

    entry.name.assign(_name, name_len);
    entry.inum = i;
    entry.name_len = name_len;
    entry.entry_len = INVARIABLE_SIZE + supplemented_size;
}

void chfs_client::modify_content(std::string &content, dirent *insert) {
//    for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
//        dirent entry = *it;
//        unsigned int entry_len = entry.entry_len;
//        unsigned int real_len = INVARIABLE_SIZE + entry.name.size();
//        unsigned int rest_len = entry_len - real_len;
//        if (real_len >= insert.entry_len) {
//        }
//    }
    // naive: directly append
    unsigned int org_size = content.size();
    content.append(std::string(insert->entry_len, 0));
    write_entry(&content[org_size], insert);
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

    return (a.type == extent_protocol::T_FILE);
}

/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool
chfs_client::isdir(inum inum) {
    // Oops! is this still correct when you implement symlink?
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    return (a.type == extent_protocol::T_DIR);
}

int
chfs_client::getfile(inum inum, fileinfo &fin) {
    int r = OK;

//    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
//    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

    release:
    return r;
}

int
chfs_client::getdir(inum inum, dirinfo &din) {
    int r = OK;

//    printf("getdir %016llx\n", inum);
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

//    std::cout << "chfs: try set file(inum = " << ino << ") size to " << size << std::endl;

    // fetch content of ino
    std::string content;
    ec->get(ino, content);

    // modify content
    // if bigger, append with '\0'.
    content.resize(size, 0);
//    std::cout << "After setattr: " << content << std::endl
//              << "content size = " << content.size() << std::endl;

    // put ino
    ec->put(ino, content);

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
//    std::cout << "No file with same name found" << std::endl;

    ec->create(extent_protocol::T_FILE, ino_out);
//    std::cout << "Create inode for file successfully." << std::endl;

    // construct the entry
    dirent entry;
    construct_entry(ino_out, entry, name);
//    std::cout << "Construct entry:" << std::endl
//              << entry << std::endl;

    // get and modify the content of parent directory
    std::string content;
    ec->get(parent, content);
    modify_content(content, &entry);

    // write back.
    ec->put(parent, content);

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
//    std::cout << "No dire with same name found" << std::endl;
    ec->create(extent_protocol::T_DIR, ino_out);
//    std::cout << "Create inode for directory successfully." << std::endl;

    // construct the entry
    dirent entry;
    construct_entry(ino_out, entry, name);
//    std::cout << "Construct entry:" << std::endl
//              << entry << std::endl;

    // get and modify the content of parent directory
    std::string content;
    ec->get(parent, content);
    modify_content(content, &entry);

    // write back.
    ec->put(parent, content);

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
    std::list <dirent> entries;
    readdir(parent, entries);
//    std::cout << "Trying to look up for " << name << std::endl;
    for (dirent &entry : entries) {
        if (!strcmp(entry.name.c_str(), name)) {
//            std::cout << "Found Entry : " << std::endl
//                      << entry << std::endl;
            ino_out = entry.inum;
            found = true;
            break;
        }
    }
    return r;
}

int
chfs_client::readdir(inum dir, std::list <dirent> &list) {
    int r = OK;

    /*
     * your code goes here.
     * note: you should parse the directory content using your defined format,
     * and push the dirents to the list.
     */
    std::string content;
    ec->get(dir, content);
    parse_content(content, list);

    return r;
}

int
chfs_client::read(inum ino, size_t size, off_t off, std::string &data) {
    int r = OK;

    /*
     * your code goes here.
     * note: read using ec->get().
     */
//    std::cout << "Trying to read (off = " << off << ", size = " << size << ") "
//              << "from file with inode = " << ino << std::endl;

    // fetch content
    std::string content;
    ec->get(ino, content);
    unsigned int content_size = content.size();

    // judge
    // if off is valid.
    if (off < content_size)
        data = content.substr(off, size);
        // else should get nothing.
    else
        data = "";

//    std::cout << "Read content: " << data << std::endl;

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

//    std::cout << "Trying to write (off = " << off << ", size = " << size << ") "
//              << "into file with inode = " << ino << std::endl;

    // fetch content
    std::string content;
    ec->get(ino, content);
//    std::cout << "Before write: " << content << std::endl;
    unsigned int content_size = content.size();

    // if bigger, then expand the content.
    // expand with '\0', automatically fill the 'holes'
    bytes_written = off <= content_size ? size : off + size - content_size;

    if (off + size > content_size)
        content.resize(off + size, 0);

    // write data
    memcpy(&content[off], data, size);
//    std::cout << "After write: " << content << std::endl
//              << "content size: " << content.size() << std::endl;

    // modify mtime
//    extent_protocol::attr a;
//    ec->getattr(ino, a);
//    a.mtime = time(nullptr);

    // put
    ec->put(ino, content);

    bytes_written = size;
    return r;
}

int chfs_client::unlink(inum parent, const char *name) {
    int r = OK;

    /*
     * your code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */

    // prepare
    std::list <dirent> entries;
    std::string content;
    ec->get(parent, content);
    parse_content(content, entries);

    // remove
    for (auto iter = entries.begin(); iter != entries.end(); ++iter) {
        if (!strcmp(iter->name.c_str(), name)) {
            // remove block
            ec->remove(iter->inum);
            // remove from list
            entries.erase(iter);
            // resize content
            content.resize(content.size() - iter->entry_len);
            break;
        }
    }

    // using naive way to modify content
    int cursor = 0;
    for (dirent &entry: entries) {
        write_entry(&content[cursor], &entry);
        cursor += entry.entry_len;
    }

    // modify parent
    ec->put(parent, content);

    return r;
}

int chfs_client::symlink(inum parent, const char *link, const char *name, inum &ino_out) {
    int r = OK;

    // first look up if there exists symlink file with the same name
    bool found = false;
    lookup(parent, name, found, ino_out);
    if (found)
        return EXIST;

    // create a new symlink file (with type extent_protocol::T_SYMLINK)
    ec->create(extent_protocol::T_SYMLINK, ino_out);
    ec->put(ino_out, std::string(link));

    // modify the content of parent
    std::string content;
    ec->get(parent, content);

    // construct the entry
    dirent entry;
    construct_entry(ino_out, entry, name);

    // modify content
    modify_content(content, &entry);

    // put
    ec->put(parent, content);

    return r;
}

int chfs_client::readlink(inum ino, std::string &data) {
    int r = OK;

    // fetch data
    ec->get(ino, data);

    return r;
}

bool chfs_client::issymlink(inum ino) {
    return !isfile(ino) && !isdir(ino);
}