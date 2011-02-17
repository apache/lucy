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

typedef struct CFCParcel CFCParcel;

CFCParcel*
CFCParcel_singleton(const char *name, const char *cnick);

CFCParcel*
CFCParcel_new(const char *name, const char *cnick);

CFCParcel*
CFCParcel_init(CFCParcel *self, const char *name, const char *cnick);

void
CFCParcel_destroy(CFCParcel *self);

CFCParcel*
CFCParcel_default_parcel(void);

int
CFCParcel_equals(CFCParcel *self, CFCParcel *other);

const char*
CFCParcel_get_name(CFCParcel *self);

const char*
CFCParcel_get_cnick(CFCParcel *self);

const char*
CFCParcel_get_prefix(CFCParcel *self);

const char*
CFCParcel_get_Prefix(CFCParcel *self);

const char*
CFCParcel_get_PREFIX(CFCParcel *self);

