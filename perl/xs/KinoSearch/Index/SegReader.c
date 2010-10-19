#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Object/Host.h"

CharBuf*
SegReader_try_init_components(SegReader *self)
{
    return (CharBuf*)Host_callback_obj(self, "try_init_components", 0);
}


