#ifndef raft_protocol_h
#define raft_protocol_h

#include "rpc.h"
#include <vector>
#include "raft_state_machine.h"

enum raft_rpc_opcodes {
    op_request_vote = 0x1212,
    op_append_entries = 0x3434,
    op_install_snapshot = 0x5656
};

enum raft_rpc_status {
    OK,
    RETRY,
    RPCERR,
    NOENT,
    IOERR
};

class request_vote_args {
public:
    // Your code here
    int term;
    int log_max_idx;
    int log_max_term;

    request_vote_args() = default;

    request_vote_args(int term, int m_idx, int m_term) : term(term), log_max_idx(m_idx), log_max_term(m_term) {}
};

marshall &operator<<(marshall &m, const request_vote_args &args);

unmarshall &operator>>(unmarshall &u, request_vote_args &args);


class request_vote_reply {
public:
    // Your code here
    int accept;
};

marshall &operator<<(marshall &m, const request_vote_reply &reply);

unmarshall &operator>>(unmarshall &u, request_vote_reply &reply);

template<typename command>
class log_entry {
public:
    int term;
    command cmd;

    log_entry() = default;

    log_entry(int term, command cmd) : term(term), cmd(cmd) {}
    // Your code here
};

template<typename command>
marshall &operator<<(marshall &m, const log_entry<command> &entry) {
    // Your code here
    return m << entry.term << entry.cmd;
}

template<typename command>
unmarshall &operator>>(unmarshall &u, log_entry<command> &entry) {
    // Your code here
    return u >> entry.term >> entry.cmd;
}

/**
 * action = 0 ping
 * 1 append
 * 2 commit
 */
template<typename command>
class append_entries_args {
public:
    int action;
    int cur_term;
    int last_term;
    int idx;
    std::vector <log_entry<command>> entries;

    append_entries_args() = default;

    append_entries_args(int action, int cur_term) : action(action), cur_term(cur_term) {}

    append_entries_args(int action, int cur_term, int last_term, int idx, std::vector <log_entry<command>> entries)
            : action(action), cur_term(cur_term), last_term(last_term), idx(idx), entries(entries) {}

    inline bool isEmpty() const {
        return entries.empty();
    }

    inline void appendCmd(const command &cmd, int term) {
        entries.push_back(log_entry<command>(cmd, term));
    }
    // Your code here
};

template<typename command>
marshall &operator<<(marshall &m, const append_entries_args<command> &args) {
    // Your code here
    return m << args.action << args.cur_term << args.last_term << args.idx << args.entries;
}

template<typename command>
unmarshall &operator>>(unmarshall &u, append_entries_args<command> &args) {
    // Your code here
    return u >> args.action >> args.cur_term >> args.last_term >> args.idx >> args.entries;
}

/**
 * code = -1 not ok
 * code = 0 ping
 * code = 1 append_ok
 */
class append_entries_reply {
public:
    // Your code here
    int code;
    int idx;
};

marshall &operator<<(marshall &m, const append_entries_reply &reply);

unmarshall &operator>>(unmarshall &u, append_entries_reply &reply);

class install_snapshot_args {
public:
    // Your code here
};

marshall &operator<<(marshall &m, const install_snapshot_args &args);

unmarshall &operator>>(unmarshall &m, install_snapshot_args &args);


class install_snapshot_reply {
public:
    // Your code here
};

marshall &operator<<(marshall &m, const install_snapshot_reply &reply);

unmarshall &operator>>(unmarshall &m, install_snapshot_reply &reply);


#endif // raft_protocol_h