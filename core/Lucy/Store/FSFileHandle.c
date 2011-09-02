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

#define C_LUCY_FSFILEHANDLE
#define C_LUCY_FILEWINDOW
#include "Lucy/Util/ToolSet.h"

#include <errno.h>
#include <stdio.h>
#include <fcntl.h> // open, POSIX flags

#ifdef CHY_HAS_UNISTD_H
  #include <unistd.h> // close
#endif

#ifdef CHY_HAS_SYS_MMAN_H
  #include <sys/mman.h>
#elif defined(CHY_HAS_WINDOWS_H)
  #include <windows.h>
  #include <io.h>
#else
  #error "No support for memory mapped files"
#endif

#include "Lucy/Store/FSFileHandle.h"
#include "Lucy/Store/FileWindow.h"

// Convert FileHandle flags to POSIX flags.
static INLINE int
SI_posix_flags(uint32_t fh_flags) {
    int posix_flags = 0;
    if (fh_flags & FH_WRITE_ONLY) { posix_flags |= O_WRONLY; }
    if (fh_flags & FH_READ_ONLY)  { posix_flags |= O_RDONLY; }
    if (fh_flags & FH_CREATE)     { posix_flags |= O_CREAT; }
    if (fh_flags & FH_EXCLUSIVE)  { posix_flags |= O_EXCL; }
#ifdef O_LARGEFILE
    posix_flags |= O_LARGEFILE;
#endif
#ifdef _O_BINARY
    posix_flags |= _O_BINARY;
#endif
    return posix_flags;
}

#define IS_64_BIT (SIZEOF_PTR == 8 ? true : false)

// Memory map a region of the file with shared (read-only) permissions.  If
// the requested length is 0, return NULL.  If an error occurs, return NULL
// and set Err_error.
static INLINE void*
SI_map(FSFileHandle *self, int64_t offset, int64_t len);

// Release a memory mapped region assigned by SI_map.
static INLINE bool_t
SI_unmap(FSFileHandle *self, char *ptr, int64_t len);

// 32-bit or 64-bit inlined helpers for FSFH_window.
static INLINE bool_t
SI_window(FSFileHandle *self, FileWindow *window, int64_t offset, int64_t len);

// Architecture- and OS- specific initialization for a read-only FSFileHandle.
static INLINE bool_t
SI_init_read_only(FSFileHandle *self);

// Windows-specific routine needed for closing read-only handles.
#ifdef CHY_HAS_WINDOWS_H
static INLINE bool_t
SI_close_win_handles(FSFileHandle *self);
#endif

FSFileHandle*
FSFH_open(const CharBuf *path, uint32_t flags) {
    FSFileHandle *self = (FSFileHandle*)VTable_Make_Obj(FSFILEHANDLE);
    return FSFH_do_open(self, path, flags);
}

FSFileHandle*
FSFH_do_open(FSFileHandle *self, const CharBuf *path, uint32_t flags) {
    FH_do_open((FileHandle*)self, path, flags);
    if (!path || !CB_Get_Size(path)) {
        Err_set_error(Err_new(CB_newf("Missing required param 'path'")));
        CFISH_DECREF(self);
        return NULL;
    }

    // Attempt to open file.
    if (flags & FH_WRITE_ONLY) {
        self->fd = open((char*)CB_Get_Ptr8(path), SI_posix_flags(flags), 0666);
        if (self->fd == -1) {
            self->fd = 0;
            Err_set_error(Err_new(CB_newf("Attempt to open '%o' failed: %s",
                                          path, strerror(errno))));
            CFISH_DECREF(self);
            return NULL;
        }
        if (flags & FH_EXCLUSIVE) {
            self->len = 0;
        }
        else {
            // Derive length.
            self->len = lseek64(self->fd, I64_C(0), SEEK_END);
            if (self->len == -1) {
                Err_set_error(Err_new(CB_newf("lseek64 on %o failed: %s",
                                              self->path, strerror(errno))));
                CFISH_DECREF(self);
                return NULL;
            }
            else {
                int64_t check_val = lseek64(self->fd, I64_C(0), SEEK_SET);
                if (check_val == -1) {
                    Err_set_error(Err_new(CB_newf("lseek64 on %o failed: %s",
                                                  self->path, strerror(errno))));
                    CFISH_DECREF(self);
                    return NULL;
                }
            }
        }
    }
    else if (flags & FH_READ_ONLY) {
        if (SI_init_read_only(self)) {
            // On 64-bit systems, map the whole file up-front.
            if (IS_64_BIT && self->len) {
                self->buf = (char*)SI_map(self, 0, self->len);
                if (!self->buf) {
                    // An error occurred during SI_map, which has set
                    // Err_error for us already.
                    CFISH_DECREF(self);
                    return NULL;
                }
            }
        }
        else {
            CFISH_DECREF(self);
            return NULL;
        }
    }
    else {
        Err_set_error(Err_new(CB_newf("Must specify FH_READ_ONLY or FH_WRITE_ONLY to open '%o'",
                                      path)));
        CFISH_DECREF(self);
        return NULL;
    }

    return self;
}

