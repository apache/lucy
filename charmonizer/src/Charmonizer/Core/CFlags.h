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

#ifndef H_CHAZ_FLAGS
#define H_CHAZ_FLAGS

#ifdef __cplusplus
extern "C" {
#endif

#define CHAZ_CFLAGS_STYLE_POSIX  1
#define CHAZ_CFLAGS_STYLE_GNU    2
#define CHAZ_CFLAGS_STYLE_MSVC   3

typedef struct chaz_CFlags chaz_CFlags;

chaz_CFlags*
chaz_CFlags_new(int style);

void
chaz_CFlags_destroy(chaz_CFlags *flags);

const char*
chaz_CFlags_get_string(chaz_CFlags *flags);

void
chaz_CFlags_append(chaz_CFlags *flags, const char *string);

void
chaz_CFlags_clear(chaz_CFlags *flags);

void
chaz_CFlags_set_output_obj(chaz_CFlags *flags, const char *filename);

void
chaz_CFlags_set_output_exe(chaz_CFlags *flags, const char *filename);

void
chaz_CFlags_add_define(chaz_CFlags *flags, const char *name,
                       const char *value);

void
chaz_CFlags_add_include_dir(chaz_CFlags *flags, const char *dir);

void
chaz_CFlags_enable_optimization(chaz_CFlags *flags);

void
chaz_CFlags_disable_strict_aliasing(chaz_CFlags *flags);

void
chaz_CFlags_set_warnings_as_errors(chaz_CFlags *flags);

void
chaz_CFlags_compile_shared_library(chaz_CFlags *flags);

void
chaz_CFlags_hide_symbols(chaz_CFlags *flags);

void
chaz_CFlags_link_shared_library(chaz_CFlags *flags);

void
chaz_CFlags_set_link_output(chaz_CFlags *flags, const char *filename);

void
chaz_CFlags_add_library_path(chaz_CFlags *flags, const char *directory);

void
chaz_CFlags_add_library(chaz_CFlags *flags, const char *library);

void
chaz_CFlags_enable_code_coverage(chaz_CFlags *flags);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_FLAGS */


