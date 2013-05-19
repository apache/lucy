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

/** Clownfish::CFC::Model::ParamList - parameter list.
 */

#ifndef H_CFCPARAMLIST
#define H_CFCPARAMLIST

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCParamList CFCParamList;
struct CFCClass;
struct CFCVariable;

/**
 * @param variadic Should be true if the function is variadic.
 */
CFCParamList*
CFCParamList_new(int variadic);

CFCParamList*
CFCParamList_init(CFCParamList *self, int variadic);

void
CFCParamList_resolve_types(CFCParamList *self, struct CFCClass **classes);

void
CFCParamList_destroy(CFCParamList *self);

/** Add a parameter to the ParamList.
 *
 * @param variable A Clownfish::CFC::Model::Variable.
 * @param value The parameter's default value, which should be NULL
 * if there is no default and thus the parameter is required.
 */
void
CFCParamList_add_param(CFCParamList *self, struct CFCVariable *variable,
                       const char *value);

struct CFCVariable**
CFCParamList_get_variables(CFCParamList *self);

const char**
CFCParamList_get_initial_values(CFCParamList *self);

void
CFCParamList_set_variadic(CFCParamList *self, int variadic);

int
CFCParamList_variadic(CFCParamList *self);

/** Return the number of variables in the ParamList, including "self" for
 * methods.
 */
size_t
CFCParamList_num_vars(CFCParamList *self);


/** Return a list of the variable's types and names, joined by commas.  For
 * example:
 *
 *     Obj* self, Foo* foo, Bar* bar
 */
const char*
CFCParamList_to_c(CFCParamList *self);

/** Return the variable's names, joined by commas.  For example:
 *
 *     self, foo, bar
 */
const char*
CFCParamList_name_list(CFCParamList *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPARAMLIST */

