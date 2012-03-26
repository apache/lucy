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

/* Charmonizer/Core/Dir.h - Directory manipulation routines.
 *
 * Compile utilities to create and remove directories.
 */

#ifndef H_CHAZ_DIR
#define H_CHAZ_DIR

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Core/Defines.h"

/* Compile the utilities which execute the Dir_mkdir and Dir_rmdir functions. */
void
chaz_Dir_init(void);

/* Tear down.
 */
void
chaz_Dir_clean_up(void);

/* Attempt to create a directory.  Returns true on success, false on failure.
 */
chaz_bool_t
chaz_Dir_mkdir(const char *filepath);

/* Attempt to remove a directory, which must be empty.  Returns true on
 * success, false on failure.
 */
chaz_bool_t
chaz_Dir_rmdir(const char *filepath);

/* The string command for mkdir. */
extern char* chaz_Dir_mkdir_command;

/* Indicate whether the mkdir takes 1 or 2 args. */
extern int chaz_Dir_mkdir_num_args;

#ifdef CHAZ_USE_SHORT_NAMES
  #define Dir_init              chaz_Dir_init
  #define Dir_clean_up          chaz_Dir_clean_up
  #define Dir_mkdir             chaz_Dir_mkdir
  #define Dir_rmdir             chaz_Dir_rmdir
  #define Dir_mkdir_command     chaz_Dir_mkdir_command
  #define Dir_mkdir_num_args    chaz_Dir_mkdir_num_args
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_DIR */


