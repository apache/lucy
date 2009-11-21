/* Chaz/Core/Util.h -- miscellaneous utilities.
 */

#ifndef H_CHAZ_UTIL
#define H_CHAZ_UTIL 1

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

extern int chaz_Util_verbosity;

/* Open a file (truncating if necessary) and write [content] to it.  die() if
 * an error occurs.
 */
void
chaz_Util_write_file(const char *filename, const char *content, 
                     size_t content_len);

/* Read an entire file into memory.
 */
char* 
chaz_Util_slurp_file(char *file_path, size_t *len_ptr);

/* Get the length of a file (may overshoot on text files under DOS).
 */
long  
chaz_Util_flength(FILE *f);

/* Print an error message to stderr and exit.
 */
void  
chaz_Util_die(char *format, ...);

/* Print an error message to stderr.
 */
void
chaz_Util_warn(char *format, ...);

/* Attept to delete a file.  Don't error if the file wasn't there to begin
 * with.  Return 1 if it seems like the file is gone because an attempt to
 * open it for reading fails (this doesn't guarantee that the file is gone,
 * but it works well enough for our purposes).  Return 0 if we can still 
 * read the file.
 */
int
chaz_Util_remove_and_verify(char *file_path);

/* Attempt to open a file for reading, then close it immediately.
 */
int
chaz_Util_can_open_file(char *file_path);

/* Make sure that the buffer pointed to by [buf_ptr] can hold least as many 
 * bytes as [new_len] + 1.
 */
size_t
chaz_Util_grow_buf(char **buf_ptr, size_t old_len, size_t new_len);

/* Concatenate each string in a null-terminated list onto the end of [buf].
 * Grow [buf] if necessary and return its size.
 */
size_t
chaz_Util_append_strings(char **buf, size_t buf_len, ...);

/* Varargs version of append_str.
 */
size_t
chaz_Util_vappend_strings(char **buf, size_t buf_len, va_list args);

/* Replace the contents of [buf] (if any) with a string formed by
 * concatenating all the char* arguments in a null-terminated list.
 * Grow [buf] if necessary and return its size.
 */
size_t
chaz_Util_join_strings(char **buf, size_t buf_len, ...);

/* Varargs version of join_strings.
 */
size_t
chaz_Util_vjoin_strings(char **buf, size_t buf_len, va_list args);

#ifdef CHAZ_USE_SHORT_NAMES
  #define verbosity              chaz_Util_verbosity 
  #define write_file             chaz_Util_write_file 
  #define slurp_file             chaz_Util_slurp_file 
  #define flength                chaz_Util_flength 
  #define die                    chaz_Util_die 
  #define warn                   chaz_Util_warn 
  #define remove_and_verify      chaz_Util_remove_and_verify 
  #define can_open_file          chaz_Util_can_open_file 
  #define grow_buf               chaz_Util_grow_buf
  #define append_strings         chaz_Util_append_strings
  #define vappend_strings        chaz_Util_vappend_strings
  #define join_strings           chaz_Util_join_strings
  #define vjoin_strings          chaz_Util_vjoin_strings
#endif

#endif /* H_CHAZ_UTIL */

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

