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

/** Clownfish::CFC::Model::Method - Metadata describing an instance method.
 *
 * Clownfish::CFC::Model::Method is a specialized subclass of
 * Clownfish::CFC::Model::Function, with the first argument required to be an
 * Obj.
 *
 * When compiling Clownfish code to C, Method objects generate all the code
 * that Function objects do, but also create symbols for indirect invocation via
 * VTable.
 */

#ifndef H_CFCMETHOD
#define H_CFCMETHOD

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCMethod CFCMethod;
struct CFCParcel;
struct CFCType;
struct CFCClass;
struct CFCParamList;
struct CFCDocuComment;

/**
 * @param parcel See Clownfish::CFC::Model::Function.
 * @param exposure See Clownfish::CFC::Model::Function.  Defaults to "parcel"
 * if not supplied.
 * @param class_name See Clownfish::CFC::Model::Function.
 * @param class_cnick See Clownfish::CFC::Model::Function.
 * @param macro_sym - The mixed case name which will be used when invoking the
 * method.
 * @param return_type See Clownfish::CFC::Model::Function.
 * @param param_list - A Clownfish::CFC::Model::ParamList.  The first element
 * must be an object of the class identified by C<class_name>.
 * @param docucomment see Clownfish::CFC::Model::Function.  May be NULL.
 * @param is_final - Indicate whether the method is final.
 * @param is_abstract - Indicate whether the method is abstract.
 */
CFCMethod*
CFCMethod_new(struct CFCParcel *parcel, const char *exposure,
              const char *class_name, const char *class_cnick,
              const char *macro_sym, struct CFCType *return_type,
              struct CFCParamList *param_list,
              struct CFCDocuComment *docucomment, int is_final,
              int is_abstract);

CFCMethod*
CFCMethod_init(CFCMethod *self, struct CFCParcel *parcel,
               const char *exposure, const char *class_name,
               const char *class_cnick, const char *macro_sym,
               struct CFCType *return_type, struct CFCParamList *param_list,
               struct CFCDocuComment *docucomment, int is_final,
               int is_abstract);

void
CFCMethod_destroy(CFCMethod *self);

/** Returns true if the methods have signatures and attributes which allow one
 * to override the other.
 */
int
CFCMethod_compatible(CFCMethod *self, CFCMethod *other);

/** Let the Method know that it is overriding a method which was defined in a
 * parent class, and verify that the override is valid.
 *
 * All methods start out believing that they are "novel", because we don't
 * know about inheritance until we build the hierarchy after all files have
 * been parsed.  override() is a way of going back and relabeling a method as
 * overridden when new information has become available: in this case, that a
 * parent class has defined a method with the same name.
 */
void
CFCMethod_override(CFCMethod *self, CFCMethod *orig);

/** As with override, above, this is for going back and changing the nature of
 * a Method after new information has become available -- typically, when we
 * discover that the method has been inherited by a "final" class.
 *
 * However, we don't modify the original Method as with override().  Inherited
 * Method objects are shared between parent and child classes; if a shared
 * Method object were to become final, it would interfere with its own
 * inheritance.  So, we make a copy, slightly modified to indicate that it is
 * "final".
 */
CFCMethod*
CFCMethod_finalize(CFCMethod *self);

/**
 * Create the symbol used to invoke the method without the parcel Prefix, e.g.
 * "LobClaw_Pinch".
 * @param invoker Class for which the symbol is created. If invoker is NULL,
 * use the class where the method is defined.
 *
 * @return the symbol.
 */
char*
CFCMethod_short_method_sym(CFCMethod *self, struct CFCClass *invoker);

/**
 * Create the fully-qualified symbol used to invoke the method, e.g.
 * "Crust_LobClaw_Pinch".
 * @param invoker Class for which the symbol is created. If invoker is NULL,
 * use the class where the method is defined.
 *
 * @return the symbol.
 */
char*
CFCMethod_full_method_sym(CFCMethod *self, struct CFCClass *invoker);

/** Create the fully qualified name of the variable which stores the method's
 * vtable offset, e.g. "Crust_LobClaw_Pinch_OFFSET".
 * @param invoker Class for which the symbol is created. If invoker is NULL,
 * use the class where the method is defined.
 *
 * @return the symbol.
 */
char*
CFCMethod_full_offset_sym(CFCMethod *self, struct CFCClass *invoker);

const char*
CFCMethod_get_macro_sym(CFCMethod *self);

const char*
CFCMethod_micro_sym(CFCMethod *self);

/** Create the typedef symbol for this method, e.g "Claw_Pinch_t".
 * @param invoker Class for which the symbol is created. If invoker is NULL,
 * use the class where the method is defined.
 *
 * @return the symbol.
 */
char*
CFCMethod_short_typedef(CFCMethod *self, struct CFCClass *invoker);

/** Create the fully-qualified typedef symbol, e.g. "Crust_Claw_Pinch_t".
 * @param invoker Class for which the symbol is created. If invoker is NULL,
 * use the class where the method is defined.
 *
 * @return the symbol.
 */
char*
CFCMethod_full_typedef(CFCMethod *self, struct CFCClass *invoker);

/** Returns the fully qualified name of the function which implements the
 * callback to the host in the event that a host method has been defined which
 * overrides this method, e.g. "crust_LobClaw_pinch_OVERRIDE".
 */
const char*
CFCMethod_full_override_sym(CFCMethod *self);

int
CFCMethod_final(CFCMethod *self);

int
CFCMethod_abstract(CFCMethod *self);

/** Returns true if this method is the first implemenation in the inheritance
 * hierarchy in which the method was declared.
 */
int
CFCMethod_novel(CFCMethod *self);

/** Return the Clownfish::CFC::Model::Type for <code>self</code>.
 */

struct CFCType*
CFCMethod_self_type(CFCMethod *self);

struct CFCParcel*
CFCMethod_get_parcel(CFCMethod *self);

const char*
CFCMethod_get_prefix(CFCMethod *self);

const char*
CFCMethod_get_Prefix(CFCMethod *self);

const char*
CFCMethod_get_exposure(CFCMethod *self);

const char*
CFCMethod_get_class_name(CFCMethod *self);

const char*
CFCMethod_get_class_cnick(CFCMethod *self);

int
CFCMethod_public(CFCMethod *self);

struct CFCType*
CFCMethod_get_return_type(CFCMethod *self);

struct CFCParamList*
CFCMethod_get_param_list(CFCMethod *self);

const char*
CFCMethod_implementing_func_sym(CFCMethod *self);

const char*
CFCMethod_short_implementing_func_sym(CFCMethod *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCMETHOD */

