%#include "common_types.x"
program BANK2_PROG {
    version BANK_V1 {
        int B2_CREDIT(AccountOp) = 1;
        int B2_DEBIT(AccountOp) = 2;
        int B2_BALANCE(string) = 3;
    } = 1;
} = 0x31230011;
