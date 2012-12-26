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


/** Clownfish::CFC::Model::File - Structured representation of the contents of
 * a Clownfish source file.
 *
 * An abstraction representing a file which contains Clownfish code.
 */
#ifndef H_CFCFILE
#define H_CFCFILE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCFile CFCFile;
struct CFCBase;
struct CFCClass;
struct CFCFileSpec;

/**
 * @param spec - A CFCFileSpec object describing the file
*/
CFCFile*
CFCFile_new(struct CFCFileSpec *spec);

CFCFile*
CFCFile_init(CFCFile *self, struct CFCFileSpec *spec);

void
CFCFile_destroy(CFCFile *self);

/** Add an element to the blocks array.  The block must be either a
 * Clownfish::CFC::Model::Class, a Clownfish::CFC::Model::Parcel, or a
 * Clownfish::CFC::Model::CBlock.
 */
void
CFCFile_add_block(CFCFile *self, CFCBase *block);

/** Given a base directory, return a path name derived from the File's
 * path_part with a ".c" extension.
 */
char*
CFCFile_c_path(CFCFile *self, const char *base_dir);

/** As c_path, but with a ".h" extension.
 */
char*
CFCFile_h_path(CFCFile *self, const char *base_dir);

/** As c_path, but with a ".cfh" extension.
 */
char*
CFCFile_cfh_path(CFCFile *self, const char *base_dir);

/** Return all blocks as an array.
 */
struct CFCBase**
CFCFile_blocks(CFCFile *self);

/** Return all Clownfish::CFC::Model::Class blocks from the file as an array.
 */
struct CFCClass**
CFCFile_classes(CFCFile *self);

/** Setter for the file's "modified" property, which is initially false.
 */
void
CFCFile_set_modified(CFCFile *self, int modified);

int
CFCFile_get_modified(CFCFile *self);

const char*
CFCFile_get_source_dir(CFCFile *self);

const char*
CFCFile_get_path_part(CFCFile *self);

int
CFCFile_included(CFCFile *self);

/** Return a string used for an include guard in a C header (e.g.
 * "H_CRUSTACEAN_LOBSTER"), unique per file.
 */
const char*
CFCFile_guard_name(CFCFile *self);

/** Return a string opening the include guard.
 */
const char*
CFCFile_guard_start(CFCFile *self);

/** Return a string closing the include guard.  Other classes count on being
 * able to match this string.
 */
const char*
CFCFile_guard_close(CFCFile *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCFILE */

