#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vb_if.h"

static void usage_line(){
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  credit <acct> <amount>\n");
    fprintf(stderr, "  debit  <acct> <amount>\n");
    fprintf(stderr, "  transfer <acctFrom> <acctTo> <amount>\n");
}

int main(){
    CLIENT* c = clnt_create("localhost", VB_PROG, VB_V1, "udp");
    if(!c){ clnt_pcreateerror("clnt_create"); return 1; }

    char cmd[256];
    while(fgets(cmd, sizeof(cmd), stdin)){
        char a[64], b[64];
        int amt;
        if(sscanf(cmd, "credit %63s %d", a, &amt)==2){
            AccountOp op; op.account=a; op.amount=amt;
            int *r = vb_credit_1(&op, c);
            printf("credit %s %d -> %d\n", a, amt, r?*r:-999);
        } else if(sscanf(cmd, "debit %63s %d", a, &amt)==2){
            AccountOp op; op.account=a; op.amount=amt;
            int *r = vb_debit_1(&op, c);
            printf("debit %s %d -> %d\n", a, amt, r?*r:-999);
        } else if(sscanf(cmd, "transfer %63s %63s %d", a, b, &amt)==3){
            TransferOp tr; tr.from=a; tr.to=b; tr.amount=amt;
            int *r = vb_transfer_1(&tr, c);
            printf("transfer %s %s %d -> %d\n", a, b, amt, r?*r:-999);
        } else if(strncmp(cmd,"help",4)==0){
            usage_line();
        } else if(strlen(cmd)>1){
            fprintf(stderr, "Could not parse: %s", cmd);
            usage_line();
        }
    }
    clnt_destroy(c);
    return 0;
}
