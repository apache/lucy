/* Charmonizer/Core/OperatingSystem.h - abstract an operating system down to a few
 * variables.
 */

#ifndef H_CHAZ_OPER_SYS
#define H_CHAZ_OPER_SYS

#ifdef __cplusplus
extern "C" {
#endif

/* Remove an executable file named [name], appending the exe_ext if needed.
 */
void
chaz_OS_remove_exe(char *name);

/* Remove an object file named [name], appending the obj_ext if needed.
 */
void
chaz_OS_remove_obj(char *name);

/* Concatenate all arguments in a NULL-terminated list into a single command
 * string, prepend the appropriate prefix, and invoke via system().
 */
int 
chaz_OS_run_local(char *arg1, ...);

/* Invoke a command and attempt to suppress output from both stdout and stderr
 * (as if they had been sent to /dev/null).  If it's not possible to run the
 * command quietly, run it anyway.
 */
int 
chaz_OS_run_quietly(const char *command);

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

/* Tear down the Charmonizer/Core/OperatingSystem module. 
 */
void
chaz_OS_clean_up(void);

#ifdef CHAZ_USE_SHORT_NAMES
  #define OS_remove_exe                chaz_OS_remove_exe
  #define OS_remove_obj                chaz_OS_remove_obj
  #define OS_run_local                 chaz_OS_run_local
  #define OS_run_quietly               chaz_OS_run_quietly
  #define OS_exe_ext                   chaz_OS_exe_ext
  #define OS_obj_ext                   chaz_OS_obj_ext
  #define OS_dev_null                  chaz_OS_dev_null
  #define OS_init                      chaz_OS_init
  #define OS_clean_up                  chaz_OS_clean_up
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */


