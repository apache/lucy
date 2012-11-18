/* This is an auto-generated file -- do not edit directly. */

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

/***************************************************************************/

#line 21 "src/Charmonizer/Core/Defines.h"
/* Charmonizer/Core/Defines.h -- Universal definitions.
 */
#ifndef H_CHAZ_DEFINES
#define H_CHAZ_DEFINES 1

#ifndef true
  #define true 1
  #define false 0
#endif

#define CHAZ_QUOTE(x) #x "\n"

#endif /* H_CHAZ_DEFINES */


/***************************************************************************/

#line 21 "src/Charmonizer/Core/Compiler.h"
/* Charmonizer/Core/Compiler.h
 */

#ifndef H_CHAZ_COMPILER
#define H_CHAZ_COMPILER

#include <stddef.h>
/* #include "Charmonizer/Core/Defines.h" */

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

#endif /* H_CHAZ_COMPILER */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/ConfWriter.h"
/* Charmonizer/Core/ConfWriter.h -- Write to a config file.
 */

#ifndef H_CHAZ_CONFWRITER
#define H_CHAZ_CONFWRITER 1

#include <stddef.h>
#include <stdarg.h>
/* #include "Charmonizer/Core/Defines.h" */

struct chaz_ConfWriter;

/* Initialize elements needed by ConfWriter.  Must be called before anything
 * else, but after os and compiler are initialized.
 */
void
chaz_ConfWriter_init(void);

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ConfWriter_clean_up(void);

/* Print output to charmony.h.
 */
void
chaz_ConfWriter_append_conf(const char *fmt, ...);

/* Add a pound-define.
 */
void
chaz_ConfWriter_add_def(const char *sym, const char *value);

/* Add a globally scoped pound-define.
 */
void
chaz_ConfWriter_add_global_def(const char *sym, const char *value);

/* Add a typedef.
 */
void
chaz_ConfWriter_add_typedef(const char *type, const char *alias);

/* Add a globally scoped typedef.
 */
void
chaz_ConfWriter_add_global_typedef(const char *type, const char *alias);

/* Pound-include a system header (within angle brackets).
 */
void
chaz_ConfWriter_add_sys_include(const char *header);

/* Pound-include a locally created header (within quotes).
 */
void
chaz_ConfWriter_add_local_include(const char *header);

/* Print a "chapter heading" comment in the conf file when starting a module.
 */
void
chaz_ConfWriter_start_module(const char *module_name);

/* Leave a little whitespace at the end of each module.
 */
void
chaz_ConfWriter_end_module(void);

void
chaz_ConfWriter_add_writer(struct chaz_ConfWriter *writer);

typedef void
(*chaz_ConfWriter_clean_up_t)(void);
typedef void
(*chaz_ConfWriter_vappend_conf_t)(const char *fmt, va_list args); 
typedef void
(*chaz_ConfWriter_add_def_t)(const char *sym, const char *value);
typedef void
(*chaz_ConfWriter_add_global_def_t)(const char *sym, const char *value);
typedef void
(*chaz_ConfWriter_add_typedef_t)(const char *type, const char *alias);
typedef void
(*chaz_ConfWriter_add_global_typedef_t)(const char *type, const char *alias);
typedef void
(*chaz_ConfWriter_add_sys_include_t)(const char *header);
typedef void
(*chaz_ConfWriter_add_local_include_t)(const char *header);
typedef void
(*chaz_ConfWriter_start_module_t)(const char *module_name);
typedef void
(*chaz_ConfWriter_end_module_t)(void);
typedef struct chaz_ConfWriter {
    chaz_ConfWriter_clean_up_t           clean_up;
    chaz_ConfWriter_vappend_conf_t       vappend_conf;
    chaz_ConfWriter_add_def_t            add_def;
    chaz_ConfWriter_add_global_def_t     add_global_def;
    chaz_ConfWriter_add_typedef_t        add_typedef;
    chaz_ConfWriter_add_global_typedef_t add_global_typedef;
    chaz_ConfWriter_add_sys_include_t    add_sys_include;
    chaz_ConfWriter_add_local_include_t  add_local_include;
    chaz_ConfWriter_start_module_t       start_module;
    chaz_ConfWriter_end_module_t         end_module;
} chaz_ConfWriter;

#endif /* H_CHAZ_CONFWRITER */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/ConfWriterC.h"
/* Charmonizer/Core/ConfWriterC.h -- Write to a C header file.
 */

#ifndef H_CHAZ_CONFWRITERC
#define H_CHAZ_CONFWRITERC 1

/* Enable writing config to a C header file.
 */
void
chaz_ConfWriterC_enable(void);

#endif /* H_CHAZ_CONFWRITERC */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/ConfWriterPerl.h"
/* Charmonizer/Core/ConfWriterPerl.h -- Write to a Perl module file.
 */

#ifndef H_CHAZ_CONFWRITERPERL
#define H_CHAZ_CONFWRITERPERL 1

/* Enable writing config to a Perl module file.
 */
void
chaz_ConfWriterPerl_enable(void);

#endif /* H_CHAZ_CONFWRITERPERL */


/***************************************************************************/

#line 21 "src/Charmonizer/Core/ConfWriterRuby.h"
/* Charmonizer/Core/ConfWriterRuby.h -- Write to a Ruby module file.
 */

#ifndef H_CHAZ_CONFWRITERRUBY
#define H_CHAZ_CONFWRITERRUBY 1

/* Enable writing config to a Ruby module file.
 */
void
chaz_ConfWriterRuby_enable(void);

#endif /* H_CHAZ_CONFWRITERRUBY */


/***************************************************************************/

#line 21 "src/Charmonizer/Core/HeaderChecker.h"
/* Charmonizer/Probe/HeaderChecker.h
 */

#ifndef H_CHAZ_HEAD_CHECK
#define H_CHAZ_HEAD_CHECK

/* #include "Charmonizer/Core/Defines.h" */

/* Bootstrap the HeadCheck.  Call this before anything else.
 */
void
chaz_HeadCheck_init(void);

/* Check for a particular header and return true if it's available.  The
 * test-compile is only run the first time a given request is made.
 */
int
chaz_HeadCheck_check_header(const char *header_name);

/* Attempt to compile a file which pulls in all the headers specified by name
 * in a null-terminated array.  If the compile succeeds, add them all to the
 * internal register and return true.
 */
int
chaz_HeadCheck_check_many_headers(const char **header_names);

/* Return true if the member is present in the struct. */
int
chaz_HeadCheck_contains_member(const char *struct_name, const char *member,
                               const char *includes);

#endif /* H_CHAZ_HEAD_CHECK */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/OperatingSystem.h"
/* Charmonizer/Core/OperatingSystem.h - abstract an operating system down to a few
 * variables.
 */

#ifndef H_CHAZ_OPER_SYS
#define H_CHAZ_OPER_SYS

/* Safely remove a file named [name]. Needed because of Windows quirks.
 * Returns true on success, false on failure.
 */
int
chaz_OS_remove(const char *name);

/* Concatenate all arguments in a NULL-terminated list into a single command
 * string, prepend the appropriate prefix, and invoke via system().
 */
int
chaz_OS_run_local(const char *arg1, ...);

/* Invoke a command and attempt to suppress output from both stdout and stderr
 * (as if they had been sent to /dev/null).  If it's not possible to run the
 * command quietly, run it anyway.
 */
int
chaz_OS_run_quietly(const char *command);

/* Attempt to create a directory.
 */
void
chaz_OS_mkdir(const char *filepath);

/* Attempt to remove a directory, which must be empty.
 */
void
chaz_OS_rmdir(const char *filepath);

/* Return the extension for an executable on this system.
 */
const char*
chaz_OS_exe_ext(void);

/* Return the extension for a compiled object on this system.
 */
const char*
chaz_OS_obj_ext(void);

/* Return the equivalent of /dev/null on this system.
 */
const char*
chaz_OS_dev_null(void);

/* Initialize the Charmonizer/Core/OperatingSystem module.
 */
void
chaz_OS_init(void);

#endif /* H_CHAZ_COMPILER */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/Util.h"
/* Chaz/Core/Util.h -- miscellaneous utilities.
 */

#ifndef H_CHAZ_UTIL
#define H_CHAZ_UTIL 1

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>

extern int chaz_Util_verbosity;

/* Open a file (truncating if necessary) and write [content] to it.  Util_die() if
 * an error occurs.
 */
void
chaz_Util_write_file(const char *filename, const char *content);

/* Read an entire file into memory.
 */
char*
chaz_Util_slurp_file(const char *file_path, size_t *len_ptr);

/* Return a newly allocated copy of a NULL-terminated string.
 */
char*
chaz_Util_strdup(const char *string);

/* Get the length of a file (may overshoot on text files under DOS).
 */
long
chaz_Util_flength(void *file);

/* Print an error message to stderr and exit.
 */
void
chaz_Util_die(const char *format, ...);

/* Print an error message to stderr.
 */
void
chaz_Util_warn(const char *format, ...);

/* Attept to delete a file.  Return true if the file is gone, whether or not
 * it was there to begin with.  Issue a warning and return false if the file
 * still exists.
 */
int
chaz_Util_remove_and_verify(const char *file_path);

/* Attempt to open a file for reading, then close it immediately.
 */
int
chaz_Util_can_open_file(const char *file_path);

#endif /* H_CHAZ_UTIL */



/***************************************************************************/

#line 21 "src/Charmonizer/Probe.h"
#ifndef H_CHAZ
#define H_CHAZ 1

#include <stddef.h>
#include <stdio.h>

/* Set up the Charmonizer environment.  This should be called before anything
 * else.
 *
 * If the environment variable CHARM_VERBOSITY has been set, it will be
 * processed at this time:
 *
 *      0 - silent
 *      1 - normal
 *      2 - debugging
 *
 * @param cc_command the string used to invoke the C compiler via system()
 * @param cc_flags flags which will be passed on to the C compiler
 */
void
chaz_Probe_init(const char *cc_command, const char *cc_flags);

/* Clean up the Charmonizer environment -- deleting tempfiles, etc.  This
 * should be called only after everything else finishes.
 */
void
chaz_Probe_clean_up(void);

#endif /* Include guard. */



/***************************************************************************/

#line 21 "src/Charmonizer/Probe/AtomicOps.h"
/* Charmonizer/Probe/AtomicOps.h
 */

#ifndef H_CHAZ_ATOMICOPS
#define H_CHAZ_ATOMICOPS

#include <stdio.h>

/* Run the AtomicOps module.
 *
 * These following symbols will be defined if the associated headers are
 * available:
 *
 * HAS_LIBKERN_OSATOMIC_H  <libkern/OSAtomic.h> (Mac OS X)
 * HAS_SYS_ATOMIC_H        <sys/atomic.h>       (Solaris)
 * HAS_INTRIN_H            <intrin.h>           (Windows)
 *
 * This symbol is defined if OSAtomicCompareAndSwapPtr is available:
 *
 * HAS_OSATOMIC_CAS_PTR
 */
void chaz_AtomicOps_run(void);

#endif /* H_CHAZ_ATOMICOPS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/DirManip.h"
/* Charmonizer/Probe/DirManip.h
 */

#ifndef H_CHAZ_DIRMANIP
#define H_CHAZ_DIRMANIP

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

#endif /* H_CHAZ_DIR_SEP */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/Floats.h"
/* Charmonizer/Probe/Floats.h -- floating point types.
 *
 * The following symbols will be created if the platform supports IEEE 754
 * floating point types:
 *
 * F32_NAN
 * F32_INF
 * F32_NEGINF
 * F64_NAN
 * F64_INF
 * F64_NEGINF
 *
 * TODO: Actually test to see whether IEEE 754 is supported, rather than just
 * lying about it.
 */

#ifndef H_CHAZ_FLOATS
#define H_CHAZ_FLOATS

/* Run the Floats module.
 */
void
chaz_Floats_run(void);

#endif /* H_CHAZ_FLOATS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/FuncMacro.h"
/* Charmonizer/Probe/FuncMacro.h
 */

#ifndef H_CHAZ_FUNC_MACRO
#define H_CHAZ_FUNC_MACRO

#include <stdio.h>

/* Run the FuncMacro module.
 *
 * If __func__ successfully resolves, this will be defined:
 *
 * HAS_ISO_FUNC_MACRO
 *
 * If __FUNCTION__ successfully resolves, this will be defined:
 *
 * HAS_GNUC_FUNC_MACRO
 *
 * If one or the other succeeds, these will be defined:
 *
 * HAS_FUNC_MACRO
 * FUNC_MACRO
 *
 * The "inline" keyword will also be probed for.  If it is available, the
 * following macro will be defined to "inline", otherwise it will be defined
 * to nothing.
 *
 * INLINE
 */
void chaz_FuncMacro_run(void);

#endif /* H_CHAZ_FUNC_MACRO */




/***************************************************************************/

#line 20 "src/Charmonizer/Probe/Headers.h"
/* Charmonizer/Probe/Headers.h
 */

#ifndef H_CHAZ_HEADERS
#define H_CHAZ_HEADERS

#include <stdio.h>
/* #include "Charmonizer/Core/Defines.h" */

/* Check whether a particular header file is available.  The test-compile is
 * only run the first time a given request is made.
 */
int
chaz_Headers_check(const char *header_name);

/* Run the Headers module.
 *
 * Exported symbols:
 *
 * If HAS_C89 is declared, this system has all the header files described in
 * Ansi C 1989.  HAS_C90 is a synonym.  (It would be surprising if they are
 * not defined, because Charmonizer itself assumes C89.)
 *
 * HAS_C89
 * HAS_C90
 *
 * One symbol is exported for each C89 header file:
 *
 * HAS_ASSERT_H
 * HAS_CTYPE_H
 * HAS_ERRNO_H
 * HAS_FLOAT_H
 * HAS_LIMITS_H
 * HAS_LOCALE_H
 * HAS_MATH_H
 * HAS_SETJMP_H
 * HAS_SIGNAL_H
 * HAS_STDARG_H
 * HAS_STDDEF_H
 * HAS_STDIO_H
 * HAS_STDLIB_H
 * HAS_STRING_H
 * HAS_TIME_H
 *
 * One symbol is exported for every POSIX header present, and HAS_POSIX is
 * exported if they're all there.
 *
 * HAS_POSIX
 *
 * HAS_CPIO_H
 * HAS_DIRENT_H
 * HAS_FCNTL_H
 * HAS_GRP_H
 * HAS_PWD_H
 * HAS_SYS_STAT_H
 * HAS_SYS_TIMES_H
 * HAS_SYS_TYPES_H
 * HAS_SYS_UTSNAME_H
 * HAS_WAIT_H
 * HAS_TAR_H
 * HAS_TERMIOS_H
 * HAS_UNISTD_H
 * HAS_UTIME_H
 *
 * If pthread.h is available, this will be exported:
 *
 * HAS_PTHREAD_H
 */
void
chaz_Headers_run(void);

#endif /* H_CHAZ_HEADERS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/Integers.h"
/* Charmonizer/Probe/Integers.h -- info about integer types and sizes.
 *
 * One or the other of these will be defined, depending on whether the
 * processor is big-endian or little-endian.
 *
 * BIG_END
 * LITTLE_END
 *
 * These will always be defined:
 *
 * SIZEOF_CHAR
 * SIZEOF_SHORT
 * SIZEOF_INT
 * SIZEOF_LONG
 * SIZEOF_PTR
 *
 * If long longs are available these symbols will be defined:
 *
 * HAS_LONG_LONG
 * SIZEOF_LONG_LONG
 *
 * Similarly, with the __int64 type (the sizeof is included for completeness):
 *
 * HAS___INT64
 * SIZEOF___INT64
 *
 * If the inttypes.h or stdint.h header files are available, these may be
 * defined:
 *
 * HAS_INTTYPES_H
 * HAS_STDINT_H
 *
 * If stdint.h is is available, it will be pound-included in the configuration
 * header.  If it is not, the following typedefs and macros will be defined if
 * possible:
 *
 * int8_t
 * int16_t
 * int32_t
 * int64_t
 * uint8_t
 * uint16_t
 * uint32_t
 * uint64_t
 * INT8_MAX
 * INT16_MAX
 * INT32_MAX
 * INT64_MAX
 * INT8_MIN
 * INT16_MIN
 * INT32_MIN
 * INT64_MIN
 * UINT8_MAX
 * UINT16_MAX
 * UINT32_MAX
 * UINT64_MAX
 * SIZE_MAX
 * INT32_C
 * INT64_C
 * UINT32_C
 * UINT64_C
 *
 * If inttypes.h is is available, it will be pound-included in the
 * configuration header.  If it is not, the following macros will be defined if
 * possible:
 *
 * PRId64
 * PRIu64
 *
 * The following typedefs will be created:
 *
 * bool_t
 *
 * Availability of integer types is indicated by which of these are defined:
 *
 * HAS_INT8_T
 * HAS_INT16_T
 * HAS_INT32_T
 * HAS_INT64_T
 *
 * If 64-bit integers are available, this macro will promote pointers to i64_t
 * safely.
 *
 * PTR_TO_I64(ptr)
 *
 * These symbols will be defined if they are not already:
 *
 * true
 * false
 */

