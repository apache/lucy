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
struct CFCParcel;

#define CFCTYPE_CONST       0x00000001
#define CFCTYPE_NULLABLE    0x00000002
#define CFCTYPE_VOID        0x00000004
#define CFCTYPE_OBJECT      0x00000008
#define CFCTYPE_PRIMITIVE   0x00000010
#define CFCTYPE_INTEGER     0x00000020
#define CFCTYPE_FLOATING    0x00000040
#define CFCTYPE_STRING_TYPE 0x00000080
#define CFCTYPE_VA_LIST     0x00000100
#define CFCTYPE_ARBITRARY   0x00000200
#define CFCTYPE_COMPOSITE   0x00000400

CFCType*
CFCType_new(int flags, struct CFCParcel *parcel, const char *specifier,
            int indirection, const char *c_string);

CFCType*
CFCType_init(CFCType *self, int flags, struct CFCParcel *parcel, 
             const char *specifier, int indirection, const char *c_string);

CFCType*
CFCType_new_float(int flags, const char *specifier);

CFCType*
CFCType_new_void(int is_const);

CFCType*
CFCType_new_va_list(void);

CFCType*
CFCType_new_arbitrary(struct CFCParcel *parcel, const char *specifier);

void
CFCType_destroy(CFCType *self);

int
CFCType_equals(CFCType *self, CFCType *other);

void
CFCType_set_specifier(CFCType *self, const char *specifier);

const char*
CFCType_get_specifier(CFCType *self);

int
CFCType_get_indirection(CFCType *self);

struct CFCParcel*
CFCType_get_parcel(CFCType *self);

void
CFCType_set_c_string(CFCType *self, const char *c_string);

const char*
CFCType_to_c(CFCType *self);

int
CFCType_set_nullable(CFCType *self, int nullable);

int
CFCType_nullable(CFCType *self);

int
CFCType_is_void(CFCType *self);

int
CFCType_is_object(CFCType *self);

int
CFCType_is_primitive(CFCType *self);

int
CFCType_is_integer(CFCType *self);

int
CFCType_is_floating(CFCType *self);

int
CFCType_is_string_type(CFCType *self);

int
CFCType_is_va_list(CFCType *self);

int
CFCType_is_arbitrary(CFCType *self);

int
CFCType_is_composite(CFCType *self);

