#define C_KINO_STEPPER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Util/Stepper.h"

Stepper*
Stepper_init(Stepper *self)
{
    ABSTRACT_CLASS_CHECK(self, STEPPER);
    return self;
}

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