#ifndef H_CHAZ_INTEGERS
#define H_CHAZ_INTEGERS

#include <stdio.h>

/* Run the Integers module.
 */
void chaz_Integers_run(void);

#endif /* H_CHAZ_INTEGERS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/LargeFiles.h"
/* Charmonizer/Probe/LargeFiles.h
 */

#ifndef H_CHAZ_LARGE_FILES
#define H_CHAZ_LARGE_FILES

#include <stdio.h>

/* The LargeFiles module attempts to detect these symbols or alias them to
 * synonyms:
 *
 * off64_t
 * fopen64
 * ftello64
 * fseeko64
 * lseek64
 * pread64
 *
 * If off64_t or its equivalent is available, this will be defined:
 *
 * HAS_64BIT_OFFSET_TYPE
 *
 * If 64-bit variants of fopen, ftell, and fseek are available, this will be
 * defined:
 *
 * HAS_64BIT_STDIO
 *
 * If 64-bit variants of pread and lseek are available, then corresponding
 * symbols will be defined:
 *
 * HAS_64BIT_PREAD
 * HAS_64BIT_LSEEK
 *
 * Use of the off64_t symbol may require sys/types.h.
 */
void chaz_LargeFiles_run(void);

#endif /* H_CHAZ_LARGE_FILES */



/***************************************************************************/

#line 21 "src/Charmonizer/Probe/Memory.h"
/* Charmonizer/Probe/Memory.h
 */

#ifndef H_CHAZ_MEMORY
#define H_CHAZ_MEMORY

/* The Memory module attempts to detect these symbols or alias them to
 * synonyms:
 *
 * alloca
 *
 * These following symbols will be defined if the associated headers are
 * available:
 *
 * HAS_SYS_MMAN_H          <sys/mman.h>
 * HAS_ALLOCA_H            <alloca.h>
 * HAS_MALLOC_H            <malloc.h>
 *
 * Defined if alloca() is available via stdlib.h:
 *
 * ALLOCA_IN_STDLIB_H
 */
void chaz_Memory_run(void);

#endif /* H_CHAZ_MEMORY */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/SymbolVisibility.h"
/* Charmonizer/Probe/SymbolVisibility.h
 */

#ifndef H_CHAZ_SYMBOLVISIBILITY
#define H_CHAZ_SYMBOLVISIBILITY

void chaz_SymbolVisibility_run(void);

#endif /* H_CHAZ_SYMBOLVISIBILITY */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/UnusedVars.h"
/* Charmonizer/Probe/UnusedVars.h
 */

#ifndef H_CHAZ_UNUSED_VARS
#define H_CHAZ_UNUSED_VARS

#include <stdio.h>

/* Run the UnusedVars module.
 *
 * These symbols are exported:
 *
 * UNUSED_VAR(var)
 * UNREACHABLE_RETURN(type)
 *
 */
void chaz_UnusedVars_run(void);

#endif /* H_CHAZ_UNUSED_VARS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/VariadicMacros.h"
/* Charmonizer/Probe/VariadicMacros.h
 */

#ifndef H_CHAZ_VARIADIC_MACROS
#define H_CHAZ_VARIADIC_MACROS

#include <stdio.h>

/* Run the VariadicMacros module.
 *
 * If your compiler supports ISO-style variadic macros, this will be defined:
 *
 * HAS_ISO_VARIADIC_MACROS
 *
 * If your compiler supports GNU-style variadic macros, this will be defined:
 *
 * HAS_GNUC_VARIADIC_MACROS
 *
 * If you have at least one of the above, this will be defined:
 *
 * HAS_VARIADIC_MACROS
 */
void chaz_VariadicMacros_run(void);

#endif /* H_CHAZ_VARIADIC_MACROS */




/***************************************************************************/

#line 17 "src/Charmonizer/Core/Compiler.c"
#include <string.h>
#include <stdlib.h>
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

/* Detect macros which may help to identify some compilers.
 */
static void
chaz_CC_detect_known_compilers(void);

/* Temporary files. */
#define CHAZ_CC_TRY_SOURCE_PATH  "_charmonizer_try.c"
#define CHAZ_CC_TRY_BASENAME     "_charmonizer_try"
#define CHAZ_CC_TARGET_PATH      "_charmonizer_target"

/* Static vars. */
static struct {
    char     *cc_command;
    char     *cc_flags;
    char    **inc_dirs;
    char     *try_exe_name;
    char     *try_obj_name;
    char      include_flag[10];
    char      object_flag[10];
    char      exe_flag[10];
    char      no_link_flag[10];
    char      error_flag[10];
    int       defines___GNUC__;
    int       defines__MSC_VER;
    int       defines___clang__;
    int       warnings_as_errors;
} chaz_CC = {
    NULL, NULL, NULL, NULL, NULL,
    "", "", "", "", "",
    0, 0, 0, 0
};

void
chaz_CC_set_warnings_as_errors(const int flag) {
    chaz_CC.warnings_as_errors = flag;
    if (chaz_CC.warnings_as_errors) {
        if (chaz_CC.defines__MSC_VER)  {
            strcpy(chaz_CC.error_flag, "/WX");
        } else {
            strcpy(chaz_CC.error_flag, "-Werror");
        }
    }
    else {
        strcpy(chaz_CC.error_flag, "");
    }
}

void
chaz_CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (chaz_Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign. */
    chaz_CC.cc_command      = chaz_Util_strdup(compiler_command);
    chaz_CC.cc_flags        = chaz_Util_strdup(compiler_flags);

    /* Init. */
    chaz_CC.inc_dirs              = (char**)calloc(sizeof(char*), 1);

    /* Add the current directory as an include dir. */
    chaz_CC_add_inc_dir(".");

    /* Set names for the targets which we "try" to compile. */
    {
        const char *exe_ext = chaz_OS_exe_ext();
        const char *obj_ext = chaz_OS_obj_ext();
        size_t exe_len = strlen(CHAZ_CC_TRY_BASENAME) + strlen(exe_ext) + 1;
        size_t obj_len = strlen(CHAZ_CC_TRY_BASENAME) + strlen(obj_ext) + 1;
        chaz_CC.try_exe_name = (char*)malloc(exe_len);
        chaz_CC.try_obj_name = (char*)malloc(obj_len);
        sprintf(chaz_CC.try_exe_name, "%s%s", CHAZ_CC_TRY_BASENAME, exe_ext);
        sprintf(chaz_CC.try_obj_name, "%s%s", CHAZ_CC_TRY_BASENAME, obj_ext);
    }

    /* If we can't compile anything, game over. */
    if (chaz_Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }
    /* Try POSIX argument style. */
    strcpy(chaz_CC.include_flag, "-I ");
    strcpy(chaz_CC.object_flag,  "-o ");
    strcpy(chaz_CC.exe_flag,     "-o ");
    strcpy(chaz_CC.no_link_flag, "-c ");
    compile_succeeded = chaz_CC_test_compile(code);
    if (!compile_succeeded) {
        /* Try MSVC argument style. */
        strcpy(chaz_CC.include_flag, "/I");
        strcpy(chaz_CC.object_flag,  "/Fo");
        strcpy(chaz_CC.exe_flag,     "/Fe");
        strcpy(chaz_CC.no_link_flag, "/c");
        compile_succeeded = chaz_CC_test_compile(code);
    }
    if (!compile_succeeded) {
        chaz_Util_die("Failed to compile a small test file");
    }

    chaz_CC_detect_known_compilers();
}

static const char detect_macro_code[] =
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(  #ifndef %s                     )
    CHAZ_QUOTE(  #error "nope"                  )
    CHAZ_QUOTE(  #endif                         )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

static int
chaz_CC_detect_macro(const char *macro) {
    size_t size = sizeof(detect_macro_code) + strlen(macro) + 20;
    char *code = (char*)malloc(size);
    int retval;
    sprintf(code, detect_macro_code, macro);
    retval = chaz_CC_test_compile(code);
    free(code);
    return retval;
}

static void
chaz_CC_detect_known_compilers(void) {
    chaz_CC.defines___GNUC__  = chaz_CC_detect_macro("__GNUC__");
    chaz_CC.defines__MSC_VER  = chaz_CC_detect_macro("_MSC_VER");
    chaz_CC.defines___clang__ = chaz_CC_detect_macro("__clang__");
}

void
chaz_CC_clean_up(void) {
    char **dirs;

    for (dirs = chaz_CC.inc_dirs; *dirs != NULL; dirs++) {
        free(*dirs);
    }
    free(chaz_CC.inc_dirs);

    free(chaz_CC.cc_command);
    free(chaz_CC.cc_flags);

    free(chaz_CC.try_obj_name);
    free(chaz_CC.try_exe_name);
}

static char*
chaz_CC_inc_dir_string(void) {
    size_t needed = 0;
    char  *inc_dir_string;
    char **dirs;
    for (dirs = chaz_CC.inc_dirs; *dirs != NULL; dirs++) {
        needed += strlen(chaz_CC.include_flag) + 2;
        needed += strlen(*dirs);
    }
    inc_dir_string = (char*)malloc(needed + 1);
    inc_dir_string[0] = '\0';
    for (dirs = chaz_CC.inc_dirs; *dirs != NULL; dirs++) {
        strcat(inc_dir_string, chaz_CC.include_flag);
        strcat(inc_dir_string, *dirs);
        strcat(inc_dir_string, " ");
    }
    return inc_dir_string;
}

int
chaz_CC_compile_exe(const char *source_path, const char *exe_name,
                    const char *code) {
    const char *exe_ext        = chaz_OS_exe_ext();
    size_t   exe_file_buf_size = strlen(exe_name) + strlen(exe_ext) + 1;
    char    *exe_file          = (char*)malloc(exe_file_buf_size);
    size_t   junk_buf_size     = exe_file_buf_size + 3;
    char    *junk              = (char*)malloc(junk_buf_size);
    size_t   exe_file_buf_len  = sprintf(exe_file, "%s%s", exe_name, exe_ext);
    char    *inc_dir_string    = chaz_CC_inc_dir_string();
    size_t   command_max_size  = strlen(chaz_CC.cc_command)
                                 + strlen(chaz_CC.error_flag)
                                 + strlen(source_path)
                                 + strlen(chaz_CC.exe_flag)
                                 + exe_file_buf_len
                                 + strlen(inc_dir_string)
                                 + strlen(chaz_CC.cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s%s %s %s",
            chaz_CC.cc_command, chaz_CC.error_flag, 
            source_path, chaz_CC.exe_flag, 
            exe_file, inc_dir_string, 
            chaz_CC.cc_flags);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    if (chaz_CC.defines__MSC_VER) {
        /* Zap MSVC junk. */
        sprintf(junk, "%s.obj", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.ilk", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.pdb", exe_name);
        chaz_Util_remove_and_verify(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(exe_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(inc_dir_string);
    free(junk);
    free(exe_file);
    return result;
}

int
chaz_CC_compile_obj(const char *source_path, const char *obj_name,
                    const char *code) {
    const char *obj_ext        = chaz_OS_obj_ext();
    size_t   obj_file_buf_size = strlen(obj_name) + strlen(obj_ext) + 1;
    char    *obj_file          = (char*)malloc(obj_file_buf_size);
    size_t   obj_file_buf_len  = sprintf(obj_file, "%s%s", obj_name, obj_ext);
    char    *inc_dir_string    = chaz_CC_inc_dir_string();
    size_t   command_max_size  = strlen(chaz_CC.cc_command)
                                 + strlen(chaz_CC.no_link_flag)
                                 + strlen(chaz_CC.error_flag)
                                 + strlen(source_path)
                                 + strlen(chaz_CC.object_flag)
                                 + obj_file_buf_len
                                 + strlen(inc_dir_string)
                                 + strlen(chaz_CC.cc_flags)
                                 + 200; /* command start, _charm_run, etc.  */
    char *command = (char*)malloc(command_max_size);
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    sprintf(command, "%s %s %s %s %s%s %s %s",
            chaz_CC.cc_command, chaz_CC.no_link_flag, chaz_CC.error_flag,
            source_path, chaz_CC.object_flag, 
            obj_file, inc_dir_string,
            chaz_CC.cc_flags);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(obj_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    free(command);
    free(inc_dir_string);
    free(obj_file);
    return result;
}

int
chaz_CC_test_compile(const char *source) {
    int compile_succeeded;
    if (!chaz_Util_remove_and_verify(chaz_CC.try_obj_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_obj_name);
    }
    compile_succeeded = chaz_CC_compile_obj(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(chaz_CC.try_obj_name);
    return compile_succeeded;
}

char*
chaz_CC_capture_output(const char *source, size_t *output_len) {
    char *captured_output = NULL;
    int compile_succeeded;

    /* Clear out previous versions and test to make sure removal worked. */
    if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
    }
    if (!chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH)) {
        chaz_Util_die("Failed to delete file '%s'", CHAZ_CC_TARGET_PATH);
    }

    /* Attempt compilation; if successful, run app and slurp output. */
    compile_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    if (compile_succeeded) {
        chaz_OS_run_local(chaz_CC.try_exe_name, NULL);
        captured_output
            = chaz_Util_slurp_file(CHAZ_CC_TARGET_PATH, output_len);
    }
    else {
        *output_len = 0;
    }

    /* Remove all the files we just created. */
    chaz_Util_remove_and_verify(CHAZ_CC_TRY_SOURCE_PATH);
    chaz_Util_remove_and_verify(chaz_CC.try_exe_name);
    chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH);

    return captured_output;
}

void
chaz_CC_add_inc_dir(const char *dir) {
    size_t num_dirs = 0;
    char **dirs = chaz_CC.inc_dirs;

    /* Count up the present number of dirs, reallocate. */
    while (*dirs++ != NULL) { num_dirs++; }
    num_dirs += 1; /* Passed-in dir. */
    chaz_CC.inc_dirs = (char**)realloc(chaz_CC.inc_dirs,
                                       (num_dirs + 1) * sizeof(char*));

    /* Put the passed-in dir at the end of the list. */
    chaz_CC.inc_dirs[num_dirs - 1] = chaz_Util_strdup(dir);
    chaz_CC.inc_dirs[num_dirs] = NULL;
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/ConfWriter.c"
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
#include <stdarg.h>
#include <stdio.h>

#define CW_MAX_WRITERS 10
static struct {
    chaz_ConfWriter *writers[CW_MAX_WRITERS];
    size_t num_writers;
} chaz_CW;

void
chaz_ConfWriter_init(void) {
    chaz_CW.num_writers = 0;
    return;
}

void
chaz_ConfWriter_clean_up(void) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->clean_up();
    }
}

void
chaz_ConfWriter_append_conf(const char *fmt, ...) {
    va_list args;
    size_t i;
    
    for (i = 0; i < chaz_CW.num_writers; i++) {
        va_start(args, fmt);
        chaz_CW.writers[i]->vappend_conf(fmt, args);
        va_end(args);
    }
}

void
chaz_ConfWriter_add_def(const char *sym, const char *value) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_def(sym, value);
    }
}

void
chaz_ConfWriter_add_global_def(const char *sym, const char *value) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_global_def(sym, value);
    }
}

void
chaz_ConfWriter_add_typedef(const char *type, const char *alias) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_typedef(type, alias);
    }
}

void
chaz_ConfWriter_add_global_typedef(const char *type, const char *alias) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_global_typedef(type, alias);
    }
}

void
chaz_ConfWriter_add_sys_include(const char *header) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_sys_include(header);
    }
}

void
chaz_ConfWriter_add_local_include(const char *header) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->add_local_include(header);
    }
}

void
chaz_ConfWriter_start_module(const char *module_name) {
    size_t i;
    if (chaz_Util_verbosity > 0) {
        printf("Running %s module...\n", module_name);
    }
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->start_module(module_name);
    }
}

void
chaz_ConfWriter_end_module(void) {
    size_t i;
    for (i = 0; i < chaz_CW.num_writers; i++) {
        chaz_CW.writers[i]->end_module();
    }
}

