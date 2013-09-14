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
#include "Bind.h"
#include "Clownfish/Util/StringHelper.h"
#include "Clownfish/Util/NumberUtils.h"

VALUE
Bind_cfish_to_ruby(cfish_Obj *obj) {
  if (CFISH_Obj_Is_A(obj, CFISH_STRING)) {
      return Bind_str_to_ruby((cfish_String*)obj);
  }
  else if (CFISH_Obj_Is_A(obj, CFISH_VARRAY)) {
      return S_cfish_array_to_ruby_array((cfish_VArray*)obj);
  }
}

VALUE
Bind_str_to_ruby(const cfish_String *str) {
    if (!str) {
        return rb_str_new2("");
    }
    else {
        return rb_str_new(CFISH_Str_Get_Ptr8(str), CFISH_Str_Get_Size(str));
    }
}

static VALUE
S_cfish_array_to_ruby_array(cfish_VArray *varray) {
    uint32_t num_elems = CFISH_VA_Get_Size(varray);

    VALUE ruby_array = rb_ary_new2(num_elems - 1);

    if (num_elems) {
        //TODO Need to determine why c99 mode is not being honored
        uint32_t i;
        for (uint32_t i = 0; i < num_elems; i++) {
            cfish_Obj *val = CFISH_VA_Fetch(varray, i);
            if (val == NULL) {
                continue;
            }
            else {
                // Recurse for each value.
                VALUE const val_ruby = Bind_cfish_to_ruby(val);
                rb_ary_store(ruby_array, i, val_ruby);
            }
        }
    }
    return ruby_array;
}
