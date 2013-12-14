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

#line 21 "src/Charmonizer/Core/SharedLibrary.h"
/* Charmonizer/Core/SharedLibrary.h
 */

#ifndef H_CHAZ_SHARED_LIB
#define H_CHAZ_SHARED_LIB

typedef struct chaz_SharedLib chaz_SharedLib;

chaz_SharedLib*
chaz_SharedLib_new(const char *name, const char *version,
                   const char *major_version);

void
chaz_SharedLib_destroy(chaz_SharedLib *flags);

const char*
chaz_SharedLib_get_name(chaz_SharedLib *lib);

const char*
chaz_SharedLib_get_version(chaz_SharedLib *lib);

const char*
chaz_SharedLib_get_major_version(chaz_SharedLib *lib);

char*
chaz_SharedLib_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_major_version_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_no_version_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_implib_filename(chaz_SharedLib *lib);

char*
chaz_SharedLib_export_filename(chaz_SharedLib *lib);

#endif /* H_CHAZ_SHARED_LIB */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/CFlags.h"
/* Charmonizer/Core/CFlags.h
 */

#ifndef H_CHAZ_CFLAGS
#define H_CHAZ_CFLAGS

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
chaz_CFlags_set_shared_library_version(chaz_CFlags *flags,
                                       chaz_SharedLib *lib);

void
chaz_CFlags_set_link_output(chaz_CFlags *flags, const char *filename);

void
chaz_CFlags_add_library_path(chaz_CFlags *flags, const char *directory);

void
chaz_CFlags_add_library(chaz_CFlags *flags, chaz_SharedLib *lib);

void
chaz_CFlags_add_external_library(chaz_CFlags *flags, const char *library);

void
chaz_CFlags_enable_code_coverage(chaz_CFlags *flags);

#endif /* H_CHAZ_CFLAGS */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/Compiler.h"
/* Charmonizer/Core/Compiler.h
 */

#ifndef H_CHAZ_COMPILER
#define H_CHAZ_COMPILER

#include <stddef.h>
/* #include "Charmonizer/Core/Defines.h" */
/* #include "Charmonizer/Core/CFlags.h" */

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
chaz_CFlags*
chaz_CC_get_extra_cflags(void);

/* Accessor for `temp_cflags`.
 */
chaz_CFlags*
chaz_CC_get_temp_cflags(void);

/* Return a new CFlags object.
 */
chaz_CFlags*
chaz_CC_new_cflags(void);

/* Return the extension for a compiled object.
 */
const char*
chaz_CC_obj_ext(void);

int
chaz_CC_gcc_version_num(void);

const char*
chaz_CC_gcc_version(void);

int
chaz_CC_msvc_version_num(void);

const char*
chaz_CC_link_command(void);

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

#line 21 "src/Charmonizer/Core/Make.h"
/* Charmonizer/Core/Make.h
 */

#ifndef H_CHAZ_MAKE
#define H_CHAZ_MAKE

/* #include "Charmonizer/Core/CFlags.h" */
/* #include "Charmonizer/Core/SharedLib.h" */

typedef struct chaz_MakeFile chaz_MakeFile;
typedef struct chaz_MakeVar chaz_MakeVar;
typedef struct chaz_MakeRule chaz_MakeRule;

typedef void (*chaz_Make_list_files_callback_t)(const char *dir, char *file,
                                                void *context);

/** Initialize the environment.
 */
void
chaz_Make_init(void);

/** Clean up the environment.
 */
void
chaz_Make_clean_up(void);

/** Return the name of the detected 'make' executable.
 */
const char*
chaz_Make_get_make(void);

/** Return the type of shell used by the detected 'make' executable.
 */
int
chaz_Make_shell_type(void);

/** Recursively list files in a directory. For every file a callback is called
 * with the filename and a context variable.
 *
 * @param dir Directory to search in.
 * @param ext File extension to search for.
 * @param callback Callback to call for every matching file.
 * @param context Context variable to pass to callback.
 */
void
chaz_Make_list_files(const char *dir, const char *ext,
                     chaz_Make_list_files_callback_t callback, void *context);

/** MakeFile constructor.
 */
chaz_MakeFile*
chaz_MakeFile_new();

/** MakeFile destructor.
 */
void
chaz_MakeFile_destroy(chaz_MakeFile *makefile);

/** Add a variable to a makefile.
 *
 * @param makefile The makefile.
 * @param name Name of the variable.
 * @param value Value of the variable. Can be NULL if you want add content
 * later.
 * @return a MakeVar.
 */
chaz_MakeVar*
chaz_MakeFile_add_var(chaz_MakeFile *makefile, const char *name,
                      const char *value);

/** Add a rule to a makefile.
 *
 * @param makefile The makefile.
 * @param target The first target of the rule. Can be NULL if you want to add
 * targets later.
 * @param prereq The first prerequisite of the rule. Can be NULL if you want to
 * add prerequisites later.
 * @return a MakeRule.
 */
chaz_MakeRule*
chaz_MakeFile_add_rule(chaz_MakeFile *makefile, const char *target,
                       const char *prereq);

/** Return the rule for the 'clean' target.
 *
 * @param makefile The makefile.
 */
chaz_MakeRule*
chaz_MakeFile_clean_rule(chaz_MakeFile *makefile);

/** Return the rule for the 'distclean' target.
 *
 * @param makefile The makefile.
 */
chaz_MakeRule*
chaz_MakeFile_distclean_rule(chaz_MakeFile *makefile);

/** Add a rule to link an executable. The executable will also be added to the
 * list of files to clean.
 *
 * @param makefile The makefile.
 * @param exe The name of the executable.
 * @param sources The list of source files.
 * @param link_flags Additional link flags.
 */
chaz_MakeRule*
chaz_MakeFile_add_exe(chaz_MakeFile *makefile, const char *exe,
                      const char *sources, chaz_CFlags *link_flags);

/** Add a rule to compile and link an executable. The executable will also be
 * added to the list of files to clean.
 *
 * @param makefile The makefile.
 * @param exe The name of the executable.
 * @param sources The list of source files.
 * @param cflags Additional compiler flags.
 */
chaz_MakeRule*
chaz_MakeFile_add_compiled_exe(chaz_MakeFile *makefile, const char *exe,
                               const char *sources, chaz_CFlags *cflags);

/** Add a rule to link a shared library. The shared library will also be added
 * to the list of files to clean.
 *
 * @param makefile The makefile.
 * @param lib The shared library.
 * @param sources The list of source files.
 * @param link_flags Additional link flags.
 */
chaz_MakeRule*
chaz_MakeFile_add_shared_lib(chaz_MakeFile *makefile, chaz_SharedLib *lib,
                             const char *sources, chaz_CFlags *link_flags);

/** Add a rule to build the lemon parser generator.
 *
 * @param makefile The makefile.
 * @param dir The lemon directory.
 */
chaz_MakeRule*
chaz_MakeFile_add_lemon_exe(chaz_MakeFile *makefile, const char *dir);

/** Add a rule for a lemon grammar.
 *
 * @param makefile The makefile.
 * @param base_name The filename of the grammar without extension.
 */
chaz_MakeRule*
chaz_MakeFile_add_lemon_grammar(chaz_MakeFile *makefile,
                                const char *base_name);

/** Write the makefile to a file named 'Makefile' in the current directory.
 *
 * @param makefile The makefile.
 */
void
chaz_MakeFile_write(chaz_MakeFile *makefile);

/** Append content to a makefile variable. The new content will be separated
 * from the existing content with whitespace.
 *
 * @param var The variable.
 * @param element The additional content.
 */
void
chaz_MakeVar_append(chaz_MakeVar *var, const char *element);

/** Add another target to a makefile rule.
 *
 * @param rule The rule.
 * @param target The additional rule.
 */
void
chaz_MakeRule_add_target(chaz_MakeRule *rule, const char *target);

/** Add another prerequisite to a makefile rule.
 *
 * @param rule The rule.
 * @param prereq The additional prerequisite.
 */
void
chaz_MakeRule_add_prereq(chaz_MakeRule *rule, const char *prereq);

/** Add a command to a rule.
 *
 * @param rule The rule.
 * @param command The additional command.
 */
void
chaz_MakeRule_add_command(chaz_MakeRule *rule, const char *command);

/** Add a command to remove one or more files.
 *
 * @param rule The rule.
 * @param files The list of files.
 */
void
chaz_MakeRule_add_rm_command(chaz_MakeRule *rule, const char *files);

/** Add a command to remove one or more directories.
 *
 * @param rule The rule.
 * @param dirs The list of directories.
 */
void
chaz_MakeRule_add_recursive_rm_command(chaz_MakeRule *rule, const char *dirs);

/** Add one or more commands to call another makefile recursively.
 *
 * @param rule The rule.
 * @param dir The directory in which to call the makefile.
 * @param target The target to call. Pass NULL for the default target.
 */
void
chaz_MakeRule_add_make_command(chaz_MakeRule *rule, const char *dir,
                               const char *target);

#endif /* H_CHAZ_MAKE */



/***************************************************************************/

#line 21 "src/Charmonizer/Core/OperatingSystem.h"
/* Charmonizer/Core/OperatingSystem.h - abstract an operating system down to a few
 * variables.
 */

#ifndef H_CHAZ_OPER_SYS
#define H_CHAZ_OPER_SYS

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

/* Return the directory separator on this system.
 */
const char*
chaz_OS_dir_sep(void);

/* Return the shell type of this system.
 */
int
chaz_OS_shell_type(void);

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

/* Join a NULL-terminated list of strings using a separator.
 */
char*
chaz_Util_join(const char *sep, ...);

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

#define CHAZ_PROBE_MAX_CC_LEN 100
#define CHAZ_PROBE_MAX_CFLAGS_LEN 2000

struct chaz_CLIArgs {
    char cc[CHAZ_PROBE_MAX_CC_LEN + 1];
    char cflags[CHAZ_PROBE_MAX_CFLAGS_LEN + 1];
    int  charmony_h;
    int  charmony_pm;
    int  charmony_rb;
    int  verbosity;
    int  write_makefile;
    int  code_coverage;
};

/* Parse command line arguments, initializing and filling in the supplied
 * `args` struct.
 *
 *     APP_NAME --cc=CC_COMMAND
 *              [--enable-c]
 *              [--enable-perl]
 *              [--enable-ruby]
 *              [-- [CFLAGS]]
 *
 * @return true if argument parsing proceeds without incident, false if
 * unexpected arguments are encountered or values are missing or invalid.
 */
int
chaz_Probe_parse_cli_args(int argc, const char *argv[],
                          struct chaz_CLIArgs *args);

/* Exit after printing usage instructions to stderr.
 */
void
chaz_Probe_die_usage(void);

/* Set up the Charmonizer environment.
 *
 * If the environment variable CHARM_VERBOSITY has been set, it will be
 * processed at this time:
 *
 *      0 - silent
 *      1 - normal
 *      2 - debugging
 */
void
chaz_Probe_init(struct chaz_CLIArgs *args);

/* Clean up the Charmonizer environment -- deleting tempfiles, etc.  This
 * should be called only after everything else finishes.
 */
void
chaz_Probe_clean_up(void);

/* Return an integer version of the GCC version number which is
 * (10000 * __GNU_C__ + 100 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__).
 */
int
chaz_Probe_gcc_version_num(void);

/* If the compiler is GCC (or claims compatibility), return an X.Y.Z string
 * version of the GCC version; otherwise, return NULL.
 */
const char*
chaz_Probe_gcc_version(void);

/* Return the integer version of MSVC defined by _MSC_VER
 */
int
chaz_Probe_msvc_version_num(void);

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

#line 21 "src/Charmonizer/Probe/Booleans.h"
/* Charmonizer/Probe/Booleans.h -- bool type.
 *
 * If stdbool.h is is available, it will be pound-included in the configuration
 * header.  If it is not, the following typedef will be defined:
 *
 * bool
 *
 * These symbols will be defined if they are not already:
 *
 * true
 * false
 */

#ifndef H_CHAZ_BOOLEANS
#define H_CHAZ_BOOLEANS

#include <stdio.h>

/* Run the Booleans module.
 */
void chaz_Booleans_run(void);