void
chaz_ConfWriter_add_writer(chaz_ConfWriter *writer) {
    chaz_CW.writers[chaz_CW.num_writers] = writer;
    chaz_CW.num_writers++;
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/ConfWriterC.c"
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/ConfWriterC.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */
/* #include "Charmonizer/Core/Compiler.h" */
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum chaz_ConfElemType {
    CHAZ_CONFELEM_DEF,
    CHAZ_CONFELEM_GLOBAL_DEF,
    CHAZ_CONFELEM_TYPEDEF,
    CHAZ_CONFELEM_GLOBAL_TYPEDEF,
    CHAZ_CONFELEM_SYS_INCLUDE,
    CHAZ_CONFELEM_LOCAL_INCLUDE
} chaz_ConfElemType;

typedef struct chaz_ConfElem {
    char *str1;
    char *str2;
    chaz_ConfElemType type;
} chaz_ConfElem;

/* Static vars. */
static struct {
    FILE          *fh;
    chaz_ConfElem *defs;
    size_t         def_cap;
    size_t         def_count;
} chaz_ConfWriterC = { NULL, NULL, 0, 0 };
static chaz_ConfWriter CWC_conf_writer;

/* Open the charmony.h file handle.  Print supplied text to it, if non-null.
 * Print an explanatory comment and open the include guard.
 */
static void
chaz_ConfWriterC_open_charmony_h(const char *charmony_start);

/* Push a new elem onto the def list. */
static void
chaz_ConfWriterC_push_def_list_item(const char *str1, const char *str2,
                     chaz_ConfElemType type);

/* Free the def list. */
static void
chaz_ConfWriterC_clear_def_list(void);

static void
chaz_ConfWriterC_clean_up(void);
static void
chaz_ConfWriterC_vappend_conf(const char *fmt, va_list args);
static void
chaz_ConfWriterC_add_def(const char *sym, const char *value);
static void
chaz_ConfWriterC_add_global_def(const char *sym, const char *value);
static void
chaz_ConfWriterC_add_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterC_add_global_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterC_add_sys_include(const char *header);
static void
chaz_ConfWriterC_add_local_include(const char *header);
static void
chaz_ConfWriterC_start_module(const char *module_name);
static void
chaz_ConfWriterC_end_module(void);

void
chaz_ConfWriterC_enable(void) {
    CWC_conf_writer.clean_up           = chaz_ConfWriterC_clean_up;
    CWC_conf_writer.vappend_conf       = chaz_ConfWriterC_vappend_conf;
    CWC_conf_writer.add_def            = chaz_ConfWriterC_add_def;
    CWC_conf_writer.add_global_def     = chaz_ConfWriterC_add_global_def;
    CWC_conf_writer.add_typedef        = chaz_ConfWriterC_add_typedef;
    CWC_conf_writer.add_global_typedef = chaz_ConfWriterC_add_global_typedef;
    CWC_conf_writer.add_sys_include    = chaz_ConfWriterC_add_sys_include;
    CWC_conf_writer.add_local_include  = chaz_ConfWriterC_add_local_include;
    CWC_conf_writer.start_module       = chaz_ConfWriterC_start_module;
    CWC_conf_writer.end_module         = chaz_ConfWriterC_end_module;
    chaz_ConfWriterC_open_charmony_h(NULL);
    chaz_ConfWriter_add_writer(&CWC_conf_writer);
    return;
}

static void
chaz_ConfWriterC_open_charmony_h(const char *charmony_start) {
    /* Open the filehandle. */
    chaz_ConfWriterC.fh = fopen("charmony.h", "w+");
    if (chaz_ConfWriterC.fh == NULL) {
        chaz_Util_die("Can't open 'charmony.h': %s", strerror(errno));
    }

    /* Print supplied text (if any) along with warning, open include guard. */
    if (charmony_start != NULL) {
        fwrite(charmony_start, sizeof(char), strlen(charmony_start),
               chaz_ConfWriterC.fh);
    }
    fprintf(chaz_ConfWriterC.fh,
            "/* Header file auto-generated by Charmonizer. \n"
            " * DO NOT EDIT THIS FILE!!\n"
            " */\n\n"
            "#ifndef H_CHARMONY\n"
            "#define H_CHARMONY 1\n\n"
           );
}

static void
chaz_ConfWriterC_clean_up(void) {
    /* Write the last bit of charmony.h and close. */
    fprintf(chaz_ConfWriterC.fh, "#endif /* H_CHARMONY */\n\n");
    if (fclose(chaz_ConfWriterC.fh)) {
        chaz_Util_die("Couldn't close 'charmony.h': %s", strerror(errno));
    }
}

static void
chaz_ConfWriterC_vappend_conf(const char *fmt, va_list args) {
    vfprintf(chaz_ConfWriterC.fh, fmt, args);
}

static int
chaz_ConfWriterC_sym_is_uppercase(const char *sym) {
    unsigned i;
    for (i = 0; sym[i] != 0; i++) {
        if (!isupper(sym[i])) {
            if (islower(sym[i])) {
                return false;
            }
            else if (sym[i] != '_') {
                return true;
            }
        }
    }
    return true;
}

static void
chaz_ConfWriterC_add_def(const char *sym, const char *value) {
    chaz_ConfWriterC_push_def_list_item(sym, value, CHAZ_CONFELEM_DEF);
}

static void
chaz_ConfWriterC_append_def_to_conf(const char *sym, const char *value) {
    if (value) {
        if (chaz_ConfWriterC_sym_is_uppercase(sym)) {
            fprintf(chaz_ConfWriterC.fh, "#define CHY_%s %s\n", sym, value);
        }
        else {
            fprintf(chaz_ConfWriterC.fh, "#define chy_%s %s\n", sym, value);
        }
    }
    else {
        if (chaz_ConfWriterC_sym_is_uppercase(sym)) {
            fprintf(chaz_ConfWriterC.fh, "#define CHY_%s\n", sym);
        }
        else {
            fprintf(chaz_ConfWriterC.fh, "#define chy_%s\n", sym);
        }
    }
}

static void
chaz_ConfWriterC_add_global_def(const char *sym, const char *value) {
    chaz_ConfWriterC_push_def_list_item(sym, value, CHAZ_CONFELEM_GLOBAL_DEF);
}

static void
chaz_ConfWriterC_append_global_def_to_conf(const char *sym,
        const char *value) {
    char *name_end = strchr(sym, '(');
    if (name_end == NULL) {
        fprintf(chaz_ConfWriterC.fh, "#ifndef %s\n", sym);
    }
    else {
        size_t  name_len = (size_t)(name_end - sym);
        char   *name     = chaz_Util_strdup(sym);
        name[name_len] = '\0';
        fprintf(chaz_ConfWriterC.fh, "#ifndef %s\n", name);
        free(name);
    }
    if (value) {
        fprintf(chaz_ConfWriterC.fh, "  #define %s %s\n", sym, value);
    }
    else {
        fprintf(chaz_ConfWriterC.fh, "  #define %s\n", sym);
    }
    fprintf(chaz_ConfWriterC.fh, "#endif\n");
}

static void
chaz_ConfWriterC_add_typedef(const char *type, const char *alias) {
    chaz_ConfWriterC_push_def_list_item(alias, type, CHAZ_CONFELEM_TYPEDEF);
}

static void
chaz_ConfWriterC_append_typedef_to_conf(const char *type, const char *alias) {
    if (chaz_ConfWriterC_sym_is_uppercase(alias)) {
        fprintf(chaz_ConfWriterC.fh, "typedef %s CHY_%s;\n", type, alias);
    }
    else {
        fprintf(chaz_ConfWriterC.fh, "typedef %s chy_%s;\n", type, alias);
    }
}

static void
chaz_ConfWriterC_add_global_typedef(const char *type, const char *alias) {
    chaz_ConfWriterC_push_def_list_item(alias, type,
            CHAZ_CONFELEM_GLOBAL_TYPEDEF);
}

static void
chaz_ConfWriterC_append_global_typedef_to_conf(const char *type,
        const char *alias) {
    fprintf(chaz_ConfWriterC.fh, "typedef %s %s;\n", type, alias);
}

static void
chaz_ConfWriterC_add_sys_include(const char *header) {
    chaz_ConfWriterC_push_def_list_item(header, NULL,
                                        CHAZ_CONFELEM_SYS_INCLUDE);
}

static void
chaz_ConfWriterC_append_sys_include_to_conf(const char *header) {
    fprintf(chaz_ConfWriterC.fh, "#include <%s>\n", header);
}

static void
chaz_ConfWriterC_add_local_include(const char *header) {
    chaz_ConfWriterC_push_def_list_item(header, NULL,
                                        CHAZ_CONFELEM_LOCAL_INCLUDE);
}

static void
chaz_ConfWriterC_append_local_include_to_conf(const char *header) {
    fprintf(chaz_ConfWriterC.fh, "#include \"%s\"\n", header);
}

static void
chaz_ConfWriterC_start_module(const char *module_name) {
    fprintf(chaz_ConfWriterC.fh, "\n/* %s */\n", module_name);
}

static void
chaz_ConfWriterC_end_module(void) {
    size_t i;
    chaz_ConfElem *defs = chaz_ConfWriterC.defs;
    for (i = 0; i < chaz_ConfWriterC.def_count; i++) {
        switch (defs[i].type) {
            case CHAZ_CONFELEM_DEF:
                chaz_ConfWriterC_append_def_to_conf(defs[i].str1,
                                                    defs[i].str2);
                break;
            case CHAZ_CONFELEM_GLOBAL_DEF:
                chaz_ConfWriterC_append_global_def_to_conf(defs[i].str1,
                                                           defs[i].str2);
                break;
            case CHAZ_CONFELEM_TYPEDEF:
                chaz_ConfWriterC_append_typedef_to_conf(defs[i].str2,
                                                        defs[i].str1);
                break;
            case CHAZ_CONFELEM_GLOBAL_TYPEDEF:
                chaz_ConfWriterC_append_global_typedef_to_conf(defs[i].str2,
                                                               defs[i].str1);
                break;
            case CHAZ_CONFELEM_SYS_INCLUDE:
                chaz_ConfWriterC_append_sys_include_to_conf(defs[i].str1);
                break;
            case CHAZ_CONFELEM_LOCAL_INCLUDE:
                chaz_ConfWriterC_append_local_include_to_conf(defs[i].str1);
                break;
            default:
                chaz_Util_die("Internal error: bad element type %d",
                              (int)defs[i].type);
        }
    }

    /* Write out short names. */
    fprintf(chaz_ConfWriterC.fh,
        "\n#if defined(CHY_USE_SHORT_NAMES) "
        "|| defined(CHAZ_USE_SHORT_NAMES)\n"
    );
    for (i = 0; i < chaz_ConfWriterC.def_count; i++) {
        switch (defs[i].type) {
            case CHAZ_CONFELEM_DEF: 
            case CHAZ_CONFELEM_TYPEDEF:
                {
                    const char *sym = defs[i].str1;
                    const char *value = defs[i].str2;
                    if (!value || strcmp(sym, value) != 0) {
                        const char *prefix
                            = chaz_ConfWriterC_sym_is_uppercase(sym)
                              ? "CHY_" : "chy_";
                        fprintf(chaz_ConfWriterC.fh, "  #define %s %s%s\n",
                                sym, prefix, sym);
                    }
                }
                break;
            case CHAZ_CONFELEM_GLOBAL_DEF:
            case CHAZ_CONFELEM_GLOBAL_TYPEDEF:
            case CHAZ_CONFELEM_SYS_INCLUDE:
            case CHAZ_CONFELEM_LOCAL_INCLUDE:
                /* no-op */
                break;
            default:
                chaz_Util_die("Internal error: bad element type %d",
                              (int)defs[i].type);
        }
    }

    fprintf(chaz_ConfWriterC.fh, "#endif /* USE_SHORT_NAMES */\n");
    fprintf(chaz_ConfWriterC.fh, "\n");

    chaz_ConfWriterC_clear_def_list();
}

static void
chaz_ConfWriterC_push_def_list_item(const char *str1, const char *str2,
                     chaz_ConfElemType type) {
    if (chaz_ConfWriterC.def_count >= chaz_ConfWriterC.def_cap) { 
        size_t amount;
        chaz_ConfWriterC.def_cap += 10;
        amount = chaz_ConfWriterC.def_cap * sizeof(chaz_ConfElem);
        chaz_ConfWriterC.defs
            = (chaz_ConfElem*)realloc(chaz_ConfWriterC.defs, amount);
    }
    chaz_ConfWriterC.defs[chaz_ConfWriterC.def_count].str1
        = str1 ? chaz_Util_strdup(str1) : NULL;
    chaz_ConfWriterC.defs[chaz_ConfWriterC.def_count].str2
        = str2 ? chaz_Util_strdup(str2) : NULL;
    chaz_ConfWriterC.defs[chaz_ConfWriterC.def_count].type = type;
    chaz_ConfWriterC.def_count++;
}

static void
chaz_ConfWriterC_clear_def_list(void) {
    size_t i;
    for (i = 0; i < chaz_ConfWriterC.def_count; i++) {
        free(chaz_ConfWriterC.defs[i].str1);
        free(chaz_ConfWriterC.defs[i].str2);
    }
    free(chaz_ConfWriterC.defs);
    chaz_ConfWriterC.defs      = NULL;
    chaz_ConfWriterC.def_cap   = 0;
    chaz_ConfWriterC.def_count = 0;
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/ConfWriterPerl.c"
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/ConfWriterPerl.h" */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Static vars. */
static struct {
    FILE *fh;
} chaz_CWPerl = { NULL };
static chaz_ConfWriter CWPerl_conf_writer;

/* Open the Charmony.pm file handle.
 */
static void
chaz_ConfWriterPerl_open_config_pm(void);

static void
chaz_ConfWriterPerl_clean_up(void);
static void
chaz_ConfWriterPerl_vappend_conf(const char *fmt, va_list args);
static void
chaz_ConfWriterPerl_add_def(const char *sym, const char *value);
static void
chaz_ConfWriterPerl_add_global_def(const char *sym, const char *value);
static void
chaz_ConfWriterPerl_add_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterPerl_add_global_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterPerl_add_sys_include(const char *header);
static void
chaz_ConfWriterPerl_add_local_include(const char *header);
static void
chaz_ConfWriterPerl_start_module(const char *module_name);
static void
chaz_ConfWriterPerl_end_module(void);

void
chaz_ConfWriterPerl_enable(void) {
    CWPerl_conf_writer.clean_up           = chaz_ConfWriterPerl_clean_up;
    CWPerl_conf_writer.vappend_conf       = chaz_ConfWriterPerl_vappend_conf;
    CWPerl_conf_writer.add_def            = chaz_ConfWriterPerl_add_def;
    CWPerl_conf_writer.add_global_def     = chaz_ConfWriterPerl_add_global_def;
    CWPerl_conf_writer.add_typedef        = chaz_ConfWriterPerl_add_typedef;
    CWPerl_conf_writer.add_global_typedef = chaz_ConfWriterPerl_add_global_typedef;
    CWPerl_conf_writer.add_sys_include    = chaz_ConfWriterPerl_add_sys_include;
    CWPerl_conf_writer.add_local_include  = chaz_ConfWriterPerl_add_local_include;
    CWPerl_conf_writer.start_module       = chaz_ConfWriterPerl_start_module;
    CWPerl_conf_writer.end_module         = chaz_ConfWriterPerl_end_module;
    chaz_ConfWriterPerl_open_config_pm();
    chaz_ConfWriter_add_writer(&CWPerl_conf_writer);
    return;
}

static void
chaz_ConfWriterPerl_open_config_pm(void) {
    /* Open the filehandle. */
    chaz_CWPerl.fh = fopen("Charmony.pm", "w+");
    if (chaz_CWPerl.fh == NULL) {
        chaz_Util_die("Can't open 'Charmony.pm': %s", strerror(errno));
    }

    /* Start the module. */
    fprintf(chaz_CWPerl.fh,
            "# Auto-generated by Charmonizer. \n"
            "# DO NOT EDIT THIS FILE!!\n"
            "\n"
            "package Charmony;\n"
            "use strict;\n"
            "use warnings;\n"
            "\n"
            "my %%defs;\n"
            "\n"
            "sub config { \\%%defs }\n"
            "\n"
           );
}

static void
chaz_ConfWriterPerl_clean_up(void) {
    /* Write the last bit of Charmony.pm and close. */
    fprintf(chaz_CWPerl.fh, "\n1;\n\n");
    if (fclose(chaz_CWPerl.fh)) {
        chaz_Util_die("Couldn't close 'Charmony.pm': %s", strerror(errno));
    }
}

static void
chaz_ConfWriterPerl_vappend_conf(const char *fmt, va_list args) {
    (void)fmt;
    (void)args;
}

static char*
chaz_ConfWriterPerl_quotify(const char *string, char *buf, size_t buf_size) {
    char *quoted = buf;

    /* Don't bother with undef values here. */
    if (!string) {
        return NULL;
    }

    /* Allocate memory if necessary. */
    {
        const char *ptr;
        size_t space = 3; /* Quotes plus NUL termination. */
        for (ptr = string; *ptr; ptr++) {
            if (*ptr == '\'' || *ptr == '\\') {
                space += 2;
            }
            else {
                space += 1;
            }
        }
        if (space > buf_size) {
            quoted = (char*)malloc(space);
        }
    }

    /* Perform copying and escaping */
    {
        const char *ptr;
        size_t pos = 0;
        quoted[pos++] = '\'';
        for (ptr = string; *ptr; ptr++) {
            if (*ptr == '\'' || *ptr == '\\') {
                quoted[pos++] = '\\';
            }
            quoted[pos++] = *ptr;
        }
        quoted[pos++] = '\'';
        quoted[pos++] = '\0';
    }

    return quoted;
}

#define CFPERL_MAX_BUF 100
static void
chaz_ConfWriterPerl_add_def(const char *sym, const char *value) {
    char sym_buf[CFPERL_MAX_BUF + 1];
    char value_buf[CFPERL_MAX_BUF + 1];
    char *quoted_sym;
    char *quoted_value;

    /* Quote key. */
    if (!sym) {
        chaz_Util_die("Can't handle NULL key");
    }
    quoted_sym = chaz_ConfWriterPerl_quotify(sym, sym_buf, CFPERL_MAX_BUF);

    /* Quote value or use "undef". */
    if (!value) {
        strcpy(value_buf, "undef");
        quoted_value = value_buf;
    }
    else {
        quoted_value = chaz_ConfWriterPerl_quotify(value, value_buf,
                                                CFPERL_MAX_BUF);
    }

    fprintf(chaz_CWPerl.fh, "$defs{%s} = %s;\n", quoted_sym, quoted_value);

    if (quoted_sym   != sym_buf)   { free(quoted_sym);   }
    if (quoted_value != value_buf) { free(quoted_value); }
}

static void
chaz_ConfWriterPerl_add_global_def(const char *sym, const char *value) {
    (void)sym;
    (void)value;
}

static void
chaz_ConfWriterPerl_add_typedef(const char *type, const char *alias) {
    (void)type;
    (void)alias;
}

static void
chaz_ConfWriterPerl_add_global_typedef(const char *type, const char *alias) {
    (void)type;
    (void)alias;
}

static void
chaz_ConfWriterPerl_add_sys_include(const char *header) {
    (void)header;
}

static void
chaz_ConfWriterPerl_add_local_include(const char *header) {
    (void)header;
}

static void
chaz_ConfWriterPerl_start_module(const char *module_name) {
    fprintf(chaz_CWPerl.fh, "# %s\n", module_name);
}

static void
chaz_ConfWriterPerl_end_module(void) {
    fprintf(chaz_CWPerl.fh, "\n");
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/ConfWriterRuby.c"
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/ConfWriterRuby.h" */
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Static vars. */
static struct {
    FILE *fh;
} chaz_CWRuby = { NULL };
static chaz_ConfWriter CWRuby_conf_writer;

/* Open the Charmony.rb file handle.
 */
static void
chaz_ConfWriterRuby_open_config_rb(void);

static void
chaz_ConfWriterRuby_clean_up(void);
static void
chaz_ConfWriterRuby_vappend_conf(const char *fmt, va_list args);
static void
chaz_ConfWriterRuby_add_def(const char *sym, const char *value);
static void
chaz_ConfWriterRuby_add_global_def(const char *sym, const char *value);
static void
chaz_ConfWriterRuby_add_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterRuby_add_global_typedef(const char *type, const char *alias);
static void
chaz_ConfWriterRuby_add_sys_include(const char *header);
static void
chaz_ConfWriterRuby_add_local_include(const char *header);
static void
chaz_ConfWriterRuby_start_module(const char *module_name);
static void
chaz_ConfWriterRuby_end_module(void);

void
chaz_ConfWriterRuby_enable(void) {
    CWRuby_conf_writer.clean_up           = chaz_ConfWriterRuby_clean_up;
    CWRuby_conf_writer.vappend_conf       = chaz_ConfWriterRuby_vappend_conf;
    CWRuby_conf_writer.add_def            = chaz_ConfWriterRuby_add_def;
    CWRuby_conf_writer.add_global_def     = chaz_ConfWriterRuby_add_global_def;
    CWRuby_conf_writer.add_typedef        = chaz_ConfWriterRuby_add_typedef;
    CWRuby_conf_writer.add_global_typedef = chaz_ConfWriterRuby_add_global_typedef;
    CWRuby_conf_writer.add_sys_include    = chaz_ConfWriterRuby_add_sys_include;
    CWRuby_conf_writer.add_local_include  = chaz_ConfWriterRuby_add_local_include;
    CWRuby_conf_writer.start_module       = chaz_ConfWriterRuby_start_module;
    CWRuby_conf_writer.end_module         = chaz_ConfWriterRuby_end_module;
    chaz_ConfWriterRuby_open_config_rb();
    chaz_ConfWriter_add_writer(&CWRuby_conf_writer);
    return;
}

static void
chaz_ConfWriterRuby_open_config_rb(void) {
    /* Open the filehandle. */
    chaz_CWRuby.fh = fopen("Charmony.rb", "w+");
    if (chaz_CWRuby.fh == NULL) {
        chaz_Util_die("Can't open 'Charmony.rb': %s", strerror(errno));
    }

    /* Start the module. */
    fprintf(chaz_CWRuby.fh,
            "# Auto-generated by Charmonizer. \n"
            "# DO NOT EDIT THIS FILE!!\n"
            "\n"
            "module Charmony\n"
            "\n"
            "defs = {}\n"
            "\n"
            "def config\ndefs\nend\n"
            "\n"
           );
}

static void
chaz_ConfWriterRuby_clean_up(void) {
    /* Write the last bit of Charmony.rb and close. */
    fprintf(chaz_CWRuby.fh, "\nend\n\n");
    if (fclose(chaz_CWRuby.fh)) {
        chaz_Util_die("Couldn't close 'Charmony.rb': %s", strerror(errno));
    }
}

static void
chaz_ConfWriterRuby_vappend_conf(const char *fmt, va_list args) {
    (void)fmt;
    (void)args;
}

static char*
chaz_ConfWriterRuby_quotify(const char *string, char *buf, size_t buf_size) {
    char *quoted = buf;

    /* Don't bother with undef values here. */
    if (!string) {
        return NULL;
    }

    /* Allocate memory if necessary. */
    {
        const char *ptr;
        size_t space = 3; /* Quotes plus NUL termination. */
        for (ptr = string; *ptr; ptr++) {
            if (*ptr == '\'' || *ptr == '\\') {
                space += 2;
            }
            else {
                space += 1;
            }
        }
        if (space > buf_size) {
            quoted = (char*)malloc(space);
        }
    }

    /* Perform copying and escaping */
    {
        const char *ptr;
        size_t pos = 0;
        quoted[pos++] = '\'';
        for (ptr = string; *ptr; ptr++) {
            if (*ptr == '\'' || *ptr == '\\') {
                quoted[pos++] = '\\';
            }
            quoted[pos++] = *ptr;
        }
        quoted[pos++] = '\'';
        quoted[pos++] = '\0';
    }

    return quoted;
}

#define CFRUBY_MAX_BUF 100
static void
chaz_ConfWriterRuby_add_def(const char *sym, const char *value) {
    char sym_buf[CFRUBY_MAX_BUF + 1];
    char value_buf[CFRUBY_MAX_BUF + 1];
    char *quoted_sym;
    char *quoted_value;

    /* Quote key. */
    if (!sym) {
        chaz_Util_die("Can't handle NULL key");
    }
    quoted_sym = chaz_ConfWriterRuby_quotify(sym, sym_buf, CFRUBY_MAX_BUF);

    /* Quote value or use "nil". */
    if (!value) {
        strcpy(value_buf, "nil");
        quoted_value = value_buf;
    }
    else {
        quoted_value = chaz_ConfWriterRuby_quotify(value, value_buf,
                                                CFRUBY_MAX_BUF);
    }

    fprintf(chaz_CWRuby.fh, "defs[%s] = %s\n", quoted_sym, quoted_value);

    if (quoted_sym   != sym_buf)   { free(quoted_sym);   }
    if (quoted_value != value_buf) { free(quoted_value); }
}

static void
chaz_ConfWriterRuby_add_global_def(const char *sym, const char *value) {
    (void)sym;
    (void)value;
}

static void
chaz_ConfWriterRuby_add_typedef(const char *type, const char *alias) {
    (void)type;
    (void)alias;
}

static void
chaz_ConfWriterRuby_add_global_typedef(const char *type, const char *alias) {
    (void)type;
    (void)alias;
}

static void
chaz_ConfWriterRuby_add_sys_include(const char *header) {
    (void)header;
}

static void
chaz_ConfWriterRuby_add_local_include(const char *header) {
    (void)header;
}

static void
chaz_ConfWriterRuby_start_module(const char *module_name) {
    fprintf(chaz_CWRuby.fh, "# %s\n", module_name);
}

static void
chaz_ConfWriterRuby_end_module(void) {
    fprintf(chaz_CWRuby.fh, "\n");
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/HeaderChecker.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
#include <string.h>
#include <stdlib.h>

typedef struct chaz_CHeader {
    const char  *name;
    int          exists;
} chaz_CHeader;

/* Keep a sorted, dynamically-sized array of names of all headers we've
 * checked for so far.
 */
static struct {
    int            cache_size;
    chaz_CHeader **header_cache;
} chaz_HeadCheck = { 0, NULL };

/* Comparison function to feed to qsort, bsearch, etc.
 */
static int
chaz_HeadCheck_compare_headers(const void *vptr_a, const void *vptr_b);

/* Run a test compilation and return a new chaz_CHeader object encapsulating
 * the results.
 */
static chaz_CHeader*
chaz_HeadCheck_discover_header(const char *header_name);

/* Extend the cache, add this chaz_CHeader object to it, and sort.
 */
static void
chaz_HeadCheck_add_to_cache(chaz_CHeader *header);

/* Like add_to_cache, but takes a individual elements instead of a
 * chaz_CHeader* and checks if header exists in array first.
 */
static void
chaz_HeadCheck_maybe_add_to_cache(const char *header_name, int exists);

void
chaz_HeadCheck_init(void) {
    chaz_CHeader *null_header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));

    /* Create terminating record for the dynamic array of chaz_CHeader
     * objects. */
    null_header->name   = NULL;
    null_header->exists = false;
    chaz_HeadCheck.header_cache    = (chaz_CHeader**)malloc(sizeof(void*));
    *(chaz_HeadCheck.header_cache) = null_header;
    chaz_HeadCheck.cache_size = 1;
}

int
chaz_HeadCheck_check_header(const char *header_name) {
    chaz_CHeader  *header;
    chaz_CHeader   key;
    chaz_CHeader  *fake = &key;
    chaz_CHeader **header_ptr;

    /* Fake up a key to feed to bsearch; see if the header's already there. */
    key.name = header_name;
    key.exists = false;
    header_ptr = (chaz_CHeader**)bsearch(&fake, chaz_HeadCheck.header_cache,
                                         chaz_HeadCheck.cache_size,
                                         sizeof(void*),
                                         chaz_HeadCheck_compare_headers);

    /* If it's not there, go try a test compile. */
    if (header_ptr == NULL) {
        header = chaz_HeadCheck_discover_header(header_name);
        chaz_HeadCheck_add_to_cache(header);
    }
    else {
        header = *header_ptr;
    }

    return header->exists;
}

int
chaz_HeadCheck_check_many_headers(const char **header_names) {
    static const char test_code[] = "int main() { return 0; }\n";
    int success;
    int i;
    char *code_buf = chaz_Util_strdup("");
    size_t needed = sizeof(test_code) + 20;

    /* Build the source code string. */
    for (i = 0; header_names[i] != NULL; i++) {
        needed += strlen(header_names[i]);
        needed += sizeof("#include <>\n");
    }
    code_buf = (char*)malloc(needed);
    code_buf[0] = '\0';
    for (i = 0; header_names[i] != NULL; i++) {
        strcat(code_buf, "#include <");
        strcat(code_buf, header_names[i]);
        strcat(code_buf, ">\n");
    }
    strcat(code_buf, test_code);

    /* If the code compiles, bulk add all header names to the cache. */
    success = chaz_CC_test_compile(code_buf);
    if (success) {
        for (i = 0; header_names[i] != NULL; i++) {
            chaz_HeadCheck_maybe_add_to_cache(header_names[i], true);
        }
    }

    free(code_buf);
    return success;
}

int
chaz_HeadCheck_contains_member(const char *struct_name, const char *member,
                               const char *includes) {
    static const char contains_code[] =
        CHAZ_QUOTE(  #include <stddef.h>                           )
        CHAZ_QUOTE(  %s                                            )
        CHAZ_QUOTE(  int main() { return offsetof(%s, %s); }       );
    long needed = sizeof(contains_code)
                  + strlen(struct_name)
                  + strlen(member)
                  + strlen(includes)
                  + 10;
    char *buf = (char*)malloc(needed);
    int retval;
    sprintf(buf, contains_code, includes, struct_name, member);
    retval = chaz_CC_test_compile(buf);
    free(buf);
    return retval;
}

static int
chaz_HeadCheck_compare_headers(const void *vptr_a, const void *vptr_b) {
    chaz_CHeader *const *const a = (chaz_CHeader*const*)vptr_a;
    chaz_CHeader *const *const b = (chaz_CHeader*const*)vptr_b;

    /* (NULL is "greater than" any string.) */
    if ((*a)->name == NULL)      { return 1; }
    else if ((*b)->name == NULL) { return -1; }
    else                         { return strcmp((*a)->name, (*b)->name); }
}

static chaz_CHeader*
chaz_HeadCheck_discover_header(const char *header_name) {
    static const char test_code[] = "int main() { return 0; }\n";
    chaz_CHeader* header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));
    size_t  needed = strlen(header_name) + sizeof(test_code) + 50;
    char *include_test = (char*)malloc(needed);

    /* Assign. */
    header->name = chaz_Util_strdup(header_name);

    /* See whether code that tries to pull in this header compiles. */
    sprintf(include_test, "#include <%s>\n%s", header_name, test_code);
    header->exists = chaz_CC_test_compile(include_test);

    free(include_test);
    return header;
}

static void
chaz_HeadCheck_add_to_cache(chaz_CHeader *header) {
    size_t amount;

    /* Realloc array -- inefficient, but this isn't a bottleneck. */
    amount = ++chaz_HeadCheck.cache_size * sizeof(void*);
    chaz_HeadCheck.header_cache
        = (chaz_CHeader**)realloc(chaz_HeadCheck.header_cache, amount);
    chaz_HeadCheck.header_cache[chaz_HeadCheck.cache_size - 1] = header;

    /* Keep the list of headers sorted. */
    qsort(chaz_HeadCheck.header_cache, chaz_HeadCheck.cache_size,
          sizeof(*(chaz_HeadCheck.header_cache)),
          chaz_HeadCheck_compare_headers);
}

static void
chaz_HeadCheck_maybe_add_to_cache(const char *header_name, int exists) {
    chaz_CHeader *header;
    chaz_CHeader  key;
    chaz_CHeader *fake = &key;

    /* Fake up a key and bsearch for it. */
    key.name   = header_name;
    key.exists = exists;
    header = (chaz_CHeader*)bsearch(&fake, chaz_HeadCheck.header_cache,
                                    chaz_HeadCheck.cache_size, sizeof(void*),
                                    chaz_HeadCheck_compare_headers);

    /* We've already done the test compile, so skip that step and add it. */
    if (header == NULL) {
        header = (chaz_CHeader*)malloc(sizeof(chaz_CHeader));
        header->name   = chaz_Util_strdup(header_name);
        header->exists = exists;
        chaz_HeadCheck_add_to_cache(header);
    }
}



/***************************************************************************/

#line 17 "src/Charmonizer/Core/OperatingSystem.c"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

static struct {
    char dev_null[20];
    char exe_ext[5];
    char obj_ext[5];
    char local_command_start[3];
    int  shell_type;
} chaz_OS = { "", "", "", "", 0 };
#define CHAZ_OS_POSIX    1
#define CHAZ_OS_CMD_EXE  2

void
chaz_OS_init(void) {
    if (chaz_Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    if (chaz_Util_verbosity) {
        printf("Trying to find a bit-bucket a la /dev/null...\n");
    }

    /* Detect shell based on whether the bitbucket is "/dev/null" or "nul". */
    if (chaz_Util_can_open_file("/dev/null")) {
        strcpy(chaz_OS.dev_null, "/dev/null");
        strcpy(chaz_OS.exe_ext, "");
        strcpy(chaz_OS.obj_ext, "");
        strcpy(chaz_OS.local_command_start, "./");
        chaz_OS.shell_type = CHAZ_OS_POSIX;
    }
    else if (chaz_Util_can_open_file("nul")) {
        strcpy(chaz_OS.dev_null, "nul");
        strcpy(chaz_OS.exe_ext, ".exe");
        strcpy(chaz_OS.obj_ext, ".obj");
        strcpy(chaz_OS.local_command_start, ".\\");
        chaz_OS.shell_type = CHAZ_OS_CMD_EXE;
    }
    else {
        /* Bail out because we couldn't find anything like /dev/null. */
        chaz_Util_die("Couldn't find anything like /dev/null");
    }
}

const char*
chaz_OS_exe_ext(void) {
    return chaz_OS.exe_ext;
}

const char*
chaz_OS_obj_ext(void) {
    return chaz_OS.obj_ext;
}

const char*
chaz_OS_dev_null(void) {
    return chaz_OS.dev_null;
}

int
chaz_OS_remove(const char *name) {
    /*
     * On Windows it can happen that another process, typically a
     * virus scanner, still has an open handle on the file. This can
     * make the subsequent recreation of a file with the same name
     * fail. As a workaround, files are renamed to a random name
     * before deletion.
     */
    int retval;

    static const size_t num_random_chars = 16;

    size_t  name_len = strlen(name);
    size_t  i;
    char   *temp_name = (char*)malloc(name_len + num_random_chars + 1);

    strcpy(temp_name, name);
    for (i = 0; i < num_random_chars; i++) {
        temp_name[name_len+i] = 'A' + rand() % 26;
    }
    temp_name[name_len+num_random_chars] = '\0';

    if (rename(name, temp_name) == 0) {
        retval = !remove(temp_name);
    }
    else {
        // Error during rename, remove using old name.
        retval = !remove(name);
    }

    free(temp_name);
    return retval;
}

int
chaz_OS_run_local(const char *arg1, ...) {
    va_list  args;
    size_t   len     = strlen(chaz_OS.local_command_start) + strlen(arg1);
    char    *command = (char*)malloc(len + 1);
    int      retval;
    char    *arg;

    /* Append all supplied texts. */
    sprintf(command, "%s%s", chaz_OS.local_command_start, arg1);
    va_start(args, arg1);
    while (NULL != (arg = va_arg(args, char*))) {
        len += strlen(arg);
        command = (char*)realloc(command, len + 1);
        strcat(command, arg);
    }
    va_end(args);

    /* Run the command. */
    retval = system(command);
    free(command);
    return retval;
}

int
chaz_OS_run_quietly(const char *command) {
    int retval = 1;
    char *quiet_command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX) {
        char pattern[] = "%s > %s 2>&1";
        size_t size = sizeof(pattern)
                      + strlen(command)
                      + strlen(chaz_OS.dev_null)
                      + 10;
        quiet_command = (char*)malloc(size);
        sprintf(quiet_command, pattern, command, chaz_OS.dev_null);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        char pattern[] = "%s > NUL 2> NUL";
        size_t size = sizeof(pattern) + strlen(command) + 10;
        quiet_command = (char*)malloc(size);
        sprintf(quiet_command, pattern, command);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    retval = system(quiet_command);
    free(quiet_command);

    return retval;
}

void
chaz_OS_mkdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
       ) {
        unsigned size = sizeof("mkdir") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "mkdir %s", filepath);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

void
chaz_OS_rmdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX) {
        unsigned size = strlen("rmdir") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "rmdir %s", filepath);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        unsigned size = strlen("rmdir /q") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "rmdir /q %s", filepath);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/Util.c"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* #include "Charmonizer/Core/Util.h" */

