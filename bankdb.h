#pragma once
#include <sqlite3.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sqlite3* db;
    const char* path;
} bankdb_t;

int bankdb_open(bankdb_t* ctx, const char* path);
void bankdb_close(bankdb_t* ctx);
int bankdb_credit(bankdb_t* ctx, const char* account, int amount);
int bankdb_debit(bankdb_t* ctx, const char* account, int amount);
int bankdb_get_balance(bankdb_t* ctx, const char* account);
int bankdb_ensure_schema(bankdb_t* ctx);

#ifdef __cplusplus
}
#endif
