﻿/* https://en.wikipedia.org/wiki/Operating_system_abstraction_layer */

/*
 * Copyright 2015-2019 Leonid Yuriev <leo@yuriev.ru>
 * and other libmdbx authors: please see AUTHORS file.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#include "./bits.h"

#if defined(_WIN32) || defined(_WIN64)

static int waitstatus2errcode(DWORD result) {
  switch (result) {
  case WAIT_OBJECT_0:
    return MDBX_SUCCESS;
  case WAIT_FAILED:
    return GetLastError();
  case WAIT_ABANDONED:
    return ERROR_ABANDONED_WAIT_0;
  case WAIT_IO_COMPLETION:
    return ERROR_USER_APC;
  case WAIT_TIMEOUT:
    return ERROR_TIMEOUT;
  default:
    return ERROR_UNHANDLED_ERROR;
  }
}

/* Map a result from an NTAPI call to WIN32 error code. */
static int ntstatus2errcode(NTSTATUS status) {
  DWORD dummy;
  OVERLAPPED ov;
  memset(&ov, 0, sizeof(ov));
  ov.Internal = status;
  return GetOverlappedResult(NULL, &ov, &dummy, FALSE) ? MDBX_SUCCESS
                                                       : GetLastError();
}

/* We use native NT APIs to setup the memory map, so that we can
 * let the DB file grow incrementally instead of always preallocating
 * the full size. These APIs are defined in <wdm.h> and <ntifs.h>
 * but those headers are meant for driver-level development and
 * conflict with the regular user-level headers, so we explicitly
 * declare them here. Using these APIs also means we must link to
 * ntdll.dll, which is not linked by default in user code. */
#pragma comment(lib, "ntdll.lib")
#ifdef MDBX_AVOID_CRT
#pragma comment(lib, "mdbx_ntdll_extra.lib")
#endif

extern NTSTATUS NTAPI NtCreateSection(
    OUT PHANDLE SectionHandle, IN ACCESS_MASK DesiredAccess,
    IN OPTIONAL POBJECT_ATTRIBUTES ObjectAttributes,
    IN OPTIONAL PLARGE_INTEGER MaximumSize, IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes, IN OPTIONAL HANDLE FileHandle);

