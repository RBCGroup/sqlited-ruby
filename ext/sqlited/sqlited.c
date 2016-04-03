#include "sqlited.h"

VALUE cSqlited;

static VALUE sqlited_initialize(VALUE file_path, VALUE self) {
  sqlitedRubyPtr ctx;
  rb_iv_set(self, "@file_path", file_path);
  return self;
}

void Init_sqlited(void) {
  cSqlited = rb_define_class("Sqlited", rb_cObject)
  rb_define_method(cSqlited, "initialize", sqlited_initialize, 1);
}