/* Global verbosity setting. */
int chaz_Util_verbosity = 1;

void
chaz_Util_write_file(const char *filename, const char *content) {
    FILE *fh = fopen(filename, "w+");
    size_t content_len = strlen(content);
    if (fh == NULL) {
        chaz_Util_die("Couldn't open '%s': %s", filename, strerror(errno));
    }
    fwrite(content, sizeof(char), content_len, fh);
    if (fclose(fh)) {
        chaz_Util_die("Error when closing '%s': %s", filename,
                      strerror(errno));
    }
}

char*
chaz_Util_slurp_file(const char *file_path, size_t *len_ptr) {
    FILE   *const file = fopen(file_path, "r");
    char   *contents;
    size_t  len;
    long    check_val;

    /* Sanity check. */
    if (file == NULL) {
        chaz_Util_die("Error opening file '%s': %s", file_path,
                      strerror(errno));
    }

    /* Find length; return NULL if the file has a zero-length. */
    len = chaz_Util_flength(file);
    if (len == 0) {
        *len_ptr = 0;
        return NULL;
    }

    /* Allocate memory and read the file. */
    contents = (char*)malloc(len * sizeof(char) + 1);
    if (contents == NULL) {
        chaz_Util_die("Out of memory at %d, %s", __FILE__, __LINE__);
    }
    contents[len] = '\0';
    check_val = fread(contents, sizeof(char), len, file);

    /* Weak error check, because CRLF might result in fewer chars read. */
    if (check_val <= 0) {
        chaz_Util_die("Tried to read %d characters of '%s', got %d", (int)len,
                      file_path, check_val);
    }

    /* Set length pointer for benefit of caller. */
    *len_ptr = check_val;

    /* Clean up. */
    if (fclose(file)) {
        chaz_Util_die("Error closing file '%s': %s", file_path,
                      strerror(errno));
    }

    return contents;
}

