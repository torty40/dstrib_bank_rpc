%#include "common_types.x"
program BANK1_PROG {
    version BANK_V1 {
        int B1_CREDIT(AccountOp) = 1;
        int B1_DEBIT(AccountOp) = 2;
        int B1_BALANCE(string) = 3;
    } = 1;
} = 0x31230010;
