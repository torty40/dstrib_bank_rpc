#include <rpc/rpc.h>
#include "bank1_if.h"
#include "src/bankdb.h"
#include <stdio.h>

static bankdb_t gdb;

int * b1_credit_1_svc(AccountOp *op, struct svc_req *rqstp){
    static int res;
    res = bankdb_credit(&gdb, op->account, op->amount);
    return &res;
}
int * b1_debit_1_svc(AccountOp *op, struct svc_req *rqstp){
    static int res;
    res = bankdb_debit(&gdb, op->account, op->amount);
    return &res;
}
int * b1_balance_1_svc(char **acc, struct svc_req *rqstp){
    static int res;
    res = bankdb_get_balance(&gdb, *acc);
    return &res;
}

int main(){
    if(bankdb_open(&gdb, "bank1.db")!=0){ fprintf(stderr,"BANK1: DB open failed\n"); return 1;}
    fprintf(stderr,"BANK1 server ready (DB=bank1.db)\n");
    svc_run(); // from generated _svc.c main, but we provide our own main to init DB; rpcgen -m gives no main.
    bankdb_close(&gdb);
    return 0;
}
