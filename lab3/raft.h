#ifndef raft_h
#define raft_h

#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <ctime>
#include <algorithm>
#include <thread>
#include <stdarg.h>
#include <random>

#include "rpc.h"
#include "raft_storage.h"
#include "raft_protocol.h"
#include "raft_state_machine.h"

inline int randRange(int min, int max) {
    int randomSeed = rand() + rand() % 500;
    srand(randomSeed);
    return (rand() % (max - min + 1)) + min;
}

inline long long getCurrentTime() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
}

template<typename state_machine, typename command>
class raft {

    static_assert(std::is_base_of<raft_state_machine, state_machine>(),
                  "state_machine must inherit from raft_state_machine");
    static_assert(std::is_base_of<raft_command, command>(), "command must inherit from raft_command");


    friend class thread_pool;

#define RAFT_LOG(fmt, args...) \
    do { \
        auto now = \
        std::chrono::duration_cast<std::chrono::milliseconds>(\
            std::chrono::system_clock::now().time_since_epoch()\
        ).count();\
        printf("[%ld][%s:%d][node %d term %d] " fmt "\n", now, __FILE__, __LINE__, my_id, current_term, ##args); \
    } while(0);

public:
    raft(
            rpcs *rpc_server,
            std::vector<rpcc *> rpc_clients,
            int idx,
            raft_storage<command> *storage,
            state_machine *state
    );

    ~raft();

    // start the raft node.
    // Please make sure all of the rpc request handlers have been registered before this method.
    void start();

    // stop the raft node.
    // Please make sure all of the background threads are joined in this method.
    // Notice: you should check whether server should be stopped by calling is_stopped().
    //         Once it returns true, you should break all of your long-running loops in the background threads.
    void stop();

    // send a new command to the raft nodes.
    // This method returns true if this raft node is the leader that successfully appends the log.
    // If this node is not the leader, returns false.
    bool new_command(command cmd, int &term, int &index);

    // returns whether this node is the leader, you should also set the current term;
    bool is_leader(int &term);

    // save a snapshot of the state machine and compact the log.
    bool save_snapshot();

private:
    std::mutex mtx;                     // A big lock to protect the whole data structure
    ThrPool *thread_pool;
    raft_storage<command> *storage;              // To persist the raft log
    state_machine *state;  // The state machine that applies the raft log, e.g. a kv store

    rpcs *rpc_server;               // RPC server to receive and handle the RPC requests
    std::vector<rpcc *> rpc_clients; // RPC clients of all raft nodes including this node
    int my_id;                     // The index of this node in rpc_clients, start from 0

    std::atomic_bool stopped;

    enum raft_role {
        follower,
        candidate,
        leader
    };

    raft_role role;
    int current_term;

    std::thread *background_election;
    std::thread *background_ping;
    std::thread *background_commit;
    std::thread *background_apply;

    // Your code here:
    int num;
    long long last_received_RPC_time = getCurrentTime();

    const int dead_thresh = 300;

    int timeout;
    int vote_no;

    int apply_index;
    int commit_index;

    std::vector<int> nextIndex;

    std::vector <log_entry<command>> logs;

    std::vector<long long> ping_pong;

    std::mutex t_mtx;
    std::mutex app_mtx;
    std::mutex log_mtx;

private:
    // RPC handlers
    int request_vote(request_vote_args arg, request_vote_reply &reply);

    int append_entries(append_entries_args<command> arg, append_entries_reply &reply);

    int install_snapshot(install_snapshot_args arg, install_snapshot_reply &reply);

    // RPC helpers
    void send_request_vote(int target, request_vote_args arg);

    void handle_request_vote_reply(int target, const request_vote_args &arg, const request_vote_reply &reply);

    void send_append_entries(int target, append_entries_args<command> arg);

    void
    handle_append_entries_reply(int target, const append_entries_args<command> &arg, const append_entries_reply &reply);

    void send_install_snapshot(int target, install_snapshot_args arg);

    void
    handle_install_snapshot_reply(int target, const install_snapshot_args &arg, const install_snapshot_reply &reply);


private:
    bool is_stopped();

    int num_nodes() { return rpc_clients.size(); }

    // background workers
    void run_background_ping();

    void run_background_election();

    void run_background_commit();

    void run_background_apply();

    // Your code here:

    inline void init_timeout();

    void start_election();

    void ping();

    int append_log(const log_entry<command> &log);

    void append_logs(const append_entries_args<command> &args);

    void check_and_send_logs();

    void notify_commit();

    inline void update_last_RPC_time();

    inline bool is_timeout();

    void init_next_index();

    inline bool is_dead(int id);

    bool ready_to_commit();

    void send_entries(int target);

    bool accept_append(const append_entries_args<command> &args);

    void apply_log();

    void recover_from_logs();

    inline void become_follower(int new_term);
};

template<typename state_machine, typename command>
raft<state_machine, command>::raft(rpcs *server, std::vector<rpcc *> clients, int idx, raft_storage<command> *storage,
                                   state_machine *state) :
        storage(storage),
        state(state),
        rpc_server(server),
        rpc_clients(clients),
        my_id(idx),
        stopped(false),
        role(follower),
        current_term(0),
        background_election(nullptr),
        background_ping(nullptr),
        background_commit(nullptr),
        background_apply(nullptr),
        vote_no(0),
        apply_index(0),
        commit_index(0) {
    thread_pool = new ThrPool(32);

    // Register the rpcs.
    rpc_server->reg(raft_rpc_opcodes::op_request_vote, this, &raft::request_vote);
    rpc_server->reg(raft_rpc_opcodes::op_append_entries, this, &raft::append_entries);
    rpc_server->reg(raft_rpc_opcodes::op_install_snapshot, this, &raft::install_snapshot);

    // Your code here:
    // Do the initialization

    // randomize the election timeout
    init_timeout();
    // init the log with 1 size.
    num = rpc_clients.size();
    nextIndex.assign(num, 0);
    ping_pong.assign(num, 0);
    logs.assign(1, log_entry<command>());
}

template<typename state_machine, typename command>
raft<state_machine, command>::~raft() {
    if (background_ping) {
        delete background_ping;
    }
    if (background_election) {
        delete background_election;
    }
    if (background_commit) {
        delete background_commit;
    }
    if (background_apply) {
        delete background_apply;
    }
    delete thread_pool;
}

/******************************************************************

                        Public Interfaces

*******************************************************************/

template<typename state_machine, typename command>
void raft<state_machine, command>::stop() {
    stopped.store(true);
    background_ping->join();
    background_election->join();
    background_commit->join();
    background_apply->join();
    thread_pool->destroy();
    RAFT_LOG("stop now.");
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::is_stopped() {
    return stopped.load();
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::is_leader(int &term) {
    term = current_term;
    return role == leader;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::start() {
    // Your code here:

    RAFT_LOG("start");
    recover_from_logs();
    commit_index = apply_index = logs.size() - 1;

    this->background_election = new std::thread(&raft::run_background_election, this);
    this->background_ping = new std::thread(&raft::run_background_ping, this);
    this->background_commit = new std::thread(&raft::run_background_commit, this);
    this->background_apply = new std::thread(&raft::run_background_apply, this);
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::new_command(command cmd, int &term, int &index) {
    // Your code here:
    if (role != leader) return false;
    term = current_term;
    RAFT_LOG("Leader received a command(v: %d)", cmd.value);
    auto entry = log_entry<command>(current_term, cmd);
    RAFT_LOG("Leader append log[term = %d, value = %d]", current_term, cmd.value);
    index = append_log(entry);
    return true;
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::save_snapshot() {
    // Your code here:
    return true;
}


/******************************************************************

                         RPC Related

*******************************************************************/
template<typename state_machine, typename command>
int raft<state_machine, command>::request_vote(request_vote_args args, request_vote_reply &reply) {
    // Your code here:
    bool accept = false;
    mtx.lock();
    int my_max_idx = logs.size() - 1;
    int my_max_log_term = logs[my_max_idx].term;
    if (args.term > current_term) {
        current_term = args.term;
        accept = my_max_log_term < args.log_max_term ||
                 (my_max_log_term == args.log_max_term && my_max_idx <= args.log_max_idx);
        if (accept && role != follower) {
            become_follower(args.term);
        }
    }
    mtx.unlock();
    reply.accept = accept;
//    if (accept)
//        RAFT_LOG("Follower decide to vote");
    return raft_rpc_status::OK;
}


template<typename state_machine, typename command>
void raft<state_machine, command>::handle_request_vote_reply(int target, const request_vote_args &arg,
                                                             const request_vote_reply &reply) {
    // Your code here:
    if (reply.accept) {
        bool vote_succeed = false;
        mtx.lock();
        if (role == candidate) {
            if (++vote_no > num / 2) {
                RAFT_LOG("get vote: %d, has become a leader", vote_no);
                role = leader; // if surpass a half, then become the leader.
                vote_succeed = true;
                vote_no = 0;
                long long curT = getCurrentTime();
                std::fill(ping_pong.begin(), ping_pong.end(), curT);
                init_next_index();
            }
        }
        mtx.unlock();
        if (vote_succeed) {
//            RAFT_LOG("leader declaim");
            ping();
            // to declaim my role.
        }
    }
    return;
}


template<typename state_machine, typename command>
int raft<state_machine, command>::append_entries(append_entries_args<command> arg, append_entries_reply &reply) {
    // Your code here:
    if (arg.cur_term >= current_term) {
        update_last_RPC_time();
        switch (arg.action) {
            case 0: {
                mtx.lock();
                if (role != follower) {
                    become_follower(arg.cur_term);
                }
                mtx.unlock();
                reply.code = 0;
                break;
            }
            case 1: {
                ///TODO: optimize concurrency
                mtx.lock();
                if (role != follower) {
                    become_follower(arg.cur_term);
                }
                mtx.unlock();
                RAFT_LOG("Receive log from leader");
                if (accept_append(arg)) {
                    RAFT_LOG("Accept Log");
                    append_logs(arg);
                    reply.code = 1;
                    reply.idx = logs.size() - 1;
                } else {
                    RAFT_LOG("Reject log");
                    reply.code = -1;
                }
                break;
            }
            case 2: {
                RAFT_LOG("leader told me to commit");
                mtx.lock();
                commit_index = arg.idx;
                mtx.unlock();
                break;
            }
        }
    }
    return raft_rpc_status::OK;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::handle_append_entries_reply(int target, const append_entries_args<command> &arg,
                                                               const append_entries_reply &reply) {
    // Your code here:
    switch (reply.code) {
        case 0: {
            ping_pong[target] = getCurrentTime();
            break;
        }
        case 1: { // append_ok
            nextIndex[target] = reply.idx + 1;
//            RAFT_LOG("Leader update nextIndex[%d] to %d", target, reply.idx + 1);
            if (ready_to_commit()) {
//                RAFT_LOG("Leader is ready to commit");
                notify_commit();
                apply_log();
            }
            break;
        }
        case -1: { // append_reject
            --nextIndex[target];
//            RAFT_LOG("Append reject by follower(id:%d)", target);
            break;
        }
        default:
            break;
    }
    return;
}


template<typename state_machine, typename command>
int raft<state_machine, command>::install_snapshot(install_snapshot_args args, install_snapshot_reply &reply) {
    // Your code here:
    update_last_RPC_time();
    return 0;
}


template<typename state_machine, typename command>
void raft<state_machine, command>::handle_install_snapshot_reply(int target, const install_snapshot_args &arg,
                                                                 const install_snapshot_reply &reply) {
    // Your code here:
    return;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::send_request_vote(int target, request_vote_args arg) {
    request_vote_reply reply;
//    RAFT_LOG("Candidate ask [%d] for vote", target);
    if (rpc_clients[target]->call(raft_rpc_opcodes::op_request_vote, arg, reply) == 0) {
        handle_request_vote_reply(target, arg, reply);
    } else {
        // RPC fails
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::send_append_entries(int target, append_entries_args<command> arg) {
    append_entries_reply reply;
    if (rpc_clients[target]->call(raft_rpc_opcodes::op_append_entries, arg, reply) == 0) {
        handle_append_entries_reply(target, arg, reply);
    } else {
        // RPC fails
//        if (arg.action == 2) {
//            thread_pool->addObjJob(this, &raft::send_append_entries, target, arg);
//        }
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::send_install_snapshot(int target, install_snapshot_args arg) {
    install_snapshot_reply reply;
    if (rpc_clients[target]->call(raft_rpc_opcodes::op_install_snapshot, arg, reply) == 0) {
        handle_install_snapshot_reply(target, arg, reply);
    } else {
        // RPC fails
    }
}

/******************************************************************

                        Background Workers

*******************************************************************/

template<typename state_machine, typename command>
void raft<state_machine, command>::run_background_election() {
    // Check the liveness of the leader.
    // Work for followers and candidates.

    // Hints: You should record the time you received the last RPC.
    //        And in this function, you can compare the current time with it.
    //        For example:
    //        if (current_time - last_received_RPC_time > timeout) start_election();
    //        Actually, the timeout should be different between the follower (e.g. 300-500ms) and the candidate (e.g. 1s).


    while (true) {
        if (is_stopped()) return;
        // Your code here:
        switch (role) {
            case follower: {
                if (is_timeout()) {
//                    RAFT_LOG("time out[%d ms], become a candidate.", timeout);
                    start_election();
                }
                break;
            }
            case candidate: {
                if (is_timeout()) {
//                    RAFT_LOG("time out, election failed, try again");
                    mtx.lock();
                    role = follower;
                    init_timeout();
                    mtx.unlock();
                }
                break;
            }
            default:
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::run_background_commit() {
    // Send logs/snapshots to the follower.
    // Only work for the leader.

    // Hints: You should check the leader's last log index and the follower's next log index.

    while (true) {
        if (is_stopped()) return;
        // Your code here:

        if (role == leader) {
            check_and_send_logs();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::run_background_apply() {
    // Apply committed logs the state machine
    // Work for all the nodes.

    // Hints: You should check the commit index and the apply index.
    //        Update the apply index and apply the log if commit_index > apply_index


    while (true) {
        if (is_stopped()) return;
        // Your code here:

        if (role == follower) {
            log_mtx.lock();
            if (commit_index > apply_index) {
                RAFT_LOG("need apply (cmt_i: %d, app_i: %d)", commit_index, apply_index);
                apply_log();
            }
            log_mtx.unlock();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::run_background_ping() {
    // Send empty append_entries RPC to the followers.

    // Only work for the leader.

    while (true) {
        if (is_stopped()) return;
        // Your code here:

        if (role == leader) {
            ping();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Change the timeout here!
    }
    return;
}


/******************************************************************

                        Other functions

*******************************************************************/
template<typename state_machine, typename command>
void raft<state_machine, command>::init_timeout() {
    timeout = randRange(300, 500);
}

template<typename state_machine, typename command>
void raft<state_machine, command>::recover_from_logs() {
    std::list <log_entry<command>> logs;
    storage->read_logs(logs);
    RAFT_LOG("read logs");
    log_mtx.lock();
    for (auto &log:logs) {
        RAFT_LOG("apply log[term = %d, value = %d]", log.term, log.cmd.value);
        state->apply_log(log.cmd);
        this->logs.push_back(log);
    }
    log_mtx.unlock();
}

template<typename state_machine, typename command>
void raft<state_machine, command>::start_election() {
//    RAFT_LOG("start election");
    mtx.lock();
    vote_no = 1;
    ++current_term;
    role = candidate;
    timeout = 1000;
    int log_max_idx = logs.size() - 1;
    int log_max_term = logs[log_max_idx].term;
    mtx.unlock();
    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
            request_vote_args args(current_term, log_max_idx, log_max_term);
            thread_pool->addObjJob(this, &raft::send_request_vote, i, args);
        }
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::ping() {
    int num = rpc_clients.size();
//    RAFT_LOG("leader ping");
    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
            append_entries_args<command> args(0, current_term);
            thread_pool->addObjJob(this, &raft::send_append_entries, i, args);
        }
    }
}

template<typename state_machine, typename command>
int raft<state_machine, command>::append_log(const log_entry<command> &log) {
    int size;
    log_mtx.lock();
    logs.push_back(log);
    size = logs.size();
    log_mtx.unlock();
    return size - 1;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::append_logs(const append_entries_args<command> &args) {
    log_mtx.lock();
    logs.resize(args.idx + 1);
    for (const auto &log:args.entries) {
        this->logs.push_back(log);
//        RAFT_LOG("Follower append log[term = %d, value = %d]", log.term, log.cmd.value);
    }
    log_mtx.unlock();
}

template<typename state_machine, typename command>
void raft<state_machine, command>::update_last_RPC_time() {
    t_mtx.lock();
    last_received_RPC_time = getCurrentTime();
    t_mtx.unlock();
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::is_timeout() {
    t_mtx.lock();
    long long current_time = getCurrentTime();
    bool ret = current_time - last_received_RPC_time > timeout;
    t_mtx.unlock();
    return ret;
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::accept_append(const append_entries_args<command> &args) {
    int last_idx = args.idx;
    bool accept;
    log_mtx.lock();
//    RAFT_LOG("last_idx: %d last_term: %d mylastterm: %d", last_idx, args.last_term, logs[last_idx].term);
    accept = last_idx == 0 || (logs.size() > (unsigned int) last_idx && logs[last_idx].term == args.last_term);
    log_mtx.unlock();
    return accept;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::send_entries(int target) {
    int last_idx = nextIndex[target] - 1;
    int last_term = logs[last_idx].term;
    std::vector <log_entry<command>> append_log(logs.begin() + nextIndex[target], logs.begin() + commit_index + 1);
    append_entries_args<command> args(1, current_term, last_term, last_idx, append_log);
    thread_pool->addObjJob(this, &raft::send_append_entries, target, args);
}

///TODO: check and send logs
template<typename state_machine, typename command>
void raft<state_machine, command>::check_and_send_logs() {

    log_mtx.lock();
    int curIdx = logs.size() - 1;
//    RAFT_LOG("Leader log size: %d, cmt_id: %d, app_id: %d", curIdx + 1, commit_index, apply_index);
    if (curIdx > apply_index && apply_index == commit_index)
        commit_index = curIdx;
    log_mtx.unlock();

    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
            bool needSend = nextIndex[i] - 1 < commit_index;
            if (needSend) {
//                RAFT_LOG("Leader send entries to followers[id = %d] (commit_id = %d)", i, commit_index);
                send_entries(i);
            }
        }
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::notify_commit() {
//    RAFT_LOG("Leader notify followers to commit (cmt_i: %d)", commit_index);
    append_entries_args<command> args(2, current_term);
    args.idx = commit_index;
    int num = rpc_clients.size();
    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
            thread_pool->addObjJob(this, &raft::send_append_entries, i, args);
        }
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::init_next_index() {
    int size = logs.size();
//    RAFT_LOG("init index with initial value: %d", size);
    for (int &next:nextIndex)
        next = size;
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::is_dead(int id) {
    return getCurrentTime() - ping_pong[id] > dead_thresh;
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::ready_to_commit() {
    int ok_no = 0;
    for (int i = 0; i < num; ++i) {
        if (i != my_id && nextIndex[i] - 1 == commit_index) {
            ++ok_no;
        }
    }
    return ok_no == num / 2;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::apply_log() {
    if (logs.size() > (unsigned int) commit_index) {
        for (int i = apply_index + 1; i <= commit_index; ++i) {
            RAFT_LOG("apply log[idx = %d, term = %d, value = %d]", i, logs[i].term, logs[i].cmd.value);
            storage->append_log(logs[i]);
            state->apply_log(logs[i].cmd);
        }
        apply_index = commit_index;
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::become_follower(int new_term) {
    raft_role old_role = role;
    role = follower;
    current_term = new_term;
    if (old_role == candidate) {
        init_timeout();
        vote_no = 0;
    } else {
        log_mtx.lock();
        commit_index = apply_index;
        log_mtx.unlock();
    }
}


#endif // raft_h