#endif /* H_CHAZ_BOOLEANS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/BuildEnv.h"
/* Charmonizer/Probe/BuildEnv.h -- Build environment.
 *
 * Capture various information about the build environment, including the C
 * compiler's interface, the shell, the operating system, etc.
 *
 * The following symbols will be defined:
 *
 * CC - String representation of the C compiler executable.
 * CFLAGS - C compiler flags.
 * EXTRA_CFLAGS - Extra C compiler flags.
 */

#ifndef H_CHAZ_BUILDENV
#define H_CHAZ_BUILDENV

#include <stdio.h>

/* Run the BuildEnv module.
 */
void chaz_BuildEnv_run(void);

#endif /* H_CHAZ_BUILDENV */




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

/* Return the name of the math library to link against or NULL.
 */
const char*
chaz_Floats_math_library(void);

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

#line 21 "src/Charmonizer/Probe/RegularExpressions.h"
/* Charmonizer/Probe/RegularExpressions.h -- regular expressions.
 */

#ifndef H_CHAZ_REGULAREXPRESSIONS
#define H_CHAZ_REGULAREXPRESSIONS

/* Run the RegularExpressions module.
 */
void chaz_RegularExpressions_run(void);

#endif /* H_CHAZ_REGULAREXPRESSIONS */




/***************************************************************************/

#line 21 "src/Charmonizer/Probe/Strings.h"
/* Charmonizer/Probe/Strings.h
 */

#ifndef H_CHAZ_STRINGS
#define H_CHAZ_STRINGS

/* The Strings module attempts to detect whether snprintf works as specified
 * by the C99 standard. It also looks for system-specific functions which can
 * be used to emulate snprintf.
 */
void chaz_Strings_run(void);

#endif /* H_CHAZ_STRINGS */



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

#line 17 "src/Charmonizer/Core/SharedLibrary.c"
#include <string.h>
#include <stdlib.h>
/* #include "Charmonizer/Core/SharedLib.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

struct chaz_SharedLib {
    char *name;
    char *version;
    char *major_version;
};

static char*
S_build_filename(chaz_SharedLib *lib, const char *version, const char *ext);

static const char*
S_get_prefix(void);

chaz_SharedLib*
chaz_SharedLib_new(const char *name, const char *version,
                   const char *major_version) {
    chaz_SharedLib *lib = (chaz_SharedLib*)malloc(sizeof(chaz_SharedLib));
    lib->name          = chaz_Util_strdup(name);
    lib->version       = chaz_Util_strdup(version);
    lib->major_version = chaz_Util_strdup(major_version);
    return lib;
}

void
chaz_SharedLib_destroy(chaz_SharedLib *lib) {
    free(lib->name);
    free(lib->version);
    free(lib->major_version);
    free(lib);
}

const char*
chaz_SharedLib_get_name(chaz_SharedLib *lib) {
    return lib->name;
}

const char*
chaz_SharedLib_get_version(chaz_SharedLib *lib) {
    return lib->version;
}

const char*
chaz_SharedLib_get_major_version(chaz_SharedLib *lib) {
    return lib->major_version;
}

char*
chaz_SharedLib_filename(chaz_SharedLib *lib) {
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    if (strcmp(shlib_ext, ".dll") == 0) {
        return S_build_filename(lib, lib->major_version, shlib_ext);
    }
    else {
        return S_build_filename(lib, lib->version, shlib_ext);
    }
}

char*
chaz_SharedLib_major_version_filename(chaz_SharedLib *lib) {
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    return S_build_filename(lib, lib->major_version, shlib_ext);
}

char*
chaz_SharedLib_no_version_filename(chaz_SharedLib *lib) {
    const char *prefix    = S_get_prefix();
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    return chaz_Util_join("", prefix, lib->name, shlib_ext, NULL);
}

char*
chaz_SharedLib_implib_filename(chaz_SharedLib *lib) {
    return S_build_filename(lib, lib->major_version, ".lib");
}

char*
chaz_SharedLib_export_filename(chaz_SharedLib *lib) {
    return S_build_filename(lib, lib->major_version, ".exp");
}

static char*
S_build_filename(chaz_SharedLib *lib, const char *version, const char *ext) {
    const char *prefix    = S_get_prefix();
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    if (strcmp(shlib_ext, ".dll") == 0) {
        return chaz_Util_join("", prefix, lib->name, "-", version, ext, NULL);
    }
    else if (strcmp(shlib_ext, ".dylib") == 0) {
        return chaz_Util_join("", prefix, lib->name, ".", version, ext, NULL);
    }
    else {
        return chaz_Util_join("", prefix, lib->name, ext, ".", version, NULL);
    }
}

static const char*
S_get_prefix() {
    if (chaz_CC_msvc_version_num()) {
        return "";
    }
    else if (chaz_OS_is_cygwin()) {
        return "cyg";
    }
    else {
        return "lib";
    }
}



/***************************************************************************/

#line 17 "src/Charmonizer/Core/CFlags.c"
#include <string.h>
#include <stdlib.h>
/* #include "Charmonizer/Core/CFlags.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */
/* #include "Charmonizer/Core/SharedLibrary.h" */

struct chaz_CFlags {
    int   style;
    char *string;
};

chaz_CFlags*
chaz_CFlags_new(int style) {
    chaz_CFlags *flags = (chaz_CFlags*)malloc(sizeof(chaz_CFlags));
    flags->style  = style;
    flags->string = chaz_Util_strdup("");
    return flags;
}

void
chaz_CFlags_destroy(chaz_CFlags *flags) {
    free(flags->string);
    free(flags);
}

const char*
chaz_CFlags_get_string(chaz_CFlags *flags) {
    return flags->string;
}

void
chaz_CFlags_append(chaz_CFlags *flags, const char *string) {
    char *new_string;
    if (flags->string[0] == '\0') {
        new_string = chaz_Util_strdup(string);
    }
    else {
        new_string = chaz_Util_join(" ", flags->string, string, NULL);
    }
    free(flags->string);
    flags->string = new_string;
}

void
chaz_CFlags_clear(chaz_CFlags *flags) {
    if (flags->string[0] != '\0') {
        free(flags->string);
        flags->string = chaz_Util_strdup("");
    }
}

void
chaz_CFlags_set_output_obj(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/c /Fo";
    }
    else {
        /* POSIX */
        output = "-c -o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_set_output_exe(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/Fe";
    }
    else {
        /* POSIX */
        output = "-o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_define(chaz_CFlags *flags, const char *name,
                       const char *value) {
    const char *define;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        define = "/D";
    }
    else {
        /* POSIX */
        define = "-D ";
    }
    if (value) {
        string = chaz_Util_join("", define, name, "=", value, NULL);
    }
    else {
        string = chaz_Util_join("", define, name, NULL);
    }
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_include_dir(chaz_CFlags *flags, const char *dir) {
    const char *include;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC)  {
        include = "/I ";
    }
    else {
        /* POSIX */
        include = "-I ";
    }
    string = chaz_Util_join("", include, dir, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_enable_optimization(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/O2";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        string = "-O2";
    }
    else {
        /* POSIX */
        string = "-O 1";
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_enable_debugging(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        chaz_CFlags_append(flags, "-g");
    }
}

void
chaz_CFlags_disable_strict_aliasing(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        return;
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        chaz_CFlags_append(flags, "-fno-strict-aliasing");
    }
    else {
        chaz_Util_die("Don't know how to disable strict aliasing with '%s'",
                      chaz_CC_get_cc());
    }
}

void
chaz_CFlags_set_warnings_as_errors(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/WX";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        string = "-Werror";
    }
    else {
        chaz_Util_die("Don't know how to set warnings as errors with '%s'",
                      chaz_CC_get_cc());
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_compile_shared_library(chaz_CFlags *flags) {
    const char *string;
    if (flags->style != CHAZ_CFLAGS_STYLE_GNU
        || strcmp(chaz_OS_shared_lib_ext(), ".dll") == 0
       ) {
        return;
    }
    if (chaz_OS_is_darwin()) {
        string = "-fno-common";
    }
    else {
        string = "-fPIC";
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_hide_symbols(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU
        && strcmp(chaz_OS_shared_lib_ext(), ".dll") != 0) {
        chaz_CFlags_append(flags, "-fvisibility=hidden");
    }
}

void
chaz_CFlags_link_shared_library(chaz_CFlags *flags) {
    const char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = "/DLL";
    }
    else if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        if (strcmp(chaz_OS_shared_lib_ext(), ".dylib") == 0) {
            string = "-dynamiclib";
        }
        else {
            string = "-shared";
        }
    }
    else {
        chaz_Util_die("Don't know how to link a shared library with '%s'",
                      chaz_CC_get_cc());
    }
    chaz_CFlags_append(flags, string);
}

void
chaz_CFlags_set_shared_library_version(chaz_CFlags *flags,
                                       chaz_SharedLib *lib) {
    const char *shlib_ext = chaz_OS_shared_lib_ext();
    char       *string;

    if (flags->style != CHAZ_CFLAGS_STYLE_GNU
        || strcmp(shlib_ext, ".dll") == 0) {
        return;
    }

    if (strcmp(chaz_OS_shared_lib_ext(), ".dylib") == 0) {
        const char *version = chaz_SharedLib_get_version(lib);
        string = chaz_Util_join(" ", "-current_version", version, NULL);
    }
    else {
        char *soname = chaz_SharedLib_major_version_filename(lib);
        string = chaz_Util_join("", "-Wl,-soname,", soname, NULL);
        free(soname);
    }
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_set_link_output(chaz_CFlags *flags, const char *filename) {
    const char *output;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        output = "/OUT:";
    }
    else {
        output = "-o ";
    }
    string = chaz_Util_join("", output, filename, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_library_path(chaz_CFlags *flags, const char *directory) {
    const char *lib_path;
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        if (strcmp(directory, ".") == 0) {
            /* The MS linker searches the current directory by default. */
            return;
        }
        else {
            lib_path = "/LIBPATH:";
        }
    }
    else {
        lib_path = "-L ";
    }
    string = chaz_Util_join("", lib_path, directory, NULL);
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_add_library(chaz_CFlags *flags, chaz_SharedLib *lib) {
    char *filename;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        filename = chaz_SharedLib_implib_filename(lib);
    }
    else {
        filename = chaz_SharedLib_filename(lib);
    }
    chaz_CFlags_append(flags, filename);
    free(filename);
}

void
chaz_CFlags_add_external_library(chaz_CFlags *flags, const char *library) {
    char *string;
    if (flags->style == CHAZ_CFLAGS_STYLE_MSVC) {
        string = chaz_Util_join("", library, ".lib", NULL);
    }
    else {
        string = chaz_Util_join(" ", "-l", library, NULL);
    }
    chaz_CFlags_append(flags, string);
    free(string);
}

void
chaz_CFlags_enable_code_coverage(chaz_CFlags *flags) {
    if (flags->style == CHAZ_CFLAGS_STYLE_GNU) {
        chaz_CFlags_append(flags, "--coverage");
    }
    else {
        chaz_Util_die("Don't know how to enable code coverage with '%s'",
                      chaz_CC_get_cc());
    }
}



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
    char     *cflags;
    char     *try_exe_name;
    char      obj_ext[10];
    char      gcc_version_str[30];
    int       cflags_style;
    int       intval___GNUC__;
    int       intval___GNUC_MINOR__;
    int       intval___GNUC_PATCHLEVEL__;
    int       intval__MSC_VER;
    int       intval___clang__;
    chaz_CFlags *extra_cflags;
    chaz_CFlags *temp_cflags;
} chaz_CC = {
    NULL, NULL, NULL,
    "", "",
    0, 0, 0, 0, 0, 0,
    NULL, NULL
};

