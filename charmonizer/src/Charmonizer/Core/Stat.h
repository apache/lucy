/* Charmonizer/Core/Stat.h - stat a file, if possible.
 *
 * This component works by attempting to compile a utility program called
 * "_charm_stat".  When Charmonizer needs to stat a file, it shells out to
 * this utility, which communicates via a file a la capture_output().
 *
 * Since we don't know whether we have 64-bit integers when Charmonizer itself
 * gets compiled, the items in the stat structure are whatever size longs are.
 * 
 * TODO: probe for which fields are available.
 */

#ifndef H_CHAZ_STAT
#define H_CHAZ_STAT

#ifdef __cplusplus
extern "C" {
#endif

#include "Charmonizer/Core/Defines.h"

typedef struct chaz_Stat chaz_Stat;

struct chaz_Stat {
    chaz_bool_t valid;
    long size;
    long blocks;
};

/* Attempt to stat a file.  If successful, store the set target->valid to true
 * and store the results in the stat structure.  If unsuccessful, set
 * target->valid to false.
 */
void
chaz_Stat_stat(const char *filepath, chaz_Stat *target);

#ifdef CHAZ_USE_SHORT_NAMES
  #define Stat                  chaz_Stat
  #define Stat_stat             chaz_Stat_stat
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_COMPILER */



