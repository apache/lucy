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
                    const char *code, size_t code_len);

/* Attempt to compile an object file.  Return true if the object file
 * exists after the attempt.
 */
int
chaz_CC_compile_obj(const char *source_path, const char *obj_path,
                    const char *code, size_t code_len);

/* Attempt to compile the supplied source code and return true if the
 * effort succeeds.
 */
int
chaz_CC_test_compile(const char *source, size_t source_len);

/* Attempt to compile the supplied source code.  If successful, capture the
 * output of the program and return a pointer to a newly allocated buffer.
 * If the compilation fails, return NULL.  The length of the captured
 * output will be placed into the integer pointed to by [output_len].
 */
char*
chaz_CC_capture_output(const char *source, size_t source_len,
                       size_t *output_len);

/* Add an include directory which will be used for all future compilation
 * attempts.
 */
void
chaz_CC_add_inc_dir(const char *dir);

/** Initialize the compiler environment.
 */
void
chaz_CC_init(const char *cc_command, const char *cc_flags);

/* Clean up the environment.
 */
void
chaz_CC_clean_up(void);

void
chaz_CC_set_warnings_as_errors(const int flag);

#ifdef CHAZ_USE_SHORT_NAMES
  #define CC_compile_exe              chaz_CC_compile_exe
  #define CC_compile_obj              chaz_CC_compile_obj
  #define CC_add_inc_dir              chaz_CC_add_inc_dir
  #define CC_clean_up                 chaz_CC_clean_up
  #define CC_test_compile             chaz_CC_test_compile
  #define CC_capture_output           chaz_CC_capture_output
  #define CC_init                     chaz_CC_init
  #define CC_set_warnings_as_errors   chaz_CC_set_warnings_as_errors
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */


