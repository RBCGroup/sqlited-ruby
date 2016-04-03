#include <stdio.h>
#include <ruby.h>
#include <sqlite3.h>

struct _sqlitedRuby {
  sqlite3 *db;
  sqlite3_stmt *stmt;
};

typedef struct _sqlitedRuby sqlitedRuby;
