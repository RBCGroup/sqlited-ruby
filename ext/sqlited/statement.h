#include <stdio.h>
#include <ruby.h>
#include <sqlite3.h>

struct _sqlitedStmtRuby {
  sqlite3_stmt *st;
  int done_p;
};

typedef struct _sqlitedStmtRuby sqlitedStmtRuby;
