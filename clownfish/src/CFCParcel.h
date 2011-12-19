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

/** Clownfish::Parcel - Collection of code.
 *
 * A Parcel is a cohesive collection of code, which could, in theory, be
 * published as as a single entity.
 *
 * Clownfish supports two-tier manual namespacing, using a prefix, an optional
 * class nickname, and the local symbol:
 * 
 *     prefix_ClassNick_local_symbol
 * 
 * Clownfish::Parcel supports the first tier, specifying initial prefixes.
 * These prefixes come in three capitalization variants: prefix_, Prefix_, and
 * PREFIX_.
 */

#ifndef H_CFCPARCEL
#define H_CFCPARCEL

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCParcel CFCParcel;

/** Add a Parcel singleton to a global registry.  May be called multiple times,
 * but only with compatible arguments.
 *
 * @param name The name of the parcel.
 * @param cnick The C nickname for the parcel, which will be used as a prefix
 * for generated global symbols.  Must be mixed case and start with a capital
 * letter.  Defaults to <code>name</code>
 */
CFCParcel*
CFCParcel_singleton(const char *name, const char *cnick);

/** Decref all singletons at shutdown.
 */
void
CFCParcel_reap_singletons(void);

CFCParcel*
CFCParcel_new(const char *name, const char *cnick);

CFCParcel*
CFCParcel_init(CFCParcel *self, const char *name, const char *cnick);

void
CFCParcel_destroy(CFCParcel *self);

/** Return the singleton for default parcel, which has no prefix.
 */
CFCParcel*
CFCParcel_default_parcel(void);

/** Return the Parcel under which Obj, CharBuf, VArray, Hash, etc. live.  At
 * some point in the future, these core object types may move to the
 * "Clownfish" Parcel, but for now they are within "Lucy".
 */
CFCParcel*
CFCParcel_clownfish_parcel(void);

int
CFCParcel_equals(CFCParcel *self, CFCParcel *other);

const char*
CFCParcel_get_name(CFCParcel *self);

const char*
CFCParcel_get_cnick(CFCParcel *self);

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

#ifdef __cplusplus
}
#endif

#endif /* H_CFCPARCEL */

