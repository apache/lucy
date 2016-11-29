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

#define CFISH_USE_SHORT_NAMES
#define LUCY_USE_SHORT_NAMES

#include "Lucy/Store/ErrorMessage.h"
#include "Clownfish/CharBuf.h"
#include "Clownfish/Err.h"
#include "Clownfish/Util/Memory.h"

#include <errno.h>
#include <string.h>

void
ErrMsg_set(const char *fmt, ...) {
    CharBuf *buf = CB_new(0);

    va_list args;
    va_start(args, fmt);
    CB_VCatF(buf, fmt, args);
    va_end(args);

    Err_set_error(Err_new(CB_Yield_String(buf)));
    DECREF(buf);
}

void
ErrMsg_set_with_errno(const char *fmt, ...) {
    int cur_errno = errno;

    CharBuf *buf = CB_new(0);

    va_list args;
    va_start(args, fmt);
    CB_VCatF(buf, fmt, args);
    va_end(args);

    CB_Cat_Trusted_Utf8(buf, ": ", 2);

    const char *msg = ErrMsg_strerror(cur_errno);

    if (msg != NULL) {
        CB_Cat_Trusted_Utf8(buf, msg, strlen(msg));
    }
    else {
        CB_catf(buf, "Unknown error: %i32", (int32_t)cur_errno);
    }

    Err_set_error(Err_new(CB_Yield_String(buf)));
    DECREF(buf);
}

void
ErrMsg_set_with_win_error(const char *fmt, ...) {
    char *win_error = Err_win_error();

    CharBuf *buf = CB_new(0);

    va_list args;
    va_start(args, fmt);
    CB_VCatF(buf, fmt, args);
    va_end(args);

    CB_Cat_Trusted_Utf8(buf, ": ", 2);
    CB_Cat_Utf8(buf, win_error, strlen(win_error));

    Err_set_error(Err_new(CB_Yield_String(buf)));
    DECREF(buf);
    FREEMEM(win_error);
}