long
chaz_Util_flength(void *file) {
    FILE *f = (FILE*)file;
    const long bookmark = ftell(f);
    long check_val;
    long len;

    /* Seek to end of file and check length. */
    check_val = fseek(f, 0, SEEK_END);
    if (check_val == -1) {
        chaz_Util_die("fseek error : %s\n", strerror(errno));
    }
    len = ftell(f);
    if (len == -1) { chaz_Util_die("ftell error : %s\n", strerror(errno)); }

    /* Return to where we were. */
    check_val = fseek(f, bookmark, SEEK_SET);
    if (check_val == -1) {
        chaz_Util_die("fseek error : %s\n", strerror(errno));
    }

    return len;
}

char*
chaz_Util_strdup(const char *string) {
    size_t len = strlen(string);
    char *copy = (char*)malloc(len + 1);
    strncpy(copy, string, len);
    copy[len] = '\0';
    return copy;
}

void
chaz_Util_die(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(1);
}

void
chaz_Util_warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
}

int
chaz_Util_remove_and_verify(const char *file_path) {
    /* Attempt to delete the file.  If it's gone after the attempt, return
     * success, whether or not it was there to begin with.
     * (ENOENT is POSIX not C89, but let's go with it for now.) */
    int result = chaz_OS_remove(file_path);
    if (result || errno == ENOENT) {
        return 1;
    }

    /* Issue a warning and return failure. */
    chaz_Util_warn("Failed to remove '%s': %s at %s line %d",
                   file_path, strerror(errno), __FILE__, __LINE__);
    return 0;
}

