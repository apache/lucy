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

#ifndef H_CFCPERLSUB
#define H_CFCPERLSUB

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerlSub CFCPerlSub;
struct CFCParamList;
struct CFCType;

#ifdef CFC_NEED_PERLSUB_STRUCT_DEF
#define CFC_NEED_BASE_STRUCT_DEF
#include "CFCBase.h"
struct CFCPerlSub {
    CFCBase base;
    struct CFCParamList *param_list;
    char *class_name;
    char *alias;
    int use_labeled_params;
    char *perl_name;
    char *c_name;
};
#endif

/** Clownfish::CFC::Binding::Perl::Subroutine - Abstract base binding for a
 * Clownfish::CFC::Function.
 * 
 * This class is used to generate binding code for invoking Clownfish's
 * functions and methods across the Perl/C barrier.
 */ 

/** Abstract constructor.
 * 
 * @param param_list A Clownfish::CFC::ParamList.
 * @param class_name The name of the Perl class that the subroutine belongs
 * to.
 * @param alias The local, unqualified name for the Perl subroutine that
 * will be used to invoke the function.
 * @param use_labeled_params True if the binding should take hash-style
 * labeled parameters, false if it should take positional arguments.
 */
CFCPerlSub*
CFCPerlSub_init(CFCPerlSub *self, struct CFCParamList *param_list,
                const char *class_name, const char *alias,
                int use_labeled_params);

void
CFCPerlSub_destroy(CFCPerlSub *self);

/** Return Perl code initializing a package-global hash where all the keys are
 * the names of labeled params.  The hash's name consists of the the binding's
 * perl_name() plus "_PARAMS".
 */
char*
CFCPerlSub_params_hash_def(CFCPerlSub *self);

char*
CFCPerlSub_build_allot_params(CFCPerlSub *self);

struct CFCParamList*
CFCPerlSub_get_param_list(CFCPerlSub *self);

/** Accessor.
 */
const char*
CFCPerlSub_get_class_name(CFCPerlSub *self);

/** Accessor.
 */
int
CFCPerlSub_use_labeled_params(CFCPerlSub *self);

/**
 * @return the fully-qualified perl sub name.
 */
const char*
CFCPerlSub_perl_name(CFCPerlSub *self);

/**
 * @return the fully-qualified name of the C function that implements the
 * XSUB.
 */
const char*
CFCPerlSub_c_name(CFCPerlSub *self);

/**
 * @return a string containing the names of arguments to feed to bound C
 * function, joined by commas.
 */
const char*
CFCPerlSub_c_name_list(CFCPerlSub *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLSUB */

