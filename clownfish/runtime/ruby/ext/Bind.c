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
  if (Cfish_Obj_Is_A(obj, CFISH_CHARBUF)) {
      return Bind_cb_to_ruby((cfish_CharBuf*)obj);
  }
}

VALUE
Bind_cb_to_ruby(const cfish_CharBuf *cb) {
    if (!cb) {
        return rb_str_new2("");
    }
    else {
        return rb_str_new((char*)Cfish_CB_Get_Ptr8(cb), Cfish_CB_Get_Size(cb));
    }
}
