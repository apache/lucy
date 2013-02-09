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

#include "Clownfish/Host.h"
#include "Clownfish/Util/Memory.h"
#include "Clownfish/Util/StringHelper.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Test/TestCharBuf.h"
#include "Clownfish/Test.h"

static VALUE mClownfish;
static VALUE cTest;

static VALUE
S_CFC_Test_Run_Tests(VALUE self_rb, VALUE package) {

  /*if (strEQ(StringValuePtr(package), "TestCharBuf")) {
    lucy_TestCB_run_tests();
  }*/

  return Qnil;
}

static void
S_init_Test(void) {
    cTest = rb_define_class_under(mClownfish, "Test", rb_cObject);
//    rb_define_singleton_method(cTest, "run_tests", S_CFC_Hierarchy_Build, 0);
}

void
Init_Clownfish() { 
    mClownfish  = rb_define_module("Clownfish");
    S_init_Test();
}

