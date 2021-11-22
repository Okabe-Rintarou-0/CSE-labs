#ifndef raft_storage_h
#define raft_storage_h

#include "raft_protocol.h"
#include <iostream>
#include <list>
#include <fcntl.h>
#include <mutex>
#include <fstream>
#include <string>

template<typename command>
class raft_storage {
public:
    raft_storage(const std::string &file_dir);

    ~raft_storage();

    // Your code here
    void append_log(const log_entry<command> &log);

    void read_logs(std::list <log_entry<command>> &logs);

private:
    static const std::string log_file;
    static const std::string meta_file;

    std::mutex log_mtx;
    std::mutex meta_mtx;

    const std::string base_dir;

    std::ofstream log_ofs;
    std::ifstream log_ifs;
    std::ofstream meta_ofs;
    std::ifstream meta_ifs;

    // user should free the space allocated
    inline char *serialize(const log_entry<command> &log, int &str_size);

    inline log_entry<command> deserialize(const std::string &str);
};

template<typename command>
raft_storage<command>::raft_storage(const std::string &dir): base_dir(dir) {
    // Your code here
    log_ofs.open(base_dir + log_file, std::ios::app);
    meta_ofs.open(base_dir + meta_file, std::ios::app);
}

template<typename command>
raft_storage<command>::~raft_storage() {
    // Your code here
    log_ofs.close();
    meta_ofs.close();
}

template<typename command>
void raft_storage<command>::append_log(const log_entry<command> &log) {
    log_mtx.lock();
    int size;
    char *serialize_log = serialize(log, size);
//    std::cout << "append log: ";
    for (int i = 0; i < size; ++i) {
        log_ofs << serialize_log[i];
//        std::cout << (int) serialize_log[i];
    }
    log_ofs << std::endl;
//    std::cout << std::endl;
    log_mtx.unlock();
    delete serialize_log;
}

template<typename command>
void raft_storage<command>::read_logs(std::list <log_entry<command>> &logs) {
    log_ifs.open(base_dir + log_file);
    std::string line;
    while (getline(log_ifs, line)) {
        if (line.empty())
            break;
        logs.push_back(deserialize(line));
    }
    log_ifs.close();
}

template<typename command>
const std::string raft_storage<command>::log_file = "/log.txt";

template<typename command>
const std::string raft_storage<command>::meta_file = "/meta.txt";

template<typename command>
char *raft_storage<command>::serialize(const log_entry<command> &log, int &str_size) {
    std::string term = std::to_string(log.term);
    int term_size = term.size();
    int size = log.cmd.size();
    char cmd[size];
    log.cmd.serialize(cmd, size);
    str_size = term_size + size + 1;
    char *ret = new char[str_size];
    int i;
    for (i = 0; i < term_size; ++i)
        ret[i] = term[i];
    ret[i++] = ':';
    for (int j = 0; j < size; ++j) {
        ret[i + j] = cmd[j];
    }
    return ret;
}

template<typename command>
log_entry<command> raft_storage<command>::deserialize(const std::string &str) {
    int pos = str.find_first_of(":");
    int term = atoi(str.substr(0, pos).c_str());
    command cmd;
    int size = cmd.size();
    cmd.deserialize(str.substr(pos + 1, str.size() - pos - 1).c_str(), size);
//    std::cout << "read term: " << term << " and command: " << cmd.value << std::endl;
    return log_entry<command>(term, cmd);
}

#endif // raft_storage_h