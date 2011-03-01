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

#ifndef H_CFCUTIL
#define H_CFCUTIL

#ifdef __cplusplus
extern "C" {
#endif

/** Create an inner Perl object with a refcount of 1.  For use in actual
 * Perl-space, it is necessary to wrap this inner object in an RV.
 */
void*
CFCUtil_make_perl_obj(void *ptr, const char *klass);

/** Throw an error if the supplied argument is NULL.
 */
void
CFCUtil_null_check(const void *arg, const char *name, const char *file, int line);
#define CFCUTIL_NULL_CHECK(arg) \
    CFCUtil_null_check(arg, #arg, __FILE__, __LINE__)

/** Portable, NULL-safe implementation of strdup().
 */
char*
CFCUtil_strdup(const char *string);

/** Portable, NULL-safe implementation of strndup().
 */
char*
CFCUtil_strndup(const char *string, size_t len);

/** Trim whitespace from the beginning and the end of a string.
 */
void
CFCUtil_trim_whitespace(char *text);

/** Attempt to allocate memory with malloc, but print an error and exit if the
 * call fails.
 */
void*
CFCUtil_wrapped_malloc(size_t count, const char *file, int line);

/** Attempt to allocate memory with calloc, but print an error and exit if the
 * call fails.
 */
void*
CFCUtil_wrapped_calloc(size_t count, size_t size, const char *file, int line);

/** Attempt to allocate memory with realloc, but print an error and exit if 
 * the call fails.
 */
void*
CFCUtil_wrapped_realloc(void *ptr, size_t size, const char *file, int line);

/** Free memory.  (Wrapping is necessary in cases where memory allocated
 * within Clownfish has to be freed in an external environment where "free"
 * may have been redefined.)
 */
void
CFCUtil_wrapped_free(void *ptr);

#define MALLOCATE(_count) \
    CFCUtil_wrapped_malloc((_count), __FILE__, __LINE__)
#define CALLOCATE(_count, _size) \
    CFCUtil_wrapped_calloc((_count), (_size), __FILE__, __LINE__)
#define REALLOCATE(_ptr, _count) \
    CFCUtil_wrapped_realloc((_ptr), (_count), __FILE__, __LINE__)
#define FREEMEM(_ptr) \
    CFCUtil_wrapped_free(_ptr)

/* Open a file (truncating if necessary) and write [content] to it.  CFCUtil_die() if
 * an error occurs.
 */
void
CFCUtil_write_file(const char *filename, const char *content, size_t len);

void
CFCUtil_write_if_changed(const char *path, const char *content, size_t len);

/* Read an entire file into memory.
 */
char* 
CFCUtil_slurp_file(const char *file_path, size_t *len_ptr);

/* Get the length of a file (may overshoot on text files under DOS).
 */
long  
CFCUtil_flength(void *file);

/* Print an error message to stderr and exit.
 */
void  
CFCUtil_die(const char *format, ...);

/* Print an error message to stderr.
 */
void
CFCUtil_warn(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* H_CFCUTIL */