void
chaz_CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (chaz_Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign, init. */
    chaz_CC.cc_command   = chaz_Util_strdup(compiler_command);
    chaz_CC.cflags       = chaz_Util_strdup(compiler_flags);
    chaz_CC.extra_cflags = NULL;
    chaz_CC.temp_cflags  = NULL;

    /* Set names for the targets which we "try" to compile. */
    chaz_CC.try_exe_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_OS_exe_ext(), NULL);

    /* If we can't compile anything, game over. */
    if (chaz_Util_verbosity) {
        printf("Trying to compile a small test file...\n");
    }
    /* Try MSVC argument style. */
    strcpy(chaz_CC.obj_ext, ".obj");
    chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
    compile_succeeded = chaz_CC_test_compile(code);
    if (!compile_succeeded) {
        /* Try POSIX argument style. */
        strcpy(chaz_CC.obj_ext, ".o");
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
        compile_succeeded = chaz_CC_test_compile(code);
    }
    if (!compile_succeeded) {
        chaz_Util_die("Failed to compile a small test file");
    }

    chaz_CC_detect_known_compilers();

    if (chaz_CC.intval___GNUC__) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_GNU;
    }
    else if (chaz_CC.intval__MSC_VER) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
    }
    else {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
    }
    chaz_CC.extra_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    chaz_CC.temp_cflags  = chaz_CFlags_new(chaz_CC.cflags_style);
}

