#include "raft_state_machine.h"
#include <stdio.h>


kv_command::kv_command() : kv_command(CMD_NONE, "", "") {}

kv_command::kv_command(command_type tp, const std::string &key, const std::string &value) :
        cmd_tp(tp), key(key), value(value), res(std::make_shared<result>()) {
    res->start = std::chrono::system_clock::now();
    res->key = key;
}

kv_command::kv_command(const kv_command &cmd) :
        cmd_tp(cmd.cmd_tp), key(cmd.key), value(cmd.value), res(cmd.res) {}

kv_command::~kv_command() {}

// [type]:0[key]:123[value]:123
int kv_command::size() const {
    // Your code here:
    return key.size() + value.size() + 22;
}


void kv_command::serialize(char *buf, int size) const {
    // Your code here:
    sprintf(buf, "type:%d key:%s value:%s", (int) cmd_tp, key.c_str(), value.c_str());
}

void kv_command::deserialize(const char *buf, int size) {
    // Your code here:
    char key_buf[100], val_buf[100];
    sscanf(buf, "type:%d key:%s value:%s", (int *) &cmd_tp, key_buf, val_buf);
    key = key_buf;
    value = val_buf;
}

marshall &operator<<(marshall &m, const kv_command &cmd) {
    // Your code here:
    return m << (int) cmd.cmd_tp << cmd.key << cmd.value;
}

unmarshall &operator>>(unmarshall &u, kv_command &cmd) {
    // Your code here:
    int cmd_tp;
    u >> cmd_tp >> cmd.key >> cmd.value;
    cmd.cmd_tp = (kv_command::command_type) cmd_tp;
    return u;
}

kv_state_machine::~kv_state_machine() {

}

void kv_state_machine::apply_log(raft_command &cmd) {
    kv_command &kv_cmd = dynamic_cast<kv_command &>(cmd);
    std::unique_lock <std::mutex> lock(kv_cmd.res->mtx);
    // Your code here:
    switch (kv_cmd.cmd_tp) {
        case kv_command::CMD_PUT: {
            put(kv_cmd.key, kv_cmd.value);
//            std::cout << "do put[" << kv_cmd.key << "] = " << kv_cmd.value << std::endl;
            kv_cmd.res->value = kv_cmd.value;
            kv_cmd.res->succ = true;
            break;
        }
        case kv_command::CMD_DEL: {
            std::string val = del(kv_cmd.key);
            kv_cmd.res->value = val;
            kv_cmd.res->succ = !val.empty();
            break;
        }
        case kv_command::CMD_GET: {
            std::string val = get(kv_cmd.key);
//            std::cout << "do get[" << kv_cmd.key << "] = " << val << std::endl;
            kv_cmd.res->value = val;
            kv_cmd.res->succ = !val.empty();
            break;
        }
        case kv_command::CMD_NONE: {
            kv_cmd.res->value = kv_cmd.value;
            kv_cmd.res->succ = true;
            break;
        }
    }
    kv_cmd.res->key = kv_cmd.key;
    kv_cmd.res->done = true;
    kv_cmd.res->cv.notify_all();
    return;
}

std::vector<char> kv_state_machine::snapshot() {
    // Your code here:
    return std::vector<char>();
}

void kv_state_machine::apply_snapshot(const std::vector<char> &snapshot) {
    // Your code here:
    return;
}
