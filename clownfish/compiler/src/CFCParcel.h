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

/** Clownfish::CFC::Model::Parcel - Collection of code.
 *
 * A Parcel is a cohesive collection of code, which could, in theory, be
 * published as as a single entity.
 *
 * Clownfish supports two-tier manual namespacing, using a prefix, an optional
 * class nickname, and the local symbol:
 *
 *     prefix_ClassNick_local_symbol
 *
 * Clownfish::CFC::Model::Parcel supports the first tier, specifying initial
 * prefixes.  These prefixes come in three capitalization variants: prefix_,
 * Prefix_, and PREFIX_.
 */

#ifndef H_CFCPARCEL
#define H_CFCPARCEL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCParcel CFCParcel;
struct CFCVersion;

/** Return the parcel which has been registered for <code>name</code>.
 */
CFCParcel*
CFCParcel_fetch(const char *name);

/** Register the supplied parcel.  Throws an error if a parcel with the same
 * name has already been registered.
 */
void
CFCParcel_register(CFCParcel *self);

/** Return a NULL-terminated list of all registered parcels that are not
 * included. Must be freed by the caller.
 */
CFCParcel**
CFCParcel_source_parcels(void);

/** Decref all singletons at shutdown.
 */
void
CFCParcel_reap_singletons(void);

CFCParcel*
CFCParcel_new(const char *name, const char *cnick,
              struct CFCVersion *version, int is_included);

CFCParcel*
CFCParcel_new_from_file(const char *path, int is_included);

CFCParcel*
CFCParcel_new_from_json(const char *json, int is_included);

CFCParcel*
CFCParcel_init(CFCParcel *self, const char *name, const char *cnick,
               struct CFCVersion *version, int is_included);

void
CFCParcel_destroy(CFCParcel *self);

/** Return the singleton for default parcel, which has no prefix.
 */
CFCParcel*
CFCParcel_default_parcel(void);

int
CFCParcel_equals(CFCParcel *self, CFCParcel *other);

const char*
CFCParcel_get_name(CFCParcel *self);

const char*
CFCParcel_get_cnick(CFCParcel *self);

struct CFCVersion*
CFCParcel_get_version(CFCParcel *self);

/** Return the all-lowercase version of the Parcel's prefix.
 */
const char*
CFCParcel_get_prefix(CFCParcel *self);

/** Return the Titlecase version of the Parcel's prefix.
 */
const char*
CFCParcel_get_Prefix(CFCParcel *self);

/** Return the all-caps version of the Parcel's prefix.
 */
const char*
CFCParcel_get_PREFIX(CFCParcel *self);

/** Return true if the parcel is from an include directory.
 */
int
CFCParcel_included(CFCParcel *self);

/** Add another Parcel that the Parcel depends on.
 */
void
CFCParcel_add_dependent_parcel(CFCParcel *self, CFCParcel *dependent);

/** Add another Parcel containing superclasses that subclasses in the Parcel
 * extend. Also adds the other Parcel to the Parcel's dependencies.
 */
void
CFCParcel_add_inherited_parcel(CFCParcel *self, CFCParcel *inherited);

/** Return a NULL-terminated array of all Parcels that the Parcel depends on.
 * Must be freed by the caller.
 */
CFCParcel**
CFCParcel_dependent_parcels(CFCParcel *self);

/** Return a NULL-terminated array of all Parcels containing superclasses that
 * subclasses in the Parcel extend. Must be freed by the caller.
 */
CFCParcel**
CFCParcel_inherited_parcels(CFCParcel *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPARCEL */

