#include "raft_protocol.h"

marshall &operator<<(marshall &m, const request_vote_args &args) {
    // Your code here
    return m << args.term << args.candidateId << args.lastLogIndex << args.lastLogTerm;
}

unmarshall &operator>>(unmarshall &u, request_vote_args &args) {
    // Your code here
    return u >> args.term >> args.candidateId >> args.lastLogIndex >> args.lastLogTerm;
}

marshall &operator<<(marshall &m, const request_vote_reply &reply) {
    // Your code here
    return m << reply.term << reply.voteGranted;
}

unmarshall &operator>>(unmarshall &u, request_vote_reply &reply) {
    // Your code here
    return u >> reply.term >> reply.voteGranted;
}

marshall &operator<<(marshall &m, const append_entries_reply &reply) {
    // Your code here

    return m << reply.term << reply.success;
}

unmarshall &operator>>(unmarshall &u, append_entries_reply &reply) {
    // Your code here

    return u >> reply.term >> reply.success;
}

marshall &operator<<(marshall &m, const install_snapshot_args &args) {
    // Your code here

    return m;
}

unmarshall &operator>>(unmarshall &u, install_snapshot_args &args) {
    // Your code here

    return u;
}

marshall &operator<<(marshall &m, const install_snapshot_reply &reply) {
    // Your code here

    return m;
}

unmarshall &operator>>(unmarshall &u, install_snapshot_reply &reply) {
    // Your code here

    return u;
}