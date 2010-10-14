#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Object/Host.h"

CharBuf*
SegReader_try_init_components(SegReader *self)
{
    return (CharBuf*)Host_callback_obj(self, "try_init_components", 0);
}

/* Copyright 2009-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

