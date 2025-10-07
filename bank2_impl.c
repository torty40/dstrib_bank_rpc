#include <rpc/rpc.h>
#include "bank2_if.h"
#include "src/bankdb.h"
#include <stdio.h>

static bankdb_t gdb;

int * b2_credit_1_svc(AccountOp *op, struct svc_req *rqstp){
    static int res;
    res = bankdb_credit(&gdb, op->account, op->amount);
    return &res;
}
int * b2_debit_1_svc(AccountOp *op, struct svc_req *rqstp){
    static int res;
    res = bankdb_debit(&gdb, op->account, op->amount);
    return &res;
}
int * b2_balance_1_svc(char **acc, struct svc_req *rqstp){
    static int res;
    res = bankdb_get_balance(&gdb, *acc);
    return &res;
}

int main(){
    if(bankdb_open(&gdb, "bank2.db")!=0){ fprintf(stderr,"BANK2: DB open failed\n"); return 1;}
    fprintf(stderr,"BANK2 server ready (DB=bank2.db)\n");
    svc_run();
    bankdb_close(&gdb);
    return 0;
}
