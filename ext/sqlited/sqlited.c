#include "sqlited.h"

VALUE cSqlited;

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

static VALUE sqlited_initialize(VALUE self, VALUE file_path) {
  sqlitedRuby *ctx;
  int status;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  status = sqlite3_open(StringValuePtr(file_path), &ctx->db);

  if (!status) {
    rb_raise(rb_eRuntimeError, "failed to initialize");
  }
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
    rb_raise(rb_eRuntimeError, "failed to exec_sql");
  }
  return self;
}

static VALUE sqlited_close(VALUE self) {
  sqlitedRuby *ctx;
  sqlite3 *db;

  Data_Get_Struct(self, sqlitedRuby, ctx);

  db = ctx->db;
  sqlite3_close(ctx->db);
  ctx->db = NULL;

  return self;
}

void Init_sqlited(void) {
  cSqlited = rb_define_class("Sqlited", rb_cObject);

  rb_define_alloc_func(cSqlited, allocate);
  rb_define_method(cSqlited, "initialize",   sqlited_initialize, 1);
  rb_define_method(cSqlited, "exec",         sqlited_exec, 1);
  rb_define_method(cSqlited, "close",        sqlited_close, 0);

  init_sqlited_statement();
}
