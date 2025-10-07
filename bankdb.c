#include "bankdb.h"
#include <stdio.h>
#include <string.h>

int bankdb_open(bankdb_t* ctx, const char* path){
    ctx->db = NULL;
    ctx->path = path;
    if(sqlite3_open(path, &ctx->db) != SQLITE_OK){
        fprintf(stderr, "sqlite open %s: %s\n", path, sqlite3_errmsg(ctx->db));
        return -1;
    }
    return bankdb_ensure_schema(ctx);
}

void bankdb_close(bankdb_t* ctx){
    if(ctx->db) sqlite3_close(ctx->db);
    ctx->db = NULL;
}

int bankdb_ensure_schema(bankdb_t* ctx){
    const char* ddl = "CREATE TABLE IF NOT EXISTS accounts("
                      "account TEXT PRIMARY KEY,"
                      "balance INTEGER NOT NULL DEFAULT 0);";
    char* err = NULL;
    if(sqlite3_exec(ctx->db, ddl, NULL, NULL, &err) != SQLITE_OK){
        fprintf(stderr, "schema error: %s\n", err);
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

static int upsert_account(sqlite3* db, const char* account){
    sqlite3_stmt* st=NULL;
    const char* sql = "INSERT INTO accounts(account,balance) VALUES(?,0) "
                      "ON CONFLICT(account) DO NOTHING;";
    if(sqlite3_prepare_v2(db, sql, -1, &st, NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text(st,1,account,-1,SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    return (rc==SQLITE_DONE)?0:-1;
}

int bankdb_credit(bankdb_t* ctx, const char* account, int amount){
    if(amount<0) return -2;
    sqlite3_exec(ctx->db, "BEGIN IMMEDIATE;", NULL,NULL,NULL);
    if(upsert_account(ctx->db, account)!=0){ sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -1;}
    sqlite3_stmt* st=NULL;
    const char* sql = "UPDATE accounts SET balance = balance + ? WHERE account=?;";
    sqlite3_prepare_v2(ctx->db, sql, -1, &st, NULL);
    sqlite3_bind_int(st,1,amount);
    sqlite3_bind_text(st,2,account,-1,SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if(rc!=SQLITE_DONE){ sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -1;}
    sqlite3_exec(ctx->db, "COMMIT;", NULL,NULL,NULL);
    return 0;
}

int bankdb_debit(bankdb_t* ctx, const char* account, int amount){
    if(amount<0) return -2;
    sqlite3_exec(ctx->db, "BEGIN IMMEDIATE;", NULL,NULL,NULL);
    if(upsert_account(ctx->db, account)!=0){ sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -1;}
    sqlite3_stmt* st=NULL;
    const char* sql1 = "SELECT balance FROM accounts WHERE account=?;";
    sqlite3_prepare_v2(ctx->db, sql1, -1, &st, NULL);
    sqlite3_bind_text(st,1,account,-1,SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    if(rc!=SQLITE_ROW){ sqlite3_finalize(st); sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -1;}
    int bal = sqlite3_column_int(st,0);
    sqlite3_finalize(st);
    if(bal < amount){ sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -3; } // insufficient

    const char* sql2 = "UPDATE accounts SET balance = balance - ? WHERE account=?;";
    sqlite3_prepare_v2(ctx->db, sql2, -1, &st, NULL);
    sqlite3_bind_int(st,1,amount);
    sqlite3_bind_text(st,2,account,-1,SQLITE_TRANSIENT);
    rc = sqlite3_step(st);
    sqlite3_finalize(st);
    if(rc!=SQLITE_DONE){ sqlite3_exec(ctx->db,"ROLLBACK;",NULL,NULL,NULL); return -1;}
    sqlite3_exec(ctx->db, "COMMIT;", NULL,NULL,NULL);
    return 0;
}

int bankdb_get_balance(bankdb_t* ctx, const char* account){
    sqlite3_stmt* st=NULL;
    const char* sql = "SELECT balance FROM accounts WHERE account=?;";
    if(sqlite3_prepare_v2(ctx->db, sql, -1, &st, NULL)!=SQLITE_OK) return -1;
    sqlite3_bind_text(st,1,account,-1,SQLITE_TRANSIENT);
    int rc = sqlite3_step(st);
    int bal = (rc==SQLITE_ROW) ? sqlite3_column_int(st,0) : -1;
    sqlite3_finalize(st);
    return bal;
}