typedef struct _SECTION_BASIC_INFORMATION {
  ULONG Unknown;
  ULONG SectionAttributes;
  LARGE_INTEGER SectionSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS {
  SectionBasicInformation,
  SectionImageInformation,
  SectionRelocationInformation, // name:wow64:whNtQuerySection_SectionRelocationInformation
  MaxSectionInfoClass
} SECTION_INFORMATION_CLASS;

extern NTSTATUS NTAPI NtQuerySection(
    IN HANDLE SectionHandle, IN SECTION_INFORMATION_CLASS InformationClass,
    OUT PVOID InformationBuffer, IN ULONG InformationBufferSize,
    OUT PULONG ResultLength OPTIONAL);

extern NTSTATUS NTAPI NtExtendSection(IN HANDLE SectionHandle,
                                      IN PLARGE_INTEGER NewSectionSize);

typedef enum _SECTION_INHERIT { ViewShare = 1, ViewUnmap = 2 } SECTION_INHERIT;

extern NTSTATUS NTAPI NtMapViewOfSection(
    IN HANDLE SectionHandle, IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits, IN SIZE_T CommitSize,
    IN OUT OPTIONAL PLARGE_INTEGER SectionOffset, IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition, IN ULONG AllocationType,
    IN ULONG Win32Protect);

extern NTSTATUS NTAPI NtUnmapViewOfSection(IN HANDLE ProcessHandle,
                                           IN OPTIONAL PVOID BaseAddress);

extern NTSTATUS NTAPI NtClose(HANDLE Handle);

extern NTSTATUS NTAPI NtAllocateVirtualMemory(
    IN HANDLE ProcessHandle, IN OUT PVOID *BaseAddress, IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize, IN ULONG AllocationType, IN ULONG Protect);

extern NTSTATUS NTAPI NtFreeVirtualMemory(IN HANDLE ProcessHandle,
                                          IN PVOID *BaseAddress,
                                          IN OUT PSIZE_T RegionSize,
                                          IN ULONG FreeType);

#ifndef WOF_CURRENT_VERSION
typedef struct _WOF_EXTERNAL_INFO {
  DWORD Version;
  DWORD Provider;
} WOF_EXTERNAL_INFO, *PWOF_EXTERNAL_INFO;
#endif /* WOF_CURRENT_VERSION */

#ifndef WIM_PROVIDER_CURRENT_VERSION
#define WIM_PROVIDER_HASH_SIZE 20

typedef struct _WIM_PROVIDER_EXTERNAL_INFO {
  DWORD Version;
  DWORD Flags;
  LARGE_INTEGER DataSourceId;
  BYTE ResourceHash[WIM_PROVIDER_HASH_SIZE];
} WIM_PROVIDER_EXTERNAL_INFO, *PWIM_PROVIDER_EXTERNAL_INFO;
#endif /* WIM_PROVIDER_CURRENT_VERSION */

#ifndef FILE_PROVIDER_CURRENT_VERSION
typedef struct _FILE_PROVIDER_EXTERNAL_INFO_V1 {
  ULONG Version;
  ULONG Algorithm;
  ULONG Flags;
} FILE_PROVIDER_EXTERNAL_INFO_V1, *PFILE_PROVIDER_EXTERNAL_INFO_V1;
#endif /* FILE_PROVIDER_CURRENT_VERSION */

#ifndef STATUS_OBJECT_NOT_EXTERNALLY_BACKED
#define STATUS_OBJECT_NOT_EXTERNALLY_BACKED ((NTSTATUS)0xC000046DL)
#endif
#ifndef STATUS_INVALID_DEVICE_REQUEST
#define STATUS_INVALID_DEVICE_REQUEST ((NTSTATUS)0xC0000010L)
#endif

#ifndef FILE_DEVICE_FILE_SYSTEM
#define FILE_DEVICE_FILE_SYSTEM 0x00000009
#endif

#ifndef FSCTL_GET_EXTERNAL_BACKING
#define FSCTL_GET_EXTERNAL_BACKING                                             \
  CTL_CODE(FILE_DEVICE_FILE_SYSTEM, 196, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

#endif /* _WIN32 || _WIN64 */

/*----------------------------------------------------------------------------*/

#if _POSIX_C_SOURCE > 200212 &&                                                \
    /* workaround for avoid musl libc wrong prototype */ (                     \
        defined(__GLIBC__) || defined(__GNU_LIBRARY__))
/* Prototype should match libc runtime. ISO POSIX (2003) & LSB 1.x-3.x */
__extern_C void __assert_fail(const char *assertion, const char *file,
                              unsigned line, const char *function)
#ifdef __THROW
    __THROW
#else
    __nothrow
#endif /* __THROW */
    __noreturn;

#elif defined(__APPLE__) || defined(__MACH__)
__extern_C void __assert_rtn(const char *function, const char *file, int line,
                             const char *assertion) /* __nothrow */
#ifdef __dead2
    __dead2
#else
    __noreturn
#endif /* __dead2 */
#ifdef __disable_tail_calls
    __disable_tail_calls
#endif /* __disable_tail_calls */
    ;

#define __assert_fail(assertion, file, line, function)                         \
  __assert_rtn(function, file, line, assertion)
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||   \
    defined(__BSD__) || defined(__NETBSD__) || defined(__bsdi__) ||            \
    defined(__DragonFly__)
__extern_C void __assert(const char *function, const char *file, int line,
                         const char *assertion) /* __nothrow */
#ifdef __dead2
    __dead2
#else
    __noreturn
#endif /* __dead2 */
#ifdef __disable_tail_calls
    __disable_tail_calls
#endif /* __disable_tail_calls */
    ;
#define __assert_fail(assertion, file, line, function)                         \
  __assert(function, file, line, assertion)

#endif /* __assert_fail */

void __cold mdbx_assert_fail(const MDBX_env *env, const char *msg,
                             const char *func, int line) {
#if MDBX_DEBUG
  if (env && env->me_assert_func) {
    env->me_assert_func(env, msg, func, line);
    return;
  }
#else
  (void)env;
#endif /* MDBX_DEBUG */

  if (mdbx_debug_logger)
    mdbx_debug_log(MDBX_DBG_ASSERT, func, line, "assert: %s\n", msg);
  else {
#if defined(_WIN32) || defined(_WIN64)
    char *message = nullptr;
    const int num = mdbx_asprintf(&message, "\r\nMDBX-ASSERTION: %s, %s:%u",
                                  msg, func ? func : "unknown", line);
    if (num < 1 || !message)
      message = "<troubles with assertion-message preparation>";
    OutputDebugStringA(message);
    if (IsDebuggerPresent())
      DebugBreak();
#else
    __assert_fail(msg, "mdbx", line, func);
#endif
  }

#if defined(_WIN32) || defined(_WIN64)
  FatalExit(ERROR_UNHANDLED_ERROR);
#else
  abort();
#endif
}

__cold void mdbx_panic(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char *message = nullptr;
  const int num = mdbx_vasprintf(&message, fmt, ap);
  va_end(ap);
  if (num < 1 || !message)
    message = "<troubles with panic-message preparation>";

#if defined(_WIN32) || defined(_WIN64)
  OutputDebugStringA("\r\nMDBX-PANIC: ");
  OutputDebugStringA(message);
  if (IsDebuggerPresent())
    DebugBreak();
  FatalExit(ERROR_UNHANDLED_ERROR);
#else
  __assert_fail(message, "mdbx", 0, "panic");
  abort();
#endif
}

/*----------------------------------------------------------------------------*/

#ifndef mdbx_vasprintf
int mdbx_vasprintf(char **strp, const char *fmt, va_list ap) {
  va_list ones;
  va_copy(ones, ap);
  int needed = vsnprintf(nullptr, 0, fmt, ap);

  if (unlikely(needed < 0 || needed >= INT_MAX)) {
    *strp = nullptr;
    va_end(ones);
    return needed;
  }

  *strp = mdbx_malloc(needed + 1);
  if (unlikely(*strp == nullptr)) {
    va_end(ones);
#if defined(_WIN32) || defined(_WIN64)
    SetLastError(MDBX_ENOMEM);
#else
    errno = MDBX_ENOMEM;
#endif
    return -1;
  }

  int actual = vsnprintf(*strp, needed + 1, fmt, ones);
  va_end(ones);

  assert(actual == needed);
  if (unlikely(actual < 0)) {
    mdbx_free(*strp);
    *strp = nullptr;
  }
  return actual;
}
#endif /* mdbx_vasprintf */

#ifndef mdbx_asprintf
int mdbx_asprintf(char **strp, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int rc = mdbx_vasprintf(strp, fmt, ap);
  va_end(ap);
  return rc;
}
#endif /* mdbx_asprintf */

#ifndef mdbx_memalign_alloc
int mdbx_memalign_alloc(size_t alignment, size_t bytes, void **result) {
#if defined(_WIN32) || defined(_WIN64)
  (void)alignment;
  *result = VirtualAlloc(NULL, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  return *result ? MDBX_SUCCESS : MDBX_ENOMEM /* ERROR_OUTOFMEMORY */;
#elif defined(_ISOC11_SOURCE)
  *result = aligned_alloc(alignment, bytes);
  return *result ? MDBX_SUCCESS : errno;
#elif _POSIX_VERSION >= 200112L
  *result = nullptr;
  return posix_memalign(result, alignment, bytes);
#elif __GLIBC_PREREQ(2, 16) || __STDC_VERSION__ >= 201112L
  *result = memalign(alignment, bytes);
  return *result ? MDBX_SUCCESS : errno;
#else
#error FIXME
#endif
}
#endif /* mdbx_memalign_alloc */

#ifndef mdbx_memalign_free
void mdbx_memalign_free(void *ptr) {
#if defined(_WIN32) || defined(_WIN64)
  VirtualFree(ptr, 0, MEM_RELEASE);
#else
  mdbx_free(ptr);
#endif
}
#endif /* mdbx_memalign_free */

#ifndef mdbx_strdup
char *mdbx_strdup(const char *str) {
  if (!str)
    return NULL;
  size_t bytes = strlen(str) + 1;
  char *dup = mdbx_malloc(bytes);
  if (dup)
    memcpy(dup, str, bytes);
  return dup;
}
#endif /* mdbx_strdup */

/*----------------------------------------------------------------------------*/

int mdbx_condmutex_init(mdbx_condmutex_t *condmutex) {
#if defined(_WIN32) || defined(_WIN64)
  int rc = MDBX_SUCCESS;
  condmutex->event = NULL;
  condmutex->mutex = CreateMutex(NULL, FALSE, NULL);
  if (!condmutex->mutex)
    return GetLastError();

  condmutex->event = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (!condmutex->event) {
    rc = GetLastError();
    (void)CloseHandle(condmutex->mutex);
    condmutex->mutex = NULL;
  }
  return rc;
#else
  memset(condmutex, 0, sizeof(mdbx_condmutex_t));
  int rc = pthread_mutex_init(&condmutex->mutex, NULL);
  if (rc == 0) {
    rc = pthread_cond_init(&condmutex->cond, NULL);
    if (rc != 0)
      (void)pthread_mutex_destroy(&condmutex->mutex);
  }
  return rc;
#endif
}

static bool is_allzeros(const void *ptr, size_t bytes) {
  const uint8_t *u8 = ptr;
  for (size_t i = 0; i < bytes; ++i)
    if (u8[i] != 0)
      return false;
  return true;
}

int mdbx_condmutex_destroy(mdbx_condmutex_t *condmutex) {
  int rc = MDBX_EINVAL;
#if defined(_WIN32) || defined(_WIN64)
  if (condmutex->event) {
    rc = CloseHandle(condmutex->event) ? MDBX_SUCCESS : GetLastError();
    if (rc == MDBX_SUCCESS)
      condmutex->event = NULL;
  }
  if (condmutex->mutex) {
    rc = CloseHandle(condmutex->mutex) ? MDBX_SUCCESS : GetLastError();
    if (rc == MDBX_SUCCESS)
      condmutex->mutex = NULL;
  }
#else
  if (!is_allzeros(&condmutex->cond, sizeof(condmutex->cond))) {
    rc = pthread_cond_destroy(&condmutex->cond);
    if (rc == 0)
      memset(&condmutex->cond, 0, sizeof(condmutex->cond));
  }
  if (!is_allzeros(&condmutex->mutex, sizeof(condmutex->mutex))) {
    rc = pthread_mutex_destroy(&condmutex->mutex);
    if (rc == 0)
      memset(&condmutex->mutex, 0, sizeof(condmutex->mutex));
  }
#endif
  return rc;
}

int mdbx_condmutex_lock(mdbx_condmutex_t *condmutex) {
#if defined(_WIN32) || defined(_WIN64)
  DWORD code = WaitForSingleObject(condmutex->mutex, INFINITE);
  return waitstatus2errcode(code);
#else
  return pthread_mutex_lock(&condmutex->mutex);
#endif
}

int mdbx_condmutex_unlock(mdbx_condmutex_t *condmutex) {
#if defined(_WIN32) || defined(_WIN64)
  return ReleaseMutex(condmutex->mutex) ? MDBX_SUCCESS : GetLastError();
#else
  return pthread_mutex_unlock(&condmutex->mutex);
#endif
}

int mdbx_condmutex_signal(mdbx_condmutex_t *condmutex) {
#if defined(_WIN32) || defined(_WIN64)
  return SetEvent(condmutex->event) ? MDBX_SUCCESS : GetLastError();
#else
  return pthread_cond_signal(&condmutex->cond);
#endif
}

int mdbx_condmutex_wait(mdbx_condmutex_t *condmutex) {
#if defined(_WIN32) || defined(_WIN64)
  DWORD code =
      SignalObjectAndWait(condmutex->mutex, condmutex->event, INFINITE, FALSE);
  if (code == WAIT_OBJECT_0)
    code = WaitForSingleObject(condmutex->mutex, INFINITE);
  return waitstatus2errcode(code);
#else
  return pthread_cond_wait(&condmutex->cond, &condmutex->mutex);
#endif
}

/*----------------------------------------------------------------------------*/

int mdbx_fastmutex_init(mdbx_fastmutex_t *fastmutex) {
#if defined(_WIN32) || defined(_WIN64)
  InitializeCriticalSection(fastmutex);
  return MDBX_SUCCESS;
#else
  return pthread_mutex_init(fastmutex, NULL);
#endif
}

int mdbx_fastmutex_destroy(mdbx_fastmutex_t *fastmutex) {
#if defined(_WIN32) || defined(_WIN64)
  DeleteCriticalSection(fastmutex);
  return MDBX_SUCCESS;
#else
  return pthread_mutex_destroy(fastmutex);
#endif
}

int mdbx_fastmutex_acquire(mdbx_fastmutex_t *fastmutex) {
#if defined(_WIN32) || defined(_WIN64)
  EnterCriticalSection(fastmutex);
  return MDBX_SUCCESS;
#else
  return pthread_mutex_lock(fastmutex);
#endif
}

int mdbx_fastmutex_release(mdbx_fastmutex_t *fastmutex) {
#if defined(_WIN32) || defined(_WIN64)
  LeaveCriticalSection(fastmutex);
  return MDBX_SUCCESS;
#else
  return pthread_mutex_unlock(fastmutex);
#endif
}

/*----------------------------------------------------------------------------*/

int mdbx_removefile(const char *pathname) {
#if defined(_WIN32) || defined(_WIN64)
  return DeleteFileA(pathname) ? MDBX_SUCCESS : GetLastError();
#else
  return unlink(pathname) ? errno : MDBX_SUCCESS;
#endif
}
int mdbx_openfile(const char *pathname, int flags, mode_t mode,
                  mdbx_filehandle_t *fd, bool exclusive) {
  *fd = INVALID_HANDLE_VALUE;
#if defined(_WIN32) || defined(_WIN64)
  (void)mode;

  DWORD DesiredAccess, ShareMode;
  DWORD FlagsAndAttributes = FILE_ATTRIBUTE_NORMAL;
  switch (flags & (O_RDONLY | O_WRONLY | O_RDWR)) {
  default:
    return ERROR_INVALID_PARAMETER;
  case O_RDONLY:
    DesiredAccess = GENERIC_READ;
    ShareMode =
        exclusive ? FILE_SHARE_READ : (FILE_SHARE_READ | FILE_SHARE_WRITE);
    break;
  case O_WRONLY: /* assume for MDBX_env_copy() and friends output */
    DesiredAccess = GENERIC_WRITE;
    ShareMode = 0;
    FlagsAndAttributes |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;
    break;
  case O_RDWR:
    DesiredAccess = GENERIC_READ | GENERIC_WRITE;
    ShareMode = exclusive ? 0 : (FILE_SHARE_READ | FILE_SHARE_WRITE);
    break;
  }

  DWORD CreationDisposition;
  switch (flags & (O_EXCL | O_CREAT)) {
  default:
    return ERROR_INVALID_PARAMETER;
  case 0:
    CreationDisposition = OPEN_EXISTING;
    break;
  case O_EXCL | O_CREAT:
    CreationDisposition = CREATE_NEW;
    FlagsAndAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    break;
  case O_CREAT:
    CreationDisposition = OPEN_ALWAYS;
    FlagsAndAttributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    break;
  }

  *fd = CreateFileA(pathname, DesiredAccess, ShareMode, NULL,
                    CreationDisposition, FlagsAndAttributes, NULL);

  if (*fd == INVALID_HANDLE_VALUE)
    return GetLastError();
  if ((flags & O_CREAT) && GetLastError() != ERROR_ALREADY_EXISTS) {
    /* set FILE_ATTRIBUTE_NOT_CONTENT_INDEXED for new file */
    DWORD FileAttributes = GetFileAttributesA(pathname);
    if (FileAttributes == INVALID_FILE_ATTRIBUTES ||
        !SetFileAttributesA(pathname, FileAttributes |
                                          FILE_ATTRIBUTE_NOT_CONTENT_INDEXED)) {
      int rc = GetLastError();
      CloseHandle(*fd);
      *fd = INVALID_HANDLE_VALUE;
      return rc;
    }
  }
#else
  (void)exclusive;
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif /* O_CLOEXEC */
  *fd = open(pathname, flags, mode);
  if (*fd < 0)
    return errno;

#if defined(FD_CLOEXEC) && !defined(O_CLOEXEC)
  int fd_flags = fcntl(*fd, F_GETFD);
  if (fd_flags != -1)
    (void)fcntl(*fd, F_SETFD, fd_flags | FD_CLOEXEC);
#endif /* FD_CLOEXEC && !O_CLOEXEC */

  if ((flags & (O_RDONLY | O_WRONLY | O_RDWR)) == O_WRONLY) {
    /* assume for MDBX_env_copy() and friends output */
#if defined(O_DIRECT)
    int fd_flags = fcntl(*fd, F_GETFD);
    if (fd_flags != -1)
      (void)fcntl(*fd, F_SETFL, fd_flags | O_DIRECT);
#endif /* O_DIRECT */
#if defined(F_NOCACHE)
    (void)fcntl(*fd, F_NOCACHE, 1);
#endif /* F_NOCACHE */
  }
#endif

  return MDBX_SUCCESS;
}

int mdbx_closefile(mdbx_filehandle_t fd) {
#if defined(_WIN32) || defined(_WIN64)
  return CloseHandle(fd) ? MDBX_SUCCESS : GetLastError();
#else
  return (close(fd) == 0) ? MDBX_SUCCESS : errno;
#endif
}

int mdbx_pread(mdbx_filehandle_t fd, void *buf, size_t bytes, uint64_t offset) {
  if (bytes > MAX_WRITE)
    return MDBX_EINVAL;
#if defined(_WIN32) || defined(_WIN64)
  OVERLAPPED ov;
  ov.hEvent = 0;
  ov.Offset = (DWORD)offset;
  ov.OffsetHigh = HIGH_DWORD(offset);

  DWORD read = 0;
  if (unlikely(!ReadFile(fd, buf, (DWORD)bytes, &read, &ov))) {
    int rc = GetLastError();
    return (rc == MDBX_SUCCESS) ? /* paranoia */ ERROR_READ_FAULT : rc;
  }
#else
  STATIC_ASSERT_MSG(sizeof(off_t) >= sizeof(size_t),
                    "libmdbx requires 64-bit file I/O on 64-bit systems");
  intptr_t read = pread(fd, buf, bytes, offset);
  if (read < 0) {
    int rc = errno;
    return (rc == MDBX_SUCCESS) ? /* paranoia */ MDBX_EIO : rc;
  }
#endif
  return (bytes == (size_t)read) ? MDBX_SUCCESS : MDBX_ENODATA;
}

int mdbx_pwrite(mdbx_filehandle_t fd, const void *buf, size_t bytes,
                uint64_t offset) {
  while (true) {
#if defined(_WIN32) || defined(_WIN64)
    OVERLAPPED ov;
    ov.hEvent = 0;
    ov.Offset = (DWORD)offset;
    ov.OffsetHigh = HIGH_DWORD(offset);

    DWORD written;
    if (unlikely(!WriteFile(fd, buf,
                            (bytes <= MAX_WRITE) ? (DWORD)bytes : MAX_WRITE,
                            &written, &ov)))
      return GetLastError();
    if (likely(bytes == written))
      return MDBX_SUCCESS;
#else
    STATIC_ASSERT_MSG(sizeof(off_t) >= sizeof(size_t),
                      "libmdbx requires 64-bit file I/O on 64-bit systems");
    const intptr_t written =
        pwrite(fd, buf, (bytes <= MAX_WRITE) ? bytes : MAX_WRITE, offset);
    if (likely(bytes == (size_t)written))
      return MDBX_SUCCESS;
    if (written < 0) {
      const int rc = errno;
      if (rc != EINTR)
        return rc;
      continue;
    }
#endif
    bytes -= written;
    offset += written;
    buf = (char *)buf + written;
  }
}

int mdbx_pwritev(mdbx_filehandle_t fd, struct iovec *iov, int iovcnt,
                 uint64_t offset, size_t expected_written) {
#if defined(_WIN32) || defined(_WIN64) || defined(__APPLE__)
  size_t written = 0;
  for (int i = 0; i < iovcnt; ++i) {
    int rc = mdbx_pwrite(fd, iov[i].iov_base, iov[i].iov_len, offset);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    written += iov[i].iov_len;
    offset += iov[i].iov_len;
  }
  return (expected_written == written) ? MDBX_SUCCESS
                                       : MDBX_EIO /* ERROR_WRITE_FAULT */;
#else
  int rc;
  intptr_t written;
  do {
    STATIC_ASSERT_MSG(sizeof(off_t) >= sizeof(size_t),
                      "libmdbx requires 64-bit file I/O on 64-bit systems");
    written = pwritev(fd, iov, iovcnt, offset);
    if (likely(expected_written == (size_t)written))
      return MDBX_SUCCESS;
    rc = errno;
  } while (rc == EINTR);
  return (written < 0) ? rc : MDBX_EIO /* Use which error code? */;
#endif
}

int mdbx_filesync(mdbx_filehandle_t fd, enum mdbx_syncmode_bits mode_bits) {
#if defined(_WIN32) || defined(_WIN64)
  return ((mode_bits & (MDBX_SYNC_DATA | MDBX_SYNC_IODQ)) == 0 ||
          FlushFileBuffers(fd))
             ? MDBX_SUCCESS
             : GetLastError();
#else

#if defined(__APPLE__) &&                                                      \
    MDBX_OSX_SPEED_INSTEADOF_DURABILITY == MDBX_OSX_WANNA_DURABILITY
  if (mode_bits & MDBX_SYNC_IODQ)
    return likely(fcntl(fd, F_FULLFSYNC) != -1) ? MDBX_SUCCESS : errno;
#endif /* MacOS */
#if defined(__linux__) || defined(__gnu_linux__)
  if (mode_bits == MDBX_SYNC_SIZE && mdbx_linux_kernel_version >= 0x03060000)
    return MDBX_SUCCESS;
#endif /* Linux */
  int rc;
  do {
#if defined(_POSIX_SYNCHRONIZED_IO) && _POSIX_SYNCHRONIZED_IO > 0
    /* LY: This code is always safe and without appreciable performance
     * degradation, even on a kernel with fdatasync's bug.
     *
     * For more info about of a corresponding fdatasync() bug
     * see http://www.spinics.net/lists/linux-ext4/msg33714.html */
    if ((mode_bits & MDBX_SYNC_SIZE) == 0) {
      if (fdatasync(fd) == 0)
        return MDBX_SUCCESS;
    } else
#else
    (void)mode_bits;
#endif
        if (fsync(fd) == 0)
      return MDBX_SUCCESS;
    rc = errno;
  } while (rc == EINTR);
  return rc;
#endif
}

int mdbx_filesize(mdbx_filehandle_t fd, uint64_t *length) {
#if defined(_WIN32) || defined(_WIN64)
  BY_HANDLE_FILE_INFORMATION info;
  if (!GetFileInformationByHandle(fd, &info))
    return GetLastError();
  *length = info.nFileSizeLow | (uint64_t)info.nFileSizeHigh << 32;
#else
  struct stat st;

  STATIC_ASSERT_MSG(sizeof(off_t) <= sizeof(uint64_t),
                    "libmdbx requires 64-bit file I/O on 64-bit systems");
  if (fstat(fd, &st))
    return errno;

  *length = st.st_size;
#endif
  return MDBX_SUCCESS;
}

int mdbx_ftruncate(mdbx_filehandle_t fd, uint64_t length) {
#if defined(_WIN32) || defined(_WIN64)
  if (mdbx_SetFileInformationByHandle) {
    FILE_END_OF_FILE_INFO EndOfFileInfo;
    EndOfFileInfo.EndOfFile.QuadPart = length;
    return mdbx_SetFileInformationByHandle(fd, FileEndOfFileInfo,
                                           &EndOfFileInfo,
                                           sizeof(FILE_END_OF_FILE_INFO))
               ? MDBX_SUCCESS
               : GetLastError();
  } else {
    LARGE_INTEGER li;
    li.QuadPart = length;
    return (SetFilePointerEx(fd, li, NULL, FILE_BEGIN) && SetEndOfFile(fd))
               ? MDBX_SUCCESS
               : GetLastError();
  }
#else
  STATIC_ASSERT_MSG(sizeof(off_t) >= sizeof(size_t),
                    "libmdbx requires 64-bit file I/O on 64-bit systems");
  return ftruncate(fd, length) == 0 ? MDBX_SUCCESS : errno;
#endif
}

int mdbx_fseek(mdbx_filehandle_t fd, uint64_t pos) {
#if defined(_WIN32) || defined(_WIN64)
  LARGE_INTEGER li;
  li.QuadPart = pos;
  return SetFilePointerEx(fd, li, NULL, FILE_BEGIN) ? MDBX_SUCCESS
                                                    : GetLastError();
#else
  STATIC_ASSERT_MSG(sizeof(off_t) >= sizeof(size_t),
                    "libmdbx requires 64-bit file I/O on 64-bit systems");
  return (lseek(fd, pos, SEEK_SET) < 0) ? errno : MDBX_SUCCESS;
#endif
}

/*----------------------------------------------------------------------------*/

int mdbx_thread_create(mdbx_thread_t *thread,
                       THREAD_RESULT(THREAD_CALL *start_routine)(void *),
                       void *arg) {
#if defined(_WIN32) || defined(_WIN64)
  *thread = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
  return *thread ? MDBX_SUCCESS : GetLastError();
#else
  return pthread_create(thread, NULL, start_routine, arg);
#endif
}

int mdbx_thread_join(mdbx_thread_t thread) {
#if defined(_WIN32) || defined(_WIN64)
  DWORD code = WaitForSingleObject(thread, INFINITE);
  return waitstatus2errcode(code);
#else
  void *unused_retval = &unused_retval;
  return pthread_join(thread, &unused_retval);
#endif
}

/*----------------------------------------------------------------------------*/

int mdbx_msync(mdbx_mmap_t *map, size_t offset, size_t length, int async) {
  uint8_t *ptr = (uint8_t *)map->address + offset;
#if defined(_WIN32) || defined(_WIN64)
  if (FlushViewOfFile(ptr, length) && (async || FlushFileBuffers(map->fd)))
    return MDBX_SUCCESS;
  return GetLastError();
#else
#ifdef __linux__
  if (async && mdbx_linux_kernel_version > 0x02061300)
    /* Since Linux 2.6.19, MS_ASYNC is in fact a no-op,
       since the kernel properly tracks dirty pages and flushes them to storage
       as necessary. */
    return MDBX_SUCCESS;
#endif /* Linux */
  const int mode = async ? MS_ASYNC : MS_SYNC;
  int rc = (msync(ptr, length, mode) == 0) ? MDBX_SUCCESS : errno;
#if defined(__APPLE__) &&                                                      \
    MDBX_OSX_SPEED_INSTEADOF_DURABILITY == MDBX_OSX_WANNA_DURABILITY
  if (rc == MDBX_SUCCESS && mode == MS_SYNC)
    rc = likely(fcntl(map->fd, F_FULLFSYNC) != -1) ? MDBX_SUCCESS : errno;
#endif /* MacOS */
  return rc;
#endif
}

int mdbx_check4nonlocal(mdbx_filehandle_t handle, int flags) {
#if defined(_WIN32) || defined(_WIN64)
  if (GetFileType(handle) != FILE_TYPE_DISK)
    return ERROR_FILE_OFFLINE;

  if (mdbx_GetFileInformationByHandleEx) {
    FILE_REMOTE_PROTOCOL_INFO RemoteProtocolInfo;
    if (mdbx_GetFileInformationByHandleEx(handle, FileRemoteProtocolInfo,
                                          &RemoteProtocolInfo,
                                          sizeof(RemoteProtocolInfo))) {

      if ((RemoteProtocolInfo.Flags & REMOTE_PROTOCOL_INFO_FLAG_OFFLINE) &&
          !(flags & MDBX_RDONLY))
        return ERROR_FILE_OFFLINE;
      if (!(RemoteProtocolInfo.Flags & REMOTE_PROTOCOL_INFO_FLAG_LOOPBACK) &&
          !(flags & MDBX_EXCLUSIVE))
        return ERROR_REMOTE_STORAGE_MEDIA_ERROR;
    }
  }

  if (mdbx_NtFsControlFile) {
    NTSTATUS rc;
    struct {
      WOF_EXTERNAL_INFO wof_info;
      union {
        WIM_PROVIDER_EXTERNAL_INFO wim_info;
        FILE_PROVIDER_EXTERNAL_INFO_V1 file_info;
      };
      size_t reserved_for_microsoft_madness[42];
    } GetExternalBacking_OutputBuffer;
    IO_STATUS_BLOCK StatusBlock;
    rc = mdbx_NtFsControlFile(handle, NULL, NULL, NULL, &StatusBlock,
                              FSCTL_GET_EXTERNAL_BACKING, NULL, 0,
                              &GetExternalBacking_OutputBuffer,
                              sizeof(GetExternalBacking_OutputBuffer));
    if (NT_SUCCESS(rc)) {
      if (!(flags & MDBX_EXCLUSIVE))
        return ERROR_REMOTE_STORAGE_MEDIA_ERROR;
    } else if (rc != STATUS_OBJECT_NOT_EXTERNALLY_BACKED &&
               rc != STATUS_INVALID_DEVICE_REQUEST)
      return ntstatus2errcode(rc);
  }

  if (mdbx_GetVolumeInformationByHandleW && mdbx_GetFinalPathNameByHandleW) {
    WCHAR *PathBuffer = mdbx_malloc(sizeof(WCHAR) * INT16_MAX);
    if (!PathBuffer)
      return MDBX_ENOMEM;

    int rc = MDBX_SUCCESS;
    DWORD VolumeSerialNumber, FileSystemFlags;
    if (!mdbx_GetVolumeInformationByHandleW(handle, PathBuffer, INT16_MAX,
                                            &VolumeSerialNumber, NULL,
                                            &FileSystemFlags, NULL, 0)) {
      rc = GetLastError();
      goto bailout;
    }

    if ((flags & MDBX_RDONLY) == 0) {
      if (FileSystemFlags &
          (FILE_SEQUENTIAL_WRITE_ONCE | FILE_READ_ONLY_VOLUME |
           FILE_VOLUME_IS_COMPRESSED)) {
        rc = ERROR_REMOTE_STORAGE_MEDIA_ERROR;
        goto bailout;
      }
    }

    if (!mdbx_GetFinalPathNameByHandleW(handle, PathBuffer, INT16_MAX,
                                        FILE_NAME_NORMALIZED |
                                            VOLUME_NAME_NT)) {
      rc = GetLastError();
      goto bailout;
    }

    if (_wcsnicmp(PathBuffer, L"\\Device\\Mup\\", 12) == 0) {
      if (!(flags & MDBX_EXCLUSIVE)) {
        rc = ERROR_REMOTE_STORAGE_MEDIA_ERROR;
        goto bailout;
      }
    } else if (mdbx_GetFinalPathNameByHandleW(handle, PathBuffer, INT16_MAX,
                                              FILE_NAME_NORMALIZED |
                                                  VOLUME_NAME_DOS)) {
      UINT DriveType = GetDriveTypeW(PathBuffer);
      if (DriveType == DRIVE_NO_ROOT_DIR &&
          _wcsnicmp(PathBuffer, L"\\\\?\\", 4) == 0 &&
          _wcsnicmp(PathBuffer + 5, L":\\", 2) == 0) {
        PathBuffer[7] = 0;
        DriveType = GetDriveTypeW(PathBuffer + 4);
      }
      switch (DriveType) {
      case DRIVE_CDROM:
        if (flags & MDBX_RDONLY)
          break;
      // fall through
      case DRIVE_UNKNOWN:
      case DRIVE_NO_ROOT_DIR:
      case DRIVE_REMOTE:
      default:
        if (!(flags & MDBX_EXCLUSIVE))
          rc = ERROR_REMOTE_STORAGE_MEDIA_ERROR;
      // fall through
      case DRIVE_REMOVABLE:
      case DRIVE_FIXED:
      case DRIVE_RAMDISK:
        break;
      }
    }
  bailout:
    mdbx_free(PathBuffer);
    return rc;
  }
#else
  (void)handle;
  /* TODO: check for NFS handle ? */
  (void)flags;
#endif
  return MDBX_SUCCESS;
}

int mdbx_mmap(int flags, mdbx_mmap_t *map, size_t size, size_t limit) {
  assert(size <= limit);
#if defined(_WIN32) || defined(_WIN64)
  map->length = 0;
  map->current = 0;
  map->section = NULL;
  map->address = nullptr;

  NTSTATUS rc = mdbx_check4nonlocal(map->fd, flags);
  if (rc != MDBX_SUCCESS)
    return rc;

  rc = mdbx_filesize(map->fd, &map->filesize);
  if (rc != MDBX_SUCCESS)
    return rc;
  if ((flags & MDBX_RDONLY) == 0 && map->filesize != size) {
    rc = mdbx_ftruncate(map->fd, size);
    if (rc == MDBX_SUCCESS)
      map->filesize = size;
    /* ignore error, because Windows unable shrink file
     * that already mapped (by another process) */
  }

  LARGE_INTEGER SectionSize;
  SectionSize.QuadPart = size;
  rc = NtCreateSection(
      &map->section,
      /* DesiredAccess */
      (flags & MDBX_WRITEMAP)
          ? SECTION_QUERY | SECTION_MAP_READ | SECTION_EXTEND_SIZE |
                SECTION_MAP_WRITE
          : SECTION_QUERY | SECTION_MAP_READ | SECTION_EXTEND_SIZE,
      /* ObjectAttributes */ NULL, /* MaximumSize (InitialSize) */ &SectionSize,
      /* SectionPageProtection */
      (flags & MDBX_RDONLY) ? PAGE_READONLY : PAGE_READWRITE,
      /* AllocationAttributes */ SEC_RESERVE, map->fd);
  if (!NT_SUCCESS(rc))
    return ntstatus2errcode(rc);

  SIZE_T ViewSize = (flags & MDBX_RDONLY) ? 0 : limit;
  rc = NtMapViewOfSection(
      map->section, GetCurrentProcess(), &map->address,
      /* ZeroBits */ 0,
      /* CommitSize */ 0,
      /* SectionOffset */ NULL, &ViewSize,
      /* InheritDisposition */ ViewUnmap,
      /* AllocationType */ (flags & MDBX_RDONLY) ? 0 : MEM_RESERVE,
      /* Win32Protect */
      (flags & MDBX_WRITEMAP) ? PAGE_READWRITE : PAGE_READONLY);
  if (!NT_SUCCESS(rc)) {
    NtClose(map->section);
    map->section = 0;
    map->address = nullptr;
    return ntstatus2errcode(rc);
  }
  assert(map->address != MAP_FAILED);

  map->current = (size_t)SectionSize.QuadPart;
  map->length = ViewSize;
  return MDBX_SUCCESS;
#else
  (void)size;
  map->address = mmap(
      NULL, limit, (flags & MDBX_WRITEMAP) ? PROT_READ | PROT_WRITE : PROT_READ,
      MAP_SHARED, map->fd, 0);
  if (likely(map->address != MAP_FAILED)) {
    map->length = limit;
    return MDBX_SUCCESS;
  }
  map->length = 0;
  map->address = nullptr;
  return errno;
#endif
}

int mdbx_munmap(mdbx_mmap_t *map) {
#if defined(_WIN32) || defined(_WIN64)
  if (map->section)
    NtClose(map->section);
  NTSTATUS rc = NtUnmapViewOfSection(GetCurrentProcess(), map->address);
  if (!NT_SUCCESS(rc))
    ntstatus2errcode(rc);

  map->length = 0;
  map->current = 0;
  map->address = nullptr;
#else
  if (unlikely(munmap(map->address, map->length)))
    return errno;
  map->length = 0;
  map->address = nullptr;
#endif
  return MDBX_SUCCESS;
}

int mdbx_mresize(int flags, mdbx_mmap_t *map, size_t size, size_t limit) {
  assert(size <= limit);
#if defined(_WIN32) || defined(_WIN64)
  assert(size != map->current || limit != map->length || size < map->filesize);

  NTSTATUS status;
  LARGE_INTEGER SectionSize;
  int err, rc = MDBX_SUCCESS;

  if (!(flags & MDBX_RDONLY) && limit == map->length && size > map->current) {
    /* growth rw-section */
    SectionSize.QuadPart = size;
    status = NtExtendSection(map->section, &SectionSize);
    if (NT_SUCCESS(status)) {
      map->current = size;
      if (map->filesize < size)
        map->filesize = size;
    }
    return ntstatus2errcode(status);
  }

  if (limit > map->length) {
    /* check ability of address space for growth before umnap */
    PVOID BaseAddress = (PBYTE)map->address + map->length;
    SIZE_T RegionSize = limit - map->length;
    status = NtAllocateVirtualMemory(GetCurrentProcess(), &BaseAddress, 0,
                                     &RegionSize, MEM_RESERVE, PAGE_NOACCESS);
    if (!NT_SUCCESS(status))
      return ntstatus2errcode(status);

    status = NtFreeVirtualMemory(GetCurrentProcess(), &BaseAddress, &RegionSize,
                                 MEM_RELEASE);
    if (!NT_SUCCESS(status))
      return ntstatus2errcode(status);
  }

  /* Windows unable:
   *  - shrink a mapped file;
   *  - change size of mapped view;
   *  - extend read-only mapping;
   * Therefore we should unmap/map entire section. */
  status = NtUnmapViewOfSection(GetCurrentProcess(), map->address);
  if (!NT_SUCCESS(status))
    return ntstatus2errcode(status);
  status = NtClose(map->section);
  map->section = NULL;
  PVOID ReservedAddress = NULL;
  SIZE_T ReservedSize = limit;

  if (!NT_SUCCESS(status)) {
  bailout_ntstatus:
    err = ntstatus2errcode(status);
  bailout:
    map->address = NULL;
    map->current = map->length = 0;
    if (ReservedAddress)
      (void)NtFreeVirtualMemory(GetCurrentProcess(), &ReservedAddress,
                                &ReservedSize, MEM_RELEASE);
    return err;
  }

  /* resizing of the file may take a while,
   * therefore we reserve address space to avoid occupy it by other threads */
  ReservedAddress = map->address;
  status = NtAllocateVirtualMemory(GetCurrentProcess(), &ReservedAddress, 0,
                                   &ReservedSize, MEM_RESERVE, PAGE_NOACCESS);
  if (!NT_SUCCESS(status)) {
    ReservedAddress = NULL;
    if (status != /* STATUS_CONFLICTING_ADDRESSES */ 0xC0000018)
      goto bailout_ntstatus /* no way to recovery */;

    /* assume we can change base address if mapping size changed or prev address
     * couldn't be used */
    map->address = NULL;
  }

retry_file_and_section:
  err = mdbx_filesize(map->fd, &map->filesize);
  if (err != MDBX_SUCCESS)
    goto bailout;

  if ((flags & MDBX_RDONLY) == 0 && map->filesize != size) {
    err = mdbx_ftruncate(map->fd, size);
    if (err == MDBX_SUCCESS)
      map->filesize = size;
    /* ignore error, because Windows unable shrink file
     * that already mapped (by another process) */
  }

  SectionSize.QuadPart = size;
  status = NtCreateSection(
      &map->section,
      /* DesiredAccess */
      (flags & MDBX_WRITEMAP)
          ? SECTION_QUERY | SECTION_MAP_READ | SECTION_EXTEND_SIZE |
                SECTION_MAP_WRITE
          : SECTION_QUERY | SECTION_MAP_READ | SECTION_EXTEND_SIZE,
      /* ObjectAttributes */ NULL,
      /* MaximumSize (InitialSize) */ &SectionSize,
      /* SectionPageProtection */
      (flags & MDBX_RDONLY) ? PAGE_READONLY : PAGE_READWRITE,
      /* AllocationAttributes */ SEC_RESERVE, map->fd);

  if (!NT_SUCCESS(status))
    goto bailout_ntstatus;

  if (ReservedAddress) {
    /* release reserved address space */
    status = NtFreeVirtualMemory(GetCurrentProcess(), &ReservedAddress,
                                 &ReservedSize, MEM_RELEASE);
    ReservedAddress = NULL;
    if (!NT_SUCCESS(status))
      goto bailout_ntstatus;
  }

retry_mapview:;
  SIZE_T ViewSize = (flags & MDBX_RDONLY) ? size : limit;
  status = NtMapViewOfSection(
      map->section, GetCurrentProcess(), &map->address,
      /* ZeroBits */ 0,
      /* CommitSize */ 0,
      /* SectionOffset */ NULL, &ViewSize,
      /* InheritDisposition */ ViewUnmap,
      /* AllocationType */ (flags & MDBX_RDONLY) ? 0 : MEM_RESERVE,
      /* Win32Protect */
      (flags & MDBX_WRITEMAP) ? PAGE_READWRITE : PAGE_READONLY);

  if (!NT_SUCCESS(status)) {
    if (status == /* STATUS_CONFLICTING_ADDRESSES */ 0xC0000018 &&
        map->address) {
      /* try remap at another base address */
      map->address = NULL;
      goto retry_mapview;
    }
    NtClose(map->section);
    map->section = NULL;

    if (map->address && (size != map->current || limit != map->length)) {
      /* try remap with previously size and limit,
       * but will return MDBX_RESULT_TRUE on success */
      rc = MDBX_RESULT_TRUE;
      size = map->current;
      limit = map->length;
      goto retry_file_and_section;
    }

    /* no way to recovery */
    goto bailout_ntstatus;
  }
  assert(map->address != MAP_FAILED);

  map->current = (size_t)SectionSize.QuadPart;
  map->length = ViewSize;
  return rc;
#else
  if (limit != map->length) {
#if defined(_GNU_SOURCE) &&                                                    \
    !(defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||   \
      defined(__BSD__) || defined(__NETBSD__) || defined(__bsdi__) ||          \
      defined(__DragonFly__) || defined(__APPLE__) || defined(__MACH__))
    void *ptr = mremap(map->address, map->length, limit,
                       /* LY: in case changing the mapping size calling code
                          must guarantees the absence of competing threads, and
                          a willingness to another base address */
                       MREMAP_MAYMOVE);
    if (ptr == MAP_FAILED) {
      int err = errno;
      return (err == EAGAIN || err == ENOMEM) ? MDBX_RESULT_TRUE : err;
    }
    map->address = ptr;
    map->length = limit;
#else
    return MDBX_RESULT_TRUE;
#endif /* mremap() <= _GNU_SOURCE && !__FreeBSD__ */
  }
  return (flags & MDBX_RDONLY) ? MDBX_SUCCESS : mdbx_ftruncate(map->fd, size);
#endif
}

/*----------------------------------------------------------------------------*/

__cold void mdbx_osal_jitter(bool tiny) {
  for (;;) {
#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) ||                \
    defined(__x86_64__)
    const unsigned salt = 277u * (unsigned)__rdtsc();
#else
    const unsigned salt = rand();
#endif

    const unsigned coin = salt % (tiny ? 29u : 43u);
    if (coin < 43 / 3)
      break;
#if defined(_WIN32) || defined(_WIN64)
    SwitchToThread();
    if (coin > 43 * 2 / 3)
      Sleep(1);
#else
    sched_yield();
    if (coin > 43 * 2 / 3)
      usleep(coin);
#endif
  }
}

#if defined(_WIN32) || defined(_WIN64)
#elif defined(__APPLE__) || defined(__MACH__)
#include <mach/mach_time.h>
#elif defined(__linux__) || defined(__gnu_linux__)
static __cold clockid_t choice_monoclock() {
  struct timespec probe;
#if defined(CLOCK_BOOTTIME)
  if (clock_gettime(CLOCK_BOOTTIME, &probe) == 0)
    return CLOCK_BOOTTIME;
#elif defined(CLOCK_MONOTONIC_RAW)
  if (clock_gettime(CLOCK_MONOTONIC_RAW, &probe) == 0)
    return CLOCK_MONOTONIC_RAW;
#elif defined(CLOCK_MONOTONIC_COARSE)
  if (clock_gettime(CLOCK_MONOTONIC_COARSE, &probe) == 0)
    return CLOCK_MONOTONIC_COARSE;
#endif
  return CLOCK_MONOTONIC;
}
#endif

uint64_t mdbx_osal_16dot16_to_monotime(uint32_t seconds_16dot16) {
#if defined(_WIN32) || defined(_WIN64)
  static LARGE_INTEGER performance_frequency;
  if (performance_frequency.QuadPart == 0)
    QueryPerformanceFrequency(&performance_frequency);
  const uint64_t ratio = performance_frequency.QuadPart;
#elif defined(__APPLE__) || defined(__MACH__)
  static uint64_t ratio;
  if (!ratio) {
    mach_timebase_info_data_t ti;
    mach_timebase_info(&ti);
    ratio = UINT64_C(1000000000) * ti.denom / ti.numer;
  }
#else
  const uint64_t ratio = UINT64_C(1000000000);
#endif
  return (ratio * seconds_16dot16 + 32768) >> 16;
}

uint64_t mdbx_osal_monotime(void) {
#if defined(_WIN32) || defined(_WIN64)
  LARGE_INTEGER counter;
  counter.QuadPart = 0;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart;
#elif defined(__APPLE__) || defined(__MACH__)
  return mach_absolute_time();
#else

#if defined(__linux__) || defined(__gnu_linux__)
  static clockid_t posix_clockid = -1;
  if (unlikely(posix_clockid < 0))
    posix_clockid = choice_monoclock();
#elif defined(CLOCK_MONOTONIC)
#define posix_clockid CLOCK_MONOTONIC
#else
#define posix_clockid CLOCK_REALTIME
#endif

  struct timespec ts;
  if (unlikely(clock_gettime(posix_clockid, &ts) != 0)) {
    ts.tv_nsec = 0;
    ts.tv_sec = 0;
  }
  return ts.tv_sec * UINT64_C(1000000000) + ts.tv_nsec;
#endif
}
