#include "tx_region.h"


int tx_region::put(const int key, const int val) {
    // TODO: Your code here
    int r;
    this->actions.push_back(action(key, val));
    this->db->vserver->execute(key,
                               chdb_protocol::Put,
                               chdb_protocol::operation_var{.tx_id = tx_id, .key = key, .value = val},
                               r);
    printf("Put store[%d]=%d\n", key, val);
    read_only = false;
    return r;
}

int tx_region::get(const int key) {
    // TODO: Your code here
    int r;
    this->actions.push_back(action(key));
    this->db->vserver->execute(key,
                               chdb_protocol::Get,
                               chdb_protocol::operation_var{.tx_id = tx_id, .key = key},
                               r);
    printf("Get store[%d]=%d\n", key, r);
    return r;
}

void tx_region::rollback() {
    for (action act:actions) {
        int r;
        this->db->vserver->rollback(act.key,
                                    chdb_protocol::Rollback,
                                    chdb_protocol::rollback_var{.tx_id = tx_id, .key = act.key},
                                    r);
    }
}

int tx_region::tx_can_commit() {
    // TODO: Your code here
    int status = chdb_protocol::prepare_ok;
    if (!read_only) {
        for (action act:actions) {
            int r;
            this->db->vserver->prepare(act.key,
                                       chdb_protocol::Prepare,
                                       chdb_protocol::prepare_var{.tx_id = tx_id},
                                       r);
            if (r == 0) {
                status = chdb_protocol::prepare_not_ok;
                break;
            }
        }
    }
    if (status == chdb_protocol::prepare_not_ok) {
        rollback();
    }
    return status;
}

int tx_region::tx_begin() {
    // TODO: Your code here
    printf("tx[%d] begin\n", tx_id);
    return 0;
}

int tx_region::tx_commit() {
    // TODO: Your code here
    printf("tx[%d] commit\n", tx_id);
    return 0;
}

int tx_region::tx_abort() {
    // TODO: Your code here
    printf("tx[%d] abort\n", tx_id);
    return 0;
}
