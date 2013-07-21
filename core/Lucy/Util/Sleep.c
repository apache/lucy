/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define C_LUCY_SLEEP

#include "charmony.h"

#include "Lucy/Util/Sleep.h"

/********************************* WINDOWS ********************************/
#ifdef CHY_HAS_WINDOWS_H

#include <windows.h>

void
lucy_Sleep_sleep(uint32_t seconds) {
    Sleep(seconds * 1000);
}

void
lucy_Sleep_millisleep(uint32_t milliseconds) {
    Sleep(milliseconds);
}

/********************************* UNIXEN *********************************/
#elif defined(CHY_HAS_UNISTD_H)

#include <unistd.h>

void
lucy_Sleep_sleep(uint32_t seconds) {
    sleep(seconds);
}

void
lucy_Sleep_millisleep(uint32_t milliseconds) {
    uint32_t seconds = milliseconds / 1000;
    milliseconds  = milliseconds % 1000;
    sleep(seconds);
    // TODO: probe for usleep.
    usleep(milliseconds * 1000);
}

#else
  #error "Can't find a known sleep API."
#endif // OS switch.


