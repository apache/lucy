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

#ifndef H_CFCCLASS
#define H_CFCCLASS

typedef struct CFCClass CFCClass;
struct CFCParcel;

CFCClass*
CFCClass_new(struct CFCParcel *parcel, const char *exposure, const char *class_name, 
             const char *class_cnick, const char *micro_sym);

CFCClass*
CFCClass_init(CFCClass *self, struct CFCParcel *parcel, const char *exposure, 
              const char *class_name, const char *class_cnick, 
              const char *micro_sym);

void
CFCClass_destroy(CFCClass *self);

const char*
CFCClass_get_cnick(CFCClass *self);

void
CFCClass_set_tree_grown(CFCClass *self, int tree_grown);

int
CFCClass_tree_grown(CFCClass *self);

void
CFCClass_set_parent(CFCClass *self, CFCClass *parent);

CFCClass*
CFCClass_get_parent(CFCClass *self);

void
CFCClass_append_autocode(CFCClass *self, const char *autocode);

const char*
CFCClass_get_autocode(CFCClass *self);

#endif /* H_CFCCLASS */

