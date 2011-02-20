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

#ifndef H_CFCFILE
#define H_CFCFILE

typedef struct CFCFile CFCFile;
struct CFCBase;
struct CFCClass;

CFCFile*
CFCFile_new(const char *source_class);

CFCFile*
CFCFile_init(CFCFile *self, const char *source_class);

void
CFCFile_destroy(CFCFile *self);

void
CFCFile_add_block(CFCFile *self, CFCBase *block);

/** Calculate the size of the buffer needed for a call to c_path(), h_path(),
 * or cfh_path().
 */
size_t
CFCFile_path_buf_size(CFCFile *self, const char *base_dir);

void
CFCFile_c_path(CFCFile *self, char *buf, size_t buf_size, 
               const char *base_dir);

void
CFCFile_h_path(CFCFile *self, char *buf, size_t buf_size, 
               const char *base_dir);

void
CFCFile_cfh_path(CFCFile *self, char *buf, size_t buf_size, 
                 const char *base_dir);

struct CFCBase**
CFCFile_blocks(CFCFile *self);

struct CFCClass**
CFCFile_classes(CFCFile *self);

void
CFCFile_set_modified(CFCFile *self, int modified);

int
CFCFile_get_modified(CFCFile *self);

const char*
CFCFile_get_source_class(CFCFile *self);

const char*
CFCFile_guard_name(CFCFile *self);

const char*
CFCFile_guard_start(CFCFile *self);

const char*
CFCFile_guard_close(CFCFile *self);

#endif /* H_CFCFILE */

