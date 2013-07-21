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

#include "charmony.h"

#include "Lucy/Util/ProcessID.h"

/********************************* WINDOWS ********************************/
#if (defined(CHY_HAS_WINDOWS_H) && defined(CHY_HAS_PROCESS_H) && !defined(__CYGWIN__))

#include <Windows.h>
#include <process.h>

int
lucy_PID_getpid(void) {
    return GetCurrentProcessId();
}

bool
lucy_PID_active(int pid) {
    // Attempt to open a handle to the process with permissions to terminate
    // -- but don't actually terminate.
    HANDLE handle = OpenProcess(PROCESS_TERMINATE, false, pid);
    if (handle != NULL) {
        // Successful open, therefore process is active.
        CloseHandle(handle);
        return true;
    }
    // If the opening attempt fails because we were denied permission, assume
    // that the process is active.
    if (GetLastError() == ERROR_ACCESS_DENIED) {
        return true;
    }

    // Can't find any trace of the process, so return false.
    return false;
}


/********************************* UNIXEN *********************************/
#elif (defined(CHY_HAS_UNISTD_H) && defined(CHY_HAS_SIGNAL_H))

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

int
lucy_PID_getpid(void) {
    return getpid();
}

bool
lucy_PID_active(int pid) {
    if (kill(pid, 0) == 0) {
        return true; // signal succeeded, therefore pid active
    }

    if (errno != ESRCH) {
        return true; // an error other than "pid not found", thus active
    }

    return false;
}

#else
  #error "Can't find a known process ID API."
#endif // OS switch.


