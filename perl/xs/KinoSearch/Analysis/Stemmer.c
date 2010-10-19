#include "xs/XSBind.h"

#include "KinoSearch/Analysis/Stemmer.h"
#include "KinoSearch/Object/Host.h"

void
kino_Stemmer_load_snowball() 
{
    kino_Host_callback(KINO_STEMMER, "lazy_load_snowball", 0);
}
    

