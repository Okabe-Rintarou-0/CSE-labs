#include "raft_protocol.h"

marshall &operator<<(marshall &m, const request_vote_args &args) {
    // Your code here
    return m << args.term << args.log_max_idx << args.log_max_term;
}

unmarshall &operator>>(unmarshall &u, request_vote_args &args) {
    // Your code here

    return u >> args.term >> args.log_max_idx >> args.log_max_term;
}

marshall &operator<<(marshall &m, const request_vote_reply &reply) {
    // Your code here

    return m << reply.accept;
}

unmarshall &operator>>(unmarshall &u, request_vote_reply &reply) {
    // Your code here

    return u >> reply.accept;
}

marshall &operator<<(marshall &m, const append_entries_reply &reply) {
    // Your code here

    return m << reply.code << reply.idx;
}

unmarshall &operator>>(unmarshall &u, append_entries_reply &reply) {
    // Your code here

    return u >> reply.code >> reply.idx;
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