struct AccountOp {
    string account<>;
    int amount;
};

struct TransferOp {
    string from<>;
    string to<>;
    int amount;
};
