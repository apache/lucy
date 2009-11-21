/* Charmonizer/Core/OperSys.h - abstract an operating system down to a few
 * variables.
 */

#ifndef H_CHAZ_OPER_SYS
#define H_CHAZ_OPER_SYS

typedef struct chaz_OperSys chaz_OperSys;

/* Remove an executable file named [name], appending the exe_ext if needed.
 */
typedef void
(*chaz_OS_remove_exe_t)(chaz_OperSys *self, char *name);

/* Remove an object file named [name], appending the obj_ext if needed.
 */
typedef void
(*chaz_OS_remove_obj_t)(chaz_OperSys *self, char *name);

/* Concatenate all arguments in a NULL-terminated list into a single command
 * string, prepend the appropriate prefix, and invoke via system().
 */
typedef int 
(*chaz_OS_run_local_t)(chaz_OperSys *self, ...);

/* Destructor.
 */
typedef void
(*chaz_OS_destroy_t)(chaz_OperSys *self);

struct chaz_OperSys {
    char       *name;
    char       *obj_ext;
    char       *exe_ext;
    char       *local_command_start;
    char       *devnull;
    char       *buf;
    size_t      buf_len;
    chaz_OS_remove_exe_t remove_exe;
    chaz_OS_remove_obj_t remove_obj;
    chaz_OS_run_local_t  run_local;
    chaz_OS_destroy_t    destroy;
};

/** Constructor. 
 * 
 * @param name A string representing the name of the operating system.
 */
chaz_OperSys*
chaz_OS_new(const char *name);

#ifdef CHAZ_USE_SHORT_NAMES
  #define OperSys                      chaz_OperSys
  #define OS_remove_exe_t              chaz_OS_remove_exe_t
  #define OS_remove_obj_t              chaz_OS_remove_obj_t
  #define OS_run_local_t               chaz_OS_run_local_t
  #define OS_destroy_t                 chaz_OS_destroy_t
  #define OS_new                       chaz_OS_new
#endif

#endif /* H_CHAZ_COMPILER */


/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

