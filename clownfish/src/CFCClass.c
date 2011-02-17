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
#include <string.h>
#include <ctype.h>
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#ifndef true
  #define true 1
  #define false 0
#endif

#define CFC_NEED_SYMBOL_STRUCT_DEF
#include "CFCSymbol.h"
#include "CFCClass.h"
#include "CFCParcel.h"

struct CFCClass {
    CFCSymbol symbol;
};

CFCClass*
CFCClass_new(struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym)
{
    CFCClass *self = (CFCClass*)CFCBase_allocate(sizeof(CFCClass),
        "Clownfish::Class");
    if (!self) { croak("malloc failed"); }
    return CFCClass_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym);
}

CFCClass*
CFCClass_init(CFCClass *self, struct CFCParcel *parcel, 
               const char *exposure, const char *class_name, 
               const char *class_cnick, const char *micro_sym)
{
    CFCSymbol_init((CFCSymbol*)self, parcel, exposure, class_name, 
        class_cnick, micro_sym);
    return self;
}

void
CFCClass_destroy(CFCClass *self)
{
    CFCSymbol_destroy((CFCSymbol*)self);
}