const char*
ErrMsg_strerror(int my_errno) {
    const char *msg = NULL;

    switch (my_errno) {
        case 0:
            msg = "No error";
            break;

        // Error macros from Linux.
#ifdef E2BIG
        case E2BIG:
            msg = "Argument list too long";
            break;
#endif
#ifdef EACCES
        case EACCES:
            msg = "Permission denied";
            break;
#endif
#ifdef EADDRINUSE
        case EADDRINUSE:
            msg = "Address already in use";
            break;
#endif
#ifdef EADDRNOTAVAIL
        case EADDRNOTAVAIL:
            msg = "Cannot assign requested address";
            break;
#endif
#ifdef EADV
        case EADV:
            msg = "Advertise error";
            break;
#endif
#ifdef EAFNOSUPPORT
        case EAFNOSUPPORT:
            msg = "Address family not supported by protocol";
            break;
#endif
#ifdef EAGAIN
        case EAGAIN:
            msg = "Try again";
            break;
#endif
#ifdef EALREADY
        case EALREADY:
            msg = "Operation already in progress";
            break;
#endif
#ifdef EBADE
        case EBADE:
            msg = "Invalid exchange";
            break;
#endif
#ifdef EBADF
        case EBADF:
            msg = "Bad file number";
            break;
#endif
#ifdef EBADFD
        case EBADFD:
            msg = "File descriptor in bad state";
            break;
#endif
#ifdef EBADMSG
        case EBADMSG:
            msg = "Not a data message";
            break;
#endif
#ifdef EBADR
        case EBADR:
            msg = "Invalid request descriptor";
            break;
#endif
#ifdef EBADRQC
        case EBADRQC:
            msg = "Invalid request code";
            break;
#endif
#ifdef EBADSLT
        case EBADSLT:
            msg = "Invalid slot";
            break;
#endif
#ifdef EBFONT
        case EBFONT:
            msg = "Bad font file format";
            break;
#endif
#ifdef EBUSY
        case EBUSY:
            msg = "Device or resource busy";
            break;
#endif
#ifdef ECANCELED
        case ECANCELED:
            msg = "Operation Canceled";
            break;
#endif
#ifdef ECHILD
        case ECHILD:
            msg = "No child processes";
            break;
#endif
#ifdef ECHRNG
        case ECHRNG:
            msg = "Channel number out of range";
            break;
#endif
#ifdef ECOMM
        case ECOMM:
            msg = "Communication error on send";
            break;
#endif
#ifdef ECONNABORTED
        case ECONNABORTED:
            msg = "Software caused connection abort";
            break;
#endif
#ifdef ECONNREFUSED
        case ECONNREFUSED:
            msg = "Connection refused";
            break;
#endif
#ifdef ECONNRESET
        case ECONNRESET:
            msg = "Connection reset by peer";
            break;
#endif
#ifdef EDEADLK
        case EDEADLK:
            msg = "Resource deadlock would occur";
            break;
#endif
#ifdef EDESTADDRREQ
        case EDESTADDRREQ:
            msg = "Destination address required";
            break;
#endif
#ifdef EDOM
        case EDOM:
            msg = "Math argument out of domain of func";
            break;
#endif
#ifdef EDOTDOT
        case EDOTDOT:
            msg = "RFS specific error";
            break;
#endif
#ifdef EDQUOT
        case EDQUOT:
            msg = "Quota exceeded";
            break;
#endif
#ifdef EEXIST
        case EEXIST:
            msg = "File exists";
            break;
#endif
#ifdef EFAULT
        case EFAULT:
            msg = "Bad address";
            break;
#endif
#ifdef EFBIG
        case EFBIG:
            msg = "File too large";
            break;
#endif
#ifdef EHOSTDOWN
        case EHOSTDOWN:
            msg = "Host is down";
            break;
#endif
#ifdef EHOSTUNREACH
        case EHOSTUNREACH:
            msg = "No route to host";
            break;
#endif
#ifdef EHWPOISON
        case EHWPOISON:
            msg = "Memory page has hardware error";
            break;
#endif
#ifdef EIDRM
        case EIDRM:
            msg = "Identifier removed";
            break;
#endif
#ifdef EILSEQ
        case EILSEQ:
            msg = "Illegal byte sequence";
            break;
#endif
#ifdef EINPROGRESS
        case EINPROGRESS:
            msg = "Operation now in progress";
            break;
#endif
#ifdef EINTR
        case EINTR:
            msg = "Interrupted system call";
            break;
#endif
#ifdef EINVAL
        case EINVAL:
            msg = "Invalid argument";
            break;
#endif
#ifdef EIO
        case EIO:
            msg = "I/O error";
            break;
#endif
#ifdef EISCONN
        case EISCONN:
            msg = "Transport endpoint is already connected";
            break;
#endif
#ifdef EISDIR
        case EISDIR:
            msg = "Is a directory";
            break;
#endif
#ifdef EISNAM
        case EISNAM:
            msg = "Is a named type file";
            break;
#endif
#ifdef EKEYEXPIRED
        case EKEYEXPIRED:
            msg = "Key has expired";
            break;
#endif
#ifdef EKEYREJECTED
        case EKEYREJECTED:
            msg = "Key was rejected by service";
            break;
#endif
#ifdef EKEYREVOKED
        case EKEYREVOKED:
            msg = "Key has been revoked";
            break;
#endif
#ifdef EL2HLT
        case EL2HLT:
            msg = "Level 2 halted";
            break;
#endif
#ifdef EL2NSYNC
        case EL2NSYNC:
            msg = "Level 2 not synchronized";
            break;
#endif
#ifdef EL3HLT
        case EL3HLT:
            msg = "Level 3 halted";
            break;
#endif
#ifdef EL3RST
        case EL3RST:
            msg = "Level 3 reset";
            break;
#endif
#ifdef ELIBACC
        case ELIBACC:
            msg = "Can not access a needed shared library";
            break;
#endif
#ifdef ELIBBAD
        case ELIBBAD:
            msg = "Accessing a corrupted shared library";
            break;
#endif
#ifdef ELIBEXEC
        case ELIBEXEC:
            msg = "Cannot exec a shared library directly";
            break;
#endif
#ifdef ELIBMAX
        case ELIBMAX:
            msg = "Attempting to link in too many shared libraries";
            break;
#endif
#ifdef ELIBSCN
        case ELIBSCN:
            msg = ".lib section in a.out corrupted";
            break;
#endif
#ifdef ELNRNG
        case ELNRNG:
            msg = "Link number out of range";
            break;
#endif
#ifdef ELOOP
        case ELOOP:
            msg = "Too many symbolic links encountered";
            break;
#endif
#ifdef EMEDIUMTYPE
        case EMEDIUMTYPE:
            msg = "Wrong medium type";
            break;
#endif
#ifdef EMFILE
        case EMFILE:
            msg = "Too many open files";
            break;
#endif
#ifdef EMLINK
        case EMLINK:
            msg = "Too many links";
            break;
#endif
#ifdef EMSGSIZE
        case EMSGSIZE:
            msg = "Message too long";
            break;
#endif
#ifdef EMULTIHOP
        case EMULTIHOP:
            msg = "Multihop attempted";
            break;
#endif
#ifdef ENAMETOOLONG
        case ENAMETOOLONG:
            msg = "File name too long";
            break;
#endif
#ifdef ENAVAIL
        case ENAVAIL:
            msg = "No XENIX semaphores available";
            break;
#endif
#ifdef ENETDOWN
        case ENETDOWN:
            msg = "Network is down";
            break;
#endif
#ifdef ENETRESET
        case ENETRESET:
            msg = "Network dropped connection because of reset";
            break;
#endif
#ifdef ENETUNREACH
        case ENETUNREACH:
            msg = "Network is unreachable";
            break;
#endif
#ifdef ENFILE
        case ENFILE:
            msg = "File table overflow";
            break;
#endif
#ifdef ENOANO
        case ENOANO:
            msg = "No anode";
            break;
#endif
#ifdef ENOBUFS
        case ENOBUFS:
            msg = "No buffer space available";
            break;
#endif
#ifdef ENOCSI
        case ENOCSI:
            msg = "No CSI structure available";
            break;
#endif
#ifdef ENODATA
        case ENODATA:
            msg = "No data available";
            break;
#endif
#ifdef ENODEV
        case ENODEV:
            msg = "No such device";
            break;
#endif
#ifdef ENOENT
        case ENOENT:
            msg = "No such file or directory";
            break;
#endif
#ifdef ENOEXEC
        case ENOEXEC:
            msg = "Exec format error";
            break;
#endif
#ifdef ENOKEY
        case ENOKEY:
            msg = "Required key not available";
            break;
#endif
#ifdef ENOLCK
        case ENOLCK:
            msg = "No record locks available";
            break;
#endif
#ifdef ENOLINK
        case ENOLINK:
            msg = "Link has been severed";
            break;
#endif
#ifdef ENOMEDIUM
        case ENOMEDIUM:
            msg = "No medium found";
            break;
#endif
#ifdef ENOMEM
        case ENOMEM:
            msg = "Out of memory";
            break;
#endif
#ifdef ENOMSG
        case ENOMSG:
            msg = "No message of desired type";
            break;
#endif
#ifdef ENONET
        case ENONET:
            msg = "Machine is not on the network";
            break;
#endif
#ifdef ENOPKG
        case ENOPKG:
            msg = "Package not installed";
            break;
#endif
#ifdef ENOPROTOOPT
        case ENOPROTOOPT:
            msg = "Protocol not available";
            break;
#endif
#ifdef ENOSPC
        case ENOSPC:
            msg = "No space left on device";
            break;
#endif
#ifdef ENOSR
        case ENOSR:
            msg = "Out of streams resources";
            break;
#endif
#ifdef ENOSTR
        case ENOSTR:
            msg = "Device not a stream";
            break;
#endif
#ifdef ENOSYS
        case ENOSYS:
            msg = "Invalid system call number";
            break;
#endif
#ifdef ENOTBLK
        case ENOTBLK:
            msg = "Block device required";
            break;
#endif
#ifdef ENOTCONN
        case ENOTCONN:
            msg = "Transport endpoint is not connected";
            break;
#endif
#ifdef ENOTDIR
        case ENOTDIR:
            msg = "Not a directory";
            break;
#endif
#ifdef ENOTEMPTY
        case ENOTEMPTY:
            msg = "Directory not empty";
            break;
#endif
#ifdef ENOTNAM
        case ENOTNAM:
            msg = "Not a XENIX named type file";
            break;
#endif
#ifdef ENOTRECOVERABLE
        case ENOTRECOVERABLE:
            msg = "State not recoverable";
            break;
#endif
#ifdef ENOTSOCK
        case ENOTSOCK:
            msg = "Socket operation on non-socket";
            break;
#endif
#ifdef ENOTTY
        case ENOTTY:
            msg = "Not a typewriter";
            break;
#endif
#ifdef ENOTUNIQ
        case ENOTUNIQ:
            msg = "Name not unique on network";
            break;
#endif
#ifdef ENXIO
        case ENXIO:
            msg = "No such device or address";
            break;
#endif
#ifdef EOPNOTSUPP
        case EOPNOTSUPP:
            msg = "Operation not supported on transport endpoint";
            break;
#endif
#ifdef EOVERFLOW
        case EOVERFLOW:
            msg = "Value too large for defined data type";
            break;
#endif
#ifdef EOWNERDEAD
        case EOWNERDEAD:
            msg = "Owner died";
            break;
#endif
#ifdef EPERM
        case EPERM:
            msg = "Operation not permitted";
            break;
#endif
#ifdef EPFNOSUPPORT
        case EPFNOSUPPORT:
            msg = "Protocol family not supported";
            break;
#endif
#ifdef EPIPE
        case EPIPE:
            msg = "Broken pipe";
            break;
#endif
#ifdef EPROTO
        case EPROTO:
            msg = "Protocol error";
            break;
#endif
#ifdef EPROTONOSUPPORT
        case EPROTONOSUPPORT:
            msg = "Protocol not supported";
            break;
#endif
#ifdef EPROTOTYPE
        case EPROTOTYPE:
            msg = "Protocol wrong type for socket";
            break;
#endif
#ifdef ERANGE
        case ERANGE:
            msg = "Math result not representable";
            break;
#endif
#ifdef EREMCHG
        case EREMCHG:
            msg = "Remote address changed";
            break;
#endif
#ifdef EREMOTE
        case EREMOTE:
            msg = "Object is remote";
            break;
#endif
#ifdef EREMOTEIO
        case EREMOTEIO:
            msg = "Remote I/O error";
            break;
#endif
#ifdef ERESTART
        case ERESTART:
            msg = "Interrupted system call should be restarted";
            break;
#endif
#ifdef ERFKILL
        case ERFKILL:
            msg = "Operation not possible due to RF-kill";
            break;
#endif
#ifdef EROFS
        case EROFS:
            msg = "Read-only file system";
            break;
#endif
#ifdef ESHUTDOWN
        case ESHUTDOWN:
            msg = "Cannot send after transport endpoint shutdown";
            break;
#endif
#ifdef ESOCKTNOSUPPORT
        case ESOCKTNOSUPPORT:
            msg = "Socket type not supported";
            break;
#endif
#ifdef ESPIPE
        case ESPIPE:
            msg = "Illegal seek";
            break;
#endif
#ifdef ESRCH
        case ESRCH:
            msg = "No such process";
            break;
#endif
#ifdef ESRMNT
        case ESRMNT:
            msg = "Srmount error";
            break;
#endif
#ifdef ESTALE
        case ESTALE:
            msg = "Stale file handle";
            break;
#endif
#ifdef ESTRPIPE
        case ESTRPIPE:
            msg = "Streams pipe error";
            break;
#endif
#ifdef ETIME
        case ETIME:
            msg = "Timer expired";
            break;
#endif
#ifdef ETIMEDOUT
        case ETIMEDOUT:
            msg = "Connection timed out";
            break;
#endif
#ifdef ETOOMANYREFS
        case ETOOMANYREFS:
            msg = "Too many references: cannot splice";
            break;
#endif
#ifdef ETXTBSY
        case ETXTBSY:
            msg = "Text file busy";
            break;
#endif
#ifdef EUCLEAN
        case EUCLEAN:
            msg = "Structure needs cleaning";
            break;
#endif
#ifdef EUNATCH
        case EUNATCH:
            msg = "Protocol driver not attached";
            break;
#endif
#ifdef EUSERS
        case EUSERS:
            msg = "Too many users";
            break;
#endif
#ifdef EXDEV
        case EXDEV:
            msg = "Cross-device link";
            break;
#endif
#ifdef EXFULL
        case EXFULL:
            msg = "Exchange full";
            break;
#endif

        // Additional error macros from Darwin.
#ifdef EAUTH
        case EAUTH:
            msg = "Authentication error";
            break;
#endif
#ifdef EBADARCH
        case EBADARCH:
            msg = "Bad CPU type in executable";
            break;
#endif
#ifdef EBADEXEC
        case EBADEXEC:
            msg = "Bad executable";
            break;
#endif
#ifdef EBADMACHO
        case EBADMACHO:
            msg = "Malformed Macho file";
            break;
#endif
#ifdef EBADRPC
        case EBADRPC:
            msg = "RPC struct is bad";
            break;
#endif
#ifdef EDEVERR
        case EDEVERR:
            msg = "Device error, e.g. paper out";
            break;
#endif
#ifdef EFTYPE
        case EFTYPE:
            msg = "Inappropriate file type or format";
            break;
#endif
#ifdef ENEEDAUTH
        case ENEEDAUTH:
            msg = "Need authenticator";
            break;
#endif
#ifdef ENOATTR
        case ENOATTR:
            msg = "Attribute not found";
            break;
#endif
#ifdef ENOPOLICY
        case ENOPOLICY:
            msg = "No such policy registered";
            break;
#endif
#ifdef EPROCLIM
        case EPROCLIM:
            msg = "Too many processes";
            break;
#endif
#ifdef EPROCUNAVAIL
        case EPROCUNAVAIL:
            msg = "Bad procedure for program";
            break;
#endif
#ifdef EPROGMISMATCH
        case EPROGMISMATCH:
            msg = "Program version wrong";
            break;
#endif
#ifdef EPROGUNAVAIL
        case EPROGUNAVAIL:
            msg = "RPC prog. not avail";
            break;
#endif
#ifdef EPWROFF
        case EPWROFF:
            msg = "Device power is off";
            break;
#endif
#ifdef EQFULL
        case EQFULL:
            msg = "Interface output queue is full";
            break;
#endif
#ifdef ERPCMISMATCH
        case ERPCMISMATCH:
            msg = "RPC version wrong";
            break;
#endif
#ifdef ESHLIBVERS
        case ESHLIBVERS:
            msg = "Shared library version mismatch";
            break;
#endif

        // These macros might share an error code
#if defined(EDEADLOCK) && (!defined(EDEADLK) || EDEADLOCK != EDEADLK)
        case EDEADLOCK:
            msg = "Resource deadlock would occur";
            break;
#endif
#if defined(ENOTSUP) && (!defined(EOPNOTSUPP) || ENOTSUP != EOPNOTSUPP)
        case ENOTSUP:
            msg = "Operation not supported";
            break;
#endif
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || EWOULDBLOCK != EAGAIN)
        case EWOULDBLOCK:
            msg = "Operation would block";
            break;
#endif

        default:
            msg = NULL;
            break;
    }

    return msg;
}

