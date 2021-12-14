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
    // Notice: you should check whether is server should be stopped by calling is_stopped().
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

    rpcs *rpc_server;               // RPC server to recieve and handle the RPC requests
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
    long long last_received_RPC_time = getCurrentTime();

    int timeout;
    int voteNumber;

    int commitIndex;
    int lastApplied;
    int votedFor = -1;
    int num;

    std::vector<int> nextIndex;
    std::vector<int> matchIndex;

    std::vector <log_entry<command>> logs;

    std::mutex t_mtx;

    std::condition_variable cv;

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

    bool is_timeout();

    void init_timeout();

    void update_last_RPC_time();

    void start_election();

    inline void update_term(int new_term);

    void ping();

    void init_after_become_leader();

    void append_logs(const append_entries_args<command> &args);

    inline void candidate_become_follower();

    inline void leader_become_follower();

    void recover_from_logs();
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
        commitIndex(0),
        lastApplied(0) {
    thread_pool = new ThrPool(32);

    // Register the rpcs.
    rpc_server->reg(raft_rpc_opcodes::op_request_vote, this, &raft::request_vote);
    rpc_server->reg(raft_rpc_opcodes::op_append_entries, this, &raft::append_entries);
    rpc_server->reg(raft_rpc_opcodes::op_install_snapshot, this, &raft::install_snapshot);

    // Your code here:
    // Do the initialization
    num = rpc_clients.size();
    logs.push_back(log_entry<command>());
    nextIndex.assign(num, 0);
    matchIndex.assign(num, 0);
    init_timeout();
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
    mtx.lock();
    recover_from_logs();
    commitIndex = lastApplied;
    mtx.unlock();

    this->background_election = new std::thread(&raft::run_background_election, this);
    this->background_ping = new std::thread(&raft::run_background_ping, this);
    this->background_commit = new std::thread(&raft::run_background_commit, this);
    this->background_apply = new std::thread(&raft::run_background_apply, this);
}

template<typename state_machine, typename command>
bool raft<state_machine, command>::new_command(command cmd, int &term, int &index) {
    // Your code here:
    std::lock_guard <std::mutex> lock(mtx);
    if (role != leader) return false;
//    RAFT_LOG("leader receive a command[value = %d]", cmd.value);
    term = current_term;
//    RAFT_LOG("Leader append log[value = %d]", cmd.value);
    index = logs.size();
    log_entry<command> log = log_entry<command>(current_term, cmd);
    storage->append_log(index, log);
    logs.push_back(log);

//    cv.wait_until(lock);
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
    std::lock_guard <std::mutex> lock(mtx);
//    RAFT_LOG("receive vote req");
    if (args.term >= current_term) {
        if (args.term > current_term) {
            if (role == leader)
                leader_become_follower();
            else if (role == candidate)
                candidate_become_follower();
            update_term(args.term);
        }
        int myLastLogIndex = logs.size() - 1;
        int myLastLogTerm = logs[myLastLogIndex].term;
        bool granted = (votedFor == -1 || votedFor == args.candidateId) && (myLastLogTerm < args.lastLogTerm ||
                                                                            (myLastLogTerm == args.lastLogTerm &&
                                                                             myLastLogIndex <= args.lastLogIndex));
        if ((reply.voteGranted = granted))
            votedFor = args.candidateId;
    } else {
//        RAFT_LOG("reject vote, my term: %d, given term: %d", current_term, args.term);
        reply.voteGranted = 0;
    }
    reply.term = current_term;
    return 0;
}


template<typename state_machine, typename command>
void raft<state_machine, command>::handle_request_vote_reply(int target, const request_vote_args &arg,
                                                             const request_vote_reply &reply) {
    // Your code here:
    std::lock_guard <std::mutex> lock(mtx);
    if (role != candidate) return;
    if (reply.voteGranted) {
//        RAFT_LOG("get vote from %d", target);
        if (role == candidate) {
            if (++voteNumber > num / 2) {
//                RAFT_LOG("get vote: %d, has become a leader", voteNumber);
                role = leader; // if surpass a half, then become the leader.
                init_after_become_leader();
                ping();
            }
        }
    } else if (reply.term > current_term) {
        update_term(reply.term);
        candidate_become_follower();
    }

    return;
}