int
chaz_Util_can_open_file(const char *file_path) {
    FILE *garbage_fh;

    /* Use fopen as a portable test for the existence of a file. */
    garbage_fh = fopen(file_path, "r");
    if (garbage_fh == NULL) {
        return 0;
    }
    else {
        fclose(garbage_fh);
        return 1;
    }
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include "Charmonizer/Probe.h" */
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

/* Write the "_charm.h" file used by every probe.
 */
static void
chaz_Probe_write_charm_h(void);

static void
chaz_Probe_remove_charm_h(void);

void
chaz_Probe_init(const char *cc_command, const char *cc_flags) {
    /* Proces CHARM_VERBOSITY environment variable. */
    const char *verbosity_env = getenv("CHARM_VERBOSITY");
    if (verbosity_env && strlen(verbosity_env)) {
        chaz_Util_verbosity = strtol(verbosity_env, NULL, 10);
    }

    /* Dispatch other initializers. */
    chaz_OS_init();
    chaz_CC_init(cc_command, cc_flags);
    chaz_ConfWriter_init();
    chaz_HeadCheck_init();
    chaz_Probe_write_charm_h();

    if (chaz_Util_verbosity) { printf("Initialization complete.\n"); }
}

void
chaz_Probe_clean_up(void) {
    if (chaz_Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch various clean up routines. */
    chaz_Probe_remove_charm_h();
    chaz_ConfWriter_clean_up();
    chaz_CC_clean_up();

    if (chaz_Util_verbosity) { printf("Cleanup complete.\n"); }
}

static void
chaz_Probe_write_charm_h(void) {
    static const char charm_h_code[] =
        CHAZ_QUOTE(  #ifndef CHARM_H                                                  )
        CHAZ_QUOTE(  #define CHARM_H 1                                                )
        CHAZ_QUOTE(  #include <stdio.h>                                               )
        CHAZ_QUOTE(  #define Charm_Setup freopen("_charmonizer_target", "w", stdout)  )
        CHAZ_QUOTE(  #endif                                                           );
    chaz_Util_write_file("_charm.h", charm_h_code);
}

static void
chaz_Probe_remove_charm_h(void) {
    chaz_Util_remove_and_verify("_charm.h");
}


/***************************************************************************/

#line 17 "src/Charmonizer/Probe/AtomicOps.c"
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/AtomicOps.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static int
chaz_AtomicOps_osatomic_cas_ptr(void) {
    static const char osatomic_casptr_code[] =
        CHAZ_QUOTE(  #include <libkern/OSAtomic.h>                                  )
        CHAZ_QUOTE(  #include <libkern/OSAtomic.h>                                  )
        CHAZ_QUOTE(  int main() {                                                   )
        CHAZ_QUOTE(      int  foo = 1;                                              )
        CHAZ_QUOTE(      int *foo_ptr = &foo;                                       )
        CHAZ_QUOTE(      int *target = NULL;                                        )
        CHAZ_QUOTE(      OSAtomicCompareAndSwapPtr(NULL, foo_ptr, (void**)&target); )
        CHAZ_QUOTE(      return 0;                                                  )
        CHAZ_QUOTE(  }                                                              );
     return chaz_CC_test_compile(osatomic_casptr_code);
}

void
chaz_AtomicOps_run(void) {
    chaz_ConfWriter_start_module("AtomicOps");

    if (chaz_HeadCheck_check_header("libkern/OSAtomic.h")) {
        chaz_ConfWriter_add_def("HAS_LIBKERN_OSATOMIC_H", NULL);

        /* Check for OSAtomicCompareAndSwapPtr, introduced in later versions
         * of OSAtomic.h. */
        if (chaz_AtomicOps_osatomic_cas_ptr()) {
            chaz_ConfWriter_add_def("HAS_OSATOMIC_CAS_PTR", NULL);
        }
    }
    if (chaz_HeadCheck_check_header("sys/atomic.h")) {
        chaz_ConfWriter_add_def("HAS_SYS_ATOMIC_H", NULL);
    }
    if (chaz_HeadCheck_check_header("windows.h")
        && chaz_HeadCheck_check_header("intrin.h")
       ) {
        chaz_ConfWriter_add_def("HAS_INTRIN_H", NULL);
    }

    chaz_ConfWriter_end_module();
}




/***************************************************************************/

#line 17 "src/Charmonizer/Probe/DirManip.c"
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Probe/DirManip.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static struct {
    int  mkdir_num_args;
    char mkdir_command[7];
} chaz_DirManip = { 0, "" };

/* Source code for rmdir. */
static int
chaz_DirManip_compile_posix_mkdir(const char *header) {
    static const char posix_mkdir_code[] =
        CHAZ_QUOTE(  #include <%s>                                      )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (mkdir(argv[1], 0777) != 0) { return 2; }   )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    char code_buf[sizeof(posix_mkdir_code) + 30];
    int mkdir_available;
    if (strlen(header) > 25) {
        chaz_Util_die("Header name too long: '%s'", header);
    }

    /* Attempt compilation. */
    sprintf(code_buf, posix_mkdir_code, header);
    mkdir_available = chaz_CC_test_compile(code_buf);

    /* Set vars on success. */
    if (mkdir_available) {
        strcpy(chaz_DirManip.mkdir_command, "mkdir");
        if (strcmp(header, "direct.h") == 0) {
            chaz_DirManip.mkdir_num_args = 1;
        }
        else {
            chaz_DirManip.mkdir_num_args = 2;
        }
    }

    return mkdir_available;
}

static int
chaz_DirManip_compile_win_mkdir(void) {
    static const char win_mkdir_code[] =
        CHAZ_QUOTE(  #include <direct.h>                                )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (_mkdir(argv[1]) != 0) { return 2; }        )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    int mkdir_available;

    mkdir_available = chaz_CC_test_compile(win_mkdir_code);
    if (mkdir_available) {
        strcpy(chaz_DirManip.mkdir_command, "_mkdir");
        chaz_DirManip.mkdir_num_args = 1;
    }
    return mkdir_available;
}

static void
chaz_DirManip_try_mkdir(void) {
    if (chaz_HeadCheck_check_header("windows.h")) {
        if (chaz_DirManip_compile_win_mkdir())               { return; }
        if (chaz_DirManip_compile_posix_mkdir("direct.h"))   { return; }
    }
    if (chaz_DirManip_compile_posix_mkdir("sys/stat.h")) { return; }
}

static int
chaz_DirManip_compile_rmdir(const char *header) {
    static const char rmdir_code[] =
        CHAZ_QUOTE(  #include <%s>                                      )
        CHAZ_QUOTE(  int main(int argc, char **argv) {                  )
        CHAZ_QUOTE(      if (argc != 2) { return 1; }                   )
        CHAZ_QUOTE(      if (rmdir(argv[1]) != 0) { return 2; }         )
        CHAZ_QUOTE(      return 0;                                      )
        CHAZ_QUOTE(  }                                                  );
    char code_buf[sizeof(rmdir_code) + 30];
    int rmdir_available;
    if (strlen(header) > 25) {
        chaz_Util_die("Header name too long: '%s'", header);
    }
    sprintf(code_buf, rmdir_code, header);
    rmdir_available = chaz_CC_test_compile(code_buf);
    return rmdir_available;
}

static void
chaz_DirManip_try_rmdir(void) {
    if (chaz_DirManip_compile_rmdir("unistd.h"))   { return; }
    if (chaz_DirManip_compile_rmdir("dirent.h"))   { return; }
    if (chaz_DirManip_compile_rmdir("direct.h"))   { return; }
}

static int
chaz_DirManip_is_cygwin(void) {
    static int is_cygwin = -1;
    static const char cygwin_code[] =
        CHAZ_QUOTE(#ifndef __CYGWIN__            )
        CHAZ_QUOTE(  #error "Not Cygwin"         )
        CHAZ_QUOTE(#endif                        )
        CHAZ_QUOTE(int main() { return 0; }      );
    if (is_cygwin == -1) {
        is_cygwin = chaz_CC_test_compile(cygwin_code);
    }
    return is_cygwin;
}

void
chaz_DirManip_run(void) {
    char dir_sep[3];
    int remove_zaps_dirs = false;
    int has_dirent_h = chaz_HeadCheck_check_header("dirent.h");
    int has_direct_h = chaz_HeadCheck_check_header("direct.h");
    int has_dirent_d_namlen = false;
    int has_dirent_d_type   = false;

    chaz_ConfWriter_start_module("DirManip");
    chaz_DirManip_try_mkdir();
    chaz_DirManip_try_rmdir();

    /* Header checks. */
    if (has_dirent_h) {
        chaz_ConfWriter_add_def("HAS_DIRENT_H", NULL);
    }
    if (has_direct_h) {
        chaz_ConfWriter_add_def("HAS_DIRECT_H", NULL);
    }

    /* Check for members in struct dirent. */
    if (has_dirent_h) {
        has_dirent_d_namlen = chaz_HeadCheck_contains_member(
                                  "struct dirent", "d_namlen",
                                  "#include <sys/types.h>\n#include <dirent.h>"
                              );
        if (has_dirent_d_namlen) {
            chaz_ConfWriter_add_def("HAS_DIRENT_D_NAMLEN", NULL);
        }
        has_dirent_d_type = chaz_HeadCheck_contains_member(
                                "struct dirent", "d_type",
                                "#include <sys/types.h>\n#include <dirent.h>"
                            );
        if (has_dirent_d_type) {
            chaz_ConfWriter_add_def("HAS_DIRENT_D_TYPE", NULL);
        }
    }

    if (chaz_DirManip.mkdir_num_args == 2) {
        /* It's two args, but the command isn't "mkdir". */
        char scratch[50];
        if (strlen(chaz_DirManip.mkdir_command) > 30) {
            chaz_Util_die("Command too long: '%s'", chaz_DirManip.mkdir_command);
        }
        sprintf(scratch, "%s(_dir, _mode)", chaz_DirManip.mkdir_command);
        chaz_ConfWriter_add_def("makedir(_dir, _mode)", scratch);
        chaz_ConfWriter_add_def("MAKEDIR_MODE_IGNORED", "0");
    }
    else if (chaz_DirManip.mkdir_num_args == 1) {
        /* It's one arg... mode arg will be ignored. */
        char scratch[50];
        if (strlen(chaz_DirManip.mkdir_command) > 30) {
            chaz_Util_die("Command too long: '%s'", chaz_DirManip.mkdir_command);
        }
        sprintf(scratch, "%s(_dir)", chaz_DirManip.mkdir_command);
        chaz_ConfWriter_add_def("makedir(_dir, _mode)", scratch);
        chaz_ConfWriter_add_def("MAKEDIR_MODE_IGNORED", "1");
    }

    if (chaz_DirManip_is_cygwin()) {
        strcpy(dir_sep, "/");
    }
    else if (chaz_HeadCheck_check_header("windows.h")) {
        strcpy(dir_sep, "\\\\");
    }
    else {
        strcpy(dir_sep, "/");
    }

    {
        char scratch[5];
        sprintf(scratch, "\"%s\"", dir_sep);
        chaz_ConfWriter_add_def("DIR_SEP", scratch);
    }

    /* See whether remove works on directories. */
    chaz_OS_mkdir("_charm_test_remove_me");
    if (0 == remove("_charm_test_remove_me")) {
        remove_zaps_dirs = true;
        chaz_ConfWriter_add_def("REMOVE_ZAPS_DIRS", NULL);
    }
    chaz_OS_rmdir("_charm_test_remove_me");

    chaz_ConfWriter_end_module();
}




/***************************************************************************/

#line 17 "src/Charmonizer/Probe/Floats.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/Floats.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
chaz_Floats_run(void) {
    chaz_ConfWriter_start_module("Floats");

    chaz_ConfWriter_append_conf(
        "typedef union { uint32_t i; float f; } chy_floatu32;\n"
        "typedef union { uint64_t i; double d; } chy_floatu64;\n"
        "static const chy_floatu32 chy_f32inf    = {UINT32_C(0x7f800000)};\n"
        "static const chy_floatu32 chy_f32neginf = {UINT32_C(0xff800000)};\n"
        "static const chy_floatu32 chy_f32nan    = {UINT32_C(0x7fc00000)};\n"
        "static const chy_floatu64 chy_f64inf    = {UINT64_C(0x7ff0000000000000)};\n"
        "static const chy_floatu64 chy_f64neginf = {UINT64_C(0xfff0000000000000)};\n"
        "static const chy_floatu64 chy_f64nan    = {UINT64_C(0x7ff8000000000000)};\n"
    );
    chaz_ConfWriter_add_def("F32_INF", "(chy_f32inf.f)");
    chaz_ConfWriter_add_def("F32_NEGINF", "(chy_f32neginf.f)");
    chaz_ConfWriter_add_def("F32_NAN", "(chy_f32nan.f)");
    chaz_ConfWriter_add_def("F64_INF", "(chy_f64inf.d)");
    chaz_ConfWriter_add_def("F64_NEGINF", "(chy_f64neginf.d)");
    chaz_ConfWriter_add_def("F64_NAN", "(chy_f64nan.d)");

    chaz_ConfWriter_end_module();
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/FuncMacro.c"
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/FuncMacro.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Probe for ISO func macro. */
static int
chaz_FuncMacro_probe_iso() {
    static const char iso_func_code[] =
        CHAZ_QUOTE(  #include "_charm.h"               )
        CHAZ_QUOTE(  int main() {                      )
        CHAZ_QUOTE(      Charm_Setup;                  )
        CHAZ_QUOTE(      printf("%s", __func__);       )
        CHAZ_QUOTE(      return 0;                     )
        CHAZ_QUOTE(  }                                 );
    size_t output_len;
    char *output;
    int success = false;

    output = chaz_CC_capture_output(iso_func_code, &output_len);
    if (output != NULL && strncmp(output, "main", 4) == 0) {
        success = true;
    }
    free(output);

    return success;
}

static int
chaz_FuncMacro_probe_gnu() {
    /* Code for verifying GNU func macro. */
    static const char gnu_func_code[] =
        CHAZ_QUOTE(  #include "_charm.h"               )
        CHAZ_QUOTE(  int main() {                      )
        CHAZ_QUOTE(      Charm_Setup;                  )
        CHAZ_QUOTE(      printf("%s", __FUNCTION__);   )
        CHAZ_QUOTE(      return 0;                     )
        CHAZ_QUOTE(  }                                 );
    size_t output_len;
    char *output;
    int success = false;

    output = chaz_CC_capture_output(gnu_func_code, &output_len);
    if (output != NULL && strncmp(output, "main", 4) == 0) {
        success = true;
    }
    free(output);

    return success;
}

/* Attempt to verify inline keyword. */
static char*
chaz_FuncMacro_try_inline(const char *keyword, size_t *output_len) {
    static const char inline_code[] =
        CHAZ_QUOTE(  #include "_charm.h"               )
        CHAZ_QUOTE(  static %s int foo() { return 1; } )
        CHAZ_QUOTE(  int main() {                      )
        CHAZ_QUOTE(      Charm_Setup;                  )
        CHAZ_QUOTE(      printf("%%d", foo());         )
        CHAZ_QUOTE(      return 0;                     )
        CHAZ_QUOTE(  }                                 );
    char code[sizeof(inline_code) + 30];
    sprintf(code, inline_code, keyword);
    return chaz_CC_capture_output(code, output_len);
}

static void
chaz_FuncMacro_probe_inline(void) {
    static const char* inline_options[] = {
        "__inline",
        "__inline__",
        "inline"
    };
    const int num_inline_options = sizeof(inline_options) / sizeof(void*);
    int has_inline = false;
    int i;

    for (i = 0; i < num_inline_options; i++) {
        const char *inline_option = inline_options[i];
        size_t output_len;
        char *output = chaz_FuncMacro_try_inline(inline_option, &output_len);
        if (output != NULL) {
            has_inline = true;
            chaz_ConfWriter_add_def("INLINE", inline_option);
            free(output);
            break;
        }
    }
    if (!has_inline) {
        chaz_ConfWriter_add_def("INLINE", NULL);
    }
}

void
chaz_FuncMacro_run(void) {
    int i;
    char *output;
    size_t output_len;
    int has_funcmac      = false;
    int has_iso_funcmac  = false;
    int has_gnuc_funcmac = false;

    chaz_ConfWriter_start_module("FuncMacro");

    /* Check for func macros. */
    if (chaz_FuncMacro_probe_iso()) {
        has_funcmac     = true;
        has_iso_funcmac = true;
    }
    if (chaz_FuncMacro_probe_gnu()) {
        has_funcmac      = true;
        has_gnuc_funcmac = true;
    }

    /* Write out common defines. */
    if (has_funcmac) {
        const char *macro_text = has_iso_funcmac
                                 ? "__func__"
                                 : "__FUNCTION__";
        chaz_ConfWriter_add_def("HAS_FUNC_MACRO", NULL);
        chaz_ConfWriter_add_def("FUNC_MACRO", macro_text);
    }

    /* Write out specific defines. */
    if (has_iso_funcmac) {
        chaz_ConfWriter_add_def("HAS_ISO_FUNC_MACRO", NULL);
    }
    if (has_gnuc_funcmac) {
        chaz_ConfWriter_add_def("HAS_GNUC_FUNC_MACRO", NULL);
    }

    /* Check for inline keyword. */
    chaz_FuncMacro_probe_inline();

    chaz_ConfWriter_end_module();
}




/***************************************************************************/

#line 17 "src/Charmonizer/Probe/Headers.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/Headers.h" */
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#define CHAZ_HEADERS_MAX_KEEPERS 200

static struct {
    int keeper_count;
    const char *keepers[CHAZ_HEADERS_MAX_KEEPERS + 1];
} chaz_Headers = { 0, { NULL } };

/* Add a header to the keepers array.
 */
static void
chaz_Headers_keep(const char *header_name);

/* Transform "header.h" into "CHY_HAS_HEADER_H, storing the result into
 * `buffer`.
 */
static void
chaz_Headers_encode_affirmation(const char *header_name, char *buffer,
                                size_t buf_size);

/* Probe for all C89 headers. */
static void
chaz_Headers_probe_c89(void);

/* Probe for all POSIX headers. */
static void
chaz_Headers_probe_posix(void);

/* Prove for selected Windows headers. */
static void
chaz_Headers_probe_win(void);

int
chaz_Headers_check(const char *header_name) {
    return chaz_HeadCheck_check_header(header_name);
}

void
chaz_Headers_run(void) {
    int i;

    chaz_ConfWriter_start_module("Headers");

    chaz_Headers_probe_posix();
    chaz_Headers_probe_c89();
    chaz_Headers_probe_win();

    /* One-offs. */
    if (chaz_HeadCheck_check_header("pthread.h")) {
        chaz_Headers_keep("pthread.h");
    }

    /* Append the config with every header detected so far. */
    for (i = 0; chaz_Headers.keepers[i] != NULL; i++) {
        char aff_buf[200];
        chaz_Headers_encode_affirmation(chaz_Headers.keepers[i], aff_buf, 200);
        chaz_ConfWriter_add_def(aff_buf, NULL);
    }

    chaz_ConfWriter_end_module();
}

static void
chaz_Headers_keep(const char *header_name) {
    if (chaz_Headers.keeper_count >= CHAZ_HEADERS_MAX_KEEPERS) {
        chaz_Util_die("Too many keepers -- increase MAX_KEEPER_COUNT");
    }
    chaz_Headers.keepers[chaz_Headers.keeper_count++] = header_name;
    chaz_Headers.keepers[chaz_Headers.keeper_count]   = NULL;
}

static void
chaz_Headers_encode_affirmation(const char *header_name, char *buffer, size_t buf_size) {
    char *buf, *buf_end;
    size_t len = strlen(header_name) + sizeof("HAS_");
    if (len + 1 > buf_size) {
        chaz_Util_die("Buffer too small: %lu", (unsigned long)buf_size);
    }

    /* Start off with "HAS_". */
    strcpy(buffer, "HAS_");

    /* Transform one char at a time. */
    for (buf = buffer + sizeof("HAS_") - 1, buf_end = buffer + len;
         buf < buf_end;
         header_name++, buf++
        ) {
        if (*header_name == '\0') {
            *buf = '\0';
            break;
        }
        else if (isalnum(*header_name)) {
            *buf = toupper(*header_name);
        }
        else {
            *buf = '_';
        }
    }
}

static void
chaz_Headers_probe_c89(void) {
    const char *c89_headers[] = {
        "assert.h",
        "ctype.h",
        "errno.h",
        "float.h",
        "limits.h",
        "locale.h",
        "math.h",
        "setjmp.h",
        "signal.h",
        "stdarg.h",
        "stddef.h",
        "stdio.h",
        "stdlib.h",
        "string.h",
        "time.h",
        NULL
    };
    int i;

    /* Test for all c89 headers in one blast. */
    if (chaz_HeadCheck_check_many_headers((const char**)c89_headers)) {
        chaz_ConfWriter_add_def("HAS_C89", NULL);
        chaz_ConfWriter_add_def("HAS_C90", NULL);
        for (i = 0; c89_headers[i] != NULL; i++) {
            chaz_Headers_keep(c89_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; c89_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(c89_headers[i])) {
                chaz_Headers_keep(c89_headers[i]);
            }
        }
    }
}

static void
chaz_Headers_probe_posix(void) {
    const char *posix_headers[] = {
        "cpio.h",
        "dirent.h",
        "fcntl.h",
        "grp.h",
        "pwd.h",
        "sys/stat.h",
        "sys/times.h",
        "sys/types.h",
        "sys/utsname.h",
        "sys/wait.h",
        "tar.h",
        "termios.h",
        "unistd.h",
        "utime.h",
        NULL
    };
    int i;

    /* Try for all POSIX headers in one blast. */
    if (chaz_HeadCheck_check_many_headers((const char**)posix_headers)) {
        chaz_ConfWriter_add_def("HAS_POSIX", NULL);
        for (i = 0; posix_headers[i] != NULL; i++) {
            chaz_Headers_keep(posix_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; posix_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(posix_headers[i])) {
                chaz_Headers_keep(posix_headers[i]);
            }
        }
    }
}


static void
chaz_Headers_probe_win(void) {
    const char *win_headers[] = {
        "io.h",
        "windows.h",
        "process.h",
        NULL
    };
    int i;

    /* Test for all Windows headers in one blast */
    if (chaz_HeadCheck_check_many_headers((const char**)win_headers)) {
        for (i = 0; win_headers[i] != NULL; i++) {
            chaz_Headers_keep(win_headers[i]);
        }
    }
    /* Test one-at-a-time. */
    else {
        for (i = 0; win_headers[i] != NULL; i++) {
            if (chaz_HeadCheck_check_header(win_headers[i])) {
                chaz_Headers_keep(win_headers[i]);
            }
        }
    }
}


/***************************************************************************/

#line 17 "src/Charmonizer/Probe/Integers.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/Integers.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Determine endian-ness of this machine.
 */
static int
chaz_Integers_machine_is_big_endian(void);

static const char chaz_Integers_sizes_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main () {                             )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(char));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(short));    )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(int));      )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(long));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(void*));    )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_type64_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));       )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_literal64_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  #define big 9000000000000000000%s         )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      int truncated = (int)big;             )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      printf("%%d\n", truncated);           )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_u64_to_double_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                       )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      unsigned __int64 int_num = 0;         )
    CHAZ_QUOTE(      double float_num;                     )
    CHAZ_QUOTE(      Charm_Setup;                          )
    CHAZ_QUOTE(      float_num = (double)int_num;          )
    CHAZ_QUOTE(      printf("%%f\n", float_num);           )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

void
chaz_Integers_run(void) {
    char *output;
    size_t output_len;
    int sizeof_char       = -1;
    int sizeof_short      = -1;
    int sizeof_int        = -1;
    int sizeof_ptr        = -1;
    int sizeof_long       = -1;
    int sizeof_long_long  = -1;
    int sizeof___int64    = -1;
    int has_8             = false;
    int has_16            = false;
    int has_32            = false;
    int has_64            = false;
    int has_long_long     = false;
    int has___int64       = false;
    int has_inttypes      = chaz_HeadCheck_check_header("inttypes.h");
    int has_stdint        = chaz_HeadCheck_check_header("stdint.h");
    int can_convert_u64_to_double = true;
    char i32_t_type[10];
    char i32_t_postfix[10];
    char u32_t_postfix[10];
    char i64_t_type[10];
    char i64_t_postfix[10];
    char u64_t_postfix[10];
    char code_buf[1000];
    char scratch[50];

    chaz_ConfWriter_start_module("Integers");

    /* Document endian-ness. */
    if (chaz_Integers_machine_is_big_endian()) {
        chaz_ConfWriter_add_def("BIG_END", NULL);
    }
    else {
        chaz_ConfWriter_add_def("LITTLE_END", NULL);
    }

    /* Record sizeof() for several common integer types. */
    output = chaz_CC_capture_output(chaz_Integers_sizes_code, &output_len);
    if (output != NULL) {
        char *end_ptr = output;

        sizeof_char  = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_short = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_int   = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_long  = strtol(output, &end_ptr, 10);
        output       = end_ptr;
        sizeof_ptr   = strtol(output, &end_ptr, 10);
    }

    /* Determine whether long longs are available. */
    sprintf(code_buf, chaz_Integers_type64_code, "long long");
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        has_long_long    = true;
        sizeof_long_long = strtol(output, NULL, 10);
    }

    /* Determine whether the __int64 type is available. */
    sprintf(code_buf, chaz_Integers_type64_code, "__int64");
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        has___int64 = true;
        sizeof___int64 = strtol(output, NULL, 10);
    }

    /* Figure out which integer types are available. */
    if (sizeof_char == 1) {
        has_8 = true;
    }
    if (sizeof_short == 2) {
        has_16 = true;
    }
    if (sizeof_int == 4) {
        has_32 = true;
        strcpy(i32_t_type, "int");
        strcpy(i32_t_postfix, "");
        strcpy(u32_t_postfix, "U");
    }
    else if (sizeof_long == 4) {
        has_32 = true;
        strcpy(i32_t_type, "long");
        strcpy(i32_t_postfix, "L");
        strcpy(u32_t_postfix, "UL");
    }
    if (sizeof_long == 8) {
        has_64 = true;
        strcpy(i64_t_type, "long");
    }
    else if (sizeof_long_long == 8) {
        has_64 = true;
        strcpy(i64_t_type, "long long");
    }
    else if (sizeof___int64 == 8) {
        has_64 = true;
        strcpy(i64_t_type, "__int64");
    }

    /* Probe for 64-bit literal syntax. */
    if (has_64 && sizeof_long == 8) {
        strcpy(i64_t_postfix, "L");
        strcpy(u64_t_postfix, "UL");
    }
    else if (has_64) {
        sprintf(code_buf, chaz_Integers_literal64_code, "LL");
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            strcpy(i64_t_postfix, "LL");
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "i64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(i64_t_postfix, "i64");
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
        }
        sprintf(code_buf, chaz_Integers_literal64_code, "ULL");
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            strcpy(u64_t_postfix, "ULL");
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "Ui64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(u64_t_postfix, "Ui64");
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
        }
    }

    /* Determine whether conversion of unsigned __int64 to double works */
    if (has___int64) {
        if (!chaz_CC_test_compile(chaz_Integers_u64_to_double_code)) {
            can_convert_u64_to_double = false;
        }
    }

    /* Write out some conditional defines. */
    if (has_inttypes) {
        chaz_ConfWriter_add_def("HAS_INTTYPES_H", NULL);
    }
    if (has_stdint) {
        chaz_ConfWriter_add_def("HAS_STDINT_H", NULL);
    }
    if (has_long_long) {
        chaz_ConfWriter_add_def("HAS_LONG_LONG", NULL);
    }
    if (has___int64) {
        chaz_ConfWriter_add_def("HAS___INT64", NULL);
    }

    /* Write out sizes. */
    sprintf(scratch, "%d", sizeof_char);
    chaz_ConfWriter_add_def("SIZEOF_CHAR", scratch);
    sprintf(scratch, "%d", sizeof_short);
    chaz_ConfWriter_add_def("SIZEOF_SHORT", scratch);
    sprintf(scratch, "%d", sizeof_int);
    chaz_ConfWriter_add_def("SIZEOF_INT", scratch);
    sprintf(scratch, "%d", sizeof_long);
    chaz_ConfWriter_add_def("SIZEOF_LONG", scratch);
    sprintf(scratch, "%d", sizeof_ptr);
    chaz_ConfWriter_add_def("SIZEOF_PTR", scratch);
    if (has_long_long) {
        sprintf(scratch, "%d", sizeof_long_long);
        chaz_ConfWriter_add_def("SIZEOF_LONG_LONG", scratch);
    }
    if (has___int64) {
        sprintf(scratch, "%d", sizeof___int64);
        chaz_ConfWriter_add_def("SIZEOF___INT64", scratch);
    }

    /* Write affirmations. */
    if (has_8) {
        chaz_ConfWriter_add_def("HAS_INT8_T", NULL);
    }
    if (has_16) {
        chaz_ConfWriter_add_def("HAS_INT16_T", NULL);
    }
    if (has_32) {
        chaz_ConfWriter_add_def("HAS_INT32_T", NULL);
    }
    if (has_64) {
        chaz_ConfWriter_add_def("HAS_INT64_T", NULL);
    }

    /* Write typedefs, maximums/minimums and literals macros. */
    chaz_ConfWriter_add_typedef("int", "bool_t");
    if (has_inttypes) {
        chaz_ConfWriter_add_sys_include("inttypes.h");
    }
    else {
        if (has_stdint) {
            chaz_ConfWriter_add_sys_include("stdint.h");
        }
        else {
            /* We support only the following subset of stdint.h
             *   int8_t
             *   int16_t
             *   int32_t
             *   int64_t
             *   uint8_t
             *   uint16_t
             *   uint32_t
             *   uint64_t
             *   INT8_MAX
             *   INT16_MAX
             *   INT32_MAX
             *   INT64_MAX
             *   INT8_MIN
             *   INT16_MIN
             *   INT32_MIN
             *   INT64_MIN
             *   UINT8_MAX
             *   UINT16_MAX
             *   UINT32_MAX
             *   UINT64_MAX
             *   SIZE_MAX
             *   INT32_C
             *   INT64_C
             *   UINT32_C
             *   UINT64_C
             */
            if (has_8) {
                chaz_ConfWriter_add_global_typedef("signed char", "int8_t");
                chaz_ConfWriter_add_global_typedef("unsigned char", "uint8_t");
                chaz_ConfWriter_add_global_def("INT8_MAX", "127");
                chaz_ConfWriter_add_global_def("INT8_MIN", "-128");
                chaz_ConfWriter_add_global_def("UINT8_MAX", "255");
            }
            if (has_16) {
                chaz_ConfWriter_add_global_typedef("signed short", "int16_t");
                chaz_ConfWriter_add_global_typedef("unsigned short",
                                                   "uint16_t");
                chaz_ConfWriter_add_global_def("INT16_MAX", "32767");
                chaz_ConfWriter_add_global_def("INT16_MIN", "-32768");
                chaz_ConfWriter_add_global_def("UINT16_MAX", "65535");
            }
            if (has_32) {
                chaz_ConfWriter_add_global_typedef(i32_t_type, "int32_t");
                sprintf(scratch, "unsigned %s", i32_t_type);
                chaz_ConfWriter_add_global_typedef(scratch, "uint32_t");
                chaz_ConfWriter_add_global_def("INT32_MAX", "2147483647");
                chaz_ConfWriter_add_global_def("INT32_MIN", "(-INT32_MAX-1)");
                chaz_ConfWriter_add_global_def("UINT32_MAX", "4294967295U");
                if (strcmp(i32_t_postfix, "") == 0) {
                    chaz_ConfWriter_add_global_def("INT32_C(n)", "n");
                }
                else {
                    sprintf(scratch, "n##%s", i32_t_postfix);
                    chaz_ConfWriter_add_global_def("INT32_C(n)", scratch);
                }
                sprintf(scratch, "n##%s", u32_t_postfix);
                chaz_ConfWriter_add_global_def("UINT32_C(n)", scratch);
            }
            if (has_64) {
                chaz_ConfWriter_add_global_typedef(i64_t_type, "int64_t");
                sprintf(scratch, "unsigned %s", i64_t_type);
                chaz_ConfWriter_add_global_typedef(scratch, "uint64_t");
                sprintf(scratch, "9223372036854775807%s", i64_t_postfix);
                chaz_ConfWriter_add_global_def("INT64_MAX", scratch);
                chaz_ConfWriter_add_global_def("INT64_MIN", "(-INT64_MAX-1)");
                sprintf(scratch, "18446744073709551615%s", u64_t_postfix);
                chaz_ConfWriter_add_global_def("UINT64_MAX", scratch);
                sprintf(scratch, "n##%s", i64_t_postfix);
                chaz_ConfWriter_add_global_def("INT64_C(n)", scratch);
                sprintf(scratch, "n##%s", u64_t_postfix);
                chaz_ConfWriter_add_global_def("UINT64_C(n)", scratch);
            }
            chaz_ConfWriter_add_global_def("SIZE_MAX", "((size_t)-1)");
        }

        /* Create the PRId64 and PRIu64 printf macros. */
        if (has_64) {
            int i;
            const char *options[] = {
                "ll",
                "l",
                "L",
                "q",  /* Some *BSDs */
                "I64", /* Microsoft */
                NULL,
            };

            /* Buffer to hold the code, and its start and end. */
            static const char format_64_code[] =
                CHAZ_QUOTE(  #include "_charm.h"                           )
                CHAZ_QUOTE(  int main() {                                  )
                CHAZ_QUOTE(      Charm_Setup;                              )
                CHAZ_QUOTE(      printf("%%%su", 18446744073709551615%s);  )
                CHAZ_QUOTE(      return 0;                                 )
                CHAZ_QUOTE( }                                              );

            for (i = 0; options[i] != NULL; i++) {
                /* Try to print 2**64-1, and see if we get it back intact. */
                sprintf(code_buf, format_64_code, options[i], u64_t_postfix);
                output = chaz_CC_capture_output(code_buf, &output_len);

                if (output_len != 0
                    && strcmp(output, "18446744073709551615") == 0
                   ) {
                    sprintf(scratch, "\"%sd\"", options[i]);
                    chaz_ConfWriter_add_global_def("PRId64", scratch);
                    sprintf(scratch, "\"%su\"", options[i]);
                    chaz_ConfWriter_add_global_def("PRIu64", scratch);
                    break;
                }
            }
        }
    }

    /* Create macro for promoting pointers to integers. */
    if (has_64) {
        if (sizeof_ptr == 8) {
            chaz_ConfWriter_add_def("PTR_TO_I64(ptr)",
                                    "((int64_t)(uint64_t)(ptr))");
        }
        else {
            chaz_ConfWriter_add_def("PTR_TO_I64(ptr)",
                                    "((int64_t)(uint32_t)(ptr))");
        }
    }

    /* Create macro for converting uint64_t to double. */
    if (can_convert_u64_to_double) {
        chaz_ConfWriter_add_def("U64_TO_DOUBLE(num)",
                                "((double)(num))");
    }
    else {
        chaz_ConfWriter_add_def(
            "U64_TO_DOUBLE(num)",
            "((num) & UINT64_C(0x8000000000000000) ? "
            "(double)(int64_t)((num) & UINT64_C(0x7FFFFFFFFFFFFFFF)) + "
            "9223372036854775808.0 : "
            "(double)(int64_t)(num))");
    }

    /* True and false. */
    chaz_ConfWriter_append_conf(
        "#ifndef true\n"
        "  #define true 1\n"
        "#endif\n"
        "#ifndef false\n"
        "  #define false 0\n"
        "#endif\n"
    );

    chaz_ConfWriter_end_module();
}

static int
chaz_Integers_machine_is_big_endian(void) {
    long one = 1;
    return !(*((char*)(&one)));
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/LargeFiles.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/LargeFiles.h" */
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Module vars. */
static struct {
    char off64_type[10];
} chaz_LargeFiles = { "" };

/* Sets of symbols which might provide large file support for stdio. */
typedef struct chaz_LargeFiles_stdio64_combo {
    const char *includes;
    const char *fopen_command;
    const char *ftell_command;
    const char *fseek_command;
} chaz_LargeFiles_stdio64_combo;

/* Sets of symbols which might provide large file support for unbuffered i/o.
 */
typedef struct chaz_LargeFiles_unbuff_combo {
    const char *includes;
    const char *lseek_command;
    const char *pread64_command;
} chaz_LargeFiles_unbuff_combo;

/* Check for a 64-bit file pointer type.
 */
static const int
chaz_LargeFiles_probe_off64(void);

/* Check what name 64-bit ftell, fseek go by.
 */
static void
chaz_LargeFiles_probe_stdio64(void);
static int
chaz_LargeFiles_try_stdio64(chaz_LargeFiles_stdio64_combo *combo);

/* Probe for 64-bit unbuffered i/o.
 */
static void
chaz_LargeFiles_probe_unbuff(void);

/* Check for a 64-bit lseek.
 */
static int
chaz_LargeFiles_probe_lseek(chaz_LargeFiles_unbuff_combo *combo);

/* Check for a 64-bit pread.
 */
static int
chaz_LargeFiles_probe_pread64(chaz_LargeFiles_unbuff_combo *combo);

void
chaz_LargeFiles_run(void) {
    int found_off64_t = false;
    unsigned i;
    const char *stat_includes = "#include <stdio.h>\n#include <sys/stat.h>";

    chaz_ConfWriter_start_module("LargeFiles");

    /* Find off64_t or equivalent. */
    found_off64_t = chaz_LargeFiles_probe_off64();
    if (found_off64_t) {
        chaz_ConfWriter_add_def("HAS_64BIT_OFFSET_TYPE", NULL);
        chaz_ConfWriter_add_def("off64_t",  chaz_LargeFiles.off64_type);
    }

    /* See if stdio variants with 64-bit support exist. */
    chaz_LargeFiles_probe_stdio64();

    /* Probe for 64-bit versions of lseek and pread (if we have an off64_t). */
    if (found_off64_t) {
        chaz_LargeFiles_probe_unbuff();
    }

    /* Make checks needed for testing. */
    if (chaz_HeadCheck_check_header("sys/stat.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_SYS_STAT_H\n");
    }
    if (chaz_HeadCheck_check_header("io.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_IO_H\n");
    }
    if (chaz_HeadCheck_check_header("fcntl.h")) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_FCNTL_H\n");
    }
    if (chaz_HeadCheck_contains_member("struct stat", "st_size", stat_includes)) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_SIZE\n");
    }
    if (chaz_HeadCheck_contains_member("struct stat", "st_blocks", stat_includes)) {
        chaz_ConfWriter_append_conf("#define CHAZ_HAS_STAT_ST_BLOCKS\n");
    }

    chaz_ConfWriter_end_module();
}

static const int
chaz_LargeFiles_probe_off64(void) {
    static const char off64_code[] =
        CHAZ_QUOTE(  %s                                        )
        CHAZ_QUOTE(  #include "_charm.h"                       )
        CHAZ_QUOTE(  int main()                                )
        CHAZ_QUOTE(  {                                         )
        CHAZ_QUOTE(      Charm_Setup;                          )
        CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));       )
        CHAZ_QUOTE(      return 0;                             )
        CHAZ_QUOTE(  }                                         );
    char code_buf[sizeof(off64_code) + 100];
    int i;
    int success = false;
    static const char* off64_options[] = {
        "off64_t",
        "off_t",
        "__int64",
        "long"
    };
    int num_off64_options = sizeof(off64_options) / sizeof(off64_options[0]);

    for (i = 0; i < num_off64_options; i++) {
        const char *candidate = off64_options[i];
        char *output;
        size_t output_len;
        int has_sys_types_h = chaz_HeadCheck_check_header("sys/types.h");
        const char *sys_types_include = has_sys_types_h
                                        ? "#include <sys/types.h>"
                                        : "";

        /* Execute the probe. */
        sprintf(code_buf, off64_code, sys_types_include, candidate);
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            long sizeof_candidate = strtol(output, NULL, 10);
            free(output);
            if (sizeof_candidate == 8) {
                strcpy(chaz_LargeFiles.off64_type, candidate);
                success = true;
                break;
            }
        }
    }
    return success;
}

static int
chaz_LargeFiles_try_stdio64(chaz_LargeFiles_stdio64_combo *combo) {
    static const char stdio64_code[] =
        CHAZ_QUOTE(  %s                                         )
        CHAZ_QUOTE(  #include "_charm.h"                        )
        CHAZ_QUOTE(  int main() {                               )
        CHAZ_QUOTE(      %s pos;                                )
        CHAZ_QUOTE(      FILE *f;                               )
        CHAZ_QUOTE(      Charm_Setup;                           )
        CHAZ_QUOTE(      f = %s("_charm_stdio64", "w");         )
        CHAZ_QUOTE(      if (f == NULL) return -1;              )
        CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));        )
        CHAZ_QUOTE(      pos = %s(stdout);                      )
        CHAZ_QUOTE(      %s(stdout, 0, SEEK_SET);               )
        CHAZ_QUOTE(      return 0;                              )
        CHAZ_QUOTE(  }                                          );
    char *output = NULL;
    size_t output_len;
    char code_buf[sizeof(stdio64_code) + 200];
    int success = false;

    /* Prepare the source code. */
    sprintf(code_buf, stdio64_code, combo->includes,
            chaz_LargeFiles.off64_type, combo->fopen_command,
            chaz_LargeFiles.off64_type, combo->ftell_command,
            combo->fseek_command);

    /* Verify compilation and that the offset type has 8 bytes. */
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        long size = strtol(output, NULL, 10);
        if (size == 8) {
            success = true;
        }
        free(output);
    }

    if (!chaz_Util_remove_and_verify("_charm_stdio64")) {
        chaz_Util_die("Failed to remove '_charm_stdio64'");
    }

    return success;
}