static const char chaz_CC_detect_macro_code[] =
    CHAZ_QUOTE(  #include <stdio.h>             )
    CHAZ_QUOTE(  int main() {                   )
    CHAZ_QUOTE(  #ifndef %s                     )
    CHAZ_QUOTE(  #error "nope"                  )
    CHAZ_QUOTE(  #endif                         )
    CHAZ_QUOTE(      printf("%%d", %s);         )
    CHAZ_QUOTE(      return 0;                  )
    CHAZ_QUOTE(  }                              );

static int
chaz_CC_detect_macro(const char *macro) {
    size_t size = sizeof(chaz_CC_detect_macro_code)
                  + (strlen(macro) * 2)
                  + 20;
    char *code = (char*)malloc(size);
    int retval = 0;
    char *output;
    size_t len;
    sprintf(code, chaz_CC_detect_macro_code, macro, macro);
    output = chaz_CC_capture_output(code, &len);
    if (output) {
        retval = atoi(output);
        free(output);
    }
    free(code);
    return retval;
}

static void
chaz_CC_detect_known_compilers(void) {
    chaz_CC.intval___GNUC__  = chaz_CC_detect_macro("__GNUC__");
    if (chaz_CC.intval___GNUC__) {
        chaz_CC.intval___GNUC_MINOR__
            = chaz_CC_detect_macro("__GNUC_MINOR__");
        chaz_CC.intval___GNUC_PATCHLEVEL__
            = chaz_CC_detect_macro("__GNUC_PATCHLEVEL__");
        sprintf(chaz_CC.gcc_version_str, "%d.%d.%d", chaz_CC.intval___GNUC__,
                chaz_CC.intval___GNUC_MINOR__,
                chaz_CC.intval___GNUC_PATCHLEVEL__);
    }
    chaz_CC.intval__MSC_VER  = chaz_CC_detect_macro("_MSC_VER");
    chaz_CC.intval___clang__ = chaz_CC_detect_macro("__clang__");
}

void
chaz_CC_clean_up(void) {
    free(chaz_CC.cc_command);
    free(chaz_CC.cflags);
    free(chaz_CC.try_exe_name);
    chaz_CFlags_destroy(chaz_CC.extra_cflags);
    chaz_CFlags_destroy(chaz_CC.temp_cflags);
}

int
chaz_CC_compile_exe(const char *source_path, const char *exe_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *exe_file = chaz_Util_join("", exe_name, chaz_OS_exe_ext(), NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_exe(local_cflags, exe_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        system(command);
    }

    if (chaz_CC.intval__MSC_VER) {
        /* Zap MSVC junk. */
        size_t  junk_buf_size = strlen(exe_file) + 4;
        char   *junk          = (char*)malloc(junk_buf_size);
        sprintf(junk, "%s.obj", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.ilk", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.pdb", exe_name);
        chaz_Util_remove_and_verify(junk);
        free(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(exe_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(exe_file);
    return result;
}

int
chaz_CC_compile_obj(const char *source_path, const char *obj_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *obj_file = chaz_Util_join("", obj_name, chaz_CC.obj_ext, NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_obj(local_cflags, obj_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
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

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(obj_file);
    return result;
}

int
chaz_CC_test_compile(const char *source) {
    int compile_succeeded;
    char *try_obj_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_CC.obj_ext, NULL);
    if (!chaz_Util_remove_and_verify(try_obj_name)) {
        chaz_Util_die("Failed to delete file '%s'", try_obj_name);
    }
    compile_succeeded = chaz_CC_compile_obj(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(try_obj_name);
    free(try_obj_name);
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
        chaz_OS_run_local_redirected(chaz_CC.try_exe_name,
                                     CHAZ_CC_TARGET_PATH);
        captured_output = chaz_Util_slurp_file(CHAZ_CC_TARGET_PATH,
                                               output_len);
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

const char*
chaz_CC_get_cc(void) {
    return chaz_CC.cc_command;
}

const char*
chaz_CC_get_cflags(void) {
    return chaz_CC.cflags;
}

chaz_CFlags*
chaz_CC_get_extra_cflags(void) {
    return chaz_CC.extra_cflags;
}

chaz_CFlags*
chaz_CC_get_temp_cflags(void) {
    return chaz_CC.temp_cflags;
}

chaz_CFlags*
chaz_CC_new_cflags(void) {
    return chaz_CFlags_new(chaz_CC.cflags_style);
}

const char*
chaz_CC_obj_ext(void) {
    return chaz_CC.obj_ext;
}

int
chaz_CC_gcc_version_num(void) {
    return 10000 * chaz_CC.intval___GNUC__
           + 100 * chaz_CC.intval___GNUC_MINOR__
           + chaz_CC.intval___GNUC_PATCHLEVEL__;
}

const char*
chaz_CC_gcc_version(void) {
    return chaz_CC.intval___GNUC__ ? chaz_CC.gcc_version_str : NULL;
}

int
chaz_CC_msvc_version_num(void) {
    return chaz_CC.intval__MSC_VER;
}

const char*
chaz_CC_link_command() {
    if (chaz_CC.intval__MSC_VER) {
        return "link";
    }
    else {
        return chaz_CC.cc_command;
    }
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
    char          *MODULE_NAME;
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
    return isupper(sym[0]);
}

static char*
chaz_ConfWriterC_uppercase_string(const char *src) {
    char *retval = chaz_Util_strdup(src);
    size_t i;
    for (i = 0; retval[i]; ++i) {
        retval[i] = toupper(retval[i]);
    }
    return retval;
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
        if (strcmp(sym, value) == 0) { return; }
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
    if (strcmp(type, alias) == 0) { return; }
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
    chaz_ConfWriterC.MODULE_NAME
        = chaz_ConfWriterC_uppercase_string(module_name);
}

static void
chaz_ConfWriterC_end_module(void) {
    size_t num_globals = 0;
    size_t i;
    chaz_ConfElem *defs = chaz_ConfWriterC.defs;
    for (i = 0; i < chaz_ConfWriterC.def_count; i++) {
        switch (defs[i].type) {
            case CHAZ_CONFELEM_GLOBAL_DEF:
                ++num_globals;
            /* fall through */
            case CHAZ_CONFELEM_DEF:
                chaz_ConfWriterC_append_def_to_conf(defs[i].str1,
                                                    defs[i].str2);
                break;
            case CHAZ_CONFELEM_GLOBAL_TYPEDEF: {
                char *sym = chaz_ConfWriterC_uppercase_string(defs[i].str1);
                chaz_ConfWriterC_append_def_to_conf(sym, defs[i].str2);
                free(sym);
                ++num_globals;
            }
            /* fall through */
            case CHAZ_CONFELEM_TYPEDEF:
                chaz_ConfWriterC_append_typedef_to_conf(defs[i].str2,
                                                        defs[i].str1);
                break;
            case CHAZ_CONFELEM_SYS_INCLUDE:
                ++num_globals;
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

    /* Write out global definitions and system includes. */
    if (num_globals) {
        fprintf(chaz_ConfWriterC.fh, "\n#ifdef CHY_EMPLOY_%s\n\n",
                chaz_ConfWriterC.MODULE_NAME);
        for (i = 0; i < chaz_ConfWriterC.def_count; i++) {
            switch (defs[i].type) {
                case CHAZ_CONFELEM_GLOBAL_DEF:
                    chaz_ConfWriterC_append_global_def_to_conf(defs[i].str1,
                                                               defs[i].str2);
                    break;
                case CHAZ_CONFELEM_GLOBAL_TYPEDEF:
                    chaz_ConfWriterC_append_global_typedef_to_conf(
                            defs[i].str2, defs[i].str1);
                    break;
                case CHAZ_CONFELEM_SYS_INCLUDE:
                    chaz_ConfWriterC_append_sys_include_to_conf(defs[i].str1);
                    break;
                case CHAZ_CONFELEM_DEF:
                case CHAZ_CONFELEM_TYPEDEF:
                case CHAZ_CONFELEM_LOCAL_INCLUDE:
                    /* no-op */
                    break;
                default:
                    chaz_Util_die("Internal error: bad element type %d",
                                  (int)defs[i].type);
            }
        }
        fprintf(chaz_ConfWriterC.fh, "\n#endif /* EMPLOY_%s */\n",
                chaz_ConfWriterC.MODULE_NAME);
    }

    fprintf(chaz_ConfWriterC.fh, "\n");

    free(chaz_ConfWriterC.MODULE_NAME);
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
    char *code_buf;
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

#line 17 "src/Charmonizer/Core/Make.c"
#include <ctype.h>
#include <string.h>
/* #include "Charmonizer/Core/Make.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */
/* #include "Charmonizer/Core/Util.h" */

struct chaz_MakeVar {
    char   *name;
    char   *value;
    size_t  num_elements;
};

struct chaz_MakeRule {
    char *targets;
    char *prereqs;
    char *commands;
};

struct chaz_MakeFile {
    chaz_MakeVar  **vars;
    size_t          num_vars;
    chaz_MakeRule **rules;
    size_t          num_rules;
    chaz_MakeRule  *clean;
    chaz_MakeRule  *distclean;
};

/* Static vars. */
static struct {
    char *make_command;
    int   is_gnu_make;
    int   is_nmake;
    int   shell_type;
} chaz_Make = {
    NULL,
    0, 0, 0
};

/* Detect make command.
 *
 * The argument list must be a NULL-terminated series of different spellings
 * of `make`, which will be auditioned in the order they are supplied.  Here
 * are several possibilities:
 *
 *      make
 *      gmake
 *      nmake
 *      dmake
 */
static int
chaz_Make_detect(const char *make1, ...);

static int
chaz_Make_audition(const char *make);

static chaz_MakeRule*
S_new_rule(const char *target, const char *prereq);

static void
S_destroy_rule(chaz_MakeRule *rule);

static void
S_write_rule(chaz_MakeRule *rule, FILE *out);

void
chaz_Make_init(void) {
    const char *make;

    chaz_Make_detect("make", "gmake", "nmake", "dmake", "mingw32-make",
                     "mingw64-make", NULL);
    make = chaz_Make.make_command;

    if (make) {
        if (strcmp(make, "make") == 0
            || strcmp(make, "gmake") == 0
            || strcmp(make, "mingw32-make") == 0
            || strcmp(make, "mingw64-make") == 0
           ) {
            /* TODO: Add a feature test for GNU make. */
            chaz_Make.is_gnu_make = 1;
            /* TODO: Feature test which shell GNU make uses on Windows. */
            chaz_Make.shell_type = CHAZ_OS_POSIX;
        }
        else if (strcmp(make, "nmake") == 0) {
            chaz_Make.is_nmake = 1;
            chaz_Make.shell_type = CHAZ_OS_CMD_EXE;
        }
    }
}

void
chaz_Make_clean_up(void) {
    free(chaz_Make.make_command);
}

const char*
chaz_Make_get_make(void) {
    return chaz_Make.make_command;
}

int
chaz_Make_shell_type(void) {
    return chaz_Make.shell_type;
}

static int
chaz_Make_detect(const char *make1, ...) {
    va_list args;
    const char *candidate;
    int found = 0;
    const char makefile_content[] = "foo:\n\techo \"foo!\"\n";
    chaz_Util_write_file("_charm_Makefile", makefile_content);

    /* Audition candidates. */
    found = chaz_Make_audition(make1);
    va_start(args, make1);
    while (!found && (NULL != (candidate = va_arg(args, const char*)))) {
        found = chaz_Make_audition(candidate);
    }
    va_end(args);

    chaz_Util_remove_and_verify("_charm_Makefile");

    return found;
}

static int
chaz_Make_audition(const char *make) {
    int succeeded = 0;
    char *command = chaz_Util_join(" ", make, "-f", "_charm_Makefile", NULL);

    chaz_Util_remove_and_verify("_charm_foo");
    chaz_OS_run_redirected(command, "_charm_foo");
    if (chaz_Util_can_open_file("_charm_foo")) {
        size_t len;
        char *content = chaz_Util_slurp_file("_charm_foo", &len);
        if (NULL != strstr(content, "foo!")) {
            succeeded = 1;
        }
        free(content);
    }
    chaz_Util_remove_and_verify("_charm_foo");

    if (succeeded) {
        chaz_Make.make_command = chaz_Util_strdup(make);
    }

    free(command);
    return succeeded;
}

chaz_MakeFile*
chaz_MakeFile_new() {
    chaz_MakeFile *makefile = (chaz_MakeFile*)malloc(sizeof(chaz_MakeFile));
    const char    *exe_ext  = chaz_OS_exe_ext();
    const char    *obj_ext  = chaz_CC_obj_ext();
    char *generated;

    makefile->vars = (chaz_MakeVar**)malloc(sizeof(chaz_MakeVar*));
    makefile->vars[0] = NULL;
    makefile->num_vars = 0;

    makefile->rules = (chaz_MakeRule**)malloc(sizeof(chaz_MakeRule*));
    makefile->rules[0] = NULL;
    makefile->num_rules = 0;

    makefile->clean     = S_new_rule("clean", NULL);
    makefile->distclean = S_new_rule("distclean", "clean");

    generated = chaz_Util_join("", "charmonizer", exe_ext, " charmonizer",
                               obj_ext, " charmony.h Makefile", NULL);
    chaz_MakeRule_add_rm_command(makefile->distclean, generated);

    free(generated);
    return makefile;
}

void
chaz_MakeFile_destroy(chaz_MakeFile *makefile) {
    size_t i;

    for (i = 0; makefile->vars[i]; i++) {
        chaz_MakeVar *var = makefile->vars[i];
        free(var->name);
        free(var->value);
        free(var);
    }
    free(makefile->vars);

    for (i = 0; makefile->rules[i]; i++) {
        S_destroy_rule(makefile->rules[i]);
    }
    free(makefile->rules);

    S_destroy_rule(makefile->clean);
    S_destroy_rule(makefile->distclean);

    free(makefile);
}

chaz_MakeVar*
chaz_MakeFile_add_var(chaz_MakeFile *makefile, const char *name,
                      const char *value) {
    chaz_MakeVar  *var      = (chaz_MakeVar*)malloc(sizeof(chaz_MakeVar));
    chaz_MakeVar **vars     = makefile->vars;
    size_t         num_vars = makefile->num_vars + 1;

    var->name         = chaz_Util_strdup(name);
    var->value        = chaz_Util_strdup("");
    var->num_elements = 0;

    if (value) { chaz_MakeVar_append(var, value); }

    vars = (chaz_MakeVar**)realloc(vars,
                                   (num_vars + 1) * sizeof(chaz_MakeVar*));
    vars[num_vars-1] = var;
    vars[num_vars]   = NULL;
    makefile->vars = vars;
    makefile->num_vars = num_vars;

    return var;
}

chaz_MakeRule*
chaz_MakeFile_add_rule(chaz_MakeFile *makefile, const char *target,
                       const char *prereq) {
    chaz_MakeRule  *rule      = S_new_rule(target, prereq);
    chaz_MakeRule **rules     = makefile->rules;
    size_t          num_rules = makefile->num_rules + 1;

    rules = (chaz_MakeRule**)realloc(rules,
                                     (num_rules + 1) * sizeof(chaz_MakeRule*));
    rules[num_rules-1] = rule;
    rules[num_rules]   = NULL;
    makefile->rules = rules;
    makefile->num_rules = num_rules;

    return rule;
}

chaz_MakeRule*
chaz_MakeFile_clean_rule(chaz_MakeFile *makefile) {
    return makefile->clean;
}

chaz_MakeRule*
chaz_MakeFile_distclean_rule(chaz_MakeFile *makefile) {
    return makefile->distclean;
}

chaz_MakeRule*
chaz_MakeFile_add_exe(chaz_MakeFile *makefile, const char *exe,
                      const char *sources, chaz_CFlags *link_flags) {
    chaz_CFlags   *local_flags  = chaz_CC_new_cflags();
    const char    *link         = chaz_CC_link_command();
    const char    *link_flags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *command;

    rule = chaz_MakeFile_add_rule(makefile, exe, sources);

    if (link_flags) {
        link_flags_string = chaz_CFlags_get_string(link_flags);
    }
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_set_link_output(local_flags, exe);
    local_flags_string = chaz_CFlags_get_string(local_flags);
    command = chaz_Util_join(" ", link, sources, link_flags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(makefile->clean, exe);

    chaz_CFlags_destroy(local_flags);
    free(command);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_compiled_exe(chaz_MakeFile *makefile, const char *exe,
                               const char *sources, chaz_CFlags *cflags) {
    chaz_CFlags   *local_flags   = chaz_CC_new_cflags();
    const char    *cc            = chaz_CC_get_cc();
    const char    *cflags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *command;

    rule = chaz_MakeFile_add_rule(makefile, exe, sources);

    if (cflags) {
        cflags_string = chaz_CFlags_get_string(cflags);
    }
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_set_output_exe(local_flags, exe);
    local_flags_string = chaz_CFlags_get_string(local_flags);
    command = chaz_Util_join(" ", cc, sources, cflags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(makefile->clean, exe);
    /* TODO: Clean .obj file on Windows. */

    chaz_CFlags_destroy(local_flags);
    free(command);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_shared_lib(chaz_MakeFile *makefile, chaz_SharedLib *lib,
                             const char *sources, chaz_CFlags *link_flags) {
    chaz_CFlags   *local_flags  = chaz_CC_new_cflags();
    const char    *link         = chaz_CC_link_command();
    const char    *shlib_ext    = chaz_OS_shared_lib_ext();
    const char    *link_flags_string = "";
    const char    *local_flags_string;
    chaz_MakeRule *rule;
    char          *filename;
    char          *command;

    filename = chaz_SharedLib_filename(lib);
    rule = chaz_MakeFile_add_rule(makefile, filename, sources);

    if (link_flags) {
        link_flags_string = chaz_CFlags_get_string(link_flags);
    }

    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(local_flags, "/nologo");
    }
    chaz_CFlags_link_shared_library(local_flags);
    if (strcmp(shlib_ext, ".dylib") == 0) {
        /* Set temporary install name with full path on Darwin. */
        const char *dir_sep = chaz_OS_dir_sep();
        char *major_v_name = chaz_SharedLib_major_version_filename(lib);
        char *install_name = chaz_Util_join("", "-install_name $(CURDIR)",
                                            dir_sep, major_v_name, NULL);
        chaz_CFlags_append(local_flags, install_name);
        free(major_v_name);
        free(install_name);
    }
    chaz_CFlags_set_shared_library_version(local_flags, lib);
    chaz_CFlags_set_link_output(local_flags, filename);
    local_flags_string = chaz_CFlags_get_string(local_flags);

    command = chaz_Util_join(" ", link, sources, link_flags_string,
                             local_flags_string, NULL);
    chaz_MakeRule_add_command(rule, command);
    free(command);

    chaz_MakeRule_add_rm_command(makefile->clean, filename);

    /* Add symlinks. */
    if (strcmp(shlib_ext, ".dll") != 0) {
        char *major_v_name = chaz_SharedLib_major_version_filename(lib);
        char *no_v_name    = chaz_SharedLib_no_version_filename(lib);

        command = chaz_Util_join(" ", "ln -sf", filename, major_v_name, NULL);
        chaz_MakeRule_add_command(rule, command);
        free(command);

        if (strcmp(shlib_ext, ".dylib") == 0) {
            command = chaz_Util_join(" ", "ln -sf", filename, no_v_name,
                                     NULL);
        }
        else {
            command = chaz_Util_join(" ", "ln -sf", major_v_name, no_v_name,
                                     NULL);
        }
        chaz_MakeRule_add_command(rule, command);
        free(command);

        chaz_MakeRule_add_rm_command(makefile->clean, major_v_name);
        chaz_MakeRule_add_rm_command(makefile->clean, no_v_name);

        free(major_v_name);
        free(no_v_name);
    }

    if (chaz_CC_msvc_version_num()) {
        /* Remove import library and export file under MSVC. */
        char *lib_filename = chaz_SharedLib_implib_filename(lib);
        char *exp_filename = chaz_SharedLib_export_filename(lib);
        chaz_MakeRule_add_rm_command(makefile->clean, lib_filename);
        chaz_MakeRule_add_rm_command(makefile->clean, exp_filename);
        free(lib_filename);
        free(exp_filename);
    }

    chaz_CFlags_destroy(local_flags);
    free(filename);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_lemon_exe(chaz_MakeFile *makefile, const char *dir) {
    chaz_CFlags   *cflags = chaz_CC_new_cflags();
    chaz_MakeRule *rule;
    const char *dir_sep = chaz_OS_dir_sep();
    const char *exe_ext = chaz_OS_exe_ext();
    char *lemon_exe = chaz_Util_join("", dir, dir_sep, "lemon", exe_ext, NULL);
    char *lemon_c   = chaz_Util_join(dir_sep, dir, "lemon.c", NULL);

    chaz_CFlags_enable_optimization(cflags);
    chaz_MakeFile_add_var(makefile, "LEMON_EXE", lemon_exe);
    rule = chaz_MakeFile_add_compiled_exe(makefile, "$(LEMON_EXE)", lemon_c,
                                          cflags);

    chaz_CFlags_destroy(cflags);
    free(lemon_c);
    free(lemon_exe);
    return rule;
}

chaz_MakeRule*
chaz_MakeFile_add_lemon_grammar(chaz_MakeFile *makefile,
                                const char *base_name) {
    char *c_file  = chaz_Util_join(".", base_name, "c", NULL);
    char *h_file  = chaz_Util_join(".", base_name, "h", NULL);
    char *y_file  = chaz_Util_join(".", base_name, "y", NULL);
    char *command = chaz_Util_join(" ", "$(LEMON_EXE) -q", y_file, NULL);

    chaz_MakeRule *rule = chaz_MakeFile_add_rule(makefile, c_file, y_file);
    chaz_MakeRule *clean_rule = chaz_MakeFile_clean_rule(makefile);

    chaz_MakeRule_add_prereq(rule, "$(LEMON_EXE)");
    chaz_MakeRule_add_command(rule, command);

    chaz_MakeRule_add_rm_command(clean_rule, h_file);
    chaz_MakeRule_add_rm_command(clean_rule, c_file);

    free(c_file);
    free(h_file);
    free(y_file);
    free(command);
    return rule;
}

void
chaz_MakeFile_write(chaz_MakeFile *makefile) {
    FILE   *out;
    size_t  i;

    out = fopen("Makefile", "w");
    if (!out) {
        chaz_Util_die("Can't open Makefile\n");
    }

    for (i = 0; makefile->vars[i]; i++) {
        chaz_MakeVar *var = makefile->vars[i];
        fprintf(out, "%s = %s\n", var->name, var->value);
    }
    fprintf(out, "\n");

    for (i = 0; makefile->rules[i]; i++) {
        S_write_rule(makefile->rules[i], out);
    }

    S_write_rule(makefile->clean, out);
    S_write_rule(makefile->distclean, out);

    if (chaz_Make.is_nmake) {
        /* Inference rule for .c files. */
        if (chaz_CC_msvc_version_num()) {
            fprintf(out, ".c.obj :\n");
            fprintf(out, "\t$(CC) /nologo $(CFLAGS) /c $< /Fo$@\n\n");
        }
        else {
            fprintf(out, ".c.o :\n");
            fprintf(out, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");
        }
    }

    fclose(out);
}

void
chaz_MakeVar_append(chaz_MakeVar *var, const char *element) {
    char *value;

    if (element[0] == '\0') { return; }

    if (var->num_elements == 0) {
        value = chaz_Util_strdup(element);
    }
    else {
        value = (char*)malloc(strlen(var->value) + strlen(element) + 20);

        if (var->num_elements == 1) {
            sprintf(value, "\\\n    %s \\\n    %s", var->value, element);
        }
        else {
            sprintf(value, "%s \\\n    %s", var->value, element);
        }
    }

    free(var->value);
    var->value = value;
    var->num_elements++;
}

static chaz_MakeRule*
S_new_rule(const char *target, const char *prereq) {
    chaz_MakeRule *rule = (chaz_MakeRule*)malloc(sizeof(chaz_MakeRule));

    rule->targets  = NULL;
    rule->prereqs  = NULL;
    rule->commands = NULL;

    if (target) { chaz_MakeRule_add_target(rule, target); }
    if (prereq) { chaz_MakeRule_add_prereq(rule, prereq); }

    return rule;
}

static void
S_destroy_rule(chaz_MakeRule *rule) {
    if (rule->targets)  { free(rule->targets); }
    if (rule->prereqs)  { free(rule->prereqs); }
    if (rule->commands) { free(rule->commands); }
    free(rule);
}

static void
S_write_rule(chaz_MakeRule *rule, FILE *out) {
    fprintf(out, "%s :", rule->targets);
    if (rule->prereqs) {
        fprintf(out, " %s", rule->prereqs);
    }
    fprintf(out, "\n");
    if (rule->commands) {
        fprintf(out, "%s", rule->commands);
    }
    fprintf(out, "\n");
}

void
chaz_MakeRule_add_target(chaz_MakeRule *rule, const char *target) {
    char *targets;

    if (!rule->targets) {
        targets = chaz_Util_strdup(target);
    }
    else {
        targets = chaz_Util_join(" ", rule->targets, target, NULL);
        free(rule->targets);
    }

    rule->targets = targets;
}

void
chaz_MakeRule_add_prereq(chaz_MakeRule *rule, const char *prereq) {
    char *prereqs;

    if (!rule->prereqs) {
        prereqs = chaz_Util_strdup(prereq);
    }
    else {
        prereqs = chaz_Util_join(" ", rule->prereqs, prereq, NULL);
        free(rule->prereqs);
    }

    rule->prereqs = prereqs;
}

void
chaz_MakeRule_add_command(chaz_MakeRule *rule, const char *command) {
    char *commands;

    if (!rule->commands) {
        commands = (char*)malloc(strlen(command) + 20);
        sprintf(commands, "\t%s\n", command);
    }
    else {
        commands = (char*)malloc(strlen(rule->commands) + strlen(command) + 20);
        sprintf(commands, "%s\t%s\n", rule->commands, command);
        free(rule->commands);
    }

    rule->commands = commands;
}

void
chaz_MakeRule_add_rm_command(chaz_MakeRule *rule, const char *files) {
    char *command;

    if (chaz_Make.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rm -f", files, NULL);
    }
    else if (chaz_Make.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join("", "for %i in (", files,
                                 ") do @if exist %i del /f %i", NULL);
    }
    else {
        chaz_Util_die("Unsupported shell type: %d", chaz_Make.shell_type);
    }

    chaz_MakeRule_add_command(rule, command);
    free(command);
}

void
chaz_MakeRule_add_recursive_rm_command(chaz_MakeRule *rule, const char *dirs) {
    char *command;

    if (chaz_Make.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rm -rf", dirs, NULL);
    }
    else if (chaz_Make.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join("", "for %i in (", dirs,
                                 ") do @if exist %i rmdir /s /q %i", NULL);
    }
    else {
        chaz_Util_die("Unsupported shell type: %d", chaz_Make.shell_type);
    }

    chaz_MakeRule_add_command(rule, command);
    free(command);
}

void
chaz_MakeRule_add_make_command(chaz_MakeRule *rule, const char *dir,
                               const char *target) {
    char *command;

    if (chaz_Make.is_gnu_make) {
        if (!target) {
            command = chaz_Util_join(" ", "$(MAKE)", "-C", dir, NULL);
        }
        else {
            command = chaz_Util_join(" ", "$(MAKE)", "-C", dir, target, NULL);
        }
        chaz_MakeRule_add_command(rule, command);
        free(command);
    }
    else if (chaz_Make.is_nmake) {
        command = chaz_Util_join(" ", "cd", dir, NULL);
        chaz_MakeRule_add_command(rule, command);
        free(command);

        if (!target) {
            chaz_MakeRule_add_command(rule, "$(MAKE) /nologo");
        }
        else {
            command = chaz_Util_join(" ", "$(MAKE) /nologo", target, NULL);
            chaz_MakeRule_add_command(rule, command);
            free(command);
        }

        chaz_MakeRule_add_command(rule, "cd $(MAKEDIR)");
    }
    else {
        chaz_Util_die("Couldn't find a supported 'make' utility.");
    }
}

void
chaz_Make_list_files(const char *dir, const char *ext,
                     chaz_Make_list_files_callback_t callback, void *context) {
    int         shell_type = chaz_OS_shell_type();
    const char *pattern;
    char       *command;
    char       *list;
    char       *prefix;
    char       *file;
    size_t      command_size;
    size_t      list_len;
    size_t      prefix_len;

    /* List files using shell. */

    if (shell_type == CHAZ_OS_POSIX) {
        pattern = "find %s -name '*.%s' -type f";
    }
    else if (shell_type == CHAZ_OS_CMD_EXE) {
        pattern = "dir %s\\*.%s /s /b /a-d";
    }
    else {
        chaz_Util_die("Unknown shell type %d", shell_type);
    }

    command_size = strlen(pattern) + strlen(dir) + strlen(ext) + 10;
    command = (char*)malloc(command_size);
    sprintf(command, pattern, dir, ext);
    list = chaz_OS_run_and_capture(command, &list_len);
    free(command);
    if (!list) {
        chaz_Util_die("Failed to list files in '%s'", dir);
    }
    list[list_len-1] = 0;

    /* Find directory prefix to strip from files */

    if (shell_type == CHAZ_OS_POSIX) {
        prefix_len = strlen(dir);
        prefix = (char*)malloc(prefix_len + 2);
        memcpy(prefix, dir, prefix_len);
        prefix[prefix_len++] = '/';
        prefix[prefix_len]   = '\0';
    }
    else {
        char   *output;
        size_t  output_len;

        /* 'dir /s' returns absolute paths, so we have to find the absolute
         * path of the directory. This is done by using the variable
         * substitution feature of the 'for' command.
         */
        pattern = "for %%I in (%s) do @echo %%~fI";
        command_size = strlen(pattern) + strlen(dir) + 10;
        command = (char*)malloc(command_size);
        sprintf(command, pattern, dir);
        output = chaz_OS_run_and_capture(command, &output_len);
        free(command);
        if (!output) { chaz_Util_die("Failed to find absolute path"); }

        /* Strip whitespace from end of output. */
        for (prefix_len = output_len; prefix_len > 0; --prefix_len) {
            if (!isspace(output[prefix_len-1])) { break; }
        }
        prefix = (char*)malloc(prefix_len + 2);
        memcpy(prefix, output, prefix_len);
        prefix[prefix_len++] = '\\';
        prefix[prefix_len]   = '\0';
        free(output);
    }

    /* Iterate file list and invoke callback. */

    for (file = strtok(list, "\r\n"); file; file = strtok(NULL, "\r\n")) {
        if (strlen(file) <= prefix_len
            || memcmp(file, prefix, prefix_len) != 0
           ) {
            chaz_Util_die("Expected prefix '%s' for file name '%s'", prefix,
                          file);
        }

        callback(dir, file + prefix_len, context);
    }

    free(prefix);
    free(list);
}


/***************************************************************************/

#line 17 "src/Charmonizer/Core/OperatingSystem.c"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

#define CHAZ_OS_TARGET_PATH  "_charmonizer_target"
#define CHAZ_OS_NAME_MAX     31

static struct {
    char name[CHAZ_OS_NAME_MAX+1];
    char dev_null[20];
    char dir_sep[2];
    char exe_ext[5];
    char shared_lib_ext[7];
    char local_command_start[3];
    int  shell_type;
} chaz_OS = { "", "", "", "", "", "", 0 };

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
        char   *uname;
        size_t  uname_len;
        size_t i;

        chaz_OS.shell_type = CHAZ_OS_POSIX;

        /* Detect Unix name. */
        uname = chaz_OS_run_and_capture("uname", &uname_len);
        for (i = 0; i < CHAZ_OS_NAME_MAX && i < uname_len; i++) {
            char c = uname[i];
            if (!c || isspace(c)) { break; }
            chaz_OS.name[i] = tolower(c);
        }
        if (i > 0) { chaz_OS.name[i] = '\0'; }
        else       { strcpy(chaz_OS.name, "unknown_unix"); }
        free(uname);

        strcpy(chaz_OS.dev_null, "/dev/null");
        strcpy(chaz_OS.dir_sep, "/");
        strcpy(chaz_OS.exe_ext, "");
        if (memcmp(chaz_OS.name, "darwin", 6) == 0) {
            strcpy(chaz_OS.shared_lib_ext, ".dylib");
        }
        else if (memcmp(chaz_OS.name, "cygwin", 6) == 0) {
            strcpy(chaz_OS.shared_lib_ext, ".dll");
        }
        else {
            strcpy(chaz_OS.shared_lib_ext, ".so");
        }
        strcpy(chaz_OS.local_command_start, "./");
    }
    else if (chaz_Util_can_open_file("nul")) {
        strcpy(chaz_OS.name, "windows");
        strcpy(chaz_OS.dev_null, "nul");
        strcpy(chaz_OS.dir_sep, "\\");
        strcpy(chaz_OS.exe_ext, ".exe");
        strcpy(chaz_OS.shared_lib_ext, ".dll");
        strcpy(chaz_OS.local_command_start, ".\\");
        chaz_OS.shell_type = CHAZ_OS_CMD_EXE;
    }
    else {
        /* Bail out because we couldn't find anything like /dev/null. */
        chaz_Util_die("Couldn't find anything like /dev/null");
    }
}

const char*
chaz_OS_name(void) {
    return chaz_OS.name;
}

int
chaz_OS_is_darwin(void) {
    return memcmp(chaz_OS.name, "darwin", 6) == 0;
}

int
chaz_OS_is_cygwin(void) {
    return memcmp(chaz_OS.name, "cygwin", 6) == 0;
}

const char*
chaz_OS_exe_ext(void) {
    return chaz_OS.exe_ext;
}

const char*
chaz_OS_shared_lib_ext(void) {
    return chaz_OS.shared_lib_ext;
}

const char*
chaz_OS_dev_null(void) {
    return chaz_OS.dev_null;
}

const char*
chaz_OS_dir_sep(void) {
    return chaz_OS.dir_sep;
}

int
chaz_OS_shell_type(void) {
    return chaz_OS.shell_type;
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
chaz_OS_run_local_redirected(const char *command, const char *path) {
    char *local_command
        = chaz_Util_join("", chaz_OS.local_command_start, command, NULL);
    int retval = chaz_OS_run_redirected(local_command, path);
    free(local_command);
    return retval;
}

int
chaz_OS_run_quietly(const char *command) {
    return chaz_OS_run_redirected(command, chaz_OS.dev_null);
}

int
chaz_OS_run_redirected(const char *command, const char *path) {
    int retval = 1;
    char *quiet_command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
        ) {
        quiet_command = chaz_Util_join(" ", command, ">", path, "2>&1", NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    retval = system(quiet_command);
    free(quiet_command);
    return retval;
}

char*
chaz_OS_run_and_capture(const char *command, size_t *output_len) {
    char *output;
    chaz_OS_run_redirected(command, CHAZ_OS_TARGET_PATH);
    output = chaz_Util_slurp_file(CHAZ_OS_TARGET_PATH, output_len);
    chaz_Util_remove_and_verify(CHAZ_OS_TARGET_PATH);
    return output;
}

void
chaz_OS_mkdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
       ) {
        command = chaz_Util_join(" ", "mkdir", filepath, NULL);
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
        command = chaz_Util_join(" ", "rmdir", filepath, NULL);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join(" ", "rmdir", "/q", filepath, NULL);
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

char*
chaz_Util_join(const char *sep, ...) {
    va_list args;
    const char *string;
    char *result, *p;
    size_t sep_len = strlen(sep);
    size_t size;
    int i;

    /* Determine result size. */
    va_start(args, sep);
    size = 1;
    string = va_arg(args, const char*);
    for (i = 0; string; ++i) {
        if (i != 0) { size += sep_len; }
        size += strlen(string);
        string = va_arg(args, const char*);
    }
    va_end(args);

    result = (char*)malloc(size);

    /* Create result string. */
    va_start(args, sep);
    p = result;
    string = va_arg(args, const char*);
    for (i = 0; string; ++i) {
        size_t len;
        if (i != 0) {
            memcpy(p, sep, sep_len);
            p += sep_len;
        }
        len = strlen(string);
        memcpy(p, string, len);
        p += len;
        string = va_arg(args, const char*);
    }
    va_end(args);

    *p = '\0';

    return result;
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
/* #include "Charmonizer/Core/ConfWriterC.h" */
/* #include "Charmonizer/Core/ConfWriterPerl.h" */
/* #include "Charmonizer/Core/ConfWriterRuby.h" */
/* #include "Charmonizer/Core/Util.h" */
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/OperatingSystem.h" */

int
chaz_Probe_parse_cli_args(int argc, const char *argv[],
                          struct chaz_CLIArgs *args) {
    int i;
    int output_enabled = 0;

    /* Zero out args struct. */
    memset(args, 0, sizeof(struct chaz_CLIArgs));

    /* Parse most args. */
    for (i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strcmp(arg, "--") == 0) {
            /* From here on out, everything will be a compiler flag. */
            i++;
            break;
        }
        if (strcmp(arg, "--enable-c") == 0) {
            args->charmony_h = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-perl") == 0) {
            args->charmony_pm = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-ruby") == 0) {
            args->charmony_rb = 1;
            output_enabled = 1;
        }
        else if (strcmp(arg, "--enable-makefile") == 0) {
            args->write_makefile = 1;
        }
        else if (strcmp(arg, "--enable-coverage") == 0) {
            args->code_coverage = 1;
        }
        else if (memcmp(arg, "--cc=", 5) == 0) {
            if (strlen(arg) > CHAZ_PROBE_MAX_CC_LEN - 5) {
                fprintf(stderr, "Exceeded max length for compiler command");
                exit(1);
            }
            strcpy(args->cc, arg + 5);
        }
    } /* preserve value of i */

    /* Accumulate compiler flags. */
    for (; i < argc; i++) {
        const char *arg = argv[i];
        size_t new_len = strlen(arg) + strlen(args->cflags) + 2;
        if (new_len >= CHAZ_PROBE_MAX_CFLAGS_LEN) {
            fprintf(stderr, "Exceeded max length for compiler flags");
            exit(1);
        }
        strcat(args->cflags, " ");
        strcat(args->cflags, arg);
    }

    /* Process CHARM_VERBOSITY environment variable. */
    {
        const char *verbosity_env = getenv("CHARM_VERBOSITY");
        if (verbosity_env && strlen(verbosity_env)) {
            args->verbosity = strtol(verbosity_env, NULL, 10);
        }
    }

    /* Validate. */
    if (!strlen(args->cc) || !output_enabled) {
        return false;
    }

    return true;
}

void
chaz_Probe_die_usage(void) {
    fprintf(stderr,
            "Usage: ./charmonize --cc=CC_COMMAND [--enable-c] "
            "[--enable-perl] [--enable-ruby] -- CFLAGS\n");
    exit(1);
}

void
chaz_Probe_init(struct chaz_CLIArgs *args) {
    int output_enabled = 0;

    {
        /* Process CHARM_VERBOSITY environment variable. */
        const char *verbosity_env = getenv("CHARM_VERBOSITY");
        if (verbosity_env && strlen(verbosity_env)) {
            chaz_Util_verbosity = strtol(verbosity_env, NULL, 10);
        }
    }

    /* Dispatch other initializers. */
    chaz_OS_init();
    chaz_CC_init(args->cc, args->cflags);
    chaz_ConfWriter_init();
    chaz_HeadCheck_init();
    chaz_Make_init();

    /* Enable output. */
    if (args->charmony_h) {
        chaz_ConfWriterC_enable();
        output_enabled = true;
    }
    if (args->charmony_pm) {
        chaz_ConfWriterPerl_enable();
        output_enabled = true;
    }
    if (args->charmony_rb) {
        chaz_ConfWriterRuby_enable();
        output_enabled = true;
    }
    if (!output_enabled) {
        fprintf(stderr, "No output formats enabled\n");
        exit(1);
    }

    if (chaz_Util_verbosity) { printf("Initialization complete.\n"); }
}

void
chaz_Probe_clean_up(void) {
    if (chaz_Util_verbosity) { printf("Cleaning up...\n"); }

    /* Dispatch various clean up routines. */
    chaz_ConfWriter_clean_up();
    chaz_CC_clean_up();
    chaz_Make_clean_up();

    if (chaz_Util_verbosity) { printf("Cleanup complete.\n"); }
}

int
chaz_Probe_gcc_version_num(void) {
    return chaz_CC_gcc_version_num();
}

const char*
chaz_Probe_gcc_version(void) {
    return chaz_CC_gcc_version_num() ? chaz_CC_gcc_version() : NULL;
}

int
chaz_Probe_msvc_version_num(void) {
    return chaz_CC_msvc_version_num();
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

#line 17 "src/Charmonizer/Probe/Booleans.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Probe/Booleans.h" */

void
chaz_Booleans_run(void) {
    int has_stdbool = chaz_HeadCheck_check_header("stdbool.h");

    chaz_ConfWriter_start_module("Booleans");

    if (has_stdbool) {
        chaz_ConfWriter_add_def("HAS_STDBOOL_H", NULL);
        chaz_ConfWriter_add_sys_include("stdbool.h");
    }
    else {
        chaz_ConfWriter_append_conf(
            "#if (defined(CHY_EMPLOY_BOOLEANS) && !defined(__cplusplus))\n"
            "  typedef int bool;\n"
            "  #ifndef true\n"
            "    #define true 1\n"
            "  #endif\n"
            "  #ifndef false\n"
            "    #define false 0\n"
            "  #endif\n"
            "#endif\n");
    }

    chaz_ConfWriter_end_module();
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/BuildEnv.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Probe/BuildEnv.h" */

void
chaz_BuildEnv_run(void) {
    chaz_CFlags *extra_cflags = chaz_CC_get_extra_cflags();
    const char  *extra_cflags_string = chaz_CFlags_get_string(extra_cflags);

    chaz_ConfWriter_start_module("BuildEnv");

    chaz_ConfWriter_add_def("CC", chaz_CC_get_cc());
    chaz_ConfWriter_add_def("CFLAGS", chaz_CC_get_cflags());
    chaz_ConfWriter_add_def("EXTRA_CFLAGS", extra_cflags_string);

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

void
chaz_DirManip_run(void) {
    const char *dir_sep = chaz_OS_dir_sep();
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

    if (strcmp(dir_sep, "\\") == 0) {
        chaz_ConfWriter_add_def("DIR_SEP", "\"\\\\\"");
        chaz_ConfWriter_add_def("DIR_SEP_CHAR", "'\\\\'");
    }
    else {
        char scratch[5];
        sprintf(scratch, "\"%s\"", dir_sep);
        chaz_ConfWriter_add_def("DIR_SEP", scratch);
        sprintf(scratch, "'%s'", dir_sep);
        chaz_ConfWriter_add_def("DIR_SEP_CHAR", scratch);
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
        "typedef union { unsigned char c[4]; float f; } chy_floatu32;\n"
        "typedef union { unsigned char c[8]; double d; } chy_floatu64;\n"
        "#ifdef CHY_BIG_END\n"
        "static const chy_floatu32 chy_f32inf\n"
        "    = { { 0x7F, 0x80, 0, 0 } };\n"
        "static const chy_floatu32 chy_f32neginf\n"
        "    = { { 0xFF, 0x80, 0, 0 } };\n"
        "static const chy_floatu32 chy_f32nan\n"
        "    = { { 0x7F, 0xC0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64inf\n"
        "    = { { 0x7F, 0xF0, 0, 0, 0, 0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64neginf\n"
        "    = { { 0xFF, 0xF0, 0, 0, 0, 0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64nan\n"
        "    = { { 0x7F, 0xF8, 0, 0, 0, 0, 0, 0 } };\n"
        "#else /* BIG_END */\n"
        "static const chy_floatu32 chy_f32inf\n"
        "    = { { 0, 0, 0x80, 0x7F } };\n"
        "static const chy_floatu32 chy_f32neginf\n"
        "    = { { 0, 0, 0x80, 0xFF } };\n"
        "static const chy_floatu32 chy_f32nan\n"
        "    = { { 0, 0, 0xC0, 0x7F } };\n"
        "static const chy_floatu64 chy_f64inf\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF0, 0x7F } };\n"
        "static const chy_floatu64 chy_f64neginf\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF0, 0xFF } };\n"
        "static const chy_floatu64 chy_f64nan\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF8, 0x7F } };\n"
        "#endif /* BIG_END */\n"
    );
    chaz_ConfWriter_add_def("F32_INF", "(chy_f32inf.f)");
    chaz_ConfWriter_add_def("F32_NEGINF", "(chy_f32neginf.f)");
    chaz_ConfWriter_add_def("F32_NAN", "(chy_f32nan.f)");
    chaz_ConfWriter_add_def("F64_INF", "(chy_f64inf.d)");
    chaz_ConfWriter_add_def("F64_NEGINF", "(chy_f64neginf.d)");
    chaz_ConfWriter_add_def("F64_NAN", "(chy_f64nan.d)");

    chaz_ConfWriter_end_module();
}

const char*
chaz_Floats_math_library(void) {
    static const char sqrt_code[] =
        CHAZ_QUOTE(  #include <math.h>                              )
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  typedef double (*sqrt_t)(double);              )
        CHAZ_QUOTE(  int main(void) {                               )
        CHAZ_QUOTE(      printf("%p\n", (sqrt_t)sqrt);              )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    chaz_CFlags *temp_cflags = chaz_CC_get_temp_cflags();
    char        *output = NULL;
    size_t       output_len;

    output = chaz_CC_capture_output(sqrt_code, &output_len);
    if (output != NULL) {
        /* Linking against libm not needed. */
        free(output);
        return NULL;
    }

    chaz_CFlags_add_external_library(temp_cflags, "m");
    output = chaz_CC_capture_output(sqrt_code, &output_len);
    chaz_CFlags_clear(temp_cflags);

    if (output == NULL) {
        chaz_Util_die("Don't know how to use math library.");
    }

    free(output);
    return "m";
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
        CHAZ_QUOTE(  #include <stdio.h>                )
        CHAZ_QUOTE(  int main() {                      )
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
        CHAZ_QUOTE(  #include <stdio.h>                )
        CHAZ_QUOTE(  int main() {                      )
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
        CHAZ_QUOTE(  #include <stdio.h>                )
        CHAZ_QUOTE(  static %s int foo() { return 1; } )
        CHAZ_QUOTE(  int main() {                      )
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
        "regex.h",
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
    CHAZ_QUOTE(  #include <stdio.h>                        )
    CHAZ_QUOTE(  int main () {                             )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(char));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(short));    )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(int));      )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(long));     )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(void*));    )
    CHAZ_QUOTE(      printf("%d ", (int)sizeof(size_t));   )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_type64_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                        )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      printf("%%d", (int)sizeof(%s));       )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_literal64_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                        )
    CHAZ_QUOTE(  #define big 9000000000000000000%s         )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      int truncated = (int)big;             )
    CHAZ_QUOTE(      printf("%%d\n", truncated);           )
    CHAZ_QUOTE(      return 0;                             )
    CHAZ_QUOTE(  }                                         );

static const char chaz_Integers_u64_to_double_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                        )
    CHAZ_QUOTE(  int main()                                )
    CHAZ_QUOTE(  {                                         )
    CHAZ_QUOTE(      unsigned %s int_num = 0;              )
    CHAZ_QUOTE(      double float_num;                     )
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
    int sizeof_size_t     = -1;
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
        char *ptr     = output;
        char *end_ptr = output;

        sizeof_char   = strtol(ptr, &end_ptr, 10);
        ptr           = end_ptr;
        sizeof_short  = strtol(ptr, &end_ptr, 10);
        ptr           = end_ptr;
        sizeof_int    = strtol(ptr, &end_ptr, 10);
        ptr           = end_ptr;
        sizeof_long   = strtol(ptr, &end_ptr, 10);
        ptr           = end_ptr;
        sizeof_ptr    = strtol(ptr, &end_ptr, 10);
        ptr           = end_ptr;
        sizeof_size_t = strtol(ptr, &end_ptr, 10);

        free(output);
    }

    /* Determine whether long longs are available. */
    sprintf(code_buf, chaz_Integers_type64_code, "long long");
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        has_long_long    = true;
        sizeof_long_long = strtol(output, NULL, 10);
        free(output);
    }

    /* Determine whether the __int64 type is available. */
    sprintf(code_buf, chaz_Integers_type64_code, "__int64");
    output = chaz_CC_capture_output(code_buf, &output_len);
    if (output != NULL) {
        has___int64 = true;
        sizeof___int64 = strtol(output, NULL, 10);
        free(output);
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
            free(output);
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "i64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(i64_t_postfix, "i64");
                free(output);
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
        }
        sprintf(code_buf, chaz_Integers_literal64_code, "ULL");
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            strcpy(u64_t_postfix, "ULL");
            free(output);
        }
        else {
            sprintf(code_buf, chaz_Integers_literal64_code, "Ui64");
            output = chaz_CC_capture_output(code_buf, &output_len);
            if (output != NULL) {
                strcpy(u64_t_postfix, "Ui64");
                free(output);
            }
            else {
                chaz_Util_die("64-bit types, but no literal syntax found");
            }
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
    sprintf(scratch, "%d", sizeof_size_t);
    chaz_ConfWriter_add_def("SIZEOF_SIZE_T", scratch);
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
    if (has_64) {
        /*
         * Determine whether unsigned 64-bit integers can be converted to
         * double. Older MSVC versions don't support this conversion.
         */
        sprintf(code_buf, chaz_Integers_u64_to_double_code, i64_t_type);
        output = chaz_CC_capture_output(code_buf, &output_len);
        if (output != NULL) {
            chaz_ConfWriter_add_def("U64_TO_DOUBLE(num)", "((double)(num))");
            free(output);
        }
        else {
            chaz_ConfWriter_add_def(
                "U64_TO_DOUBLE(num)",
                "((num) & UINT64_C(0x8000000000000000) ? "
                "(double)(int64_t)((num) & UINT64_C(0x7FFFFFFFFFFFFFFF)) + "
                "9223372036854775808.0 : "
                "(double)(int64_t)(num))");
        }
    }

    chaz_ConfWriter_end_module();

    /* Integer typedefs. */

    chaz_ConfWriter_start_module("IntegerTypes");

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
         */
        if (has_8) {
            chaz_ConfWriter_add_global_typedef("signed char", "int8_t");
            chaz_ConfWriter_add_global_typedef("unsigned char", "uint8_t");
        }
        if (has_16) {
            chaz_ConfWriter_add_global_typedef("signed short", "int16_t");
            chaz_ConfWriter_add_global_typedef("unsigned short", "uint16_t");
        }
        if (has_32) {
            chaz_ConfWriter_add_global_typedef(i32_t_type, "int32_t");
            sprintf(scratch, "unsigned %s", i32_t_type);
            chaz_ConfWriter_add_global_typedef(scratch, "uint32_t");
        }
        if (has_64) {
            chaz_ConfWriter_add_global_typedef(i64_t_type, "int64_t");
            sprintf(scratch, "unsigned %s", i64_t_type);
            chaz_ConfWriter_add_global_typedef(scratch, "uint64_t");
        }
    }

    chaz_ConfWriter_end_module();

    /* Integer limits. */

    chaz_ConfWriter_start_module("IntegerLimits");

    if (has_stdint) {
        chaz_ConfWriter_add_sys_include("stdint.h");
    }
    else {
        /* We support only the following subset of stdint.h
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
         */
        if (has_8) {
            chaz_ConfWriter_add_global_def("INT8_MAX", "127");
            chaz_ConfWriter_add_global_def("INT8_MIN", "-128");
            chaz_ConfWriter_add_global_def("UINT8_MAX", "255");
        }
        if (has_16) {
            chaz_ConfWriter_add_global_def("INT16_MAX", "32767");
            chaz_ConfWriter_add_global_def("INT16_MIN", "-32768");
            chaz_ConfWriter_add_global_def("UINT16_MAX", "65535");
        }
        if (has_32) {
            chaz_ConfWriter_add_global_def("INT32_MAX", "2147483647");
            chaz_ConfWriter_add_global_def("INT32_MIN", "(-INT32_MAX-1)");
            chaz_ConfWriter_add_global_def("UINT32_MAX", "4294967295U");
        }
        if (has_64) {
            sprintf(scratch, "9223372036854775807%s", i64_t_postfix);
            chaz_ConfWriter_add_global_def("INT64_MAX", scratch);
            chaz_ConfWriter_add_global_def("INT64_MIN", "(-INT64_MAX-1)");
            sprintf(scratch, "18446744073709551615%s", u64_t_postfix);
            chaz_ConfWriter_add_global_def("UINT64_MAX", scratch);
        }
        chaz_ConfWriter_add_global_def("SIZE_MAX", "((size_t)-1)");
    }

    chaz_ConfWriter_end_module();

    /* Integer literals. */

    chaz_ConfWriter_start_module("IntegerLiterals");

    if (has_stdint) {
        chaz_ConfWriter_add_sys_include("stdint.h");
    }
    else {
        /* We support only the following subset of stdint.h
         *   INT32_C
         *   INT64_C
         *   UINT32_C
         *   UINT64_C
         */
        if (has_32) {
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
            sprintf(scratch, "n##%s", i64_t_postfix);
            chaz_ConfWriter_add_global_def("INT64_C(n)", scratch);
            sprintf(scratch, "n##%s", u64_t_postfix);
            chaz_ConfWriter_add_global_def("UINT64_C(n)", scratch);
        }
    }

    chaz_ConfWriter_end_module();

    /* Integer format strings. */

    chaz_ConfWriter_start_module("IntegerFormatStrings");

    if (has_inttypes) {
        chaz_ConfWriter_add_sys_include("inttypes.h");
    }
    else {
        /* We support only the following subset of inttypes.h
         *   PRId64
         *   PRIu64
         */
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
                CHAZ_QUOTE(  #include <stdio.h>                            )
                CHAZ_QUOTE(  int main() {                                  )
                CHAZ_QUOTE(      printf("%%%su", 18446744073709551615%s);  )
                CHAZ_QUOTE(      return 0;                                 )
                CHAZ_QUOTE( }                                              );

            for (i = 0; options[i] != NULL; i++) {
                /* Try to print 2**64-1, and see if we get it back intact. */
                sprintf(code_buf, format_64_code, options[i], u64_t_postfix);
                output = chaz_CC_capture_output(code_buf, &output_len);

                if (output != NULL
                    && strcmp(output, "18446744073709551615") == 0
                   ) {
                    sprintf(scratch, "\"%sd\"", options[i]);
                    chaz_ConfWriter_add_global_def("PRId64", scratch);
                    sprintf(scratch, "\"%su\"", options[i]);
                    chaz_ConfWriter_add_global_def("PRIu64", scratch);
                    free(output);
                    break;
                }
            }
        }
    }

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
static int
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

static int
chaz_LargeFiles_probe_off64(void) {
    static const char off64_code[] =
        CHAZ_QUOTE(  %s                                        )
        CHAZ_QUOTE(  #include <stdio.h>                        )
        CHAZ_QUOTE(  int main()                                )
        CHAZ_QUOTE(  {                                         )
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
        CHAZ_QUOTE(  #include <stdio.h>                         )
        CHAZ_QUOTE(  int main() {                               )
        CHAZ_QUOTE(      %s pos;                                )
        CHAZ_QUOTE(      FILE *f;                               )
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
        CHAZ_QUOTE( #include <stdio.h>                                       )
        CHAZ_QUOTE( int main() {                                             )
        CHAZ_QUOTE(     int fd;                                              )
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
        CHAZ_QUOTE(  #include <stdio.h>                     )
        CHAZ_QUOTE(  int main() {                           )
        CHAZ_QUOTE(      int fd = 20;                       )
        CHAZ_QUOTE(      char buf[1];                       )
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
        /*
         * FIXME: Under MinGW, alloca is defined in malloc.h. This probe
         * produces compiler warnings but works regardless. These warnings
         * are subsequently repeated during the build.
         */
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

#line 17 "src/Charmonizer/Probe/RegularExpressions.c"
/* #include "Charmonizer/Core/HeaderChecker.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Probe/RegularExpressions.h" */

void
chaz_RegularExpressions_run(void) {
    int has_regex_h     = chaz_HeadCheck_check_header("regex.h");
    int has_pcre_h      = chaz_HeadCheck_check_header("pcre.h");
    int has_pcreposix_h = chaz_HeadCheck_check_header("pcreposix.h");

    chaz_ConfWriter_start_module("RegularExpressions");

    /* PCRE headers. */
    if (has_pcre_h) {
        chaz_ConfWriter_add_def("HAS_PCRE_H", NULL);
    }
    if (has_pcreposix_h) {
        chaz_ConfWriter_add_def("HAS_PCREPOSIX_H", NULL);
    }

    /* Check for OS X enhanced regexes. */
    if (has_regex_h) {
        const char *reg_enhanced_code =
            CHAZ_QUOTE(  #include <regex.h>                             )
            CHAZ_QUOTE(  int main(int argc, char **argv) {              )
            CHAZ_QUOTE(      regex_t re;                                )
            CHAZ_QUOTE(      if (regcomp(&re, "^", REG_ENHANCED)) {     )
            CHAZ_QUOTE(          return 1;                              )
            CHAZ_QUOTE(      }                                          )
            CHAZ_QUOTE(      return 0;                                  )
            CHAZ_QUOTE(  }                                              );

        if (chaz_CC_test_compile(reg_enhanced_code)) {
            chaz_ConfWriter_add_def("HAS_REG_ENHANCED", NULL);
        }
    }

    chaz_ConfWriter_end_module();
}



/***************************************************************************/

#line 17 "src/Charmonizer/Probe/Strings.c"
/* #include "Charmonizer/Core/Compiler.h" */
/* #include "Charmonizer/Core/ConfWriter.h" */
/* #include "Charmonizer/Probe/Strings.h" */

/* Check for C99-compatible snprintf and possible replacements.
 */
static void
chaz_Strings_probe_c99_snprintf(void);

void
chaz_Strings_run(void) {
    chaz_ConfWriter_start_module("Strings");

    /* Check for C99 snprintf. */
    chaz_Strings_probe_c99_snprintf();

    chaz_ConfWriter_end_module();
}

static void
chaz_Strings_probe_c99_snprintf(void) {
    static const char snprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      char buf[4];                               )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = snprintf(buf, 4, "%s", "12345");  )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    static const char detect__scprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = _scprintf("%s", "12345");         )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    static const char detect__snprintf_code[] =
        CHAZ_QUOTE(  #include <stdio.h>                             )
        CHAZ_QUOTE(  int main() {                                   )
        CHAZ_QUOTE(      char buf[6];                               )
        CHAZ_QUOTE(      int  result;                               )
        CHAZ_QUOTE(      result = _snprintf(buf, 6, "%s", "12345"); )
        CHAZ_QUOTE(      printf("%d", result);                      )
        CHAZ_QUOTE(      return 0;                                  )
        CHAZ_QUOTE(  }                                              );
    char   *output = NULL;
    size_t  output_len;

    /* If the buffer passed to snprintf is too small, verify that snprintf
     * returns the length of the untruncated string which would have been
     * written to a large enough buffer.
     */
    output = chaz_CC_capture_output(snprintf_code, &output_len);
    if (output != NULL) {
        long result = strtol(output, NULL, 10);
        if (result == 5) {
            chaz_ConfWriter_add_def("HAS_C99_SNPRINTF", NULL);
        }
        free(output);
    }

    /* Test for _scprintf and _snprintf found in the MSVCRT.
     */
    output = chaz_CC_capture_output(detect__scprintf_code, &output_len);
    if (output != NULL) {
        chaz_ConfWriter_add_def("HAS__SCPRINTF", NULL);
        free(output);
    }
    output = chaz_CC_capture_output(detect__snprintf_code, &output_len);
    if (output != NULL) {
        chaz_ConfWriter_add_def("HAS__SNPRINTF", NULL);
        free(output);
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
    chaz_CFlags *temp_cflags = chaz_CC_get_temp_cflags();
    int can_control_visibility = false;
    char code_buf[sizeof(chaz_SymbolVisibility_symbol_exporting_code) + 100];

    chaz_ConfWriter_start_module("SymbolVisibility");
    chaz_CFlags_set_warnings_as_errors(temp_cflags);

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

    chaz_CFlags_clear(temp_cflags);

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
    CHAZ_QUOTE(  #include <stdio.h>                                    )
    CHAZ_QUOTE(  #define ISO_TEST(fmt, ...) \\                         )
    "                printf(fmt, __VA_ARGS__)                        \n"
    CHAZ_QUOTE(  int main() {                                          )
    CHAZ_QUOTE(      ISO_TEST("%d %d", 1, 1);                          )
    CHAZ_QUOTE(      return 0;                                         )
    CHAZ_QUOTE(  }                                                     );

/* Code for verifying GNU-style variadic macros. */
static const char chaz_VariadicMacros_gnuc_code[] =
    CHAZ_QUOTE(  #include <stdio.h>                                    )
    CHAZ_QUOTE(  #define GNU_TEST(fmt, args...) printf(fmt, ##args)    )
    CHAZ_QUOTE(  int main() {                                          )
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
        free(output);
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
        free(output);
    }

    chaz_ConfWriter_end_module();
}




#line 1 "clownfish/compiler/common/charmonizer.main"
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

/* Source fragment for Lucy's charmonizer.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* #include "Charmonizer/Probe.h" */
/* #include "Charmonizer/Probe/Integers.h" */

typedef struct SourceFileContext {
    chaz_MakeVar *common_objs;
    chaz_MakeVar *test_cfc_objs;
} SourceFileContext;

static void
S_add_compiler_flags(struct chaz_CLIArgs *args) {
    chaz_CFlags *extra_cflags = chaz_CC_get_extra_cflags();

    if (chaz_Probe_gcc_version_num()) {
        if (getenv("LUCY_VALGRIND")) {
            chaz_CFlags_append(extra_cflags, "-fno-inline-functions");
        }
        else if (getenv("LUCY_DEBUG")) {
            chaz_CFlags_append(extra_cflags,
                "-DLUCY_DEBUG -pedantic -Wall -Wextra -Wno-variadic-macros"
            );
        }
        if (args->charmony_pm) {
            chaz_CFlags_append(extra_cflags, "-DPERL_GCC_PEDANTIC");
        }

        /* Tell GCC explicitly to run with maximum options. */
        chaz_CFlags_append(extra_cflags, "-std=gnu99 -D_GNU_SOURCE");
    }
    else if (chaz_Probe_msvc_version_num()) {
        /* Compile as C++ under MSVC. */
        chaz_CFlags_append(extra_cflags, "/TP");

        /* Thwart stupid warnings. */
        chaz_CFlags_append(extra_cflags, "/D_CRT_SECURE_NO_WARNINGS");
        chaz_CFlags_append(extra_cflags, "/D_SCL_SECURE_NO_WARNINGS");

        if (chaz_Probe_msvc_version_num() < 1300) {
            /* Redefine 'for' to fix broken 'for' scoping under MSVC6. */
            chaz_CFlags_append(extra_cflags, "/Dfor=\"if(0);else for\"");
        }
    }
}

static void
S_source_file_callback(const char *dir, char *file, void *context) {
    SourceFileContext *sfc = (SourceFileContext*)context;
    const char *dir_sep = chaz_OS_dir_sep();
    const char *obj_ext = chaz_CC_obj_ext();
    size_t file_len = strlen(file);
    char *obj_file;

    if (strcmp(file, "CFCParseHeader.c") == 0) { return; }

    /* Strip extension */
    if (file_len <= 2 || memcmp(file + file_len - 2, ".c", 2) != 0) {
        chaz_Util_warn("Unexpected source file name: %s", file);
        return;
    }
    file[file_len-2] = '\0';

    obj_file = chaz_Util_join("", dir, dir_sep, file, obj_ext, NULL);
    if (strlen(file) >= 7 && memcmp(file, "CFCTest", 7) == 0) {
        chaz_MakeVar_append(sfc->test_cfc_objs, obj_file);
    }
    else {
        chaz_MakeVar_append(sfc->common_objs, obj_file);
    }

    free(obj_file);
}

static void
S_write_makefile(struct chaz_CLIArgs *args) {
    SourceFileContext sfc;

    const char *base_dir = "..";
    const char *dir_sep  = chaz_OS_dir_sep();
    const char *exe_ext  = chaz_OS_exe_ext();
    const char *obj_ext  = chaz_CC_obj_ext();

    char *lemon_dir    = chaz_Util_join(dir_sep, base_dir, "..", "..", "lemon",
                                        NULL);
    char *src_dir      = chaz_Util_join(dir_sep, base_dir, "src", NULL);
    char *include_dir  = chaz_Util_join(dir_sep, base_dir, "include", NULL);
    char *parse_header = chaz_Util_join(dir_sep, src_dir, "CFCParseHeader",
                                        NULL);
    char *cfc_exe      = chaz_Util_join("", "cfc", exe_ext, NULL);
    char *test_cfc_exe = chaz_Util_join("", "t", dir_sep, "test_cfc", exe_ext,
                                        NULL);

    char *scratch;

    chaz_MakeFile *makefile;
    chaz_MakeVar  *var;
    chaz_MakeRule *rule;
    chaz_MakeRule *clean_rule;

    chaz_CFlags *extra_cflags = chaz_CC_get_extra_cflags();
    chaz_CFlags *makefile_cflags;
    chaz_CFlags *link_flags;

    printf("Creating Makefile...\n");

    makefile = chaz_MakeFile_new();

    /* Directories */

    chaz_MakeFile_add_var(makefile, "BASE_DIR", base_dir);

    /* C compiler */

    chaz_MakeFile_add_var(makefile, "CC", chaz_CC_get_cc());

    makefile_cflags = chaz_CC_new_cflags();

    chaz_CFlags_enable_optimization(makefile_cflags);
    chaz_CFlags_add_include_dir(makefile_cflags, ".");
    chaz_CFlags_add_include_dir(makefile_cflags, include_dir);
    chaz_CFlags_add_include_dir(makefile_cflags, src_dir);
    if (args->code_coverage) {
        chaz_CFlags_enable_code_coverage(makefile_cflags);
    }

    var = chaz_MakeFile_add_var(makefile, "CFLAGS", NULL);
    chaz_MakeVar_append(var, chaz_CFlags_get_string(extra_cflags));
    chaz_MakeVar_append(var, chaz_CFlags_get_string(makefile_cflags));
    chaz_MakeVar_append(var, chaz_CC_get_cflags());

    chaz_CFlags_destroy(makefile_cflags);

    /* Object files */

    sfc.common_objs   = chaz_MakeFile_add_var(makefile, "COMMON_OBJS", NULL);
    sfc.test_cfc_objs = chaz_MakeFile_add_var(makefile, "TEST_CFC_OBJS", NULL);

    chaz_Make_list_files(src_dir, "c", S_source_file_callback, &sfc);

    scratch = chaz_Util_join("", parse_header, obj_ext, NULL);
    chaz_MakeVar_append(sfc.common_objs, scratch);
    free(scratch);

    scratch = chaz_Util_join("", "t", dir_sep, "test_cfc", obj_ext, NULL);
    chaz_MakeVar_append(sfc.test_cfc_objs, scratch);
    free(scratch);

    scratch = chaz_Util_join("", "cfc", obj_ext, NULL);
    chaz_MakeFile_add_var(makefile, "CFC_OBJS", scratch);
    free(scratch);

    /* Rules */

    chaz_MakeFile_add_rule(makefile, "all", cfc_exe);

    chaz_MakeFile_add_lemon_exe(makefile, lemon_dir);
    chaz_MakeFile_add_lemon_grammar(makefile, parse_header);

    /*
     * The dependency is actually on CFCParseHeader.h, but make doesn't cope
     * well with multiple output files.
     */
    scratch = chaz_Util_join(".", parse_header, "c", NULL);
    chaz_MakeFile_add_rule(makefile, "$(COMMON_OBJS)", scratch);
    free(scratch);

    link_flags = chaz_CC_new_cflags();
    if (chaz_CC_msvc_version_num()) {
        chaz_CFlags_append(link_flags, "/nologo");
    }
    if (args->code_coverage) {
        chaz_CFlags_enable_code_coverage(link_flags);
    }
    chaz_MakeFile_add_exe(makefile, cfc_exe, "$(COMMON_OBJS) $(CFC_OBJS)",
                          link_flags);
    chaz_MakeFile_add_exe(makefile, test_cfc_exe,
                          "$(COMMON_OBJS) $(TEST_CFC_OBJS)", link_flags);
    chaz_CFlags_destroy(link_flags);

    rule = chaz_MakeFile_add_rule(makefile, "test", test_cfc_exe);
    chaz_MakeRule_add_command(rule, test_cfc_exe);

    if (args->code_coverage) {
        rule = chaz_MakeFile_add_rule(makefile, "coverage", test_cfc_exe);
        chaz_MakeRule_add_command(rule,
                                  "lcov"
                                  " --zerocounters"
                                  " --directory $(BASE_DIR)");
        chaz_MakeRule_add_command(rule, test_cfc_exe);
        chaz_MakeRule_add_command(rule,
                                  "lcov"
                                  " --capture"
                                  " --directory $(BASE_DIR)"
                                  " --base-directory ."
                                  " --rc lcov_branch_coverage=1"
                                  " --output-file cfc.info");
        chaz_MakeRule_add_command(rule,
                                  "genhtml"
                                  " --branch-coverage"
                                  " --output-directory coverage"
                                  " cfc.info");
    }

    clean_rule = chaz_MakeFile_clean_rule(makefile);

    chaz_MakeRule_add_rm_command(clean_rule, "$(COMMON_OBJS)");
    chaz_MakeRule_add_rm_command(clean_rule, "$(CFC_OBJS)");
    chaz_MakeRule_add_rm_command(clean_rule, "$(TEST_CFC_OBJS)");

    if (args->code_coverage) {
        chaz_MakeRule_add_rm_command(clean_rule, "cfc.info");
        chaz_MakeRule_add_recursive_rm_command(clean_rule, "coverage");
    }

    chaz_MakeFile_write(makefile);

    chaz_MakeFile_destroy(makefile);
    free(lemon_dir);
    free(src_dir);
    free(include_dir);
    free(parse_header);
    free(cfc_exe);
    free(test_cfc_exe);
}

int main(int argc, const char **argv) {
    /* Initialize. */
    struct chaz_CLIArgs args;
    {
        int result = chaz_Probe_parse_cli_args(argc, argv, &args);
        if (!result) {
            chaz_Probe_die_usage();
        }
        chaz_Probe_init(&args);
        S_add_compiler_flags(&args);
    }

    /* Define stdint types in charmony.h. */
    chaz_ConfWriter_append_conf("#define CHY_EMPLOY_INTEGERTYPES\n\n");
    chaz_ConfWriter_append_conf("#define CHY_EMPLOY_INTEGERLITERALS\n\n");

    /* Run probe modules. */
    chaz_BuildEnv_run();
    chaz_DirManip_run();
    chaz_Headers_run();
    chaz_AtomicOps_run();
    chaz_FuncMacro_run();
    chaz_Booleans_run();
    chaz_Integers_run();
    chaz_Strings_run();
    chaz_Memory_run();
    chaz_SymbolVisibility_run();
    chaz_UnusedVars_run();
    chaz_VariadicMacros_run();

    if (args.write_makefile) {
        S_write_makefile(&args);
    }

    /* Clean up. */
    chaz_Probe_clean_up();

    return 0;
}