bool_t
FSFH_close(FSFileHandle *self) {
    // On 64-bit systems, cancel the whole-file mapping.
    if (IS_64_BIT && (self->flags & FH_READ_ONLY) && self->buf != NULL) {
        if (!SI_unmap(self, self->buf, self->len)) { return false; }
        self->buf = NULL;
    }

    // Close system-specific handles.
    if (self->fd) {
        if (close(self->fd)) {
            Err_set_error(Err_new(CB_newf("Failed to close file: %s",
                                          strerror(errno))));
            return false;
        }
        self->fd  = 0;
    }
    #if (defined(CHY_HAS_WINDOWS_H) && !defined(CHY_HAS_SYS_MMAN_H))
    if (self->win_fhandle) {
        if (!SI_close_win_handles(self)) { return false; }
    }
    #endif

    return true;
}

bool_t
FSFH_write(FSFileHandle *self, const void *data, size_t len) {
    if (len) {
        // Write data, track file length, check for errors.
        int64_t check_val = write(self->fd, data, len);
        self->len += check_val;
        if ((size_t)check_val != len) {
            if (check_val == -1) {
                Err_set_error(Err_new(CB_newf("Error when writing %u64 bytes: %s",
                                              (uint64_t)len, strerror(errno))));
            }
            else {
                Err_set_error(Err_new(CB_newf("Attempted to write %u64 bytes, but wrote %i64",
                                              (uint64_t)len, check_val)));
            }
            return false;
        }
    }

    return true;
}

int64_t
FSFH_length(FSFileHandle *self) {
    return self->len;
}

bool_t
FSFH_window(FSFileHandle *self, FileWindow *window, int64_t offset,
            int64_t len) {
    const int64_t end = offset + len;
    if (!(self->flags & FH_READ_ONLY)) {
        Err_set_error(Err_new(CB_newf("Can't read from write-only handle")));
        return false;
    }
    else if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from negative offset %i64",
                                      offset)));
        return false;
    }
    else if (end > self->len) {
        Err_set_error(Err_new(CB_newf("Tried to read past EOF: offset %i64 + request %i64 > len %i64",
                                      offset, len, self->len)));
        return false;
    }
    else {
        return SI_window(self, window, offset, len);
    }
}

/********************************* 64-bit *********************************/

#if IS_64_BIT

static INLINE bool_t
SI_window(FSFileHandle *self, FileWindow *window, int64_t offset,
          int64_t len) {
    FileWindow_Set_Window(window, self->buf + offset, offset, len);
    return true;
}

bool_t
FSFH_release_window(FSFileHandle *self, FileWindow *window) {
    UNUSED_VAR(self);
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

bool_t
FSFH_read(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    const int64_t end = offset + len;

    if (self->flags & FH_WRITE_ONLY) {
        Err_set_error(Err_new(CB_newf("Can't read from write-only filehandle")));
        return false;
    }
    if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from an offset less than 0 (%i64)",
                                      offset)));
        return false;
    }
    else if (end > self->len) {
        Err_set_error(Err_new(CB_newf("Tried to read past EOF: offset %i64 + request %u64 > len %i64",
                                      offset, (uint64_t)len, self->len)));
        return false;
    }
    memcpy(dest, self->buf + offset, len);
    return true;
}

/********************************* 32-bit *********************************/

#else

static INLINE bool_t
SI_window(FSFileHandle *self, FileWindow *window, int64_t offset,
          int64_t len) {
    // Release the previously mmap'd region, if any.
    FSFH_release_window(self, window);

    {
        // Start map on a page boundary.  Ensure that the window is at
        // least wide enough to view all the data spec'd in the original
        // request.
        const int64_t remainder       = offset % self->page_size;
        const int64_t adjusted_offset = offset - remainder;
        const int64_t adjusted_len    = len + remainder;
        char *const buf
            = (char*)SI_map(self, adjusted_offset, adjusted_len);
        if (len && buf == NULL) {
            return false;
        }
        else {
            FileWindow_Set_Window(window, buf, adjusted_offset,
                                  adjusted_len);
        }
    }

    return true;
}

