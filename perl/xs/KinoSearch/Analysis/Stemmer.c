#include "xs/XSBind.h"

#include "KinoSearch/Analysis/Stemmer.h"
#include "KinoSearch/Object/Host.h"

void
kino_Stemmer_load_snowball() 
{
    kino_Host_callback(KINO_STEMMER, "lazy_load_snowball", 0);
}
    
/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

