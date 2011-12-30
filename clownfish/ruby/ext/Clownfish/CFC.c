#include "ruby.h"
#include "CFC.h"

VALUE cCFC;

static VALUE t_init(VALUE self)
{
    
}

void Init_CFC() { 
    cCFC = rb_define_class("CFC", rb_cObject);
    rb_define_method(cCFC, "initialize", t_init, 0);
}

