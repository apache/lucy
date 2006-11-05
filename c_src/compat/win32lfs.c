#include "win32lfs.h"
#include <errno.h>

off64_t
ftello64(FILE *fh)
{
    fpos_t pos;
    if (fgetpos(fh, &pos))
        return -1;
    else
        return (off64_t)pos;
}

int
fseeko64(FILE *fh, off64_t offset, int whence)
{
    fpos_t pos;

    switch (whence) {

    case SEEK_SET:
        break;

    case SEEK_END:
        fseek(fh, 0, SEEK_END); /* sync buffering */
        if ((pos = _telli64(fileno(fh))) == -1)
            return -1;
        offset += pos;
        break;

    case SEEK_CUR:
        if (fgetpos(fh, &pos))
            return -1;
        offset += pos;
        break;

    default:
        errno = EINVAL;
        return -1;
    }

    return fsetpos(fh, &offset);
}

