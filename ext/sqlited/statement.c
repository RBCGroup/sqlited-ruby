#include "sqlited.h"
#include "statement.h"

VALUE cSqlitedStatement;

static void deallocate(void * ctx)
{
  sqlitedStmtRuby *c = (sqlitedStmtRuby *)ctx;
  xfree(c);
}

static VALUE allocate(VALUE klass)
{
  sqlitedStmtRuby *ctx = xcalloc((size_t)1, sizeof(sqlitedStmtRuby));
  ctx->stmt = NULL;
  ctx->done_p = 0;

  return Data_Wrap_Struct(klass, NULL, deallocate, ctx);
}

static VALUE sqlited_statement_initialize(VALUE self, VALUE sqlited, VALUE sql) {
  sqlitedRuby *ctx;
  sqlitedStmtRuby *stmt_ctx;
  const char *tail = NULL;
  int status;

  Data_Get_Struct(sqlited, sqlitedRuby, ctx);
  Data_Get_Struct(self, sqlitedStmtRuby, stmt_ctx);

  status = sqlite3_prepare_v2(
    ctx->db,
    (const char *)StringValuePtr(sql),
    (int)RSTRING_LEN(sql),
    &stmt_ctx->stmt,
    &tail
  );
  if (status != SQLITE_OK) {
    rb_raise(rb_eRuntimeError, "failed to prepare_sql");
  }
  return self;
}

static VALUE sqlited_statement_step(VALUE self) {
  sqlitedStmtRuby *stmt_ctx;
  int status, column_count;
  VALUE columns;

  Data_Get_Struct(self, sqlitedStmtRuby, stmt_ctx);

  status = sqlite3_step(stmt_ctx->stmt);
  column_count = sqlite3_column_count(stmt_ctx->stmt);
  columns = rb_ary_new2((long)column_count);

  switch(status) {
    case SQLITE_ROW:
      {
        int i;
        for (i = 0 ; i < column_count ; i++) {
          switch (sqlite3_column_type(stmt_ctx->stmt, i)) {
            case SQLITE_INTEGER:
              rb_ary_push(columns, LL2NUM(sqlite3_column_int64(stmt_ctx->stmt, i)));
              break;
            case SQLITE_FLOAT:
              rb_ary_push(columns, rb_float_new(sqlite3_column_double(stmt_ctx->stmt, i)));
              break;
            case SQLITE_NULL:
              rb_ary_push(columns, Qnil);
              break;
            case SQLITE_TEXT:
              {
                VALUE str = rb_tainted_str_new(
                  (const char *)sqlite3_column_text(stmt_ctx->stmt, i),
                  (long)sqlite3_column_bytes(stmt_ctx->stmt, i)
                );
                rb_ary_push(columns, str);
              }
              break;
            default:
              rb_raise(rb_eRuntimeError, "unknown type");
          }
        }
      }
      break;
    case SQLITE_DONE:
      stmt_ctx->done_p = 1;
      return Qnil;
      break;
    case SQLITE_OK:
      sqlite3_reset(stmt_ctx->stmt);
      stmt_ctx->done_p = 0;
      return Qnil;
      break;
    default:
      rb_raise(rb_eRuntimeError, "something wrong occured");
  }
  return columns;
}

static VALUE sqlited_statement_close(VALUE self)
{
  sqlitedStmtRuby *stmt_ctx;

  Data_Get_Struct(self, sqlitedStmtRuby, stmt_ctx);

  sqlite3_finalize(stmt_ctx->stmt);
  stmt_ctx->stmt = NULL;

  return self;
}

void init_sqlited_statement(void) {
  cSqlitedStatement = rb_define_class_under(cSqlited, "Statement", rb_cObject);

  rb_define_alloc_func(cSqlitedStatement, allocate);
  rb_define_method(cSqlitedStatement, "initialize",   sqlited_statement_initialize, 2);
  rb_define_method(cSqlitedStatement, "step",         sqlited_statement_step, 0);
  rb_define_method(cSqlitedStatement, "close",        sqlited_statement_close, 0);
}
