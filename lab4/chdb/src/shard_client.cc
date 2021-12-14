#include "shard_client.h"


int shard_client::put(chdb_protocol::operation_var var, int &r) {
    // TODO: Your code here
    printf("Receive put Request! tx id:%d, key: %d, value: %d\n", var.tx_id, var.key, var.value);
    r = var.tx_id;
    store[primary_replica][var.key] = value_entry(var.value);
    return 0;
}

int shard_client::get(chdb_protocol::operation_var var, int &r) {
    // TODO: Your code here
    printf("Receive get Request! tx id:%d, key: %d\n", var.tx_id, var.key);
    r = store[primary_replica][var.key].value;
    return 0;
}

int shard_client::commit(chdb_protocol::commit_var var, int &r) {
    // TODO: Your code here

    // do backup
    backup();
    return 0;
}

int shard_client::rollback(chdb_protocol::rollback_var var, int &r) {
    // TODO: Your code here
    printf("Receive rollback Request! tx id:%d, key: %d\n", var.tx_id, var.key);
    store[primary_replica].erase(var.key);
    return 0;
}

int shard_client::check_prepare_state(chdb_protocol::check_prepare_state_var var, int &r) {
    // TODO: Your code here
    return 0;
}

int shard_client::prepare(chdb_protocol::prepare_var var, int &r) {
    // TODO: Your code here
    r = active;
    return 0;
}

shard_client::~shard_client() {
    delete node;
}

void shard_client::backup() {
    const auto &st = get_store();
    for (int i = 0; i < store.size(); ++i) {
        if (i != primary_replica)
            store[i] = st;
    }
}