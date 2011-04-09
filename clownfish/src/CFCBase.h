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

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCBase CFCBase;

#ifdef CFC_NEED_BASE_STRUCT_DEF
struct CFCBase {
    void *perl_obj;
};
#endif

/** Allocate a new CFC object.
 *
 * @param size Size of the desired allocation in bytes.
 * @param klass Class name.
 */
CFCBase*
CFCBase_allocate(size_t size, const char *klass);

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

/** Return the CFC object's cached Perl object.
 */
void*
CFCBase_get_perl_obj(CFCBase *self);

/** Return the class name of the CFC object.  (Not the class name of any
 * parsed object the CFC object might represent.)
 */
const char*
CFCBase_get_cfc_class(CFCBase *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCBASE */

