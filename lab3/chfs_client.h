#ifndef chfs_client_h
#define chfs_client_h

#include <string>
//#include "chfs_protocol.h"
#include "extent_client.h"
#include <vector>

class chfs_client {
    extent_client *ec;
public:

    typedef unsigned long long inum;
    enum xxstatus {
        OK, RPCERR, NOENT, IOERR, EXIST
    };
    typedef int status;

    struct fileinfo {
        unsigned long long size;
        unsigned long atime;
        unsigned long mtime;
        unsigned long ctime;
    };
    struct dirinfo {
        unsigned long atime;
        unsigned long mtime;
        unsigned long ctime;
    };
    struct dirent {
        chfs_client::inum inum;
        uint16_t entry_len;
        char name_len;
        std::string name;
    };

private:
    static std::string filename(inum);

    static inum n2i(std::string);

public:
    chfs_client(std::string);

    chfs_client(std::string, std::string);

    bool isfile(inum);

    bool isdir(inum);

    int getfile(inum, fileinfo &);

    int getdir(inum, dirinfo &);

    int setattr(inum, size_t);

    int lookup(inum, const char *, bool &, inum &);

    int create(inum, const char *, mode_t, inum &);

    int readdir(inum, std::list <dirent> &);

    int write(inum, size_t, off_t, const char *, size_t &);

    int read(inum, size_t, off_t, std::string &);

    int unlink(inum, const char *);

    int mkdir(inum, const char *, mode_t, inum &);

    // my func
    void parse_content(std::string &, std::list <dirent> &);

    void write_entry(char *, dirent *);

    void read_entry(char *, dirent *);

    void construct_entry(inum i, dirent &, const char *);

    void modify_content(std::string &, dirent *);

    bool issymlink(inum);

    int symlink(inum, const char *, const char *, inum &);

    int readlink(inum, std::string &);

    /** you may need to add symbolic link related methods here.*/
};

#endif 