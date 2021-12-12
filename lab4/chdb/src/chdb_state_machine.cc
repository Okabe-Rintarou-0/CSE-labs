#include "chdb_state_machine.h"

chdb_command::chdb_command() {
    // TODO: Your code here
}

chdb_command::chdb_command(command_type tp, const int &key, const int &value, const int &tx_id)
        : cmd_tp(tp), key(key), value(value), tx_id(tx_id), res(std::make_shared<result>()) {
    // TODO: Your code here
    res->start = std::chrono::system_clock::now();
    res->key = key;
}

chdb_command::chdb_command(const chdb_command &cmd) :
        cmd_tp(cmd.cmd_tp), key(cmd.key), value(cmd.value), tx_id(cmd.tx_id), res(cmd.res) {
    // TODO: Your code here
}

inline static void serializeInt(char *buf, int x) {
    buf[0] = (x >> 24) & 0xff;
    buf[1] = (x >> 16) & 0xff;
    buf[2] = (x >> 8) & 0xff;
    buf[3] = x & 0xff;
}

inline static int deserializeInt(const char *buf) {
    int ret;
    ret = (buf[0] & 0xff) << 24;
    ret |= (buf[1] & 0xff) << 16;
    ret |= (buf[2] & 0xff) << 8;
    ret |= buf[3] & 0xff;
    return ret;
}

void chdb_command::serialize(char *buf, int size) const {
    // TODO: Your code here
    auto p_buf = buf;

    serializeInt(p_buf, tx_id);
    p_buf += 4;

    serializeInt(p_buf, key);
    p_buf += 4;

    serializeInt(p_buf, value);
    p_buf += 4;

    serializeInt(p_buf, (int) cmd_tp);
}

void chdb_command::deserialize(const char *buf, int size) {
    // TODO: Your code here
    auto p_buf = buf;

    tx_id = deserializeInt(p_buf);
    p_buf += 4;

    key = deserializeInt(p_buf);
    p_buf += 4;

    value = deserializeInt(p_buf);
    p_buf += 4;

    cmd_tp = (command_type) deserializeInt(p_buf);
}

marshall &operator<<(marshall &m, const chdb_command &cmd) {
    // TODO: Your code here
    return m << cmd.tx_id << cmd.key << cmd.value << (int) cmd.cmd_tp;
}

unmarshall &operator>>(unmarshall &u, chdb_command &cmd) {
    // TODO: Your code here
    int cmd_tp_tmp;
    u >> cmd.tx_id >> cmd.key >> cmd.value >> cmd_tp_tmp;
    cmd.cmd_tp = (chdb_command::command_type) cmd_tp_tmp;
    return u;
}

void chdb_state_machine::apply_log(raft_command &cmd) {
    // TODO: Your code here
    chdb_command &chdb_cmd = dynamic_cast<chdb_command &>(cmd);
    printf("try to apply\n");
    std::unique_lock <std::mutex> lock(chdb_cmd.res->mtx);
    // Your code here:
    printf("got a lock\n");
    switch (chdb_cmd.cmd_tp) {
        case chdb_command::CMD_PUT: {
            put(chdb_cmd.key, chdb_cmd.value);
            chdb_cmd.res->value = chdb_cmd.value;
            chdb_cmd.res->succ = true;
            break;
        }
        case chdb_command::CMD_GET: {
            bool succ;
            int val = get(chdb_cmd.key, succ);
            chdb_cmd.res->value = val;
            chdb_cmd.res->succ = succ;
            break;
        }
        case chdb_command::CMD_NONE: {
            chdb_cmd.res->value = chdb_cmd.value;
            chdb_cmd.res->succ = true;
            break;
        }
    }
    printf("applied\n");
    chdb_cmd.res->key = chdb_cmd.key;
    chdb_cmd.res->done = true;
    chdb_cmd.res->cv.notify_all();
    return;
}