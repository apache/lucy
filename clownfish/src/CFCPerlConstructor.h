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

#ifndef H_CFCPERLCONSTRUCTOR
#define H_CFCPERLCONSTRUCTOR

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerlConstructor CFCPerlConstructor;
struct CFCClass;

/** Clownfish::CFC::Binding::Perl::Constructor - Binding for an object method.
 *
 * This class isa Clownfish::CFC::Binding::Perl::Subroutine -- see its
 * documentation for various code-generating routines.
 * 
 * Constructors are always bound to accept labeled params, even if there is only
 * a single argument.
 */

/**
 * @param klass A L<Clownfish::CFC::Class>.
 * @param alias The Perl name for the constructor.
 * @param initializer The name of the function which should be bound (default
 * "init").
 */
CFCPerlConstructor*
CFCPerlConstructor_new(struct CFCClass *klass, const char *alias,
                       const char *initializer);

CFCPerlConstructor*
CFCPerlConstructor_init(CFCPerlConstructor *self, struct CFCClass *klass,
                        const char *alias, const char *initializer);

void
CFCPerlConstructor_destroy(CFCPerlConstructor *self);

/** Generate C code for the XSUB.
 */
char*
CFCPerlConstructor_xsub_def(CFCPerlConstructor *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLCONSTRUCTOR */

