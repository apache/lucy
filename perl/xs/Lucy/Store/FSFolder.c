#include "Lucy/Util/ToolSet.h"
#include "Lucy/Object/Host.h"
#include "Lucy/Store/FSFolder.h"

CharBuf*
FSFolder_absolutify(const CharBuf *path)
{
   
    return Host_callback_str(FSFOLDER, "absolutify", 1, 
        ARG_STR("path", path));
}

