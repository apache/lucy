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

#ifndef H_CFCBASE
#define H_CFCBASE

/** Clownfish::Base - Base class for all CFC objects.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct CFCBase CFCBase;
typedef struct CFCMeta CFCMeta;
typedef void (*CFCBase_destroy_t)(CFCBase *self);

#ifdef CFC_NEED_BASE_STRUCT_DEF
struct CFCBase {
    const CFCMeta *meta;
    int refcount;
};
#endif
struct CFCMeta {
    char *cfc_class;
    size_t obj_alloc_size;
    CFCBase_destroy_t destroy;
};

/** Allocate a new CFC object.
 *
 * @param size Size of the desired allocation in bytes.
 * @param klass Class name.
 */
CFCBase*
CFCBase_allocate(const CFCMeta *meta);

/** Clean up CFCBase member variables as necessary and free the object blob
 * itself.
 */
void
CFCBase_destroy(CFCBase *self);

/** Increment the refcount of the object.
 *
 * @return the object itself, allowing an assignment idiom.
 */
CFCBase*
CFCBase_incref(CFCBase *self);

/** Decrement the refcount of the object.
 *
 * @return the modified refcount.
 */
unsigned
CFCBase_decref(CFCBase *self);

/** Return the CFC object's refcount.
 */
unsigned
CFCBase_get_refcount(CFCBase *self);

/** Return the class name of the CFC object.  (Not the class name of any
 * parsed object the CFC object might represent.)
 */
const char*
CFCBase_get_cfc_class(CFCBase *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCBASE */

