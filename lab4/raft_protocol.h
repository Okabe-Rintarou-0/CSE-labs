#ifndef raft_protocol_h
#define raft_protocol_h

#include "rpc.h"
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
    int candidateId;
    int lastLogIndex;
    int lastLogTerm;

    request_vote_args() = default;

    request_vote_args(int term, int candidateId, int lastLogIndex, int lastLogTerm) : term(term),
                                                                                      candidateId(candidateId),
                                                                                      lastLogIndex(lastLogIndex),
                                                                                      lastLogTerm(lastLogTerm) {}
};

marshall &operator<<(marshall &m, const request_vote_args &args);

unmarshall &operator>>(unmarshall &u, request_vote_args &args);


class request_vote_reply {
public:
    // Your code here
    int term;
    int voteGranted;

    request_vote_reply() = default;

    request_vote_reply(int term, int voteGranted) : term(term), voteGranted(voteGranted) {}
};

marshall &operator<<(marshall &m, const request_vote_reply &reply);

unmarshall &operator>>(unmarshall &u, request_vote_reply &reply);

template<typename command>
class log_entry {
public:
    // Your code here
    int term = 0;
    command cmd;

    log_entry() = default;

    log_entry(int term, command cmd) : term(term), cmd(cmd) {}
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

template<typename command>
class append_entries_args {
public:
    // Your code here
    int term;
    int leaderId;
    int prevLogIndex;
    int prevLogTerm;
    std::vector <log_entry<command>> entries;

    int leaderCommit;

    append_entries_args() = default;

    append_entries_args(int term, int leaderId, int prevLogIndex, int prevLogTerm,
                        const std::vector <log_entry<command>> &entries, int leaderCommit) : term(term),
                                                                                             leaderId(leaderId),
                                                                                             prevLogIndex(prevLogIndex),
                                                                                             prevLogTerm(prevLogTerm),
                                                                                             entries(entries),
                                                                                             leaderCommit(
                                                                                                     leaderCommit) {}

    append_entries_args(int term, int leaderId, int leaderCommit) : term(term),
                                                                    leaderId(leaderId),
                                                                    entries(std::vector<log_entry<command>>()),
                                                                    leaderCommit(leaderCommit) {}
};

template<typename command>
marshall &operator<<(marshall &m, const append_entries_args<command> &args) {
    // Your code here
    return m << args.term << args.leaderId << args.prevLogIndex << args.prevLogTerm << args.entries
             << args.leaderCommit;
}

template<typename command>
unmarshall &operator>>(unmarshall &u, append_entries_args<command> &args) {
    // Your code here
    return u >> args.term >> args.leaderId >> args.prevLogIndex >> args.prevLogTerm >> args.entries
             >> args.leaderCommit;
}

class append_entries_reply {
public:
    // Your code here
    int term;
    int success;
};

marshall &operator<<(marshall &m, const append_entries_reply &reply);

unmarshall &operator>>(unmarshall &m, append_entries_reply &reply);


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