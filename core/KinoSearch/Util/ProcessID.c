#include "KinoSearch/Util/ProcessID.h"

/********************************* WINDOWS ********************************/
#if (defined(CHY_HAS_WINDOWS_H) && defined(CHY_HAS_PROCESS_H))

#include <Windows.h>
#include <process.h>

int 
kino_PID_getpid(void)
{
    return GetCurrentProcessId();
}

chy_bool_t
kino_PID_active(int pid)
{
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
kino_PID_getpid(void)
{
    return getpid();
}

chy_bool_t
kino_PID_active(int pid)
{
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


