#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include "vb_if.h"
#include "bank1_if.h"
#include "bank2_if.h"

static int which_bank(const char* account){
    if(account && account[0]=='A') return 1;
    if(account && account[0]=='B') return 2;
    return 0;
}

static int call_bank_credit(int bank, const char* account, int amount){
    CLIENT* c = NULL;
    AccountOp op; op.account=(char*)account; op.amount=amount;
    int *res = NULL;
    if(bank==1){
        c = clnt_create("localhost", BANK1_PROG, BANK_V1, "udp");
        if(!c) return -100;
        res = b1_credit_1(&op, c);
    } else if(bank==2){
        c = clnt_create("localhost", BANK2_PROG, BANK_V1, "udp");
        if(!c) return -100;
        res = b2_credit_1(&op, c);
    }
    if(c) clnt_destroy(c);
    return res? *res : -101;
}

static int call_bank_debit(int bank, const char* account, int amount){
    CLIENT* c = NULL;
    AccountOp op; op.account=(char*)account; op.amount=amount;
    int *res = NULL;
    if(bank==1){
        c = clnt_create("localhost", BANK1_PROG, BANK_V1, "udp");
        if(!c) return -100;
        res = b1_debit_1(&op, c);
    } else if(bank==2){
        c = clnt_create("localhost", BANK2_PROG, BANK_V1, "udp");
        if(!c) return -100;
        res = b2_debit_1(&op, c);
    }
    if(c) clnt_destroy(c);
    return res? *res : -101;
}

int * vb_credit_1_svc(AccountOp *op, struct svc_req *rq){
    static int res;
    int bank = which_bank(op->account);
    if(!bank){ res = -2; return &res; }
    res = call_bank_credit(bank, op->account, op->amount);
    return &res;
}

int * vb_debit_1_svc(AccountOp *op, struct svc_req *rq){
    static int res;
    int bank = which_bank(op->account);
    if(!bank){ res = -2; return &res; }
    res = call_bank_debit(bank, op->account, op->amount);
    return &res;
}

int * vb_transfer_1_svc(TransferOp *tr, struct svc_req *rq){
    static int res;
    int b_from = which_bank(tr->from);
    int b_to   = which_bank(tr->to);
    if(!b_from || !b_to){ res = -2; return &res; }

    // Simple two-phase: debit then credit; if credit fails, refund.
    int deb = call_bank_debit(b_from, tr->from, tr->amount);
    if(deb!=0){ res = deb; return &res; }

    int cre = call_bank_credit(b_to, tr->to, tr->amount);
    if(cre!=0){
        // refund
        call_bank_credit(b_from, tr->from, tr->amount);
        res = cre;
        return &res;
    }
    res = 0;
    return &res;
}

int main(){
    fprintf(stderr,"Virtual Bank server ready (maps A*→BANK1, B*→BANK2)\n");
    svc_run();
    return 0;
}
