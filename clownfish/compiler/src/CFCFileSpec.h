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


/** Clownfish::CFC::Model::FileSpec - Specification of an input file
 *
 * This class contains data related to input files that is known before
 * parsing and can be referenced from classes created during parsing like
 * CFCFile or CFCClass.
 */
#ifndef H_CFCFILESPEC
#define H_CFCFILESPEC

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CFCFileSpec CFCFileSpec;
struct CFCBase;
struct CFCClass;

/**
 * @param path_part - The path of the file, relative to the source directory
 * and excluding the .cfh extension. Should be "Foo/Bar" if the source file
 * is found at 'Foo/Bar.cfh' within the source directory. That implies that
 * the output C header file should be 'Foo/Bar.h' within the target include
 * directory.
 * @param source_dir The source directory in which the file was found.
 * @param is_included Should be true if the file is from an include dir.
*/
CFCFileSpec*
CFCFileSpec_new(const char *source_dir, const char *path_part,
                int is_included);

CFCFileSpec*
CFCFileSpec_init(CFCFileSpec *self, const char *source_dir,
                 const char *path_part, int is_included);

void
CFCFileSpec_destroy(CFCFileSpec *self);

const char*
CFCFileSpec_get_source_dir(CFCFileSpec *self);

const char*
CFCFileSpec_get_path_part(CFCFileSpec *self);

int
CFCFileSpec_included(CFCFileSpec *self);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCFILESPEC */

