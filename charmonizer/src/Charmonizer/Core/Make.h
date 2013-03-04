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

#ifndef H_CHAZ_MAKE
#define H_CHAZ_MAKE

#ifdef __cplusplus
extern "C" {
#endif

typedef struct chaz_MakeFile chaz_MakeFile;
typedef struct chaz_MakeVar chaz_MakeVar;
typedef struct chaz_MakeRule chaz_MakeRule;

typedef void (*chaz_Make_list_files_callback_t)(char *file, void *context);

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

/** Add a file to the 'clean' target.
 *
 * @param makefile The makefile.
 * @param target The filename.
 */
void
chaz_MakeFile_add_to_cleanup(chaz_MakeFile *makefile, const char *target);

/** Add a directory to the 'clean' target.
 *
 * @param makefile The makefile.
 * @param dir The directory.
 */
void
chaz_MakeFile_add_dir_to_cleanup(chaz_MakeFile *makefile, const char *dir);

/** Add a rule to link an executable. The executable will also be added to the
 * list of files to clean.
 *
 * @param makefile The makefile.
 * @param exe The name of the executable.
 * @param objects The list of object files.
 * @param extra_link_flags Additional link flags.
 */
chaz_MakeRule*
chaz_MakeFile_add_exe(chaz_MakeFile *makefile, const char *exe,
                      const char *objects, const char *extra_link_flags);

/** Add a rule to link a shared object. The shared object will also be added to
 * the list of files to clean.
 *
 * @param makefile The makefile.
 * @param shared_obj The name of the shared object.
 * @param objects The list of object files.
 * @param extra_link_flags Additional link flags.
 */
chaz_MakeRule*
chaz_MakeFile_add_shared_obj(chaz_MakeFile *makefile, const char *shared_obj,
                             const char *objects,
                             const char *extra_link_flags);

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

/** Add one or more commands to call another makefile recursively.
 *
 * @param rule The rule.
 * @param dir The directory in which to call the makefile.
 * @param target The target to call. Pass NULL for the default target.
 */
void
chaz_MakeRule_add_command_make(chaz_MakeRule *rule, const char *dir,
                               const char *target);

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_MAKE */


