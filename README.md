# Distributed Banking System (RPC + SQLite3)

This starter implements a **client → virtual bank middleware → bank servers (BANK1, BANK2)** architecture using ONC RPC (`rpcgen`) and SQLite3 persistence.

- BANK1 and BANK2 are independent RPC servers with their own `accounts` database.
- The **Virtual Bank** is an RPC server for the client and an RPC client of BANK1/2.
- Simple mapping rule: accounts starting with `A` → BANK1; accounts starting with `B` → BANK2.
- Banks never allow negative balances — debits/transfer roll back atomically.

Test sequence (run `make test`):

```
credit A12345 100
credit B12345 100
transfer A12345 B12345 25
transfer B12345 A12345 10
transfer B12345 A12345 120      # should fail (insufficient funds)
```

Outputs will show DB contents for BANK1 and BANK2.

> This project is tailored for Debian-based VMs with libtirpc (ONC RPC) and sqlite3 installed.
