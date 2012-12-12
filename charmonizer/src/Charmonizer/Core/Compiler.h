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

/* Charmonizer/Core/Compiler.h
 */

#ifndef H_CHAZ_COMPILER
#define H_CHAZ_COMPILER

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "Charmonizer/Core/Defines.h"

/* Attempt to compile and link an executable.  Return true if the executable
 * file exists after the attempt.
 */
int
chaz_CC_compile_exe(const char *source_path, const char *exe_path,
                    const char *code);

/* Attempt to compile an object file.  Return true if the object file
 * exists after the attempt.
 */
int
chaz_CC_compile_obj(const char *source_path, const char *obj_path,
                    const char *code);

/* Attempt to compile the supplied source code and return true if the
 * effort succeeds.
 */
int
chaz_CC_test_compile(const char *source);

/* Attempt to compile the supplied source code.  If successful, capture the
 * output of the program and return a pointer to a newly allocated buffer.
 * If the compilation fails, return NULL.  The length of the captured
 * output will be placed into the integer pointed to by [output_len].
 */
char*
chaz_CC_capture_output(const char *source, size_t *output_len);

/** Initialize the compiler environment.
 */
void
chaz_CC_init(const char *cc_command, const char *cflags);

/* Clean up the environment.
 */
void
chaz_CC_clean_up(void);

void
chaz_CC_set_warnings_as_errors(const int flag);

/* Concatenate onto the end of the "extra" cflags.  A space will be inserted
 * automatically.
 */
void
chaz_CC_add_extra_cflags(const char *);

/* Accessor for the compiler executable's string representation.
 */
const char*
chaz_CC_get_cc(void);

/* Accessor for `cflags`.
 */
const char*
chaz_CC_get_cflags(void);

/* Accessor for `extra_cflags`.
 */
const char*
chaz_CC_get_extra_cflags(void);

int
chaz_CC_gcc_version_num();

const char*
chaz_CC_gcc_version();

int
chaz_CC_compiler_is_msvc(void);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */


