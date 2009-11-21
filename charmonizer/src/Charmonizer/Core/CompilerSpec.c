#define CHAZ_USE_SHORT_NAMES

#include <string.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/CompilerSpec.h"

/* detect a supported compiler */
#ifdef __GNUC__
    static CompilerSpec spec = { "gcc", "-I ", "-o ", "-o " };
#elif defined(_MSC_VER)
    static CompilerSpec spec = { "MSVC", "/I", "/Fo", "/Fe" };
#else
  #error "Couldn't detect a supported compiler"
#endif


chaz_CompilerSpec*
chaz_CCSpec_find_spec()
{
    if (verbosity)
        printf("Trying to find a supported compiler...\n");

    return &spec;
}

/**
 * Copyright 2006 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

