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

typedef struct CFCMethod CFCMethod;

CFCMethod*
CFCMethod_new(void *parcel, const char *exposure, const char *class_name, 
              const char *class_cnick, const char *micro_sym, 
              void *return_type, void *param_list, void *docucomment, 
              int is_inline);

CFCMethod*
CFCMethod_init(CFCMethod *self, void *parcel, const char *exposure, 
               const char *class_name, const char *class_cnick, 
               const char *micro_sym, void *return_type, void *param_list,
               void *docucomment, int is_inline);

void
CFCMethod_destroy(CFCMethod *self);


