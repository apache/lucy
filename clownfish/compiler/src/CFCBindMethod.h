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

/** Clownfish::CFC::Binding::Core::Method - Generate core C code for a method.
 *
 * Clownfish::CFC::Model::Method is an abstract specification; this class
 * generates C code which implements the specification.
 */

#ifndef H_CFCBINDMETHOD
#define H_CFCBINDMETHOD

#ifdef __cplusplus
extern "C" {
#endif

struct CFCMethod;
struct CFCClass;

/** Return C code for the static inline vtable method invocation function.
 * @param method A L<Clownfish::CFC::Model::Method>.
 * @param class The L<Clownfish::CFC::Model::Class> which will be invoking the
 * method.  (LobsterClaw needs its own method invocation function even if the
 * method was defined in Claw.)
 */
char*
CFCBindMeth_method_def(struct CFCMethod *method, struct CFCClass *klass);

/** Return C code expressing a typedef declaration for the method.
 */
char*
CFCBindMeth_typedef_dec(struct CFCMethod *method, struct CFCClass *klass);

/** Return C code defining the MethodSpec object for this method, which
 * is used during VTable initialization.
 */
char*
CFCBindMeth_spec_def(struct CFCMethod *method);

/** Return C code implementing a version of the method which throws an
 * "abstract method" error at runtime, for methods which are declared as
 * "abstract" in a Clownfish header file.
 */
char*
CFCBindMeth_abstract_method_def(struct CFCMethod *method);

/** Return C code declaring a callback to the Host for this method.
 */
char*
CFCBindMeth_callback_dec(struct CFCMethod *method);

/** Return C code implementing a callback to the Host for this method.  This
 * code is used when a Host method has overridden a method in a Clownfish
 * class.
 */
char*
CFCBindMeth_callback_def(struct CFCMethod *method);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCBINDMETHOD */


