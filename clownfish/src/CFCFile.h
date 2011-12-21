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


/** Clownfish::CFC::File - Structured representation of the contents of a Clownfish
 * source file.
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

/**
 * @param source_class The class name associated with the source file,
 * regardless of how what classes are defined in the source file. Example: If
 * source_class is "Foo::Bar", that implies that the source file could be
 * found at 'Foo/Bar.cfh' within the source directory and that the output C
 * header file should be 'Foo/Bar.h' within the target include directory.
*/
CFCFile*
CFCFile_new(const char *source_class);

CFCFile*
CFCFile_init(CFCFile *self, const char *source_class);

void
CFCFile_destroy(CFCFile *self);

/** Add an element to the blocks array.  The block must be either a
 * Clownfish::CFC::Class, a Clownfish::CFC::Parcel, or a Clownfish::CFC::CBlock.
 */
void
CFCFile_add_block(CFCFile *self, CFCBase *block);

/** Calculate the size of the buffer needed for a call to c_path(), h_path(),
 * or cfh_path().
 */
size_t
CFCFile_path_buf_size(CFCFile *self, const char *base_dir);

/** Given a base directory, return a path name derived from the File's
 * source_class with a ".c" extension.
 */
void
CFCFile_c_path(CFCFile *self, char *buf, size_t buf_size,
               const char *base_dir);

/** As c_path, but with a ".h" extension.
 */
void
CFCFile_h_path(CFCFile *self, char *buf, size_t buf_size,
               const char *base_dir);

/** As c_path, but with a ".cfh" extension.
 */
void
CFCFile_cfh_path(CFCFile *self, char *buf, size_t buf_size,
                 const char *base_dir);

/** Return all blocks as an array.
 */
struct CFCBase**
CFCFile_blocks(CFCFile *self);

/** Return all Clownfish::CFC::Class blocks from the file as an array.
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
CFCFile_get_source_class(CFCFile *self);

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

