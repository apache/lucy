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

typedef struct CFCDocuComment CFCDocuComment;

/** Remove comment open, close, and left border from raw comment text.
 */
void
CFCDocuComment_strip(char *comment);

CFCDocuComment*
CFCDocuComment_new(const char *description, const char *brief, 
                   const char *long_description, void *param_names, 
                   void *param_docs, const char *retval);

CFCDocuComment*
CFCDocuComment_init(CFCDocuComment *self, const char *description, 
                    const char *brief, const char *long_description, 
                    void *param_names, void *param_docs, const char *retval);

void
CFCDocuComment_destroy(CFCDocuComment *self);

const char*
CFCDocuComment_get_description(CFCDocuComment *self);

const char*
CFCDocuComment_get_brief(CFCDocuComment *self);

const char*
CFCDocuComment_get_long(CFCDocuComment *self);

void*
CFCDocuComment_get_param_names(CFCDocuComment *self);

void*
CFCDocuComment_get_param_docs(CFCDocuComment *self);

// May be NULL.
const char*
CFCDocuComment_get_retval(CFCDocuComment *self);

