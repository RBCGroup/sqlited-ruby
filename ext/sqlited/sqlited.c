#include "sqlited.h"

VALUE cSqlited;

static VALUE sqlited_initialize(VALUE self, VALUE file_path) {
  sqlitedRuby *ctx;
  int status;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  status = sqlite3_open(StringValuePtr(file_path), &ctx->db);
  return self;
}

static VALUE sqlited_exec(VALUE self, VALUE sql) {
  sqlitedRuby *ctx;
  int status;
  char *errMsg = NULL;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  status = sqlite3_exec(
    ctx->db,
    (const char *)StringValuePtr(sql),
    NULL,
    NULL,
    &errMsg
  );
  if (!status) {
    // TODO: err
  }
  return self;
}

static VALUE sqlited_prepare_sql(VALUE self, VALUE sql) {
  sqlitedRuby *ctx;
  const char *tail = NULL;
  int status;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  status = sqlite3_prepare_v2(
    ctx->db,
    (const char *)StringValuePtr(sql),
    (int)RSTRING_LEN(sql),
    &ctx->stmt,
    &tail
  );

  return self;
}

static VALUE sqlited_step(VALUE self) {
  sqlitedRuby *ctx;
  int status, column_count;
  VALUE columns;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  status = sqlite3_step(ctx->stmt);
  column_count = sqlite3_column_count(ctx->stmt);
  columns = rb_ary_new2((long)column_count);

  switch(status) {
    case SQLITE_ROW:
      {
        int i;
        for (i = 0 ; i < column_count ; i++) {
          switch (sqlite3_column_type(ctx->stmt, i)) {
            case SQLITE_INTEGER:
              rb_ary_push(columns, LL2NUM(sqlite3_column_int64(ctx->stmt, i)));
              break;
            case SQLITE_FLOAT:
              rb_ary_push(columns, rb_float_new(sqlite3_column_double(ctx->stmt, i)));
              break;
            case SQLITE_NULL:
              rb_ary_push(columns, Qnil);
              break;
            case SQLITE_TEXT:
              {
                VALUE str = rb_tainted_str_new(
                  (const char *)sqlite3_column_text(ctx->stmt, i),
                  (long)sqlite3_column_bytes(ctx->stmt, i)
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
      return Qnil;
      break;
    case SQLITE_OK:
      sqlite3_reset(ctx->stmt);
      return Qnil;
      break;
    default:
      rb_raise(rb_eRuntimeError, "something wrong occured");
  }
  return columns;
}

static VALUE sqlited_close(VALUE self) {
  sqlitedRuby *ctx;
  sqlite3 *db;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  db = ctx->db;
  sqlite3_close(ctx->db);
  ctx->db = NULL;
  ctx->stmt = NULL;

  return self;
}

static void deallocate(void * ctx) {
  sqlitedRuby *c = (sqlitedRuby *)ctx;
  sqlite3 *db = c->db;

  if(db) sqlite3_close(db);
  xfree(c);
}

static VALUE allocate(VALUE klass) {
  sqlitedRuby *ctx = xcalloc((size_t)1, sizeof(sqlitedRuby));
  return Data_Wrap_Struct(klass, NULL, deallocate, ctx);
}


void Init_sqlited(void) {
  cSqlited = rb_define_class("Sqlited", rb_cObject);

  rb_define_alloc_func(cSqlited, allocate);
  rb_define_method(cSqlited, "initialize",   sqlited_initialize, 1);
  rb_define_method(cSqlited, "exec",         sqlited_exec, 1);
  rb_define_method(cSqlited, "prepare_sql",  sqlited_prepare_sql, 1);
  rb_define_method(cSqlited, "step",         sqlited_step, 0);
  rb_define_method(cSqlited, "close",        sqlited_close, 0);
}
