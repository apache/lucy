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

#ifndef H_CFCPERLCLASS
#define H_CFCPERLCLASS

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCPerlClass CFCPerlClass;
struct CFCParcel;
struct CFCClass;
struct CFCFunction;
struct CFCPerlPod;
struct CFCPerlMethod;
struct CFCPerlConstructor;

/** Clownfish::CFC::Binding::Perl::Class - Generate Perl binding code for a
 * Clownfish::CFC::Model::Class.
 */

/** 
 * @param parcel A Clownfish::CFC::Model::Parcel.
 * @param class_name The name of the class to be registered.
 */
CFCPerlClass*
CFCPerlClass_new(struct CFCParcel *parcel, const char *class_name);

CFCPerlClass*
CFCPerlClass_init(CFCPerlClass *self, struct CFCParcel *parcel,
                  const char *class_name);

void
CFCPerlClass_destroy(CFCPerlClass *self);

/** Add a new class binding to the registry.  Each unique parcel/class-name
 * combination may only be registered once.
 */
void
CFCPerlClass_add_to_registry(CFCPerlClass *self);

/** Given a class name, return a class binding if one exists.
 */
CFCPerlClass*
CFCPerlClass_singleton(const char *class_name);

/** All registered class bindings.
 */
CFCPerlClass**
CFCPerlClass_registry();

/** Release all memory and references held by the registry.
 */
void
CFCPerlClass_clear_registry(void);

/** Override the specification for a method being made available from Perl.
 * The default spec is to make the method available under a lower-cased alias
 * of the Clownfish method name.
 *
 * @param alias The Perl name for the method.
 * @param method The Clownfish name for the method.
 */
void
CFCPerlClass_bind_method(CFCPerlClass *self, const char *alias,
                         const char *method);

/** Specify that a constructor should be made available from Perl-space.
 *
 * @param alias The Perl name for the constructor (default "new").
 * @param initializer The Clownfish name for the initialization function which
 * will be invoked (default "init").
 */
void
CFCPerlClass_bind_constructor(CFCPerlClass *self, const char *alias,
                              const char *initializer);

/** Block the automatic generation of a method binding.
 */
void
CFCPerlClass_exclude_method(CFCPerlClass *self, const char *method);

/** Block the automatic generation of a constructor binding.
 */
void
CFCPerlClass_exclude_constructor(CFCPerlClass *self);

/** Return an array of Clownfish::CFC::Binding::Perl::Method objects
 * representing all bound methods.
 */
struct CFCPerlMethod**
CFCPerlClass_method_bindings(CFCPerlClass *self);

/** Return an array of Clownfish::CFC::Binding::Perl::Constructor objects
 * representing all bound constructors.
 */
struct CFCPerlConstructor**
CFCPerlClass_constructor_bindings(CFCPerlClass *self);

/** Auto-generate POD according to the spec supplied via set_pod_spec().  If
 * no spec was supplied, return NULL.
 */
char*
CFCPerlClass_create_pod(CFCPerlClass *self);

/** Accessor for the Clownfish::CFC::Model::Class module to be bound.
 */
struct CFCClass*
CFCPerlClass_get_client(CFCPerlClass *self);

/** Accessor for class name. 
 */
const char*
CFCPerlClass_get_class_name(CFCPerlClass *self);

/** Concatenate verbatim XS code.
 */
void
CFCPerlClass_append_xs(CFCPerlClass *self, const char *xs);

/** Accessor for verbatim XS code.
 */
const char*
CFCPerlClass_get_xs_code(CFCPerlClass *self);

/** Supply a specification which will cause POD to be generated for this class
 * binding.
 */
void
CFCPerlClass_set_pod_spec(CFCPerlClass *self, struct CFCPerlPod *pod_spec);

/** Accessor for pod spec.
 */
struct CFCPerlPod*
CFCPerlClass_get_pod_spec(CFCPerlClass *self);

/** Alias a class name to this class.
 *
 * TODO: remove this feature as soon as KinoSearch compatibility is no longer
 * necessary.
 */
void
CFCPerlClass_add_class_alias(CFCPerlClass *self, const char *alias);

/** Return a NULL-terminated list of class names which will be aliased to this
 * class.
 */
char**
CFCPerlClass_get_class_aliases(CFCPerlClass *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPERLCLASS */

