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

#include "CFCCClass.h"
#include "CFCClass.h"
#include "CFCMethod.h"
#include "CFCUtil.h"

// Declare dummy host callbacks.
char*
CFCCClass_callback_decs(CFCClass *klass) {
    CFCMethod **fresh_methods = CFCClass_fresh_methods(klass);
    char       *cb_decs       = CFCUtil_strdup("");

    for (int meth_num = 0; fresh_methods[meth_num] != NULL; meth_num++) {
        CFCMethod *method = fresh_methods[meth_num];

        // Define callback to NULL.
        if (CFCMethod_novel(method) && !CFCMethod_final(method)) {
            const char *override_sym = CFCMethod_full_override_sym(method);
            cb_decs = CFCUtil_cat(cb_decs, "#define ", override_sym, " NULL\n",
                                  NULL);
        }
    }

    FREEMEM(fresh_methods);

    return cb_decs;
}

