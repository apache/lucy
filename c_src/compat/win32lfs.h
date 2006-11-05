#ifndef H_WIN32LFS
#define H_WIN32LFS 1

#define LUCY_HAS_LARGE_FILE_SUPPORT
#if defined(CHAZ_USE_SHORT_NAMES) || defined(LUCY_USE_SHORT_NAMES)
  #define HAS_LARGE_FILE_SUPPORT
#endif 

#include <io.h>
#include <stdio.h>

typedef __int64 off64_t;

off64_t
ftello64(FILE *fh);

int
fseeko64(FILE *fh, off64_t offset, int whence);

#endif /* H_WIN32LFS */

