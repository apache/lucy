#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "CFC.h"

MODULE = Clownfish    PACKAGE = Clownfish::Type

SV*
_new(klass)
    const char *klass;
CODE:
    CFCType *self = CFCType_new();
    RETVAL = newSV(0);
	sv_setref_pv(RETVAL, klass, (void*)self);
OUTPUT: RETVAL

void
_destroy(self)
    CFCType *self;
PPCODE:
    CFCType_destroy(self);

