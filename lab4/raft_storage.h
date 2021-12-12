#ifndef raft_storage_h
#define raft_storage_h

#include "raft_protocol.h"
#include <iostream>
#include <list>
#include <fcntl.h>
#include <mutex>
#include <fstream>
#include <string>

enum actionType {
    APPEND, APPLY
};

template<typename command>
struct action {
    action() = default;

    action(actionType actionType, int index) : type(actionType), index(index) {}

    action(actionType actionType, int index, const log_entry<command> &log) : type(actionType), index(index),
                                                                              log(log) {}

    actionType type;
    int index;
    log_entry<command> log;
};

template<typename command>
class raft_storage {
public:
    raft_storage(const std::string &file_dir);

    ~raft_storage();

    // Your code here
    void append_log(int index, const log_entry<command> &log);

    void apply_log(int index);

    void read_logs(std::list <action<command>> &actions);

    void store_metadata(int term);

    int read_term();

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

    inline action<command> deserialize(const std::string &str);
};

template<typename command>
raft_storage<command>::raft_storage(const std::string &dir): base_dir(dir) {
    // Your code here
    log_ofs.open(base_dir + log_file, std::ios::app);
//    meta_ofs.open(base_dir + meta_file, std::ios::trunc);
}

template<typename command>
raft_storage<command>::~raft_storage() {
    // Your code here
    log_ofs.close();
    meta_ofs.close();
}

template<typename command>
void raft_storage<command>::append_log(int index, const log_entry<command> &log) {
    log_mtx.lock();
    int size;
    char *serialize_log = serialize(log, size);
    log_ofs << "[APPEND] " << "index: " << index << " ";
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
void raft_storage<command>::apply_log(int index) {
    log_mtx.lock();
    log_ofs << "[APPLY] " << "index: " << index << std::endl;
    log_mtx.unlock();
}

template<typename command>
void raft_storage<command>::store_metadata(int term) {
    meta_ofs.open(base_dir + meta_file, std::ios::trunc);
    meta_ofs << "term" << ":" << term << std::endl;
    meta_ofs.close();
}

template<typename command>
int raft_storage<command>::read_term() {
    meta_ifs.open(base_dir + meta_file);
    std::string line;
    getline(meta_ifs, line);
//    std::cout << "read line: " << line << std::endl;
    int pos = line.find_first_of(":");
    std::string termStr = line.substr(pos + 1, line.size() - pos - 1);
    return atoi(termStr.c_str());
}

template<typename command>
void raft_storage<command>::read_logs(std::list <action<command>> &actions) {
    log_ifs.open(base_dir + log_file);
    std::string line;
    while (getline(log_ifs, line)) {
        if (line.empty())
            break;
//        std::cout << "read line: " << line << std::endl;
        actions.push_back(deserialize(line));
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
action<command> raft_storage<command>::deserialize(const std::string &str) {
    int len = str.size();
    if (str.find("[APPEND]") != std::string::npos) {
        int pos1 = str.find("index") + 7;
        std::string index;
        for (; pos1 < len; ++pos1) {
            if (str[pos1] == ' ')
                break;
            index += str[pos1];
        }
        pos1 += 1;
        int pos2 = str.find_first_of(":", pos1);
        int term = atoi(str.substr(pos1, pos2 - pos1).c_str());
        command cmd;
        int size = cmd.size();
        cmd.deserialize(str.substr(pos2 + 1, len - pos2 - 1).c_str(), size);
//        printf("read log from disk[term = %d, value = %d, index = %s]\n", term, cmd.value, index.c_str());
        return action<command>(APPEND, atoi(index.c_str()), log_entry<command>(term, cmd));
    } else if (str.find("[APPLY]") != std::string::npos) {
        int pos1 = str.find("index") + 7;
        std::string index;
        for (; pos1 < len; ++pos1) {
            if (str[pos1] == ' ')
                break;
            index += str[pos1];
        }
//        printf("Read apply[index = %s]\n", index.c_str());
        return action<command>(APPLY, atoi(index.c_str()));
    }
//    std::cout << "read term: " << term << " and command: " << cmd.value << std::endl;
    return action<command>();
}

#endif // raft_storage_h