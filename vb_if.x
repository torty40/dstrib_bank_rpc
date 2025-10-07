%#include "common_types.x"
program VB_PROG {
    version VB_V1 {
        int VB_CREDIT(AccountOp) = 1;
        int VB_DEBIT(AccountOp) = 2;
        int VB_TRANSFER(TransferOp) = 3;
    } = 1;
} = 0x31230020;
