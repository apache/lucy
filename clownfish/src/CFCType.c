#include <stdlib.h>

#include "CFCType.h"

struct CFCType {
    int   flags;
};

CFCType*
CFCType_new()
{
    CFCType *self = (CFCType*)malloc(sizeof(CFCType));
    if (!self) { croak("malloc failed"); }
    return self;
}

void
CFCType_destroy(CFCType *self)
{
    free(self);
}