bool_t
FSFH_release_window(FSFileHandle *self, FileWindow *window) {
    if (!SI_unmap(self, window->buf, window->len)) { return false; }
    FileWindow_Set_Window(window, NULL, 0, 0);
    return true;
}

#endif // IS_64_BIT vs. 32-bit

/********************************* UNIXEN *********************************/

#ifdef CHY_HAS_SYS_MMAN_H

static INLINE bool_t
SI_init_read_only(FSFileHandle *self) {
    // Open.
    self->fd = open((char*)CB_Get_Ptr8(self->path),
                    SI_posix_flags(self->flags), 0666);
    if (self->fd == -1) {
        self->fd = 0;
        Err_set_error(Err_new(CB_newf("Can't open '%o': %s", self->path,
                                      strerror(errno))));
        return false;
    }

    // Derive len.
    self->len = lseek64(self->fd, I64_C(0), SEEK_END);
    if (self->len == -1) {
        Err_set_error(Err_new(CB_newf("lseek64 on %o failed: %s", self->path,
                                      strerror(errno))));
        return false;
    }
    else {
        int64_t check_val = lseek64(self->fd, I64_C(0), SEEK_SET);
        if (check_val == -1) {
            Err_set_error(Err_new(CB_newf("lseek64 on %o failed: %s",
                                          self->path, strerror(errno))));
            return false;
        }
    }

    // Get system page size.
#if defined(_SC_PAGESIZE)
    self->page_size = sysconf(_SC_PAGESIZE);
#elif defined(_SC_PAGE_SIZE)
    self->page_size = sysconf(_SC_PAGE_SIZE);
#else
    #error "Can't determine system memory page size"
#endif

    return true;
}

static INLINE void*
SI_map(FSFileHandle *self, int64_t offset, int64_t len) {
    void *buf = NULL;

    if (len) {
        // Read-only memory mapping.
        buf = mmap(NULL, len, PROT_READ, MAP_SHARED, self->fd, offset);
        if (buf == (void*)(-1)) {
            Err_set_error(Err_new(CB_newf("mmap of offset %i64 and length %i64 (page size %i64) "
                                          "against '%o' failed: %s",
                                          offset, len, self->page_size,
                                          self->path, strerror(errno))));
            return NULL;
        }
    }

    return buf;
}

static INLINE bool_t
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    if (buf != NULL) {
        if (munmap(buf, len)) {
            Err_set_error(Err_new(CB_newf("Failed to munmap '%o': %s",
                                          self->path, strerror(errno))));
            return false;
        }
    }
    return true;
}

#if !IS_64_BIT
bool_t
FSFH_read(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    int64_t check_val;

    // Sanity check.
    if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from an offset less than 0 (%i64)",
                                      offset)));
        return false;
    }

    // Read.
    check_val = pread64(self->fd, dest, len, offset);
    if (check_val != (int64_t)len) {
        if (check_val == -1) {
            Err_set_error(Err_new(CB_newf("Tried to read %u64 bytes, got %i64: %s",
                                          (uint64_t)len, check_val, strerror(errno))));
        }
        else {
            Err_set_error(Err_new(CB_newf("Tried to read %u64 bytes, got %i64",
                                          (uint64_t)len, check_val)));
        }
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

/********************************* WINDOWS **********************************/

#elif defined(CHY_HAS_WINDOWS_H)

static INLINE bool_t
SI_init_read_only(FSFileHandle *self) {
    LARGE_INTEGER large_int;
    char *filepath = (char*)CB_Get_Ptr8(self->path);
    SYSTEM_INFO sys_info;

    // Get system page size.
    GetSystemInfo(&sys_info);
    self->page_size = sys_info.dwAllocationGranularity;

    // Open.
    self->win_fhandle = CreateFile(
                            filepath,
                            GENERIC_READ,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_READONLY | FILE_FLAG_OVERLAPPED,
                            NULL
                        );
    if (self->win_fhandle == INVALID_HANDLE_VALUE) {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(CB_newf("CreateFile for %o failed: %s",
                                      self->path, win_error)));
        FREEMEM(win_error);
        return false;
    }

    // Derive len.
    GetFileSizeEx(self->win_fhandle, &large_int);
    self->len = large_int.QuadPart;
    if (self->len < 0) {
        Err_set_error(Err_new(CB_newf("GetFileSizeEx for %o returned a negative length: '%i64'",
                                      self->path, self->len)));
        return false;
    }

    // Init mapping handle.
    self->buf = NULL;
    if (self->len) {
        self->win_maphandle = CreateFileMapping(self->win_fhandle, NULL,
                                                PAGE_READONLY, 0, 0, NULL);
        if (self->win_maphandle == NULL) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(CB_newf("CreateFileMapping for %o failed: %s",
                                          self->path, win_error)));
            FREEMEM(win_error);
            return false;
        }
    }

    return true;
}

