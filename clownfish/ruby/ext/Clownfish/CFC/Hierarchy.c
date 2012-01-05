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

VALUE rb_cCFC_H;
VALUE rb_mCFC;

static VALUE ch_alloc(VALUE klass) {
    CFCHierarchy *rb_oCFC_H = CFCHierarchy_allocate();
    VALUE obj = Data_Wrap_Struct(klass, 0, CFCHierarchy_destroy, rb_oCFC_H);

    return obj;
}

static VALUE t_init(VALUE self, VALUE source, VALUE dest) {
    char *s = StringValuePtr(source);
    char *d = StringValuePtr(dest);
    CFCHierarchy *rb_oCFC_H; 

    Data_Get_Struct(self, CFCHierarchy, rb_oCFC_H);
    CFCHierarchy_init(rb_oCFC_H, s, d);

    return self;
}

static VALUE build(VALUE self) {
    CFCHierarchy *rb_oCFC_H; 

    Data_Get_Struct(self, CFCHierarchy, rb_oCFC_H);
    CFCHierarchy_build(rb_oCFC_H);

    return self;
}

void Init_Hierarchy() { 
    rb_mCFC     = rb_define_module("CFC");
    rb_cCFC_H   = rb_define_class_under(rb_mCFC, "Hierarchy", rb_cObject);

    rb_define_alloc_func(rb_cCFC_H, ch_alloc);
    rb_define_method(rb_cCFC_H, "initialize", t_init, 2);
    rb_define_method(rb_cCFC_H, "build", build, 0);
}

