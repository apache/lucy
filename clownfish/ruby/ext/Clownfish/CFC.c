/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ruby.h"
#include "CFC.h"

static VALUE mBinding;
static VALUE mClownfish;
static VALUE mCFC;
static VALUE cBindCore;
static VALUE cHierarchy;

static VALUE cfc_binding_core_alloc(VALUE klass) {
    CFCBindCore *self = CFCBindCore_allocate();

    VALUE self_rb = Data_Wrap_Struct(klass, 0, CFCBindCore_destroy, self);

    return self_rb;
}

static VALUE cfc_binding_core_init(VALUE self_rb, VALUE hierarchy, VALUE dest, VALUE header, VALUE footer) {
    CFCHierarchy* hierarchy_obj;
    CFCBindCore *self;

    Data_Get_Struct(hierarchy,CFCHierarchy,hierarchy_obj);
    Data_Get_Struct(self_rb, CFCBindCore, self);

    CFCBindCore_init(self, hierarchy_obj, StringValuePtr(dest), StringValuePtr(header), StringValuePtr(footer));

    return self_rb;
}

static VALUE cfc_binding_core_write_all_modified(int argc, VALUE *argv, VALUE self_rb) {
    CFCBindCore *self;

    int modified = argc > 0 && RTEST(argv[0]) ? 1 : 0;

    Data_Get_Struct(self_rb, CFCBindCore, self);
    CFCBindCore_write_all_modified(self,modified);

    return Qnil;
}

static VALUE cfc_hierarchy_alloc(VALUE klass) {
    CFCHierarchy *self = CFCHierarchy_allocate();

    VALUE self_rb = Data_Wrap_Struct(klass, 0, CFCHierarchy_destroy, self);

    return self_rb;
}

static VALUE cfc_hierarchy_init(VALUE self_rb, VALUE source, VALUE dest) {

    char *s = StringValuePtr(source);
    char *d = StringValuePtr(dest);
    CFCHierarchy *self;

    Data_Get_Struct(self_rb, CFCHierarchy, self);
    CFCHierarchy_init(self, s, d);

    return self_rb;
}

static VALUE cfc_hierarchy_build(VALUE self_rb) {
    CFCHierarchy *self;

    Data_Get_Struct(self_rb, CFCHierarchy, self);
    CFCHierarchy_build(self);

    return Qnil;
}

void Init_CFC() { 
    mClownfish  = rb_define_module("Clownfish");
    mCFC        = rb_define_module_under(mClownfish, "CFC");
    mBinding    = rb_define_module_under(mCFC, "Binding");
    cHierarchy  = rb_define_class_under(mCFC, "Hierarchy", rb_cObject);
    cBindCore   = rb_define_class_under(mBinding, "Core", rb_cObject);

    rb_define_alloc_func(cHierarchy, cfc_hierarchy_alloc);
    rb_define_method(cHierarchy, "initialize", cfc_hierarchy_init, 2);
    rb_define_method(cHierarchy, "build", cfc_hierarchy_build, 0);

    rb_define_alloc_func(cBindCore, cfc_binding_core_alloc);
    rb_define_method(cBindCore, "initialize", cfc_binding_core_init, 4);
    rb_define_method(cBindCore, "write_all_modified", cfc_binding_core_write_all_modified, -1);
}

