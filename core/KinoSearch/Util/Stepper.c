#define C_KINO_STEPPER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Util/Stepper.h"

Stepper*
Stepper_init(Stepper *self)
{
    ABSTRACT_CLASS_CHECK(self, STEPPER);
    return self;
}


