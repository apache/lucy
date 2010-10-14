#include "KinoSearch/Util/ToolSet.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Store/FSFolder.h"

CharBuf*
FSFolder_absolutify(const CharBuf *path)
{
   
    return Host_callback_str(FSFOLDER, "absolutify", 1, 
        ARG_STR("path", path));
}

