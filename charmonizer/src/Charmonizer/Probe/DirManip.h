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

/* Charmonizer/Probe/DirManip.h
 */

#ifndef H_CHAZ_DIRMANIP
#define H_CHAZ_DIRMANIP

#ifdef __cplusplus
extern "C" {
#endif

/* The DirManip module exports or aliases symbols related to directory and file
 * manipulation.
 *
 * Defined if the header files dirent.h and direct.h are available, respectively:
 *
 * HAS_DIRENT_H
 * HAS_DIRECT_H
 *
 * Defined if struct dirent has these members.
 *
 * HAS_DIRENT_D_NAMLEN
 * HAS_DIRENT_D_TYPE
 *
 * The "makedir" macro will be aliased to the POSIX-specified two-argument
 * "mkdir" interface:
 *
 * makedir
 *
 * On some systems, the second argument to makedir will be ignored, in which
 * case this symbol will be true; otherwise, it will be false: (TODO: This
 * isn't verified and may sometimes be incorrect.)
 *
 * MAKEDIR_MODE_IGNORED
 *
 * String representing the system's directory separator:
 *
 * DIR_SEP
 *
 * True if the remove() function removes directories, false otherwise:
 *
 * REMOVE_ZAPS_DIRS
 */
void chaz_DirManip_run(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define DirManip_run    chaz_DirManip_run
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_DIR_SEP */



