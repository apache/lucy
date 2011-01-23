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

typedef struct CFCType CFCType;

#define CFCTYPE_CONST    0x1
#define CFCTYPE_NULLABLE 0x2

CFCType*
CFCType_new(int flags, void *parcel, const char *specifier, int indirection,
            const char *c_string);

CFCType*
CFCType_init(CFCType *self, int flags, void *parcel, const char *specifier,
             int indirection, const char *c_string);

void
CFCType_destroy(CFCType *self);

void
CFCType_set_specifier(CFCType *self, const char *specifier);

const char*
CFCType_get_specifier(CFCType *self);

int
CFCType_get_indirection(CFCType *self);

void*
CFCType_get_parcel(CFCType *self);

void
CFCType_set_c_string(CFCType *self, const char *c_string);

const char*
CFCType_to_c(CFCType *self);

int
CFCType_set_nullable(CFCType *self, int nullable);

int
CFCType_nullable(CFCType *self);

