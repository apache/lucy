/* Charmonizer/Core/Defines.h -- Universal definitions.
 */
#ifndef H_CHAZ_DEFINES
#define H_CHAZ_DEFINES 1

#ifdef __cplusplus
extern "C" {
#endif

typedef int chaz_bool_t;

#ifndef true
  #define true 1
  #define false 0
#endif

#define CHAZ_QUOTE(x) #x "\n" 

#if (defined(CHAZ_USE_SHORT_NAMES) || defined(CHY_USE_SHORT_NAMES))
  #define QUOTE CHAZ_QUOTE
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_DEFINES */