static void
chaz_LargeFiles_probe_stdio64(void) {
    int i;
    static chaz_LargeFiles_stdio64_combo stdio64_combos[] = {
        { "#include <sys/types.h>\n", "fopen64",   "ftello64",  "fseeko64"  },
        { "#include <sys/types.h>\n", "fopen",     "ftello64",  "fseeko64"  },
        { "#include <sys/types.h>\n", "fopen",     "ftello",    "fseeko"    },
        { "",                         "fopen",     "ftell",     "fseek"     },
        { "",                         "fopen",     "_ftelli64", "_fseeki64" },
        { "",                         "fopen",     "ftell",     "fseek"     },
        { NULL, NULL, NULL, NULL }
    };

    for (i = 0; stdio64_combos[i].includes != NULL; i++) {
        chaz_LargeFiles_stdio64_combo combo = stdio64_combos[i];
        if (chaz_LargeFiles_try_stdio64(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_STDIO", NULL);
            chaz_ConfWriter_add_def("fopen64",  combo.fopen_command);
            chaz_ConfWriter_add_def("ftello64", combo.ftell_command);
            chaz_ConfWriter_add_def("fseeko64", combo.fseek_command);
            break;
        }
    }
}

static int
chaz_LargeFiles_probe_lseek(chaz_LargeFiles_unbuff_combo *combo) {
    static const char lseek_code[] =
        CHAZ_QUOTE( %s                                                       )
        CHAZ_QUOTE( #include "_charm.h"                                      )
        CHAZ_QUOTE( int main() {                                             )
        CHAZ_QUOTE(     int fd;                                              )
        CHAZ_QUOTE(     Charm_Setup;                                         )
        CHAZ_QUOTE(     fd = open("_charm_lseek", O_WRONLY | O_CREAT, 0666); )
        CHAZ_QUOTE(     if (fd == -1) { return -1; }                         )
        CHAZ_QUOTE(     %s(fd, 0, SEEK_SET);                                 )
        CHAZ_QUOTE(     printf("%%d", 1);                                    )
        CHAZ_QUOTE(     if (close(fd)) { return -1; }                        )
        CHAZ_QUOTE(     return 0;                                            )
        CHAZ_QUOTE( }                                                        );
    char code_buf[sizeof(lseek_code) + 100];
    char *output = NULL;
    size_t output_len;
    int success = false;

    /* Verify compilation. */
    sprintf(code_buf, lseek_code, combo->includes, combo->lseek_command);
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    if (!chaz_Util_remove_and_verify("_charm_lseek")) {
        chaz_Util_die("Failed to remove '_charm_lseek'");
    }

    return success;
}

static int
chaz_LargeFiles_probe_pread64(chaz_LargeFiles_unbuff_combo *combo) {
    /* Code for checking 64-bit pread.  The pread call will fail, but that's
     * fine as long as it compiles. */
    static const char pread64_code[] =
        CHAZ_QUOTE(  %s                                     )
        CHAZ_QUOTE(  #include "_charm.h"                    )
        CHAZ_QUOTE(  int main() {                           )
        CHAZ_QUOTE(      int fd = 20;                       )
        CHAZ_QUOTE(      char buf[1];                       )
        CHAZ_QUOTE(      Charm_Setup;                       )
        CHAZ_QUOTE(      printf("1");                       )
        CHAZ_QUOTE(      %s(fd, buf, 1, 1);                 )
        CHAZ_QUOTE(      return 0;                          )
        CHAZ_QUOTE(  }                                      );
    char code_buf[sizeof(pread64_code) + 100];
    char *output = NULL;
    size_t output_len;
    int success = false;

    /* Verify compilation. */
    sprintf(code_buf, pread64_code, combo->includes, combo->pread64_command);
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        success = true;
        free(output);
    }

    return success;
}

static void
chaz_LargeFiles_probe_unbuff(void) {
    static chaz_LargeFiles_unbuff_combo unbuff_combos[] = {
        { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek64",   "pread64" },
        { "#include <unistd.h>\n#include <fcntl.h>\n", "lseek",     "pread"      },
        { "#include <io.h>\n#include <fcntl.h>\n",     "_lseeki64", "NO_PREAD64" },
        { NULL, NULL, NULL }
    };
    int i;

    for (i = 0; unbuff_combos[i].lseek_command != NULL; i++) {
        chaz_LargeFiles_unbuff_combo combo = unbuff_combos[i];
        if (chaz_LargeFiles_probe_lseek(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_LSEEK", NULL);
            chaz_ConfWriter_add_def("lseek64", combo.lseek_command);
            break;
        }
    }
    for (i = 0; unbuff_combos[i].pread64_command != NULL; i++) {
        chaz_LargeFiles_unbuff_combo combo = unbuff_combos[i];
        if (chaz_LargeFiles_probe_pread64(&combo)) {
            chaz_ConfWriter_add_def("HAS_64BIT_PREAD", NULL);
            chaz_ConfWriter_add_def("pread64", combo.pread64_command);
            break;
        }
    }
};


/***************************************************************************/

#line 17 "src/Charmonizer/Probe/Memory.c"
/* #include "Charmonizer/Probe/Memory.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* Probe for alloca() or equivalent. */
static void
chaz_Memory_probe_alloca(void);

void
chaz_Memory_run(void) {
    chaz_ConfWriter_start_module("Memory");

    chaz_Memory_probe_alloca();

    chaz_ConfWriter_end_module();
}

static void
chaz_Memory_probe_alloca(void) {
    static const char alloca_code[] =
        "#include <%s>\n"
        CHAZ_QUOTE(  int main() {                   )
        CHAZ_QUOTE(      void *foo = %s(1);         )
        CHAZ_QUOTE(      return 0;                  )
        CHAZ_QUOTE(  }                              );
    int has_sys_mman_h = false;
    int has_alloca_h   = false;
    int has_malloc_h   = false;
    int need_stdlib_h  = false;
    int has_alloca     = false;
    int has_builtin_alloca    = false;
    int has_underscore_alloca = false;
    char code_buf[sizeof(alloca_code) + 100];

    {
        /* OpenBSD needs sys/types.h for sys/mman.h to work and mmap() to be
         * available. Everybody else that has sys/mman.h should have
         * sys/types.h as well. */
        const char *mman_headers[] = {
            "sys/types.h",
            "sys/mman.h",
            NULL
        };
        if (chaz_HeadCheck_check_many_headers((const char**)mman_headers)) {
            has_sys_mman_h = true;
            chaz_ConfWriter_add_def("HAS_SYS_MMAN_H", NULL);
        }
    }

    /* Unixen. */
    sprintf(code_buf, alloca_code, "alloca.h", "alloca");
    if (chaz_CC_test_compile(code_buf)) {
        has_alloca_h = true;
        has_alloca   = true;
        chaz_ConfWriter_add_def("HAS_ALLOCA_H", NULL);
        chaz_ConfWriter_add_def("alloca", "alloca");
    }
    if (!has_alloca) {
        sprintf(code_buf, alloca_code, "stdlib.h", "alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_alloca    = true;
            need_stdlib_h = true;
            chaz_ConfWriter_add_def("ALLOCA_IN_STDLIB_H", NULL);
            chaz_ConfWriter_add_def("alloca", "alloca");
        }
    }
    if (!has_alloca) {
        sprintf(code_buf, alloca_code, "stdio.h", /* stdio.h is filler */
                "__builtin_alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_builtin_alloca = true;
            chaz_ConfWriter_add_def("alloca", "__builtin_alloca");
        }
    }

    /* Windows. */
    if (!(has_alloca || has_builtin_alloca)) {
        sprintf(code_buf, alloca_code, "malloc.h", "alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_malloc_h = true;
            has_alloca   = true;
            chaz_ConfWriter_add_def("HAS_MALLOC_H", NULL);
            chaz_ConfWriter_add_def("alloca", "alloca");
        }
    }
    if (!(has_alloca || has_builtin_alloca)) {
        sprintf(code_buf, alloca_code, "malloc.h", "_alloca");
        if (chaz_CC_test_compile(code_buf)) {
            has_malloc_h = true;
            has_underscore_alloca = true;
            chaz_ConfWriter_add_def("HAS_MALLOC_H", NULL);
            chaz_ConfWriter_add_def("chy_alloca", "_alloca");
        }
    }
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/SymbolVisibility.c"
/* #include "Charmonizer/Probe/SymbolVisibility.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char chaz_SymbolVisibility_symbol_exporting_code[] =
    CHAZ_QUOTE(  %s int exported_function() {   )
    CHAZ_QUOTE(      return 42;                 )
    CHAZ_QUOTE(  }                              )
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

void
chaz_SymbolVisibility_run(void) {
    int can_control_visibility = false;
    char code_buf[sizeof(chaz_SymbolVisibility_symbol_exporting_code) + 100];

    chaz_ConfWriter_start_module("SymbolVisibility");
    chaz_CC_set_warnings_as_errors(1);

    /* Windows. */
    if (!can_control_visibility) {
        char export_win[] = "__declspec(dllexport)";
        sprintf(code_buf, chaz_SymbolVisibility_symbol_exporting_code,
                export_win);
        if (chaz_CC_test_compile(code_buf)) {
            can_control_visibility = true;
            chaz_ConfWriter_add_def("EXPORT", export_win);
            chaz_ConfWriter_add_def("IMPORT", "__declspec(dllimport)");
        }
    }

    /* GCC. */
    if (!can_control_visibility) {
        char export_gcc[] = "__attribute__ ((visibility (\"default\")))";
        sprintf(code_buf, chaz_SymbolVisibility_symbol_exporting_code,
                export_gcc);
        if (chaz_CC_test_compile(code_buf)) {
            can_control_visibility = true;
            chaz_ConfWriter_add_def("EXPORT", export_gcc);
            chaz_ConfWriter_add_def("IMPORT", NULL);
        }
    }
    chaz_CC_set_warnings_as_errors(0);

    /* Default. */
    if (!can_control_visibility) {
        chaz_ConfWriter_add_def("EXPORT", NULL);
        chaz_ConfWriter_add_def("IMPORT", NULL);
    }

    chaz_ConfWriter_end_module();
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/UnusedVars.c"
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/UnusedVars.h" */
#include <string.h>
#include <stdio.h>


void
chaz_UnusedVars_run(void) {
    chaz_ConfWriter_start_module("UnusedVars");

    /* Write the macros (no test, these are the same everywhere). */
    chaz_ConfWriter_add_def("UNUSED_VAR(x)", "((void)x)");
    chaz_ConfWriter_add_def("UNREACHABLE_RETURN(type)", "return (type)0");

    chaz_ConfWriter_end_module();
}




/***************************************************************************/

#line 17 "src/Charmonizer/Probe/VariadicMacros.c"
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Probe/VariadicMacros.h" */
#include <string.h>
#include <stdio.h>


/* Code for verifying ISO-style variadic macros. */
static const char chaz_VariadicMacros_iso_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                                   )
    CHAZ_QUOTE(  #define ISO_TEST(fmt, ...) \\                         )
    "                printf(fmt, __VA_ARGS__)                        \n"
    CHAZ_QUOTE(  int main() {                                          )
    CHAZ_QUOTE(      Charm_Setup;                                      )
    CHAZ_QUOTE(      ISO_TEST("%d %d", 1, 1);                          )
    CHAZ_QUOTE(      return 0;                                         )
    CHAZ_QUOTE(  }                                                     );

/* Code for verifying GNU-style variadic macros. */
static const char chaz_VariadicMacros_gnuc_code[] =
    CHAZ_QUOTE(  #include "_charm.h"                                   )
    CHAZ_QUOTE(  #define GNU_TEST(fmt, args...) printf(fmt, ##args)    )
    CHAZ_QUOTE(  int main() {                                          )
    CHAZ_QUOTE(      Charm_Setup;                                      )
    CHAZ_QUOTE(      GNU_TEST("%d %d", 1, 1);                          )
    CHAZ_QUOTE(      return 0;                                         )
    CHAZ_QUOTE(  }                                                     );

void
chaz_VariadicMacros_run(void) {
    char *output;
    size_t output_len;
    int has_varmacros      = false;
    int has_iso_varmacros  = false;
    int has_gnuc_varmacros = false;

    chaz_ConfWriter_start_module("VariadicMacros");

    /* Test for ISO-style variadic macros. */
    output = chaz_CC_capture_output(chaz_VariadicMacros_iso_code, &output_len);
    if (output != NULL) {
        has_varmacros = true;
        has_iso_varmacros = true;
        chaz_ConfWriter_add_def("HAS_VARIADIC_MACROS", NULL);
        chaz_ConfWriter_add_def("HAS_ISO_VARIADIC_MACROS", NULL);
    }

    /* Test for GNU-style variadic macros. */
    output = chaz_CC_capture_output(chaz_VariadicMacros_gnuc_code, &output_len);
    if (output != NULL) {
        has_gnuc_varmacros = true;
        if (has_varmacros == false) {
            has_varmacros = true;
            chaz_ConfWriter_add_def("HAS_VARIADIC_MACROS", NULL);
        }
        chaz_ConfWriter_add_def("HAS_GNUC_VARIADIC_MACROS", NULL);
    }

    chaz_ConfWriter_end_module();
}




#line 1 "clownfish/runtime/common/charmonizer.main"
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

/* Source fragment for the Clownfish runtime's charmonizer.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include "Charmonizer/Probe.h" */
/* #include "Charmonizer/Probe/AtomicOps.h" */
/* #include "Charmonizer/Probe/DirManip.h" */
/* #include "Charmonizer/Probe/Floats.h" */
/* #include "Charmonizer/Probe/FuncMacro.h" */
/* #include "Charmonizer/Probe/Headers.h" */
/* #include "Charmonizer/Probe/Integers.h" */
/* #include "Charmonizer/Probe/LargeFiles.h" */
/* #include "Charmonizer/Probe/Memory.h" */
/* #include "Charmonizer/Probe/SymbolVisibility.h" */
/* #include "Charmonizer/Probe/UnusedVars.h" */
/* #include "Charmonizer/Probe/VariadicMacros.h" */
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/ConfWriterC.h" */
/* #include "Charmonizer/Core/ConfWriterPerl.h" */
/* #include "Charmonizer/Core/ConfWriterRuby.h" */

#define MAX_CC_LEN 128
#define MAX_FLAGS_LEN 2048

struct CLIArgs {
    char cc_command[MAX_CC_LEN + 1];
    char cc_flags[MAX_FLAGS_LEN + 1];
    int  enable_c;
    int  enable_perl;
    int  enable_ruby;
};

/* Parse command line arguments. */
static void
S_parse_arguments(int argc, char **argv, struct CLIArgs *args) {
    int i;
    int output_enabled = 0;

    /* Parse most args. */
    for (i = 1; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "--") == 0) {
            /* From here on out, everything will be a compiler flag. */
            i++;
            break;
        }
        if (strcmp(arg, "--enable-c") == 0) {
            args->enable_c = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-perl") == 0) {
            args->enable_perl = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-ruby") == 0) {
            args->enable_ruby = 1;
            output_enabled = 1;
        }
        else if (memcmp(arg, "--cc=", 5) == 0) {
            if (strlen(arg) > MAX_CC_LEN - 5) {
                fprintf(stderr, "Exceeded max length for compiler command");
                exit(1);
            }
            strcpy(args->cc_command, arg + 5);
        }
    }

    /* Accumulate compiler flags. */
    for (; i < argc; i++) {
        char *arg = argv[i];
        if (strlen(arg) + strlen(args->cc_flags) + 2 >= MAX_FLAGS_LEN) {
            fprintf(stderr, "Exceeded max length for compiler flags");
            exit(1);
        }
        strcat(args->cc_flags, " ");
        strcat(args->cc_flags, arg);

    }

    /* Validate. */
    if (!args->cc_command
        || !strlen(args->cc_command)
        || !output_enabled
       ) {
        fprintf(stderr,
                "Usage: ./charmonize --cc=CC_COMMAND [--enable-c] "
                "[--enable-perl] [--enable-ruby] -- CC_FLAGS\n");
        exit(1);
    }

}

int main(int argc, char **argv) {
    struct CLIArgs args;
    memset(&args, 0, sizeof(struct CLIArgs));

    S_parse_arguments(argc, argv, &args);
    chaz_Probe_init(args.cc_command, args.cc_flags);
    if (args.enable_c) {
        chaz_ConfWriterC_enable();
    }
    if (args.enable_perl) {
        chaz_ConfWriterPerl_enable();
    }
    if (args.enable_ruby) {
        chaz_ConfWriterRuby_enable();
    }

    /* Run probe modules. */
    chaz_DirManip_run();
    chaz_Headers_run();
    chaz_AtomicOps_run();
    chaz_FuncMacro_run();
    chaz_Integers_run();
    chaz_Floats_run();
    chaz_LargeFiles_run();
    chaz_Memory_run();
    chaz_SymbolVisibility_run();
    chaz_UnusedVars_run();
    chaz_VariadicMacros_run();

    /* Write custom postamble. */
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_SYS_TYPES_H\n"
        "  #include <sys/types.h>\n"
        "#endif\n\n"
    );
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_STDARG_H\n"
        "  #include <stdarg.h>\n"
        "#endif\n\n"
    );
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_ALLOCA_H\n"
        "  #include <alloca.h>\n"
        "#elif defined(CHY_HAS_MALLOC_H)\n"
        "  #include <malloc.h>\n"
        "#elif defined(CHY_ALLOCA_IN_STDLIB_H)\n"
        "  #include <stdlib.h>\n"
        "#endif\n\n"
    );
    chaz_ConfWriter_append_conf(
        "#ifdef CHY_HAS_WINDOWS_H\n"
        "  /* Target Windows XP. */\n"
        "  #ifndef WINVER\n"
        "    #define WINVER 0x0500\n"
        "  #endif\n"
        "  #ifndef _WIN32_WINNT\n"
        "    #define _WIN32_WINNT 0x0500\n"
        "  #endif\n"
        "#endif\n\n"
    );

    /* Clean up. */
    chaz_Probe_clean_up();

    return 0;
}


