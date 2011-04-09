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

#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
#include "CFCParamList.h"
#include "CFCVariable.h"
#include "CFCSymbol.h"
#include "CFCUtil.h"

struct CFCParamList {
    CFCBase       base;
    CFCVariable **variables;
    char        **values;
    int           variadic;
    size_t        num_vars;
    char         *c_string;
    char         *name_list;
};

// 
static void
S_generate_c_strings(CFCParamList *self);

CFCParamList*
CFCParamList_new(int variadic)
{
    CFCParamList *self = (CFCParamList*)CFCBase_allocate(sizeof(CFCParamList), 
        "Clownfish::ParamList");
    return CFCParamList_init(self, variadic);
}

CFCParamList*
CFCParamList_init(CFCParamList *self, int variadic)
{
    self->variadic  = variadic;
    self->num_vars  = 0;
    self->variables = (CFCVariable**)CALLOCATE(1, sizeof(void*));
    self->values    = (char**)CALLOCATE(1, sizeof(char*));
    S_generate_c_strings(self);
    return self;
}

void
CFCParamList_add_param(CFCParamList *self, CFCVariable *variable, 
                       const char *value)
{
    CFCUTIL_NULL_CHECK(variable);
    self->num_vars++;
    size_t amount = (self->num_vars + 1) * sizeof(void*);
    self->variables = (CFCVariable**)REALLOCATE(self->variables, amount);
    self->values    = (char**)REALLOCATE(self->values, amount);
    self->variables[self->num_vars - 1] 
        = (CFCVariable*)CFCBase_incref((CFCBase*)variable);
    self->values[self->num_vars - 1] = value ? CFCUtil_strdup(value) : NULL;
    self->variables[self->num_vars] = NULL;
    self->values[self->num_vars] = NULL;

    S_generate_c_strings(self);
}

void
CFCParamList_destroy(CFCParamList *self)
{
    size_t i;
    for (i = 0; i < self->num_vars; i++) {
        CFCBase_decref((CFCBase*)self->variables[i]);
        FREEMEM(self->values[i]);
    }
    FREEMEM(self->variables);
    FREEMEM(self->values);
    FREEMEM(self->c_string);
    FREEMEM(self->name_list);
    CFCBase_destroy((CFCBase*)self);
}

static void
S_generate_c_strings(CFCParamList *self)
{
    size_t c_string_size = 1;
    size_t name_list_size = 1;
    size_t i;

    // Calc space requirements and allocate memory.
    for (i = 0; i < self->num_vars; i++) {
        CFCVariable *var = self->variables[i];
        c_string_size += sizeof(", ");
        c_string_size += strlen(CFCVariable_local_c(var));
        name_list_size += sizeof(", ");
        name_list_size += strlen(CFCSymbol_micro_sym((CFCSymbol*)var));
    }
    if (self->variadic) {
        c_string_size += sizeof(", ...");
    }
    FREEMEM(self->c_string);
    FREEMEM(self->name_list);
    self->c_string  = (char*)MALLOCATE(c_string_size);
    self->name_list = (char*)MALLOCATE(name_list_size);
    self->c_string[0] = '\0';
    self->name_list[0] = '\0';

    // Build the strings.
    for (i = 0; i < self->num_vars; i++) {
        CFCVariable *var = self->variables[i];
        strcat(self->c_string, CFCVariable_local_c(var));
        strcat(self->name_list, CFCSymbol_micro_sym((CFCSymbol*)var));
        if (i == self->num_vars - 1) {
            if (self->variadic) {
                strcat(self->c_string, ", ...");
            }
        }
        else {
            strcat(self->c_string, ", ");
            strcat(self->name_list, ", ");
        }
    }
}

CFCVariable**
CFCParamList_get_variables(CFCParamList *self)
{
    return self->variables;
}

const char**
CFCParamList_get_initial_values(CFCParamList *self)
{
    return (const char**)self->values;
}

size_t
CFCParamList_num_vars(CFCParamList *self)
{
    return self->num_vars;
}

int
CFCParamList_variadic(CFCParamList *self)
{
    return self->variadic;
}

const char*
CFCParamList_to_c(CFCParamList *self)
{
    return self->c_string;
}

const char*
CFCParamList_name_list(CFCParamList *self)
{
    return self->name_list;
}