template<typename state_machine, typename command>
int raft<state_machine, command>::append_entries(append_entries_args<command> arg, append_entries_reply &reply) {
    // Your code here:
//    RAFT_LOG("receive");
    std::lock_guard <std::mutex> lock(mtx);

    if (arg.term >= current_term) {
//        RAFT_LOG("last received: %lld", last_received_RPC_time);
        update_term(arg.term);
        if (role == leader) {
            leader_become_follower();
        } else if (role == candidate) {
            candidate_become_follower();
        }

        reply.success = (logs.size() > (unsigned int) arg.prevLogIndex &&
                         logs[arg.prevLogIndex].term == arg.prevLogTerm);

        if (reply.success) {
            logs.resize(arg.prevLogIndex + 1);
//            RAFT_LOG("succeed with prevlogINex = %d, logsize = %d", arg.prevLogIndex, logs.size());
            if (!arg.entries.empty()) append_logs(arg);
            if (arg.leaderCommit > commitIndex) {
//                RAFT_LOG("commitIndex set to %d", commitIndex);
                commitIndex = std::min((int) (logs.size() - 1), arg.leaderCommit);
            }
        } else if (logs.size() > (unsigned int) arg.prevLogIndex) {
//            RAFT_LOG("fail and resize to %d", arg.prevLogIndex)
            logs.resize(arg.prevLogIndex);
        }
        reply.term = current_term;
        update_last_RPC_time();
    } else {
        reply.success = 0;
        reply.term = current_term;
    }
    return 0;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::handle_append_entries_reply(int target, const append_entries_args<command> &arg,
                                                               const append_entries_reply &reply) {
    // Your code here:
    int sizeAfterAppend = arg.prevLogIndex + 1 + arg.entries.size();
    std::lock_guard <std::mutex> lock(mtx);
    if (role != leader) return;
    if (reply.success) {
        if (!arg.entries.empty()) {
            nextIndex[target] = sizeAfterAppend;
            matchIndex[target] = sizeAfterAppend - 1;
        }
    } else if (reply.term > current_term) {
        update_term(reply.term);
        leader_become_follower();
    } else {
        nextIndex[target] = arg.prevLogIndex;
    }
    return;
}


template<typename state_machine, typename command>
int raft<state_machine, command>::install_snapshot(install_snapshot_args args, install_snapshot_reply &reply) {
    // Your code here:
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
//                    RAFT_LOG("current time: %lld", getCurrentTime());
//                    RAFT_LOG("time out[%d ms], become a candidate.", timeout);
                    start_election();
                }
                break;
            }
            case candidate: {
                if (is_timeout()) {
//                    RAFT_LOG("time out, election failed, try again");
                    candidate_become_follower();
//                    last_received_RPC_time = getCurrentTime();
//                    start_election();
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
        mtx.lock();
        if (role == leader) {
            for (int i = 0; i < num; ++i) {
                if (i != my_id) {
                    int logIndex = logs.size() - 1;
//                    RAFT_LOG("logIndex = %d and nextIndex[%d] = %d", logIndex, i, nextIndex[i]);
                    if (logIndex >= nextIndex[i]) {
//                        RAFT_LOG("Leader send logs to Follower[%d]", i);
                        int prevLogIndex = nextIndex[i] - 1;
                        int prevLogTerm = logs[prevLogIndex].term;
                        auto entries = std::vector<log_entry<command>>(logs.begin() + nextIndex[i], logs.end());
                        append_entries_args<command> args(current_term, my_id, prevLogIndex, prevLogTerm, entries,
                                                          commitIndex);
                        thread_pool->addObjJob(this, &raft::send_append_entries, i, args);
                    }
                }
            }
            int median;
            std::vector<int> tmp;
            for (int i = 0; i < num; ++i) {
                if (i != my_id)
                    tmp.push_back(matchIndex[i]);
            }
            tmp.push_back(logs.size());
            std::sort(tmp.begin(), tmp.end());
            median = tmp[num / 2];
            if (median > commitIndex && logs[median].term == current_term) {
                commitIndex = median;
//                RAFT_LOG("commitIndex set to %d", commitIndex);
                ping();
//                RAFT_LOG("get majority append ok, commit id = %d", commitIndex);
            }
        }
        mtx.unlock();
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

        mtx.lock();
        if (commitIndex > lastApplied) {
            ++lastApplied;
//            RAFT_LOG("apply log[id = %d, value = %d]", lastApplied, logs[lastApplied].cmd.value);
            storage->apply_log(lastApplied);
            state->apply_log(logs[lastApplied].cmd);
        }
        mtx.unlock();

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
        mtx.lock();
        if (role == leader) {
            ping();
        }
        mtx.unlock();

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
bool raft<state_machine, command>::is_timeout() {
//    t_mtx.lock();
    long long current_time = getCurrentTime();
    bool ret = current_time - last_received_RPC_time > timeout;
//    RAFT_LOG("cur: %d, last received: %d", current_time, last_received_RPC_time);
//    t_mtx.unlock();
    return ret;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::update_last_RPC_time() {
//    t_mtx.lock();
    last_received_RPC_time = getCurrentTime();
//    t_mtx.unlock();
}

template<typename state_machine, typename command>
void raft<state_machine, command>::start_election() {
//    RAFT_LOG("start election");
    std::lock_guard <std::mutex> lock(mtx);
    voteNumber = 1;
    votedFor = my_id;
    update_term(current_term + 1);
    role = candidate;
    timeout = 1000;
    int lastLogIndex = logs.size() - 1;
    int lastLogTerm = logs[lastLogIndex].term;
    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
            request_vote_args args(current_term, my_id, lastLogIndex, lastLogTerm);
            thread_pool->addObjJob(this, &raft::send_request_vote, i, args);
        }
    }
}


template<typename state_machine, typename command>
void raft<state_machine, command>::update_term(int new_term) {
    current_term = new_term;
    storage->store_metadata(new_term);
}


template<typename state_machine, typename command>
void raft<state_machine, command>::ping() {
//    RAFT_LOG("leader ping");
    for (int i = 0; i < num; ++i) {
        if (i != my_id) {
//            send_append_entries(i, args);
            int prevLogIndex = nextIndex[i] - 1;
            int prevLogTerm = logs[prevLogIndex].term;
            auto entries = std::vector<log_entry<command>>();
            append_entries_args<command> args(current_term, my_id, prevLogIndex, prevLogTerm, entries,
                                              commitIndex);
            thread_pool->addObjJob(this, &raft::send_append_entries, i, args);
        }
    }
}

// need lock
template<typename state_machine, typename command>
void raft<state_machine, command>::init_after_become_leader() {
    std::fill(nextIndex.begin(), nextIndex.end(), logs.size());
    std::fill(matchIndex.begin(), matchIndex.end(), 0);
    voteNumber = 0;
    votedFor = -1;
}

template<typename state_machine, typename command>
void raft<state_machine, command>::append_logs(const append_entries_args<command> &args) {
    auto &entries = args.entries;
    int index = args.prevLogIndex + 1;
    for (const auto &entry:entries) {
//        RAFT_LOG("Follower append log[value = %d]", entry.cmd.value);
        storage->append_log(index++, entry);
        logs.push_back(entry);
    }
}

template<typename state_machine, typename command>
void raft<state_machine, command>::candidate_become_follower() {
    role = follower;
    voteNumber = 0;
    votedFor = -1;
    init_timeout();
}

template<typename state_machine, typename command>
void raft<state_machine, command>::leader_become_follower() {
    role = follower;
    init_timeout();
}

template<typename state_machine, typename command>
void raft<state_machine, command>::recover_from_logs() {
    std::list <action<command>> actions;
    storage->read_logs(actions);
    current_term = storage->read_term();
//    RAFT_LOG("read logs");
//    RAFT_LOG("read term: %d", current_term);
    for (auto &action:actions) {
        if (action.type == APPEND) {
            logs.resize(action.index + 1);
            logs[action.index] = action.log;
//            RAFT_LOG("append log[id = %d, term = %d, value = %d]", action.index, action.log.term, action.log.cmd.value);
        } else {
//            RAFT_LOG("apply log[id = %d, term = %d, value = %d]", action.index, logs[action.index].term,
//                     logs[action.index].cmd.value);
            state->apply_log(logs[action.index].cmd);
            lastApplied = action.index;
        }
//        RAFT_LOG("apply log[term = %d, value = %d]", log.term, log.cmd.value);
    }
}

#endif // raft_h