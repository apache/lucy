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

#include "XSBind.h"
#include "Lucy/Store/FSFolder.h"

cfish_CharBuf*
lucy_FSFolder_absolutify(const cfish_CharBuf *path) {
    dSP;
    ENTER;
    SAVETMPS;
    EXTEND(SP, 2);
    PUSHMARK(SP);
    PUSHmortal;
    mPUSHs(XSBind_cb_to_sv(path));
    PUTBACK;
    call_pv("Lucy::Store::FSFolder::absolutify", G_SCALAR);
    SPAGAIN;
    cfish_CharBuf *absolutified
        = (cfish_CharBuf*)XSBind_perl_to_cfish(POPs);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return absolutified;
}

