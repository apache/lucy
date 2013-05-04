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

/* Charmonizer/Core/OperatingSystem.h - abstract an operating system down to a few
 * variables.
 */

#ifndef H_CHAZ_OPER_SYS
#define H_CHAZ_OPER_SYS

#ifdef __cplusplus
extern "C" {
#endif

#define CHAZ_OS_POSIX    1
#define CHAZ_OS_CMD_EXE  2

/* Safely remove a file named [name]. Needed because of Windows quirks.
 * Returns true on success, false on failure.
 */
int
chaz_OS_remove(const char *name);

/* Invoke a command and attempt to suppress output from both stdout and stderr
 * (as if they had been sent to /dev/null).  If it's not possible to run the
 * command quietly, run it anyway.
 */
int
chaz_OS_run_quietly(const char *command);

/* Capture both stdout and stderr for a command to the supplied filepath.
 */
int
chaz_OS_run_redirected(const char *command, const char *path);

/* Run a command beginning with the name of an executable in the current
 * working directory and capture both stdout and stderr to the supplied
 * filepath.
 */
int
chaz_OS_run_local_redirected(const char *command, const char *path);

/* Run a command and return the output from stdout.
 */
char*
chaz_OS_run_and_capture(const char *command, size_t *output_len);

/* Attempt to create a directory.
 */
void
chaz_OS_mkdir(const char *filepath);

/* Attempt to remove a directory, which must be empty.
 */
void
chaz_OS_rmdir(const char *filepath);

/* Return the operating system name.
 */
const char*
chaz_OS_name(void);

int
chaz_OS_is_darwin(void);

int
chaz_OS_is_cygwin(void);

/* Return the extension for an executable on this system.
 */
const char*
chaz_OS_exe_ext(void);

/* Return the extension for a shared object on this system.
 */
const char*
chaz_OS_shared_lib_ext(void);

/* Return the equivalent of /dev/null on this system.
 */
const char*
chaz_OS_dev_null(void);

/* Return the shell type of this system.
 */
int
chaz_OS_shell_type(void);

/* Initialize the Charmonizer/Core/OperatingSystem module.
 */
void
chaz_OS_init(void);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */


