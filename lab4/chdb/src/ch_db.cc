#include "ch_db.h"

int view_server::execute(unsigned int query_key, unsigned int proc, const chdb_protocol::operation_var &var, int &r) {
    // TODO: Your code here
    chdb_command::command_type cmd_type;
    switch (proc) {
        case chdb_protocol::Get:
            cmd_type = chdb_command::CMD_GET;
            break;
        case chdb_protocol::Put:
            cmd_type = chdb_command::CMD_PUT;
            break;
        default:
            chdb_command::CMD_NONE;
            break;
    }

//    printf("Append raft cmd.\n");
//    chdb_command cmd(cmd_type, var.key, var.value, var.tx_id);
//    int leader = raft_group->check_exact_one_leader();
//    int term, index;
//    ASSERT(raft_group->nodes[leader]->new_command(cmd, term, index), "invalid leader");
//    std::unique_lock <std::mutex> lock(cmd.res->mtx);
//    if (!cmd.res->done) {
//        if (cmd.res->cv.wait_until(lock, std::chrono::system_clock::now() + std::chrono::milliseconds(2500)) ==
//            std::cv_status::timeout) {
//            printf("Timeout!\n");
//        }
//    }
//    printf("Append finish.\n");

    int base_port = this->node->port();
    int shard_offset = this->dispatch(query_key, shard_num());

    return this->node->template call(base_port + shard_offset, proc, var, r);
}

int view_server::prepare(unsigned int query_key, unsigned int proc, const chdb_protocol::prepare_var &var, int &r) {
    int base_port = this->node->port();
    int shard_offset = this->dispatch(query_key, shard_num());

    return this->node->template call(base_port + shard_offset, proc, var, r);
}

int view_server::rollback(unsigned int query_key, unsigned int proc, const chdb_protocol::rollback_var &var, int &r) {
    int base_port = this->node->port();
    int shard_offset = this->dispatch(query_key, shard_num());

    return this->node->template call(base_port + shard_offset, proc, var, r);
}

view_server::~view_server() {
#if RAFT_GROUP
    delete this->raft_group;
#endif
    delete this->node;

}