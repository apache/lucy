#define C_KINO_TERMSTEPPER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/TermStepper.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/StringHelper.h"

TermStepper*
TermStepper_init(TermStepper *self)
{
    Stepper_init((Stepper*)self);
    self->value = NULL;
    return self;
}

void
TermStepper_destroy(TermStepper *self)
{
    DECREF(self->value);
    SUPER_DESTROY(self, TERMSTEPPER);
}

void
TermStepper_reset(TermStepper *self)
{
    DECREF(self->value);
    self->value = NULL;
}

Obj*
TermStepper_get_value(TermStepper *self)
{
    return self->value;
}

void
TermStepper_set_value(TermStepper *self, Obj *value)
{
    DECREF(self->value);
    self->value = value ? INCREF(value) : NULL;
}


