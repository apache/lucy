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

#define CFC_NEED_SYMBOL_STRUCT_DEF
#include "CFCSymbol.h"

CFCSymbol*
CFCSymbol_new(void *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym);

CFCSymbol*
CFCSymbol_new(void *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym)
{
    CFCSymbol *self = (CFCSymbol*)malloc(sizeof(CFCSymbol));
    if (!self) { croak("malloc failed"); }
    return CFCSymbol_init(self, parcel, exposure, class_name, class_cnick,
        micro_sym);
}

CFCSymbol*
CFCSymbol_init(CFCSymbol *self, void *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym)
{
    self->parcel      = newSVsv((SV*)parcel);
    self->exposure    = savepv(exposure);
    self->class_name  = savepv(class_name);
    self->class_cnick = savepv(class_cnick);
    self->micro_sym   = savepv(micro_sym);
    return self;
}

void
CFCSymbol_destroy(CFCSymbol *self)
{
    SvREFCNT_dec((SV*)self->parcel);
    Safefree(self->exposure);
    Safefree(self->class_name);
    Safefree(self->class_cnick);
    Safefree(self->micro_sym);
    free(self);
}

void*
CFCSymbol_get_parcel(CFCSymbol *self)
{
    return self->parcel;
}

const char*
CFCSymbol_get_class_name(CFCSymbol *self)
{
    return self->class_name;
}

const char*
CFCSymbol_get_class_cnick(CFCSymbol *self)
{
    return self->class_cnick;
}

const char*
CFCSymbol_get_exposure(CFCSymbol *self)
{
    return self->exposure;
}

const char*
CFCSymbol_micro_sym(CFCSymbol *self)
{
    return self->micro_sym;
}

