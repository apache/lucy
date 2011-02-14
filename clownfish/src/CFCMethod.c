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

#include <stdlib.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#define CFC_NEED_FUNCTION_STRUCT_DEF
#include "CFCFunction.h"
#include "CFCMethod.h"

struct CFCMethod {
    CFCFunction function;
};

CFCMethod*
CFCMethod_new(void *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym, 
              void *return_type, void *param_list, void *docucomment, 
              int is_inline)
{
    CFCMethod *self = (CFCMethod*)malloc(sizeof(CFCMethod));
    if (!self) { croak("malloc failed"); }
    return CFCMethod_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym, return_type, param_list, docucomment, is_inline);
}

CFCMethod*
CFCMethod_init(CFCMethod *self, void *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, void *return_type, void *param_list, 
               void *docucomment, int is_inline)
{
    CFCFunction_init((CFCFunction*)self, parcel, exposure, class_name,
        class_cnick, micro_sym, return_type, param_list, docucomment,
        is_inline);
    return self;
}

void
CFCMethod_destroy(CFCMethod *self)
{
    CFCFunction_destroy((CFCFunction*)self);
}


