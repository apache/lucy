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

typedef struct CFCParamList CFCParamList;

CFCParamList*
CFCParamList_new(int variadic);

CFCParamList*
CFCParamList_init(CFCParamList *self, int variadic);

void
CFCParamList_destroy(CFCParamList *self);

void
CFCParamList_add_param(CFCParamList *self, void *variable, void *value);

void*
CFCParamList_get_variables(CFCParamList *self);

void*
CFCParamList_get_initial_values(CFCParamList *self);

int
CFCParamList_variadic(CFCParamList *self);

