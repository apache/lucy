#define C_KINO_LOCKFACTORY
#include "KinoSearch/Util/ToolSet.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "KinoSearch/Store/LockFactory.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Store/Lock.h"
#include "KinoSearch/Store/SharedLock.h"

LockFactory*
LockFact_new(Folder *folder, const CharBuf *host)
{
    LockFactory *self = (LockFactory*)VTable_Make_Obj(LOCKFACTORY);
    return LockFact_init(self, folder, host);
}

LockFactory*
LockFact_init(LockFactory *self, Folder *folder, const CharBuf *host)
{
    self->folder    = (Folder*)INCREF(folder);
    self->host      = CB_Clone(host);
    return self;
}

void
LockFact_destroy(LockFactory *self)
{
    DECREF(self->folder);
    DECREF(self->host);
    SUPER_DESTROY(self, LOCKFACTORY);
}

Lock*
LockFact_make_lock(LockFactory *self, const CharBuf *name, int32_t timeout, 
                   int32_t interval)
{
    return (Lock*)LFLock_new(self->folder, name, self->host, timeout, 
        interval);
}

Lock*
LockFact_make_shared_lock(LockFactory *self, const CharBuf *name, 
                          int32_t timeout, int32_t interval)
{
    return (Lock*)ShLock_new(self->folder, name, self->host, timeout, 
        interval);
}