static INLINE void*
SI_map(FSFileHandle *self, int64_t offset, int64_t len) {
    void *buf = NULL;

    if (len) {
        // Read-only memory map.
        uint64_t offs = (uint64_t)offset;
        DWORD file_offset_hi = offs >> 32;
        DWORD file_offset_lo = offs & 0xFFFFFFFF;
        size_t amount = (size_t)len;
        buf = MapViewOfFile(self->win_maphandle, FILE_MAP_READ,
                            file_offset_hi, file_offset_lo, amount);
        if (buf == NULL) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(CB_newf("MapViewOfFile for %o failed: %s",
                                          self->path, win_error)));
            FREEMEM(win_error);
        }
    }

    return buf;
}

static INLINE bool_t
SI_unmap(FSFileHandle *self, char *buf, int64_t len) {
    if (buf != NULL) {
        if (!UnmapViewOfFile(buf)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(CB_newf("Failed to unmap '%o': %s",
                                          self->path, win_error)));
            FREEMEM(win_error);
            return false;
        }
    }
    return true;
}

static INLINE bool_t
SI_close_win_handles(FSFileHandle *self) {
    // Close both standard handle and mapping handle.
    if (self->win_maphandle) {
        if (!CloseHandle(self->win_maphandle)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(CB_newf("Failed to close file mapping handle: %s",
                                          win_error)));
            FREEMEM(win_error);
            return false;
        }
        self->win_maphandle = NULL;
    }
    if (self->win_fhandle) {
        if (!CloseHandle(self->win_fhandle)) {
            char *win_error = Err_win_error();
            Err_set_error(Err_new(CB_newf("Failed to close file handle: %s",
                                          win_error)));
            FREEMEM(win_error);
            return false;
        }
        self->win_fhandle = NULL;
    }

    return true;
}

#if !IS_64_BIT
bool_t
FSFH_read(FSFileHandle *self, char *dest, int64_t offset, size_t len) {
    BOOL check_val;
    DWORD got;
    OVERLAPPED read_op_state;
    uint64_t offs = (uint64_t)offset;

    read_op_state.hEvent     = NULL;
    read_op_state.OffsetHigh = offs >> 32;
    read_op_state.Offset     = offs & 0xFFFFFFFF;

    // Sanity check.
    if (offset < 0) {
        Err_set_error(Err_new(CB_newf("Can't read from an offset less than 0 (%i64)",
                                      offset)));
        return false;
    }

    // ReadFile() takes a DWORD (unsigned 32-bit integer) as a length
    // argument, so throw a sensible error rather than wrap around.
    if (len > U32_MAX) {
        Err_set_error(Err_new(CB_newf("Can't read more than 4 GB (%u64)",
                                      (uint64_t)len)));
        return false;
    }

    // Read.
    check_val = ReadFile(self->win_fhandle, dest, len, &got, &read_op_state);
    if (!check_val && GetLastError() == ERROR_IO_PENDING) {
        // Read has been queued by the OS and will soon complete.  Wait for
        // it, since this is a blocking IO call from the point of the rest of
        // the library.
        check_val = GetOverlappedResult(self->win_fhandle, &read_op_state,
                                        &got, TRUE);
    }

    // Verify that the read has succeeded by now.
    if (!check_val) {
        char *win_error = Err_win_error();
        Err_set_error(Err_new(CB_newf("Failed to read %u64 bytes: %s",
                                      (uint64_t)len, win_error)));
        FREEMEM(win_error);
        return false;
    }

    return true;
}
#endif // !IS_64_BIT

#endif // CHY_HAS_SYS_MMAN_H vs. CHY_HAS_WINDOWS_H


