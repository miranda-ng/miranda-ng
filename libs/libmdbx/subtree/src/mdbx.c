﻿/*
 * Copyright 2015-2019 Leonid Yuriev <leo@yuriev.ru>
 * and other libmdbx authors: please see AUTHORS file.
 * All rights reserved.
 *
 * This code is derived from "LMDB engine" written by
 * Howard Chu (Symas Corporation), which itself derived from btree.c
 * written by Martin Hedenfalk.
 *
 * ---
 *
 * Portions Copyright 2011-2015 Howard Chu, Symas Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 *
 * ---
 *
 * Portions Copyright (c) 2009, 2010 Martin Hedenfalk <martin@bzero.se>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include "./bits.h"

/*----------------------------------------------------------------------------*/
/* Internal inlines */

static __inline bool mdbx_is_power2(size_t x) { return (x & (x - 1)) == 0; }

static __inline size_t mdbx_roundup2(size_t value, size_t granularity) {
  assert(mdbx_is_power2(granularity));
  return (value + granularity - 1) & ~(granularity - 1);
}

static __inline unsigned mdbx_log2(size_t value) {
  assert(mdbx_is_power2(value));

  unsigned log = 0;
  while (value > 1) {
    log += 1;
    value >>= 1;
  }
  return log;
}

/* Address of node i in page p */
static __inline MDBX_node *NODEPTR(MDBX_page *p, unsigned i) {
  assert(NUMKEYS(p) > (unsigned)(i));
  return (MDBX_node *)((char *)(p) + (p)->mp_ptrs[i] + PAGEHDRSZ);
}

/* Get the page number pointed to by a branch node */
static __inline pgno_t NODEPGNO(const MDBX_node *node) {
  pgno_t pgno;
  if (UNALIGNED_OK) {
    pgno = node->mn_ksize_and_pgno;
    if (sizeof(pgno_t) > 4)
      pgno &= MAX_PAGENO;
  } else {
    pgno = node->mn_lo | ((pgno_t)node->mn_hi << 16);
    if (sizeof(pgno_t) > 4)
      pgno |= ((uint64_t)node->mn_flags) << 32;
  }
  return pgno;
}

/* Set the page number in a branch node */
static __inline void SETPGNO(MDBX_node *node, pgno_t pgno) {
  assert(pgno <= MAX_PAGENO);

  if (UNALIGNED_OK) {
    if (sizeof(pgno_t) > 4)
      pgno |= ((uint64_t)node->mn_ksize) << 48;
    node->mn_ksize_and_pgno = pgno;
  } else {
    node->mn_lo = (uint16_t)pgno;
    node->mn_hi = (uint16_t)(pgno >> 16);
    if (sizeof(pgno_t) > 4)
      node->mn_flags = (uint16_t)((uint64_t)pgno >> 32);
  }
}

/* Get the size of the data in a leaf node */
static __inline size_t NODEDSZ(const MDBX_node *node) {
  size_t size;
  if (UNALIGNED_OK) {
    size = node->mn_dsize;
  } else {
    size = node->mn_lo | ((size_t)node->mn_hi << 16);
  }
  return size;
}

/* Set the size of the data for a leaf node */
static __inline void SETDSZ(MDBX_node *node, size_t size) {
  assert(size < INT_MAX);
  if (UNALIGNED_OK) {
    node->mn_dsize = (uint32_t)size;
  } else {
    node->mn_lo = (uint16_t)size;
    node->mn_hi = (uint16_t)(size >> 16);
  }
}

static __inline size_t pgno2bytes(const MDBX_env *env, pgno_t pgno) {
  mdbx_assert(env, (1u << env->me_psize2log) == env->me_psize);
  return ((size_t)pgno) << env->me_psize2log;
}

static __inline MDBX_page *pgno2page(const MDBX_env *env, pgno_t pgno) {
  return (MDBX_page *)(env->me_map + pgno2bytes(env, pgno));
}

static __inline pgno_t bytes2pgno(const MDBX_env *env, size_t bytes) {
  mdbx_assert(env, (env->me_psize >> env->me_psize2log) == 1);
  return (pgno_t)(bytes >> env->me_psize2log);
}

static __inline size_t pgno_align2os_bytes(const MDBX_env *env, pgno_t pgno) {
  return mdbx_roundup2(pgno2bytes(env, pgno), env->me_os_psize);
}

static __inline pgno_t pgno_align2os_pgno(const MDBX_env *env, pgno_t pgno) {
  return bytes2pgno(env, pgno_align2os_bytes(env, pgno));
}

/* Perform act while tracking temporary cursor mn */
#define WITH_CURSOR_TRACKING(mn, act)                                          \
  do {                                                                         \
    mdbx_cassert(&(mn),                                                        \
                 mn.mc_txn->mt_cursors != NULL /* must be not rdonly txt */);  \
    MDBX_cursor mc_dummy, *tracked,                                            \
        **tp = &(mn).mc_txn->mt_cursors[mn.mc_dbi];                            \
    if ((mn).mc_flags & C_SUB) {                                               \
      mc_dummy.mc_flags = C_INITIALIZED;                                       \
      mc_dummy.mc_xcursor = (MDBX_xcursor *)&(mn);                             \
      tracked = &mc_dummy;                                                     \
    } else {                                                                   \
      tracked = &(mn);                                                         \
    }                                                                          \
    tracked->mc_next = *tp;                                                    \
    *tp = tracked;                                                             \
    { act; }                                                                   \
    *tp = tracked->mc_next;                                                    \
  } while (0)

/*----------------------------------------------------------------------------*/
/* LY: temporary workaround for Elbrus's memcmp() bug. */

#if defined(__e2k__) && !__GLIBC_PREREQ(2, 24)
int __hot mdbx_e2k_memcmp_bug_workaround(const void *s1, const void *s2,
                                         size_t n) {
  if (unlikely(n > 42
               /* LY: align followed access if reasonable possible */
               && (((uintptr_t)s1) & 7) != 0 &&
               (((uintptr_t)s1) & 7) == (((uintptr_t)s2) & 7))) {
    if (((uintptr_t)s1) & 1) {
      const int diff = *(uint8_t *)s1 - *(uint8_t *)s2;
      if (diff)
        return diff;
      s1 = (char *)s1 + 1;
      s2 = (char *)s2 + 1;
      n -= 1;
    }

    if (((uintptr_t)s1) & 2) {
      const uint16_t a = *(uint16_t *)s1;
      const uint16_t b = *(uint16_t *)s2;
      if (likely(a != b))
        return (__builtin_bswap16(a) > __builtin_bswap16(b)) ? 1 : -1;
      s1 = (char *)s1 + 2;
      s2 = (char *)s2 + 2;
      n -= 2;
    }

    if (((uintptr_t)s1) & 4) {
      const uint32_t a = *(uint32_t *)s1;
      const uint32_t b = *(uint32_t *)s2;
      if (likely(a != b))
        return (__builtin_bswap32(a) > __builtin_bswap32(b)) ? 1 : -1;
      s1 = (char *)s1 + 4;
      s2 = (char *)s2 + 4;
      n -= 4;
    }
  }

  while (n >= 8) {
    const uint64_t a = *(uint64_t *)s1;
    const uint64_t b = *(uint64_t *)s2;
    if (likely(a != b))
      return (__builtin_bswap64(a) > __builtin_bswap64(b)) ? 1 : -1;
    s1 = (char *)s1 + 8;
    s2 = (char *)s2 + 8;
    n -= 8;
  }

  if (n & 4) {
    const uint32_t a = *(uint32_t *)s1;
    const uint32_t b = *(uint32_t *)s2;
    if (likely(a != b))
      return (__builtin_bswap32(a) > __builtin_bswap32(b)) ? 1 : -1;
    s1 = (char *)s1 + 4;
    s2 = (char *)s2 + 4;
  }

  if (n & 2) {
    const uint16_t a = *(uint16_t *)s1;
    const uint16_t b = *(uint16_t *)s2;
    if (likely(a != b))
      return (__builtin_bswap16(a) > __builtin_bswap16(b)) ? 1 : -1;
    s1 = (char *)s1 + 2;
    s2 = (char *)s2 + 2;
  }

  return (n & 1) ? *(uint8_t *)s1 - *(uint8_t *)s2 : 0;
}

int __hot mdbx_e2k_strcmp_bug_workaround(const char *s1, const char *s2) {
  while (true) {
    int diff = *(uint8_t *)s1 - *(uint8_t *)s2;
    if (likely(diff != 0) || *s1 == '\0')
      return diff;
    s1 += 1;
    s2 += 1;
  }
}

int __hot mdbx_e2k_strncmp_bug_workaround(const char *s1, const char *s2,
                                          size_t n) {
  while (n > 0) {
    int diff = *(uint8_t *)s1 - *(uint8_t *)s2;
    if (likely(diff != 0) || *s1 == '\0')
      return diff;
    s1 += 1;
    s2 += 1;
    n -= 1;
  }
  return 0;
}

size_t __hot mdbx_e2k_strlen_bug_workaround(const char *s) {
  size_t n = 0;
  while (*s) {
    s += 1;
    n += 1;
  }
  return n;
}

size_t __hot mdbx_e2k_strnlen_bug_workaround(const char *s, size_t maxlen) {
  size_t n = 0;
  while (maxlen > n && *s) {
    s += 1;
    n += 1;
  }
  return n;
}
#endif /* Elbrus's memcmp() bug. */

/*----------------------------------------------------------------------------*/
/* rthc (tls keys and destructors) */

typedef struct rthc_entry_t {
  MDBX_reader *begin;
  MDBX_reader *end;
  mdbx_thread_key_t key;
} rthc_entry_t;

#if MDBX_DEBUG
#define RTHC_INITIAL_LIMIT 1
#else
#define RTHC_INITIAL_LIMIT 16
#endif

#if defined(_WIN32) || defined(_WIN64)
static CRITICAL_SECTION rthc_critical_section;
#else
int __cxa_thread_atexit_impl(void (*dtor)(void *), void *obj, void *dso_symbol)
    __attribute__((__weak__));
#ifdef __APPLE__ /* FIXME: Thread-Local Storage destructors & DSO-unloading */
int __cxa_thread_atexit_impl(void (*dtor)(void *), void *obj,
                             void *dso_symbol) {
  (void)dtor;
  (void)obj;
  (void)dso_symbol;
  return -1;
}
#endif           /* __APPLE__ */

static pthread_mutex_t mdbx_rthc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t mdbx_rthc_cond = PTHREAD_COND_INITIALIZER;
static mdbx_thread_key_t mdbx_rthc_key;
static volatile uint32_t mdbx_rthc_pending;

static void __cold mdbx_workaround_glibc_bug21031(void) {
  /* Workaround for https://sourceware.org/bugzilla/show_bug.cgi?id=21031
   *
   * Due race between pthread_key_delete() and __nptl_deallocate_tsd()
   * The destructor(s) of thread-local-storate object(s) may be running
   * in another thread(s) and be blocked or not finished yet.
   * In such case we get a SEGFAULT after unload this library DSO.
   *
   * So just by yielding a few timeslices we give a chance
   * to such destructor(s) for completion and avoids segfault. */
  sched_yield();
  sched_yield();
  sched_yield();
}
#endif

static unsigned rthc_count, rthc_limit;
static rthc_entry_t *rthc_table;
static rthc_entry_t rthc_table_static[RTHC_INITIAL_LIMIT];

static __cold void mdbx_rthc_lock(void) {
#if defined(_WIN32) || defined(_WIN64)
  EnterCriticalSection(&rthc_critical_section);
#else
  mdbx_ensure(nullptr, pthread_mutex_lock(&mdbx_rthc_mutex) == 0);
#endif
}

static __cold void mdbx_rthc_unlock(void) {
#if defined(_WIN32) || defined(_WIN64)
  LeaveCriticalSection(&rthc_critical_section);
#else
  mdbx_ensure(nullptr, pthread_mutex_unlock(&mdbx_rthc_mutex) == 0);
#endif
}

static __inline int mdbx_thread_key_create(mdbx_thread_key_t *key) {
  int rc;
#if defined(_WIN32) || defined(_WIN64)
  *key = TlsAlloc();
  rc = (*key != TLS_OUT_OF_INDEXES) ? MDBX_SUCCESS : GetLastError();
#else
  rc = pthread_key_create(key, nullptr);
#endif
  mdbx_trace("&key = %p, value 0x%x, rc %d", key, (unsigned)*key, rc);
  return rc;
}

static __inline void mdbx_thread_key_delete(mdbx_thread_key_t key) {
  mdbx_trace("key = 0x%x", (unsigned)key);
#if defined(_WIN32) || defined(_WIN64)
  mdbx_ensure(nullptr, TlsFree(key));
#else
  mdbx_ensure(nullptr, pthread_key_delete(key) == 0);
  mdbx_workaround_glibc_bug21031();
#endif
}

static __inline void *mdbx_thread_rthc_get(mdbx_thread_key_t key) {
#if defined(_WIN32) || defined(_WIN64)
  return TlsGetValue(key);
#else
  return pthread_getspecific(key);
#endif
}

static void mdbx_thread_rthc_set(mdbx_thread_key_t key, const void *value) {
#if defined(_WIN32) || defined(_WIN64)
  mdbx_ensure(nullptr, TlsSetValue(key, (void *)value));
#else
#define MDBX_THREAD_RTHC_ZERO 0
#define MDBX_THREAD_RTHC_REGISTERD 1
#define MDBX_THREAD_RTHC_COUNTED 2
  static __thread uint32_t thread_registration_state;
  if (value && unlikely(thread_registration_state == MDBX_THREAD_RTHC_ZERO)) {
    thread_registration_state = MDBX_THREAD_RTHC_REGISTERD;
    mdbx_trace("thread registered 0x%" PRIxPTR, (uintptr_t)mdbx_thread_self());
    if (&__cxa_thread_atexit_impl == nullptr ||
        __cxa_thread_atexit_impl(mdbx_rthc_thread_dtor,
                                 &thread_registration_state,
                                 (void *)&mdbx_version /* dso_anchor */)) {
      mdbx_ensure(nullptr, pthread_setspecific(
                               mdbx_rthc_key, &thread_registration_state) == 0);
      thread_registration_state = MDBX_THREAD_RTHC_COUNTED;
      const unsigned count_before = mdbx_atomic_add32(&mdbx_rthc_pending, 1);
      mdbx_ensure(nullptr, count_before < INT_MAX);
      mdbx_trace("fallback to pthreads' tsd, key 0x%x, count %u",
                 (unsigned)mdbx_rthc_key, count_before);
      (void)count_before;
    }
  }
  mdbx_ensure(nullptr, pthread_setspecific(key, value) == 0);
#endif
}

__cold void mdbx_rthc_global_init(void) {
  rthc_limit = RTHC_INITIAL_LIMIT;
  rthc_table = rthc_table_static;
#if defined(_WIN32) || defined(_WIN64)
  InitializeCriticalSection(&rthc_critical_section);
#else
  mdbx_ensure(nullptr,
              pthread_key_create(&mdbx_rthc_key, mdbx_rthc_thread_dtor) == 0);
  mdbx_trace("pid %d, &mdbx_rthc_key = %p, value 0x%x", mdbx_getpid(),
             &mdbx_rthc_key, (unsigned)mdbx_rthc_key);
#endif
}

/* dtor called for thread, i.e. for all mdbx's environment objects */
__cold void mdbx_rthc_thread_dtor(void *ptr) {
  mdbx_rthc_lock();
  mdbx_trace(">> pid %d, thread 0x%" PRIxPTR ", rthc %p", mdbx_getpid(),
             (uintptr_t)mdbx_thread_self(), ptr);

  const mdbx_pid_t self_pid = mdbx_getpid();
  for (unsigned i = 0; i < rthc_count; ++i) {
    const mdbx_thread_key_t key = rthc_table[i].key;
    MDBX_reader *const rthc = mdbx_thread_rthc_get(key);
    if (rthc < rthc_table[i].begin || rthc >= rthc_table[i].end)
      continue;
#if !defined(_WIN32) && !defined(_WIN64)
    if (pthread_setspecific(key, nullptr) != 0) {
      mdbx_trace("== thread 0x%" PRIxPTR
                 ", rthc %p: ignore race with tsd-key deletion",
                 (uintptr_t)mdbx_thread_self(), ptr);
      continue /* ignore race with tsd-key deletion by mdbx_env_close() */;
    }
#endif

    mdbx_trace("== thread 0x%" PRIxPTR
               ", rthc %p, [%i], %p ... %p (%+i), rtch-pid %i, "
               "current-pid %i",
               (uintptr_t)mdbx_thread_self(), rthc, i, rthc_table[i].begin,
               rthc_table[i].end, (int)(rthc - rthc_table[i].begin),
               rthc->mr_pid, self_pid);
    if (rthc->mr_pid == self_pid) {
      mdbx_trace("==== thread 0x%" PRIxPTR ", rthc %p, cleanup",
                 (uintptr_t)mdbx_thread_self(), rthc);
      rthc->mr_pid = 0;
    }
  }

#if defined(_WIN32) || defined(_WIN64)
  mdbx_trace("<< thread 0x%" PRIxPTR ", rthc %p", (uintptr_t)mdbx_thread_self(),
             ptr);
  mdbx_rthc_unlock();
#else
  const char self_registration = *(char *)ptr;
  *(char *)ptr = MDBX_THREAD_RTHC_ZERO;
  mdbx_trace("== thread 0x%" PRIxPTR ", rthc %p, pid %d, self-status %d",
             (uintptr_t)mdbx_thread_self(), ptr, mdbx_getpid(),
             self_registration);
  if (self_registration == MDBX_THREAD_RTHC_COUNTED)
    mdbx_ensure(nullptr, mdbx_atomic_sub32(&mdbx_rthc_pending, 1) > 0);

  if (mdbx_rthc_pending == 0) {
    mdbx_trace("== thread 0x%" PRIxPTR ", rthc %p, pid %d, wake",
               (uintptr_t)mdbx_thread_self(), ptr, mdbx_getpid());
    mdbx_ensure(nullptr, pthread_cond_broadcast(&mdbx_rthc_cond) == 0);
  }

  mdbx_trace("<< thread 0x%" PRIxPTR ", rthc %p", (uintptr_t)mdbx_thread_self(),
             ptr);
  /* Allow tail call optimization, i.e. gcc should generate the jmp instruction
   * instead of a call for pthread_mutex_unlock() and therefore CPU could not
   * return to current DSO's code section, which may be unloaded immediately
   * after the mutex got released. */
  pthread_mutex_unlock(&mdbx_rthc_mutex);
#endif
}

__cold void mdbx_rthc_global_dtor(void) {
  mdbx_trace(
      ">> pid %d, &mdbx_rthc_global_dtor %p, &mdbx_rthc_thread_dtor = %p, "
      "&mdbx_rthc_remove = %p",
      mdbx_getpid(), &mdbx_rthc_global_dtor, &mdbx_rthc_thread_dtor,
      &mdbx_rthc_remove);

  mdbx_rthc_lock();
#if !defined(_WIN32) && !defined(_WIN64)
  char *rthc = (char *)pthread_getspecific(mdbx_rthc_key);
  mdbx_trace("== thread 0x%" PRIxPTR ", rthc %p, pid %d, self-status %d",
             (uintptr_t)mdbx_thread_self(), rthc, mdbx_getpid(),
             rthc ? *rthc : -1);
  if (rthc) {
    const char self_registration = *(char *)rthc;
    *rthc = MDBX_THREAD_RTHC_ZERO;
    if (self_registration == MDBX_THREAD_RTHC_COUNTED)
      mdbx_ensure(nullptr, mdbx_atomic_sub32(&mdbx_rthc_pending, 1) > 0);
  }

  struct timespec abstime;
  mdbx_ensure(nullptr, clock_gettime(CLOCK_REALTIME, &abstime) == 0);
  abstime.tv_nsec += 1000000000l / 10;
  if (abstime.tv_nsec >= 1000000000l) {
    abstime.tv_nsec -= 1000000000l;
    abstime.tv_sec += 1;
  }
#if MDBX_DEBUG > 0
  abstime.tv_sec += 600;
#endif

  for (unsigned left; (left = mdbx_rthc_pending) > 0;) {
    mdbx_trace("pid %d, pending %u, wait for...", mdbx_getpid(), left);
    const int rc =
        pthread_cond_timedwait(&mdbx_rthc_cond, &mdbx_rthc_mutex, &abstime);
    if (rc && rc != EINTR)
      break;
  }
  mdbx_thread_key_delete(mdbx_rthc_key);
#endif

  const mdbx_pid_t self_pid = mdbx_getpid();
  for (unsigned i = 0; i < rthc_count; ++i) {
    const mdbx_thread_key_t key = rthc_table[i].key;
    mdbx_thread_key_delete(key);
    for (MDBX_reader *rthc = rthc_table[i].begin; rthc < rthc_table[i].end;
         ++rthc) {
      mdbx_trace("== [%i] = key %zu, %p ... %p, rthc %p (%+i), "
                 "rthc-pid %i, current-pid %i",
                 i, (size_t)key, rthc_table[i].begin, rthc_table[i].end, rthc,
                 (int)(rthc - rthc_table[i].begin), rthc->mr_pid, self_pid);
      if (rthc->mr_pid == self_pid) {
        rthc->mr_pid = 0;
        mdbx_trace("== cleanup %p", rthc);
      }
    }
  }

  rthc_limit = rthc_count = 0;
  if (rthc_table != rthc_table_static)
    mdbx_free(rthc_table);
  rthc_table = nullptr;
  mdbx_rthc_unlock();

#if defined(_WIN32) || defined(_WIN64)
  DeleteCriticalSection(&rthc_critical_section);
#else
  /* LY: yielding a few timeslices to give a more chance
   * to racing destructor(s) for completion. */
  mdbx_workaround_glibc_bug21031();
#endif

  mdbx_trace("<< pid %d\n", mdbx_getpid());
}

__cold int mdbx_rthc_alloc(mdbx_thread_key_t *key, MDBX_reader *begin,
                           MDBX_reader *end) {
#ifndef NDEBUG
  *key = (mdbx_thread_key_t)0xBADBADBAD;
#endif /* NDEBUG */
  int rc = mdbx_thread_key_create(key);
  if (rc != MDBX_SUCCESS)
    return rc;

  mdbx_rthc_lock();
  mdbx_trace(">> key %zu, rthc_count %u, rthc_limit %u", (size_t)*key,
             rthc_count, rthc_limit);
  if (rthc_count == rthc_limit) {
    rthc_entry_t *new_table =
        mdbx_realloc((rthc_table == rthc_table_static) ? nullptr : rthc_table,
                     sizeof(rthc_entry_t) * rthc_limit * 2);
    if (new_table == nullptr) {
      rc = MDBX_ENOMEM;
      goto bailout;
    }
    if (rthc_table == rthc_table_static)
      memcpy(new_table, rthc_table_static, sizeof(rthc_table_static));
    rthc_table = new_table;
    rthc_limit *= 2;
  }
  mdbx_trace("== [%i] = key %zu, %p ... %p", rthc_count, (size_t)*key, begin,
             end);
  rthc_table[rthc_count].key = *key;
  rthc_table[rthc_count].begin = begin;
  rthc_table[rthc_count].end = end;
  ++rthc_count;
  mdbx_trace("<< key %zu, rthc_count %u, rthc_limit %u", (size_t)*key,
             rthc_count, rthc_limit);
  mdbx_rthc_unlock();
  return MDBX_SUCCESS;

bailout:
  mdbx_thread_key_delete(*key);
  mdbx_rthc_unlock();
  return rc;
}

__cold void mdbx_rthc_remove(const mdbx_thread_key_t key) {
  mdbx_thread_key_delete(key);
  mdbx_rthc_lock();
  mdbx_trace(">> key %zu, rthc_count %u, rthc_limit %u", (size_t)key,
             rthc_count, rthc_limit);

  for (unsigned i = 0; i < rthc_count; ++i) {
    if (key == rthc_table[i].key) {
      const mdbx_pid_t self_pid = mdbx_getpid();
      mdbx_trace("== [%i], %p ...%p, current-pid %d", i, rthc_table[i].begin,
                 rthc_table[i].end, self_pid);

      for (MDBX_reader *rthc = rthc_table[i].begin; rthc < rthc_table[i].end;
           ++rthc) {
        if (rthc->mr_pid == self_pid) {
          rthc->mr_pid = 0;
          mdbx_trace("== cleanup %p", rthc);
        }
      }
      if (--rthc_count > 0)
        rthc_table[i] = rthc_table[rthc_count];
      else if (rthc_table != rthc_table_static) {
        mdbx_free(rthc_table);
        rthc_table = rthc_table_static;
        rthc_limit = RTHC_INITIAL_LIMIT;
      }
      break;
    }
  }

  mdbx_trace("<< key %zu, rthc_count %u, rthc_limit %u", (size_t)key,
             rthc_count, rthc_limit);
  mdbx_rthc_unlock();
}

/*----------------------------------------------------------------------------*/

static __inline size_t pnl2bytes(const size_t size) {
  assert(size > 0 && size <= MDBX_PNL_MAX * 2);
  size_t bytes =
      mdbx_roundup2(MDBX_ASSUME_MALLOC_OVERHEAD + sizeof(pgno_t) * (size + 2),
                    MDBX_PNL_GRANULATE * sizeof(pgno_t)) -
      MDBX_ASSUME_MALLOC_OVERHEAD;
  return bytes;
}

static __inline pgno_t bytes2pnl(const size_t bytes) {
  size_t size = bytes / sizeof(pgno_t);
  assert(size > 2 && size <= MDBX_PNL_MAX * 2);
  return (pgno_t)size - 2;
}

static MDBX_PNL mdbx_pnl_alloc(size_t size) {
  const size_t bytes = pnl2bytes(size);
  MDBX_PNL pl = mdbx_malloc(bytes);
  if (likely(pl)) {
#if __GLIBC_PREREQ(2, 12)
    const size_t bytes = malloc_usable_size(pl);
#endif
    pl[0] = bytes2pnl(bytes);
    assert(pl[0] >= size);
    pl[1] = 0;
    pl += 1;
  }
  return pl;
}

static void mdbx_pnl_free(MDBX_PNL pl) {
  if (likely(pl))
    mdbx_free(pl - 1);
}

/* Shrink the PNL to the default size if it has grown larger */
static void mdbx_pnl_shrink(MDBX_PNL *ppl) {
  assert(bytes2pnl(pnl2bytes(MDBX_PNL_INITIAL)) == MDBX_PNL_INITIAL);
  assert(MDBX_PNL_SIZE(*ppl) <= MDBX_PNL_MAX &&
         MDBX_PNL_ALLOCLEN(*ppl) >= MDBX_PNL_SIZE(*ppl));
  MDBX_PNL_SIZE(*ppl) = 0;
  if (unlikely(MDBX_PNL_ALLOCLEN(*ppl) >
               MDBX_PNL_INITIAL + MDBX_CACHELINE_SIZE / sizeof(pgno_t))) {
    const size_t bytes = pnl2bytes(MDBX_PNL_INITIAL);
    MDBX_PNL pl = mdbx_realloc(*ppl - 1, bytes);
    if (likely(pl)) {
#if __GLIBC_PREREQ(2, 12)
      const size_t bytes = malloc_usable_size(pl);
#endif
      *pl = bytes2pnl(bytes);
      *ppl = pl + 1;
    }
  }
}

/* Grow the PNL to the size growed to at least given size */
static int mdbx_pnl_reserve(MDBX_PNL *ppl, const size_t wanna) {
  const size_t allocated = MDBX_PNL_ALLOCLEN(*ppl);
  assert(MDBX_PNL_SIZE(*ppl) <= MDBX_PNL_MAX &&
         MDBX_PNL_ALLOCLEN(*ppl) >= MDBX_PNL_SIZE(*ppl));
  if (likely(allocated >= wanna))
    return MDBX_SUCCESS;

  if (unlikely(wanna > /* paranoia */ MDBX_PNL_MAX))
    return MDBX_TXN_FULL;

  const size_t size = (wanna + wanna - allocated < MDBX_PNL_MAX)
                          ? wanna + wanna - allocated
                          : MDBX_PNL_MAX;
  const size_t bytes = pnl2bytes(size);
  MDBX_PNL pl = mdbx_realloc(*ppl - 1, bytes);
  if (likely(pl)) {
#if __GLIBC_PREREQ(2, 12)
    const size_t bytes = malloc_usable_size(pl);
#endif
    *pl = bytes2pnl(bytes);
    assert(*pl >= wanna);
    *ppl = pl + 1;
    return MDBX_SUCCESS;
  }
  return MDBX_ENOMEM;
}

/* Make room for num additional elements in an PNL */
static __inline int __must_check_result mdbx_pnl_need(MDBX_PNL *ppl,
                                                      size_t num) {
  assert(MDBX_PNL_SIZE(*ppl) <= MDBX_PNL_MAX &&
         MDBX_PNL_ALLOCLEN(*ppl) >= MDBX_PNL_SIZE(*ppl));
  assert(num <= MDBX_PNL_MAX);
  const size_t wanna = MDBX_PNL_SIZE(*ppl) + num;
  return likely(MDBX_PNL_ALLOCLEN(*ppl) >= wanna)
             ? MDBX_SUCCESS
             : mdbx_pnl_reserve(ppl, wanna);
}

static __inline void mdbx_pnl_xappend(MDBX_PNL pl, pgno_t id) {
  assert(MDBX_PNL_SIZE(pl) < MDBX_PNL_ALLOCLEN(pl));
  MDBX_PNL_SIZE(pl) += 1;
  MDBX_PNL_LAST(pl) = id;
}

/* Append an ID onto an PNL */
static int __must_check_result mdbx_pnl_append(MDBX_PNL *ppl, pgno_t id) {
  /* Too big? */
  if (unlikely(MDBX_PNL_SIZE(*ppl) == MDBX_PNL_ALLOCLEN(*ppl))) {
    int rc = mdbx_pnl_need(ppl, MDBX_PNL_GRANULATE);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  mdbx_pnl_xappend(*ppl, id);
  return MDBX_SUCCESS;
}

/* Append an PNL onto an PNL */
static int __must_check_result mdbx_pnl_append_list(MDBX_PNL *ppl,
                                                    MDBX_PNL append) {
  int rc = mdbx_pnl_need(ppl, MDBX_PNL_SIZE(append));
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  memcpy(MDBX_PNL_END(*ppl), MDBX_PNL_BEGIN(append),
         MDBX_PNL_SIZE(append) * sizeof(pgno_t));
  MDBX_PNL_SIZE(*ppl) += MDBX_PNL_SIZE(append);
  return MDBX_SUCCESS;
}

/* Append an ID range onto an PNL */
static int __must_check_result mdbx_pnl_append_range(MDBX_PNL *ppl, pgno_t id,
                                                     size_t n) {
  int rc = mdbx_pnl_need(ppl, n);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  pgno_t *ap = MDBX_PNL_END(*ppl);
  MDBX_PNL_SIZE(*ppl) += (unsigned)n;
  for (pgno_t *const end = MDBX_PNL_END(*ppl); ap < end;)
    *ap++ = id++;
  return MDBX_SUCCESS;
}

static bool mdbx_pnl_check(MDBX_PNL pl, bool allocated) {
  if (pl) {
    assert(MDBX_PNL_SIZE(pl) <= MDBX_PNL_MAX);
    if (allocated) {
      assert(MDBX_PNL_ALLOCLEN(pl) >= MDBX_PNL_SIZE(pl));
    }
    for (const pgno_t *scan = &MDBX_PNL_LAST(pl); --scan > pl;) {
      assert(MDBX_PNL_ORDERED(scan[0], scan[1]));
      assert(scan[0] >= NUM_METAS);
      if (unlikely(MDBX_PNL_DISORDERED(scan[0], scan[1]) ||
                   scan[0] < NUM_METAS))
        return false;
    }
  }
  return true;
}

/* Merge an PNL onto an PNL. The destination PNL must be big enough */
static void __hot mdbx_pnl_xmerge(MDBX_PNL pnl, MDBX_PNL merge) {
  assert(mdbx_pnl_check(pnl, true));
  assert(mdbx_pnl_check(merge, false));
  pgno_t old_id, merge_id, i = MDBX_PNL_SIZE(merge), j = MDBX_PNL_SIZE(pnl),
                           k = i + j, total = k;
  pnl[0] =
      MDBX_PNL_ASCENDING ? 0 : ~(pgno_t)0; /* delimiter for pl scan below */
  old_id = pnl[j];
  while (i) {
    merge_id = merge[i--];
    for (; MDBX_PNL_ORDERED(merge_id, old_id); old_id = pnl[--j])
      pnl[k--] = old_id;
    pnl[k--] = merge_id;
  }
  MDBX_PNL_SIZE(pnl) = total;
  assert(mdbx_pnl_check(pnl, true));
}

/* Sort an PNL */
static void __hot mdbx_pnl_sort(MDBX_PNL pnl) {
  /* Max possible depth of int-indexed tree * 2 items/level */
  int istack[sizeof(int) * CHAR_BIT * 2];
  int i, j, k, l, ir, jstack;
  pgno_t a;

/* Quicksort + Insertion sort for small arrays */
#define PNL_SMALL 8
#define PNL_SWAP(a, b)                                                         \
  do {                                                                         \
    pgno_t tmp_pgno = (a);                                                     \
    (a) = (b);                                                                 \
    (b) = tmp_pgno;                                                            \
  } while (0)

  ir = (int)MDBX_PNL_SIZE(pnl);
  l = 1;
  jstack = 0;
  while (1) {
    if (ir - l < PNL_SMALL) { /* Insertion sort */
      for (j = l + 1; j <= ir; j++) {
        a = pnl[j];
        for (i = j - 1; i >= 1; i--) {
          if (MDBX_PNL_DISORDERED(a, pnl[i]))
            break;
          pnl[i + 1] = pnl[i];
        }
        pnl[i + 1] = a;
      }
      if (jstack == 0)
        break;
      ir = istack[jstack--];
      l = istack[jstack--];
    } else {
      k = (l + ir) >> 1; /* Choose median of left, center, right */
      PNL_SWAP(pnl[k], pnl[l + 1]);
      if (MDBX_PNL_ORDERED(pnl[ir], pnl[l]))
        PNL_SWAP(pnl[l], pnl[ir]);

      if (MDBX_PNL_ORDERED(pnl[ir], pnl[l + 1]))
        PNL_SWAP(pnl[l + 1], pnl[ir]);

      if (MDBX_PNL_ORDERED(pnl[l + 1], pnl[l]))
        PNL_SWAP(pnl[l], pnl[l + 1]);

      i = l + 1;
      j = ir;
      a = pnl[l + 1];
      while (1) {
        do
          i++;
        while (MDBX_PNL_ORDERED(pnl[i], a));
        do
          j--;
        while (MDBX_PNL_ORDERED(a, pnl[j]));
        if (j < i)
          break;
        PNL_SWAP(pnl[i], pnl[j]);
      }
      pnl[l + 1] = pnl[j];
      pnl[j] = a;
      jstack += 2;
      if (ir - i + 1 >= j - l) {
        istack[jstack] = ir;
        istack[jstack - 1] = i;
        ir = j - 1;
      } else {
        istack[jstack] = j - 1;
        istack[jstack - 1] = l;
        l = i;
      }
    }
  }
#undef PNL_SMALL
#undef PNL_SWAP
  assert(mdbx_pnl_check(pnl, false));
}

/* Search for an ID in an PNL.
 * [in] pl The PNL to search.
 * [in] id The ID to search for.
 * Returns The index of the first ID greater than or equal to id. */
static unsigned __hot mdbx_pnl_search(MDBX_PNL pnl, pgno_t id) {
  assert(mdbx_pnl_check(pnl, true));

  /* binary search of id in pl
   * if found, returns position of id
   * if not found, returns first position greater than id */
  unsigned base = 0;
  unsigned cursor = 1;
  int val = 0;
  unsigned n = MDBX_PNL_SIZE(pnl);

  while (n > 0) {
    unsigned pivot = n >> 1;
    cursor = base + pivot + 1;
    val = MDBX_PNL_ASCENDING ? mdbx_cmp2int(id, pnl[cursor])
                             : mdbx_cmp2int(pnl[cursor], id);

    if (val < 0) {
      n = pivot;
    } else if (val > 0) {
      base = cursor;
      n -= pivot + 1;
    } else {
      return cursor;
    }
  }

  if (val > 0)
    ++cursor;

  return cursor;
}

/*----------------------------------------------------------------------------*/

static __inline size_t txl2bytes(const size_t size) {
  assert(size > 0 && size <= MDBX_TXL_MAX * 2);
  size_t bytes =
      mdbx_roundup2(MDBX_ASSUME_MALLOC_OVERHEAD + sizeof(txnid_t) * (size + 2),
                    MDBX_TXL_GRANULATE * sizeof(txnid_t)) -
      MDBX_ASSUME_MALLOC_OVERHEAD;
  return bytes;
}

static __inline size_t bytes2txl(const size_t bytes) {
  size_t size = bytes / sizeof(txnid_t);
  assert(size > 2 && size <= MDBX_TXL_MAX * 2);
  return size - 2;
}

static MDBX_TXL mdbx_txl_alloc(void) {
  const size_t bytes = txl2bytes(MDBX_TXL_INITIAL);
  MDBX_TXL tl = mdbx_malloc(bytes);
  if (likely(tl)) {
#if __GLIBC_PREREQ(2, 12)
    const size_t bytes = malloc_usable_size(tl);
#endif
    tl[0] = bytes2txl(bytes);
    assert(tl[0] >= MDBX_TXL_INITIAL);
    tl[1] = 0;
    tl += 1;
  }
  return tl;
}

static void mdbx_txl_free(MDBX_TXL tl) {
  if (likely(tl))
    mdbx_free(tl - 1);
}

static int mdbx_txl_reserve(MDBX_TXL *ptl, const size_t wanna) {
  const size_t allocated = (size_t)MDBX_PNL_ALLOCLEN(*ptl);
  assert(MDBX_PNL_SIZE(*ptl) <= MDBX_TXL_MAX &&
         MDBX_PNL_ALLOCLEN(*ptl) >= MDBX_PNL_SIZE(*ptl));
  if (likely(allocated >= wanna))
    return MDBX_SUCCESS;

  if (unlikely(wanna > /* paranoia */ MDBX_TXL_MAX))
    return MDBX_TXN_FULL;

  const size_t size = (wanna + wanna - allocated < MDBX_TXL_MAX)
                          ? wanna + wanna - allocated
                          : MDBX_TXL_MAX;
  const size_t bytes = txl2bytes(size);
  MDBX_TXL tl = mdbx_realloc(*ptl - 1, bytes);
  if (likely(tl)) {
#if __GLIBC_PREREQ(2, 12)
    const size_t bytes = malloc_usable_size(tl);
#endif
    *tl = bytes2txl(bytes);
    assert(*tl >= wanna);
    *ptl = tl + 1;
    return MDBX_SUCCESS;
  }
  return MDBX_ENOMEM;
}

static __inline int __must_check_result mdbx_txl_need(MDBX_TXL *ptl,
                                                      size_t num) {
  assert(MDBX_PNL_SIZE(*ptl) <= MDBX_TXL_MAX &&
         MDBX_PNL_ALLOCLEN(*ptl) >= MDBX_PNL_SIZE(*ptl));
  assert(num <= MDBX_PNL_MAX);
  const size_t wanna = (size_t)MDBX_PNL_SIZE(*ptl) + num;
  return likely(MDBX_PNL_ALLOCLEN(*ptl) >= wanna)
             ? MDBX_SUCCESS
             : mdbx_txl_reserve(ptl, wanna);
}

static __inline void mdbx_txl_xappend(MDBX_TXL tl, txnid_t id) {
  assert(MDBX_PNL_SIZE(tl) < MDBX_PNL_ALLOCLEN(tl));
  MDBX_PNL_SIZE(tl) += 1;
  MDBX_PNL_LAST(tl) = id;
}

static int mdbx_txl_cmp(const void *pa, const void *pb) {
  const txnid_t a = *(MDBX_TXL)pa;
  const txnid_t b = *(MDBX_TXL)pb;
  return mdbx_cmp2int(b, a);
}

static void mdbx_txl_sort(MDBX_TXL ptr) {
  /* LY: temporary */
  qsort(ptr + 1, (size_t)ptr[0], sizeof(*ptr), mdbx_txl_cmp);
}

static int __must_check_result mdbx_txl_append(MDBX_TXL *ptl, txnid_t id) {
  if (unlikely(MDBX_PNL_SIZE(*ptl) == MDBX_PNL_ALLOCLEN(*ptl))) {
    int rc = mdbx_txl_need(ptl, MDBX_TXL_GRANULATE);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  mdbx_txl_xappend(*ptl, id);
  return MDBX_SUCCESS;
}

static int __must_check_result mdbx_txl_append_list(MDBX_TXL *ptl,
                                                    MDBX_TXL append) {
  int rc = mdbx_txl_need(ptl, (size_t)MDBX_PNL_SIZE(append));
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  memcpy(MDBX_PNL_END(*ptl), MDBX_PNL_BEGIN(append),
         (size_t)MDBX_PNL_SIZE(append) * sizeof(txnid_t));
  MDBX_PNL_SIZE(*ptl) += MDBX_PNL_SIZE(append);
  return MDBX_SUCCESS;
}

/*----------------------------------------------------------------------------*/

/* Returns the index of the first dirty-page whose pgno
 * member is greater than or equal to id. */
static unsigned __hot mdbx_dpl_search(MDBX_DPL dl, pgno_t id) {
  /* binary search of id in array
   * if found, returns position of id
   * if not found, returns first position greater than id */
  unsigned base = 0;
  unsigned cursor = 1;
  int val = 0;
  unsigned n = dl->length;

#if MDBX_DEBUG
  for (const MDBX_DP *ptr = dl + dl->length; --ptr > dl;) {
    assert(ptr[0].pgno < ptr[1].pgno);
    assert(ptr[0].pgno >= NUM_METAS);
  }
#endif

  while (n > 0) {
    unsigned pivot = n >> 1;
    cursor = base + pivot + 1;
    val = mdbx_cmp2int(id, dl[cursor].pgno);

    if (val < 0) {
      n = pivot;
    } else if (val > 0) {
      base = cursor;
      n -= pivot + 1;
    } else {
      return cursor;
    }
  }

  if (val > 0)
    ++cursor;

  return cursor;
}

static int mdbx_dpl_cmp(const void *pa, const void *pb) {
  const MDBX_DP a = *(MDBX_DPL)pa;
  const MDBX_DP b = *(MDBX_DPL)pb;
  return mdbx_cmp2int(a.pgno, b.pgno);
}

static void mdbx_dpl_sort(MDBX_DPL dl) {
  assert(dl->length <= MDBX_DPL_TXNFULL);
  /* LY: temporary */
  qsort(dl + 1, dl->length, sizeof(*dl), mdbx_dpl_cmp);
}

static int __must_check_result mdbx_dpl_insert(MDBX_DPL dl, pgno_t pgno,
                                               MDBX_page *page) {
  assert(dl->length <= MDBX_DPL_TXNFULL);
  unsigned x = mdbx_dpl_search(dl, pgno);
  assert((int)x > 0);
  if (unlikely(dl[x].pgno == pgno && x <= dl->length))
    return /* duplicate */ MDBX_PROBLEM;

  if (unlikely(dl->length == MDBX_DPL_TXNFULL))
    return MDBX_TXN_FULL;

  /* insert page */
  for (unsigned i = dl->length += 1; i > x; --i)
    dl[i] = dl[i - 1];

  dl[x].pgno = pgno;
  dl[x].ptr = page;
  return MDBX_SUCCESS;
}

static int __must_check_result mdbx_dpl_append(MDBX_DPL dl, pgno_t pgno,
                                               MDBX_page *page) {
  assert(dl->length <= MDBX_DPL_TXNFULL);
#if MDBX_DEBUG
  for (unsigned i = dl->length; i > 0; --i) {
    assert(dl[i].pgno != pgno);
    if (unlikely(dl[i].pgno == pgno))
      return MDBX_PROBLEM;
  }
#endif

  if (unlikely(dl->length == MDBX_DPL_TXNFULL))
    return MDBX_TXN_FULL;

  /* append page */
  const unsigned i = dl->length += 1;
  dl[i].pgno = pgno;
  dl[i].ptr = page;
  return MDBX_SUCCESS;
}

/*----------------------------------------------------------------------------*/

int mdbx_runtime_flags = MDBX_DBG_PRINT
#if MDBX_DEBUG
                         | MDBX_DBG_ASSERT
#endif
#if MDBX_DEBUG > 1
                         | MDBX_DBG_TRACE
#endif
#if MDBX_DEBUG > 2
                         | MDBX_DBG_AUDIT
#endif
#if MDBX_DEBUG > 3
                         | MDBX_DBG_EXTRA
#endif
    ;

MDBX_debug_func *mdbx_debug_logger;

static int mdbx_page_alloc(MDBX_cursor *mc, unsigned num, MDBX_page **mp,
                           int flags);
static int mdbx_page_new(MDBX_cursor *mc, uint32_t flags, unsigned num,
                         MDBX_page **mp);
static int mdbx_page_touch(MDBX_cursor *mc);
static int mdbx_cursor_touch(MDBX_cursor *mc);

#define MDBX_END_NAMES                                                         \
  {                                                                            \
    "committed", "empty-commit", "abort", "reset", "reset-tmp", "fail-begin",  \
        "fail-beginchild"                                                      \
  }
enum {
  /* mdbx_txn_end operation number, for logging */
  MDBX_END_COMMITTED,
  MDBX_END_EMPTY_COMMIT,
  MDBX_END_ABORT,
  MDBX_END_RESET,
  MDBX_END_RESET_TMP,
  MDBX_END_FAIL_BEGIN,
  MDBX_END_FAIL_BEGINCHILD
};
#define MDBX_END_OPMASK 0x0F  /* mask for mdbx_txn_end() operation number */
#define MDBX_END_UPDATE 0x10  /* update env state (DBIs) */
#define MDBX_END_FREE 0x20    /* free txn unless it is MDBX_env.me_txn0 */
#define MDBX_END_EOTDONE 0x40 /* txn's cursors already closed */
#define MDBX_END_SLOT 0x80    /* release any reader slot if MDBX_NOTLS */
static int mdbx_txn_end(MDBX_txn *txn, unsigned mode);

static int __must_check_result mdbx_page_get(MDBX_cursor *mc, pgno_t pgno,
                                             MDBX_page **mp, int *lvl);
static int __must_check_result mdbx_page_search_root(MDBX_cursor *mc,
                                                     MDBX_val *key, int modify);

#define MDBX_PS_MODIFY 1
#define MDBX_PS_ROOTONLY 2
#define MDBX_PS_FIRST 4
#define MDBX_PS_LAST 8
static int __must_check_result mdbx_page_search(MDBX_cursor *mc, MDBX_val *key,
                                                int flags);
static int __must_check_result mdbx_page_merge(MDBX_cursor *csrc,
                                               MDBX_cursor *cdst);

#define MDBX_SPLIT_REPLACE MDBX_APPENDDUP /* newkey is not new */
static int __must_check_result mdbx_page_split(MDBX_cursor *mc,
                                               const MDBX_val *newkey,
                                               MDBX_val *newdata,
                                               pgno_t newpgno, unsigned nflags);

static int __must_check_result mdbx_read_header(MDBX_env *env, MDBX_meta *meta,
                                                uint64_t *filesize);
static int __must_check_result mdbx_sync_locked(MDBX_env *env, unsigned flags,
                                                MDBX_meta *const pending);
static void mdbx_env_close0(MDBX_env *env);

static MDBX_node *mdbx_node_search(MDBX_cursor *mc, MDBX_val *key, int *exactp);

static int __must_check_result mdbx_node_add_branch(MDBX_cursor *mc,
                                                    unsigned indx,
                                                    const MDBX_val *key,
                                                    pgno_t pgno);
static int __must_check_result mdbx_node_add_leaf(MDBX_cursor *mc,
                                                  unsigned indx,
                                                  const MDBX_val *key,
                                                  MDBX_val *data,
                                                  unsigned flags);
static int __must_check_result mdbx_node_add_leaf2(MDBX_cursor *mc,
                                                   unsigned indx,
                                                   const MDBX_val *key);

static void mdbx_node_del(MDBX_cursor *mc, size_t ksize);
static void mdbx_node_shrink(MDBX_page *mp, unsigned indx);
static int __must_check_result mdbx_node_move(MDBX_cursor *csrc,
                                              MDBX_cursor *cdst, int fromleft);
static int __must_check_result mdbx_node_read(MDBX_cursor *mc, MDBX_node *leaf,
                                              MDBX_val *data);
static size_t mdbx_leaf_size(MDBX_env *env, const MDBX_val *key,
                             const MDBX_val *data);
static size_t mdbx_branch_size(MDBX_env *env, const MDBX_val *key);

static int __must_check_result mdbx_rebalance(MDBX_cursor *mc);
static int __must_check_result mdbx_update_key(MDBX_cursor *mc,
                                               const MDBX_val *key);

static void mdbx_cursor_pop(MDBX_cursor *mc);
static int __must_check_result mdbx_cursor_push(MDBX_cursor *mc, MDBX_page *mp);

static int __must_check_result mdbx_cursor_check(MDBX_cursor *mc, bool pending);
static int __must_check_result mdbx_cursor_del0(MDBX_cursor *mc);
static int __must_check_result mdbx_del0(MDBX_txn *txn, MDBX_dbi dbi,
                                         MDBX_val *key, MDBX_val *data,
                                         unsigned flags);
static int __must_check_result mdbx_cursor_sibling(MDBX_cursor *mc,
                                                   int move_right);
static int __must_check_result mdbx_cursor_next(MDBX_cursor *mc, MDBX_val *key,
                                                MDBX_val *data,
                                                MDBX_cursor_op op);
static int __must_check_result mdbx_cursor_prev(MDBX_cursor *mc, MDBX_val *key,
                                                MDBX_val *data,
                                                MDBX_cursor_op op);
static int __must_check_result mdbx_cursor_set(MDBX_cursor *mc, MDBX_val *key,
                                               MDBX_val *data,
                                               MDBX_cursor_op op, int *exactp);
static int __must_check_result mdbx_cursor_first(MDBX_cursor *mc, MDBX_val *key,
                                                 MDBX_val *data);
static int __must_check_result mdbx_cursor_last(MDBX_cursor *mc, MDBX_val *key,
                                                MDBX_val *data);

static int __must_check_result mdbx_cursor_init(MDBX_cursor *mc, MDBX_txn *txn,
                                                MDBX_dbi dbi);
static int __must_check_result mdbx_xcursor_init0(MDBX_cursor *mc);
static int __must_check_result mdbx_xcursor_init1(MDBX_cursor *mc,
                                                  MDBX_node *node);
static int __must_check_result mdbx_xcursor_init2(MDBX_cursor *mc,
                                                  MDBX_xcursor *src_mx,
                                                  int force);

static int __must_check_result mdbx_drop0(MDBX_cursor *mc, int subs);

static MDBX_cmp_func mdbx_cmp_memn, mdbx_cmp_memnr, mdbx_cmp_int_ai,
    mdbx_cmp_int_a2, mdbx_cmp_int_ua;

static const char *__mdbx_strerr(int errnum) {
  /* Table of descriptions for MDBX errors */
  static const char *const tbl[] = {
      "MDBX_KEYEXIST: Key/data pair already exists",
      "MDBX_NOTFOUND: No matching key/data pair found",
      "MDBX_PAGE_NOTFOUND: Requested page not found",
      "MDBX_CORRUPTED: Database is corrupted",
      "MDBX_PANIC: Update of meta page failed or environment had fatal error",
      "MDBX_VERSION_MISMATCH: DB version mismatch libmdbx",
      "MDBX_INVALID: File is not an MDBX file",
      "MDBX_MAP_FULL: Environment mapsize limit reached",
      "MDBX_DBS_FULL: Too may DBI (maxdbs reached)",
      "MDBX_READERS_FULL: Too many readers (maxreaders reached)",
      NULL /* MDBX_TLS_FULL (-30789): unused in MDBX */,
      "MDBX_TXN_FULL: Transaction has too many dirty pages - transaction too "
      "big",
      "MDBX_CURSOR_FULL: Internal error - cursor stack limit reached",
      "MDBX_PAGE_FULL: Internal error - page has no more space",
      "MDBX_MAP_RESIZED: Database contents grew beyond environment mapsize",
      "MDBX_INCOMPATIBLE: Operation and DB incompatible, or DB flags changed",
      "MDBX_BAD_RSLOT: Invalid reuse of reader locktable slot",
      "MDBX_BAD_TXN: Transaction must abort, has a child, or is invalid",
      "MDBX_BAD_VALSIZE: Unsupported size of key/DB name/data, or wrong "
      "DUPFIXED size",
      "MDBX_BAD_DBI: The specified DBI handle was closed/changed unexpectedly",
      "MDBX_PROBLEM: Unexpected problem - txn should abort",
      "MDBX_BUSY: Another write transaction is started",
  };

  if (errnum >= MDBX_KEYEXIST && errnum <= MDBX_LAST_ERRCODE) {
    int i = errnum - MDBX_KEYEXIST;
    return tbl[i];
  }

  switch (errnum) {
  case MDBX_SUCCESS:
    return "MDBX_SUCCESS: Successful";
  case MDBX_EMULTIVAL:
    return "MDBX_EMULTIVAL: Unable to update multi-value for the given key";
  case MDBX_EBADSIGN:
    return "MDBX_EBADSIGN: Wrong signature of a runtime object(s)";
  case MDBX_WANNA_RECOVERY:
    return "MDBX_WANNA_RECOVERY: Database should be recovered, but this could "
           "NOT be done in a read-only mode";
  case MDBX_EKEYMISMATCH:
    return "MDBX_EKEYMISMATCH: The given key value is mismatched to the "
           "current cursor position";
  case MDBX_TOO_LARGE:
    return "MDBX_TOO_LARGE: Database is too large for current system, "
           "e.g. could NOT be mapped into RAM";
  case MDBX_THREAD_MISMATCH:
    return "MDBX_THREAD_MISMATCH: A thread has attempted to use a not "
           "owned object, e.g. a transaction that started by another thread";
  default:
    return NULL;
  }
}

const char *__cold mdbx_strerror_r(int errnum, char *buf, size_t buflen) {
  const char *msg = __mdbx_strerr(errnum);
  if (!msg) {
    if (!buflen || buflen > INT_MAX)
      return NULL;
#if defined(_WIN32) || defined(_WIN64)
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, (DWORD)buflen,
        NULL);
    return size ? buf : NULL;
#elif defined(_GNU_SOURCE) && defined(__GLIBC__)
    /* GNU-specific */
    msg = strerror_r(errnum, buf, buflen);
#elif (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600)
    /* XSI-compliant */
    int rc = strerror_r(errnum, buf, buflen);
    if (rc) {
      rc = snprintf(buf, buflen, "error %d", errnum);
      assert(rc > 0);
    }
    return buf;
#else
    strncpy(buf, strerror(errnum), buflen);
    buf[buflen - 1] = '\0';
    return buf;
#endif
  }
  return msg;
}

const char *__cold mdbx_strerror(int errnum) {
  const char *msg = __mdbx_strerr(errnum);
  if (!msg) {
#if defined(_WIN32) || defined(_WIN64)
    static char buffer[1024];
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer,
        sizeof(buffer), NULL);
    if (size)
      msg = buffer;
#else
    if (errnum < 0) {
      static char buffer[32];
      int rc = snprintf(buffer, sizeof(buffer) - 1, "unknown error %d", errnum);
      assert(rc > 0);
      (void)rc;
      return buffer;
    }
    msg = strerror(errnum);
#endif
  }
  return msg;
}

static txnid_t mdbx_oomkick(MDBX_env *env, const txnid_t laggard);

void __cold mdbx_debug_log(int type, const char *function, int line,
                           const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  if (mdbx_debug_logger)
    mdbx_debug_logger(type, function, line, fmt, args);
  else {
#if defined(_WIN32) || defined(_WIN64)
    if (IsDebuggerPresent()) {
      int prefix_len = 0;
      char *prefix = nullptr;
      if (function && line > 0)
        prefix_len = mdbx_asprintf(&prefix, "%s:%d ", function, line);
      else if (function)
        prefix_len = mdbx_asprintf(&prefix, "%s: ", function);
      else if (line > 0)
        prefix_len = mdbx_asprintf(&prefix, "%d: ", line);
      if (prefix_len > 0 && prefix) {
        OutputDebugStringA(prefix);
        mdbx_free(prefix);
      }
      char *msg = nullptr;
      int msg_len = mdbx_vasprintf(&msg, fmt, args);
      if (msg_len > 0 && msg) {
        OutputDebugStringA(msg);
        mdbx_free(msg);
      }
    }
#else
    if (function && line > 0)
      fprintf(stderr, "%s:%d ", function, line);
    else if (function)
      fprintf(stderr, "%s: ", function);
    else if (line > 0)
      fprintf(stderr, "%d: ", line);
    vfprintf(stderr, fmt, args);
    fflush(stderr);
#endif
  }
  va_end(args);
}

/* Dump a key in ascii or hexadecimal. */
char *mdbx_dkey(const MDBX_val *key, char *const buf, const size_t bufsize) {
  if (!key)
    return "<null>";
  if (!buf || bufsize < 4)
    return nullptr;
  if (!key->iov_len)
    return "<empty>";

  const uint8_t *const data = key->iov_base;
  bool is_ascii = true;
  unsigned i;
  for (i = 0; is_ascii && i < key->iov_len; i++)
    if (data[i] < ' ' || data[i] > 127)
      is_ascii = false;

  if (is_ascii) {
    int len =
        snprintf(buf, bufsize, "%.*s",
                 (key->iov_len > INT_MAX) ? INT_MAX : (int)key->iov_len, data);
    assert(len > 0 && (unsigned)len < bufsize);
    (void)len;
  } else {
    char *const detent = buf + bufsize - 2;
    char *ptr = buf;
    *ptr++ = '<';
    for (i = 0; i < key->iov_len; i++) {
      const ptrdiff_t left = detent - ptr;
      assert(left > 0);
      int len = snprintf(ptr, left, "%02x", data[i]);
      if (len < 0 || len >= left)
        break;
      ptr += len;
    }
    if (ptr < detent) {
      ptr[0] = '>';
      ptr[1] = '\0';
    }
  }
  return buf;
}

#if 0  /* LY: debug stuff */
static const char *mdbx_leafnode_type(MDBX_node *n) {
  static char *const tp[2][2] = {{"", ": DB"}, {": sub-page", ": sub-DB"}};
  return F_ISSET(n->mn_flags, F_BIGDATA) ? ": overflow page"
                                         : tp[F_ISSET(n->mn_flags, F_DUPDATA)]
                                             [F_ISSET(n->mn_flags, F_SUBDATA)];
}

/* Display all the keys in the page. */
static void mdbx_page_list(MDBX_page *mp) {
  pgno_t pgno = mp->mp_pgno;
  const char *type, *state = IS_DIRTY(mp) ? ", dirty" : "";
  MDBX_node *node;
  unsigned i, nkeys, nsize, total = 0;
  MDBX_val key;
  DKBUF;

  switch (mp->mp_flags &
          (P_BRANCH | P_LEAF | P_LEAF2 | P_META | P_OVERFLOW | P_SUBP)) {
  case P_BRANCH:
    type = "Branch page";
    break;
  case P_LEAF:
    type = "Leaf page";
    break;
  case P_LEAF | P_SUBP:
    type = "Leaf sub-page";
    break;
  case P_LEAF | P_LEAF2:
    type = "Leaf2 page";
    break;
  case P_LEAF | P_LEAF2 | P_SUBP:
    type = "Leaf2 sub-page";
    break;
  case P_OVERFLOW:
    mdbx_print("Overflow page %" PRIu64 " pages %u%s\n", pgno, mp->mp_pages,
               state);
    return;
  case P_META:
    mdbx_print("Meta-page %" PRIu64 " txnid %" PRIu64 "\n", pgno,
               ((MDBX_meta *)PAGEDATA(mp))->mm_txnid);
    return;
  default:
    mdbx_print("Bad page %" PRIu64 " flags 0x%X\n", pgno, mp->mp_flags);
    return;
  }

  nkeys = NUMKEYS(mp);
  mdbx_print("%s %" PRIu64 " numkeys %u%s\n", type, pgno, nkeys, state);

  for (i = 0; i < nkeys; i++) {
    if (IS_LEAF2(mp)) { /* LEAF2 pages have no mp_ptrs[] or node headers */
      key.iov_len = nsize = mp->mp_leaf2_ksize;
      key.iov_base = LEAF2KEY(mp, i, nsize);
      total += nsize;
      mdbx_print("key %u: nsize %u, %s\n", i, nsize, DKEY(&key));
      continue;
    }
    node = NODEPTR(mp, i);
    key.iov_len = node->mn_ksize;
    key.iov_base = node->mn_data;
    nsize = NODESIZE + key.iov_len;
    if (IS_BRANCH(mp)) {
      mdbx_print("key %u: page %" PRIu64 ", %s\n", i, NODEPGNO(node),
                 DKEY(&key));
      total += nsize;
    } else {
      if (F_ISSET(node->mn_flags, F_BIGDATA))
        nsize += sizeof(pgno_t);
      else
        nsize += NODEDSZ(node);
      total += nsize;
      nsize += sizeof(indx_t);
      mdbx_print("key %u: nsize %u, %s%s\n", i, nsize, DKEY(&key),
                 mdbx_leafnode_type(node));
    }
    total = EVEN(total);
  }
  mdbx_print("Total: header %u + contents %u + unused %u\n",
             IS_LEAF2(mp) ? PAGEHDRSZ : PAGEHDRSZ + mp->mp_lower, total,
             SIZELEFT(mp));
}

static void mdbx_cursor_chk(MDBX_cursor *mc) {
  unsigned i;
  MDBX_node *node;
  MDBX_page *mp;

  if (!mc->mc_snum || !(mc->mc_flags & C_INITIALIZED))
    return;
  for (i = 0; i < mc->mc_top; i++) {
    mp = mc->mc_pg[i];
    node = NODEPTR(mp, mc->mc_ki[i]);
    if (unlikely(NODEPGNO(node) != mc->mc_pg[i + 1]->mp_pgno))
      mdbx_print("oops!\n");
  }
  if (unlikely(mc->mc_ki[i] >= NUMKEYS(mc->mc_pg[i])))
    mdbx_print("ack!\n");
  if (XCURSOR_INITED(mc)) {
    node = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
    if (((node->mn_flags & (F_DUPDATA | F_SUBDATA)) == F_DUPDATA) &&
        mc->mc_xcursor->mx_cursor.mc_pg[0] != NODEDATA(node)) {
      mdbx_print("blah!\n");
    }
  }
}
#endif /* 0 */

int mdbx_cmp(MDBX_txn *txn, MDBX_dbi dbi, const MDBX_val *a,
             const MDBX_val *b) {
  mdbx_assert(NULL, txn->mt_signature == MDBX_MT_SIGNATURE);
  return txn->mt_dbxs[dbi].md_cmp(a, b);
}

int mdbx_dcmp(MDBX_txn *txn, MDBX_dbi dbi, const MDBX_val *a,
              const MDBX_val *b) {
  mdbx_assert(NULL, txn->mt_signature == MDBX_MT_SIGNATURE);
  return txn->mt_dbxs[dbi].md_dcmp(a, b);
}

/* Allocate memory for a page.
 * Re-use old malloc'd pages first for singletons, otherwise just malloc.
 * Set MDBX_TXN_ERROR on failure. */
static MDBX_page *mdbx_page_malloc(MDBX_txn *txn, unsigned num) {
  MDBX_env *env = txn->mt_env;
  MDBX_page *np = env->me_dpages;
  size_t size = env->me_psize;
  if (likely(num == 1 && np)) {
    ASAN_UNPOISON_MEMORY_REGION(np, size);
    VALGRIND_MEMPOOL_ALLOC(env, np, size);
    VALGRIND_MAKE_MEM_DEFINED(&np->mp_next, sizeof(np->mp_next));
    env->me_dpages = np->mp_next;
  } else {
    size = pgno2bytes(env, num);
    np = mdbx_malloc(size);
    if (unlikely(!np)) {
      txn->mt_flags |= MDBX_TXN_ERROR;
      return np;
    }
    VALGRIND_MEMPOOL_ALLOC(env, np, size);
  }

  if ((env->me_flags & MDBX_NOMEMINIT) == 0) {
    /* For a single page alloc, we init everything after the page header.
     * For multi-page, we init the final page; if the caller needed that
     * many pages they will be filling in at least up to the last page. */
    size_t skip = PAGEHDRSZ;
    if (num > 1)
      skip += pgno2bytes(env, num - 1);
    memset((char *)np + skip, 0, size - skip);
  }
#if MDBX_DEBUG
  np->mp_pgno = 0;
#endif
  VALGRIND_MAKE_MEM_UNDEFINED(np, size);
  np->mp_flags = 0;
  np->mp_pages = num;
  return np;
}

/* Free a dirty page */
static void mdbx_dpage_free(MDBX_env *env, MDBX_page *dp, unsigned pages) {
#if MDBX_DEBUG
  dp->mp_pgno = MAX_PAGENO;
#endif
  if (pages == 1) {
    dp->mp_next = env->me_dpages;
    VALGRIND_MEMPOOL_FREE(env, dp);
    env->me_dpages = dp;
  } else {
    /* large pages just get freed directly */
    VALGRIND_MEMPOOL_FREE(env, dp);
    mdbx_free(dp);
  }
}

/* Return all dirty pages to dpage list */
static void mdbx_dlist_free(MDBX_txn *txn) {
  MDBX_env *env = txn->mt_env;
  MDBX_DPL dl = txn->mt_rw_dirtylist;
  size_t i, n = dl->length;

  for (i = 1; i <= n; i++) {
    MDBX_page *dp = dl[i].ptr;
    mdbx_dpage_free(env, dp, IS_OVERFLOW(dp) ? dp->mp_pages : 1);
  }

  dl->length = 0;
}

static size_t bytes_align2os_bytes(const MDBX_env *env, size_t bytes) {
  return mdbx_roundup2(mdbx_roundup2(bytes, env->me_psize), env->me_os_psize);
}

static void __cold mdbx_kill_page(MDBX_env *env, MDBX_page *mp) {
  const size_t len = env->me_psize - PAGEHDRSZ;
  void *ptr = (env->me_flags & MDBX_WRITEMAP)
                  ? &mp->mp_data
                  : (void *)((uint8_t *)env->me_pbuf + env->me_psize);
  memset(ptr, 0x6F /* 'o', 111 */, len);
  if (ptr != &mp->mp_data)
    (void)mdbx_pwrite(env->me_fd, ptr, len,
                      pgno2bytes(env, mp->mp_pgno) + PAGEHDRSZ);

  VALGRIND_MAKE_MEM_NOACCESS(&mp->mp_data, len);
  ASAN_POISON_MEMORY_REGION(&mp->mp_data, len);
}

static __inline MDBX_db *mdbx_outer_db(MDBX_cursor *mc) {
  mdbx_cassert(mc, (mc->mc_flags & C_SUB) != 0);
  MDBX_xcursor *mx = container_of(mc->mc_db, MDBX_xcursor, mx_db);
  MDBX_cursor_couple *couple = container_of(mx, MDBX_cursor_couple, inner);
  mdbx_cassert(mc, mc->mc_db == &couple->outer.mc_xcursor->mx_db);
  mdbx_cassert(mc, mc->mc_dbx == &couple->outer.mc_xcursor->mx_dbx);
  return couple->outer.mc_db;
}

static int mdbx_page_befree(MDBX_cursor *mc, MDBX_page *mp) {
  MDBX_txn *txn = mc->mc_txn;

  mdbx_cassert(mc, (mc->mc_flags & C_SUB) == 0);
  if (IS_BRANCH(mp)) {
    mc->mc_db->md_branch_pages--;
  } else if (IS_LEAF(mp)) {
    mc->mc_db->md_leaf_pages--;
  } else {
    mdbx_cassert(mc, IS_OVERFLOW(mp));
    mc->mc_db->md_overflow_pages -= mp->mp_pages;
    return mdbx_pnl_append_range(&txn->mt_befree_pages, mp->mp_pgno,
                                 mp->mp_pages);
  }

  return mdbx_pnl_append(&txn->mt_befree_pages, mp->mp_pgno);
}

/* Loosen or free a single page.
 *
 * Saves single pages to a list for future reuse
 * in this same txn. It has been pulled from the freeDB
 * and already resides on the dirty list, but has been
 * deleted. Use these pages first before pulling again
 * from the freeDB.
 *
 * If the page wasn't dirtied in this txn, just add it
 * to this txn's free list. */
static int mdbx_page_loose(MDBX_cursor *mc, MDBX_page *mp) {
  int loose = 0;
  const pgno_t pgno = mp->mp_pgno;
  MDBX_txn *txn = mc->mc_txn;

  if (unlikely(mc->mc_flags & C_SUB)) {
    MDBX_db *outer = mdbx_outer_db(mc);
    if (IS_BRANCH(mp))
      outer->md_branch_pages--;
    else {
      mdbx_cassert(mc, IS_LEAF(mp));
      outer->md_leaf_pages--;
    }
  }

  if (IS_BRANCH(mp))
    mc->mc_db->md_branch_pages--;
  else {
    mdbx_cassert(mc, IS_LEAF(mp));
    mc->mc_db->md_leaf_pages--;
  }

  if (IS_DIRTY(mp)) {
    if (txn->mt_parent) {
      /* LY: TODO: use dedicated flag for tracking parent's dirty pages */
      mdbx_cassert(mc, (txn->mt_env->me_flags & MDBX_WRITEMAP) == 0);
      MDBX_DP *dl = txn->mt_rw_dirtylist;
      /* If txn has a parent,
       * make sure the page is in our dirty list. */
      if (dl->length) {
        unsigned x = mdbx_dpl_search(dl, pgno);
        if (x <= dl->length && dl[x].pgno == pgno) {
          if (unlikely(mp != dl[x].ptr)) { /* bad cursor? */
            mdbx_error("wrong page 0x%p #%" PRIaPGNO
                       " in the dirtylist[%d], expecting %p",
                       dl[x].ptr, pgno, x, mp);
            mc->mc_flags &= ~(C_INITIALIZED | C_EOF);
            txn->mt_flags |= MDBX_TXN_ERROR;
            return MDBX_PROBLEM;
          }
          /* ok, it's ours */
          loose = 1;
        }
      }
    } else {
      /* no parent txn, so it's just ours */
      loose = 1;
    }
  }

  if (loose) {
    mdbx_debug("loosen db %d page %" PRIaPGNO, DDBI(mc), mp->mp_pgno);
    MDBX_page **link = &NEXT_LOOSE_PAGE(mp);
    if (unlikely(txn->mt_env->me_flags & MDBX_PAGEPERTURB))
      mdbx_kill_page(txn->mt_env, mp);
    mp->mp_flags = P_LOOSE | P_DIRTY;
    VALGRIND_MAKE_MEM_UNDEFINED(mp, PAGEHDRSZ);
    ASAN_UNPOISON_MEMORY_REGION(link, sizeof(*link));
    *link = txn->mt_loose_pages;
    txn->mt_loose_pages = mp;
    txn->mt_loose_count++;
  } else {
    int rc = mdbx_pnl_append(&txn->mt_befree_pages, pgno);
    mdbx_tassert(txn, rc == MDBX_SUCCESS);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  return MDBX_SUCCESS;
}

/* Set or clear P_KEEP in dirty, non-overflow, non-sub pages watched by txn.
 *
 * [in] mc      A cursor handle for the current operation.
 * [in] pflags  Flags of the pages to update:
 *                - P_DIRTY to set P_KEEP,
 *                - P_DIRTY|P_KEEP to clear it.
 * [in] all     No shortcuts. Needed except after a full mdbx_page_flush().
 *
 * Returns 0 on success, non-zero on failure. */
static int mdbx_pages_xkeep(MDBX_cursor *mc, unsigned pflags, bool all) {
  const unsigned Mask = P_SUBP | P_DIRTY | P_LOOSE | P_KEEP;
  MDBX_txn *txn = mc->mc_txn;
  MDBX_cursor *m3, *m0 = mc;
  MDBX_xcursor *mx;
  MDBX_page *dp, *mp;
  MDBX_node *leaf;
  unsigned i, j;
  int rc = MDBX_SUCCESS, level;

  /* Mark pages seen by cursors: First m0, then tracked cursors */
  for (i = txn->mt_numdbs;;) {
    if (mc->mc_flags & C_INITIALIZED) {
      for (m3 = mc;; m3 = &mx->mx_cursor) {
        mp = NULL;
        for (j = 0; j < m3->mc_snum; j++) {
          mp = m3->mc_pg[j];
          if ((mp->mp_flags & Mask) == pflags)
            mp->mp_flags ^= P_KEEP;
        }
        mx = m3->mc_xcursor;
        /* Proceed to mx if it is at a sub-database */
        if (!(mx && (mx->mx_cursor.mc_flags & C_INITIALIZED)))
          break;
        if (!(mp && IS_LEAF(mp)))
          break;
        leaf = NODEPTR(mp, m3->mc_ki[j - 1]);
        if (!(leaf->mn_flags & F_SUBDATA))
          break;
      }
    }
    mc = mc->mc_next;
    for (; !mc || mc == m0; mc = txn->mt_cursors[--i])
      if (i == 0)
        goto mark_done;
  }

mark_done:
  if (all) {
    /* Mark dirty root pages */
    for (i = 0; i < txn->mt_numdbs; i++) {
      if (txn->mt_dbflags[i] & DB_DIRTY) {
        pgno_t pgno = txn->mt_dbs[i].md_root;
        if (pgno == P_INVALID)
          continue;
        if (unlikely((rc = mdbx_page_get(m0, pgno, &dp, &level)) !=
                     MDBX_SUCCESS))
          break;
        if ((dp->mp_flags & Mask) == pflags && level <= 1)
          dp->mp_flags ^= P_KEEP;
      }
    }
  }

  return rc;
}

static int mdbx_page_flush(MDBX_txn *txn, pgno_t keep);

/* Spill pages from the dirty list back to disk.
 * This is intended to prevent running into MDBX_TXN_FULL situations,
 * but note that they may still occur in a few cases:
 *
 * 1) our estimate of the txn size could be too small. Currently this
 *  seems unlikely, except with a large number of MDBX_MULTIPLE items.
 *
 * 2) child txns may run out of space if their parents dirtied a
 *  lot of pages and never spilled them. TODO: we probably should do
 *  a preemptive spill during mdbx_txn_begin() of a child txn, if
 *  the parent's dirtyroom is below a given threshold.
 *
 * Otherwise, if not using nested txns, it is expected that apps will
 * not run into MDBX_TXN_FULL any more. The pages are flushed to disk
 * the same way as for a txn commit, e.g. their P_DIRTY flag is cleared.
 * If the txn never references them again, they can be left alone.
 * If the txn only reads them, they can be used without any fuss.
 * If the txn writes them again, they can be dirtied immediately without
 * going thru all of the work of mdbx_page_touch(). Such references are
 * handled by mdbx_page_unspill().
 *
 * Also note, we never spill DB root pages, nor pages of active cursors,
 * because we'll need these back again soon anyway. And in nested txns,
 * we can't spill a page in a child txn if it was already spilled in a
 * parent txn. That would alter the parent txns' data even though
 * the child hasn't committed yet, and we'd have no way to undo it if
 * the child aborted.
 *
 * [in] m0    cursor A cursor handle identifying the transaction and
 *            database for which we are checking space.
 * [in] key   For a put operation, the key being stored.
 * [in] data  For a put operation, the data being stored.
 *
 * Returns 0 on success, non-zero on failure. */
static int mdbx_page_spill(MDBX_cursor *m0, MDBX_val *key, MDBX_val *data) {
  MDBX_txn *txn = m0->mc_txn;
  MDBX_DPL dl = txn->mt_rw_dirtylist;

  if (m0->mc_flags & C_SUB)
    return MDBX_SUCCESS;

  /* Estimate how much space this op will take */
  pgno_t i = m0->mc_db->md_depth;
  /* Named DBs also dirty the main DB */
  if (m0->mc_dbi >= CORE_DBS)
    i += txn->mt_dbs[MAIN_DBI].md_depth;
  /* For puts, roughly factor in the key+data size */
  if (key)
    i += bytes2pgno(txn->mt_env, LEAFSIZE(key, data) + txn->mt_env->me_psize);
  i += i; /* double it for good measure */
  pgno_t need = i;

  if (txn->mt_dirtyroom > i)
    return MDBX_SUCCESS;

  if (!txn->mt_spill_pages) {
    txn->mt_spill_pages = mdbx_pnl_alloc(MDBX_DPL_TXNFULL);
    if (unlikely(!txn->mt_spill_pages))
      return MDBX_ENOMEM;
  } else {
    /* purge deleted slots */
    MDBX_PNL sl = txn->mt_spill_pages;
    pgno_t num = MDBX_PNL_SIZE(sl), j = 0;
    for (i = 1; i <= num; i++) {
      if ((sl[i] & 1) == 0)
        sl[++j] = sl[i];
    }
    MDBX_PNL_SIZE(sl) = j;
  }

  /* Preserve pages which may soon be dirtied again */
  int rc = mdbx_pages_xkeep(m0, P_DIRTY, true);
  if (unlikely(rc != MDBX_SUCCESS))
    goto bailout;

  /* Less aggressive spill - we originally spilled the entire dirty list,
   * with a few exceptions for cursor pages and DB root pages. But this
   * turns out to be a lot of wasted effort because in a large txn many
   * of those pages will need to be used again. So now we spill only 1/8th
   * of the dirty pages. Testing revealed this to be a good tradeoff,
   * better than 1/2, 1/4, or 1/10. */
  if (need < MDBX_DPL_TXNFULL / 8)
    need = MDBX_DPL_TXNFULL / 8;

  /* Save the page IDs of all the pages we're flushing */
  /* flush from the tail forward, this saves a lot of shifting later on. */
  for (i = dl->length; i && need; i--) {
    pgno_t pn = dl[i].pgno << 1;
    MDBX_page *dp = dl[i].ptr;
    if (dp->mp_flags & (P_LOOSE | P_KEEP))
      continue;
    /* Can't spill twice,
     * make sure it's not already in a parent's spill list. */
    if (txn->mt_parent) {
      MDBX_txn *tx2;
      for (tx2 = txn->mt_parent; tx2; tx2 = tx2->mt_parent) {
        if (tx2->mt_spill_pages) {
          unsigned j = mdbx_pnl_search(tx2->mt_spill_pages, pn);
          if (j <= MDBX_PNL_SIZE(tx2->mt_spill_pages) &&
              tx2->mt_spill_pages[j] == pn) {
            dp->mp_flags |= P_KEEP;
            break;
          }
        }
      }
      if (tx2)
        continue;
    }
    rc = mdbx_pnl_append(&txn->mt_spill_pages, pn);
    if (unlikely(rc != MDBX_SUCCESS))
      goto bailout;
    need--;
  }
  mdbx_pnl_sort(txn->mt_spill_pages);

  /* Flush the spilled part of dirty list */
  rc = mdbx_page_flush(txn, i);
  if (unlikely(rc != MDBX_SUCCESS))
    goto bailout;

  /* Reset any dirty pages we kept that page_flush didn't see */
  rc = mdbx_pages_xkeep(m0, P_DIRTY | P_KEEP, i != 0);

bailout:
  txn->mt_flags |= rc ? MDBX_TXN_ERROR : MDBX_TXN_SPILLS;
  return rc;
}

/*----------------------------------------------------------------------------*/

#define METAPAGE(env, n) (&pgno2page(env, n)->mp_meta)

#define METAPAGE_END(env) METAPAGE(env, NUM_METAS)

static __inline txnid_t meta_txnid(const MDBX_env *env, const MDBX_meta *meta,
                                   bool allow_volatile) {
  mdbx_assert(env, meta >= METAPAGE(env, 0) || meta < METAPAGE_END(env));
  txnid_t a = meta->mm_txnid_a;
  txnid_t b = meta->mm_txnid_b;
  if (allow_volatile)
    return (a == b) ? a : 0;
  mdbx_assert(env, a == b);
  return a;
}

static __inline txnid_t mdbx_meta_txnid_stable(const MDBX_env *env,
                                               const MDBX_meta *meta) {
  return meta_txnid(env, meta, false);
}

static __inline txnid_t mdbx_meta_txnid_fluid(const MDBX_env *env,
                                              const MDBX_meta *meta) {
  return meta_txnid(env, meta, true);
}

static __inline void mdbx_meta_update_begin(const MDBX_env *env,
                                            MDBX_meta *meta, txnid_t txnid) {
  mdbx_assert(env, meta >= METAPAGE(env, 0) || meta < METAPAGE_END(env));
  mdbx_assert(env, meta->mm_txnid_a < txnid && meta->mm_txnid_b < txnid);
  meta->mm_txnid_a = txnid;
  (void)env;
  mdbx_flush_noncoherent_cpu_writeback();
}

static __inline void mdbx_meta_update_end(const MDBX_env *env, MDBX_meta *meta,
                                          txnid_t txnid) {
  mdbx_assert(env, meta >= METAPAGE(env, 0) || meta < METAPAGE_END(env));
  mdbx_assert(env, meta->mm_txnid_a == txnid);
  mdbx_assert(env, meta->mm_txnid_b < txnid);

  mdbx_jitter4testing(true);
  meta->mm_txnid_b = txnid;
  mdbx_flush_noncoherent_cpu_writeback();
}

static __inline void mdbx_meta_set_txnid(const MDBX_env *env, MDBX_meta *meta,
                                         txnid_t txnid) {
  mdbx_assert(env, meta < METAPAGE(env, 0) || meta > METAPAGE_END(env));
  meta->mm_txnid_a = txnid;
  meta->mm_txnid_b = txnid;
}

static __inline uint64_t mdbx_meta_sign(const MDBX_meta *meta) {
  uint64_t sign = MDBX_DATASIGN_NONE;
#if 0 /* TODO */
  sign = hippeus_hash64(...);
#else
  (void)meta;
#endif
  /* LY: newer returns MDBX_DATASIGN_NONE or MDBX_DATASIGN_WEAK */
  return (sign > MDBX_DATASIGN_WEAK) ? sign : ~sign;
}

enum meta_choise_mode { prefer_last, prefer_noweak, prefer_steady };

static __inline bool mdbx_meta_ot(const enum meta_choise_mode mode,
                                  const MDBX_env *env, const MDBX_meta *a,
                                  const MDBX_meta *b) {
  mdbx_jitter4testing(true);
  txnid_t txnid_a = mdbx_meta_txnid_fluid(env, a);
  txnid_t txnid_b = mdbx_meta_txnid_fluid(env, b);

  mdbx_jitter4testing(true);
  switch (mode) {
  default:
    assert(false);
    __unreachable();
    /* fall through */
    __fallthrough;
  case prefer_steady:
    if (META_IS_STEADY(a) != META_IS_STEADY(b))
      return META_IS_STEADY(b);
    /* fall through */
    __fallthrough;
  case prefer_noweak:
    if (META_IS_WEAK(a) != META_IS_WEAK(b))
      return !META_IS_WEAK(b);
    /* fall through */
    __fallthrough;
  case prefer_last:
    mdbx_jitter4testing(true);
    if (txnid_a == txnid_b)
      return META_IS_STEADY(b) || (META_IS_WEAK(a) && !META_IS_WEAK(b));
    return txnid_a < txnid_b;
  }
}

static __inline bool mdbx_meta_eq(const MDBX_env *env, const MDBX_meta *a,
                                  const MDBX_meta *b) {
  mdbx_jitter4testing(true);
  const txnid_t txnid = mdbx_meta_txnid_fluid(env, a);
  if (!txnid || txnid != mdbx_meta_txnid_fluid(env, b))
    return false;

  mdbx_jitter4testing(true);
  if (META_IS_STEADY(a) != META_IS_STEADY(b))
    return false;

  mdbx_jitter4testing(true);
  return true;
}

static int mdbx_meta_eq_mask(const MDBX_env *env) {
  MDBX_meta *m0 = METAPAGE(env, 0);
  MDBX_meta *m1 = METAPAGE(env, 1);
  MDBX_meta *m2 = METAPAGE(env, 2);

  int rc = mdbx_meta_eq(env, m0, m1) ? 1 : 0;
  if (mdbx_meta_eq(env, m1, m2))
    rc += 2;
  if (mdbx_meta_eq(env, m2, m0))
    rc += 4;
  return rc;
}

static __inline MDBX_meta *mdbx_meta_recent(const enum meta_choise_mode mode,
                                            const MDBX_env *env, MDBX_meta *a,
                                            MDBX_meta *b) {
  const bool a_older_that_b = mdbx_meta_ot(mode, env, a, b);
  mdbx_assert(env, !mdbx_meta_eq(env, a, b));
  return a_older_that_b ? b : a;
}

static __inline MDBX_meta *mdbx_meta_ancient(const enum meta_choise_mode mode,
                                             const MDBX_env *env, MDBX_meta *a,
                                             MDBX_meta *b) {
  const bool a_older_that_b = mdbx_meta_ot(mode, env, a, b);
  mdbx_assert(env, !mdbx_meta_eq(env, a, b));
  return a_older_that_b ? a : b;
}

static __inline MDBX_meta *
mdbx_meta_mostrecent(const enum meta_choise_mode mode, const MDBX_env *env) {
  MDBX_meta *m0 = METAPAGE(env, 0);
  MDBX_meta *m1 = METAPAGE(env, 1);
  MDBX_meta *m2 = METAPAGE(env, 2);

  MDBX_meta *head = mdbx_meta_recent(mode, env, m0, m1);
  head = mdbx_meta_recent(mode, env, head, m2);
  return head;
}

static __hot MDBX_meta *mdbx_meta_steady(const MDBX_env *env) {
  return mdbx_meta_mostrecent(prefer_steady, env);
}

static __hot MDBX_meta *mdbx_meta_head(const MDBX_env *env) {
  return mdbx_meta_mostrecent(prefer_last, env);
}

static __hot txnid_t mdbx_reclaiming_detent(const MDBX_env *env) {
  if (F_ISSET(env->me_flags, MDBX_UTTERLY_NOSYNC))
    return likely(env->me_txn0->mt_owner == mdbx_thread_self())
               ? env->me_txn0->mt_txnid - 1
               : mdbx_meta_txnid_fluid(env, mdbx_meta_head(env));

  return mdbx_meta_txnid_stable(env, mdbx_meta_steady(env));
}

static const char *mdbx_durable_str(const MDBX_meta *const meta) {
  if (META_IS_WEAK(meta))
    return "Weak";
  if (META_IS_STEADY(meta))
    return (meta->mm_datasync_sign == mdbx_meta_sign(meta)) ? "Steady"
                                                            : "Tainted";
  return "Legacy";
}

/*----------------------------------------------------------------------------*/

/* Find oldest txnid still referenced. */
static txnid_t mdbx_find_oldest(MDBX_txn *txn) {
  mdbx_tassert(txn, (txn->mt_flags & MDBX_RDONLY) == 0);
  MDBX_env *env = txn->mt_env;
  const txnid_t edge = mdbx_reclaiming_detent(env);
  mdbx_tassert(txn, edge <= txn->mt_txnid);

  MDBX_lockinfo *const lck = env->me_lck;
  if (unlikely(lck == NULL /* exclusive mode */))
    return env->me_lckless_stub.oldest = edge;

  const txnid_t last_oldest = lck->mti_oldest_reader;
  mdbx_tassert(txn, edge >= last_oldest);
  if (likely(last_oldest == edge))
    return edge;

  const uint32_t nothing_changed = MDBX_STRING_TETRAD("None");
  const uint32_t snap_readers_refresh_flag = lck->mti_readers_refresh_flag;
  mdbx_jitter4testing(false);
  if (snap_readers_refresh_flag == nothing_changed)
    return last_oldest;

  txnid_t oldest = edge;
  lck->mti_readers_refresh_flag = nothing_changed;
  mdbx_flush_noncoherent_cpu_writeback();
  const unsigned snap_nreaders = lck->mti_numreaders;
  for (unsigned i = 0; i < snap_nreaders; ++i) {
    if (lck->mti_readers[i].mr_pid) {
      /* mdbx_jitter4testing(true); */
      const txnid_t snap = lck->mti_readers[i].mr_txnid;
      if (oldest > snap && last_oldest <= /* ignore pending updates */ snap) {
        oldest = snap;
        if (oldest == last_oldest)
          return oldest;
      }
    }
  }

  if (oldest != last_oldest) {
    mdbx_notice("update oldest %" PRIaTXN " -> %" PRIaTXN, last_oldest, oldest);
    mdbx_tassert(txn, oldest >= lck->mti_oldest_reader);
    lck->mti_oldest_reader = oldest;
  }
  return oldest;
}

/* Find largest mvcc-snapshot still referenced. */
static __cold pgno_t mdbx_find_largest(MDBX_env *env, pgno_t largest) {
  MDBX_lockinfo *const lck = env->me_lck;
  if (likely(lck != NULL /* exclusive mode */)) {
    const unsigned snap_nreaders = lck->mti_numreaders;
    for (unsigned i = 0; i < snap_nreaders; ++i) {
    retry:
      if (lck->mti_readers[i].mr_pid) {
        /* mdbx_jitter4testing(true); */
        const pgno_t snap_pages = lck->mti_readers[i].mr_snapshot_pages_used;
        const txnid_t snap_txnid = lck->mti_readers[i].mr_txnid;
        mdbx_memory_barrier();
        if (unlikely(snap_pages != lck->mti_readers[i].mr_snapshot_pages_used ||
                     snap_txnid != lck->mti_readers[i].mr_txnid))
          goto retry;
        if (largest < snap_pages &&
            lck->mti_oldest_reader <= /* ignore pending updates */ snap_txnid &&
            snap_txnid <= env->me_txn0->mt_txnid)
          largest = snap_pages;
      }
    }
  }

  return largest;
}

/* Add a page to the txn's dirty list */
static int __must_check_result mdbx_page_dirty(MDBX_txn *txn, MDBX_page *mp) {
  int (*const adder)(MDBX_DPL, pgno_t pgno, MDBX_page * page) =
      (txn->mt_flags & MDBX_TXN_WRITEMAP) ? mdbx_dpl_append : mdbx_dpl_insert;
  const int rc = adder(txn->mt_rw_dirtylist, mp->mp_pgno, mp);
  if (unlikely(rc != MDBX_SUCCESS)) {
    txn->mt_flags |= MDBX_TXN_ERROR;
    return rc;
  }
  txn->mt_dirtyroom--;
  return MDBX_SUCCESS;
}

__cold static int mdbx_mapresize(MDBX_env *env, const pgno_t size_pgno,
                                 const pgno_t limit_pgno) {
#ifdef USE_VALGRIND
  const size_t prev_mapsize = env->me_mapsize;
  void *const prev_mapaddr = env->me_map;
#endif

  const size_t limit_bytes = pgno_align2os_bytes(env, limit_pgno);
  const size_t size_bytes = pgno_align2os_bytes(env, size_pgno);

  mdbx_info("resize datafile/mapping: "
            "present %" PRIuPTR " -> %" PRIuPTR ", "
            "limit %" PRIuPTR " -> %" PRIuPTR,
            env->me_dbgeo.now, size_bytes, env->me_dbgeo.upper, limit_bytes);

  mdbx_assert(env, limit_bytes >= size_bytes);
  mdbx_assert(env, bytes2pgno(env, size_bytes) == size_pgno);
  mdbx_assert(env, bytes2pgno(env, limit_bytes) == limit_pgno);

#if defined(_WIN32) || defined(_WIN64)
  /* Acquire guard in exclusive mode for:
   *   - to avoid collision between read and write txns around env->me_dbgeo;
   *   - to avoid attachment of new reading threads (see mdbx_rdt_lock); */
  mdbx_srwlock_AcquireExclusive(&env->me_remap_guard);
  mdbx_handle_array_t *suspended = NULL;
  mdbx_handle_array_t array_onstack;
  int rc = MDBX_SUCCESS;
  if (limit_bytes == env->me_dxb_mmap.length &&
      size_bytes == env->me_dxb_mmap.current &&
      env->me_dxb_mmap.current == env->me_dxb_mmap.filesize)
    goto bailout;

  /* 1) Windows allows only extending a read-write section, but not a
   *    corresponing mapped view. Therefore in other cases we must suspend
   *    the local threads for safe remap.
   * 2) At least on Windows 10 1803 the entire mapped section is unavailable
   *    for short time during NtExtendSection() or VirtualAlloc() execution.
   *
   * THEREFORE LOCAL THREADS SUSPENDING IS ALWAYS REQUIRED! */
  array_onstack.limit = ARRAY_LENGTH(array_onstack.handles);
  array_onstack.count = 0;
  suspended = &array_onstack;
  rc = mdbx_suspend_threads_before_remap(env, &suspended);
  if (rc != MDBX_SUCCESS) {
    mdbx_error("failed suspend-for-remap: errcode %d", rc);
    goto bailout;
  }
#else
  /* Acquire guard to avoid collision between read and write txns
   * around env->me_dbgeo */
  int rc = mdbx_fastmutex_acquire(&env->me_remap_guard);
  if (rc != MDBX_SUCCESS)
    return rc;
  if (limit_bytes == env->me_dxb_mmap.length &&
      bytes2pgno(env, size_bytes) == env->me_dbgeo.now)
    goto bailout;
#endif /* Windows */

  rc = mdbx_mresize(env->me_flags, &env->me_dxb_mmap, size_bytes, limit_bytes);

bailout:
  if (rc == MDBX_SUCCESS) {
#if defined(_WIN32) || defined(_WIN64)
    mdbx_assert(env, size_bytes == env->me_dxb_mmap.current);
    mdbx_assert(env, size_bytes <= env->me_dxb_mmap.filesize);
    mdbx_assert(env, limit_bytes == env->me_dxb_mmap.length);
#endif
    env->me_dbgeo.now = size_bytes;
    env->me_dbgeo.upper = limit_bytes;
    if (env->me_txn) {
      mdbx_tassert(env->me_txn, size_pgno >= env->me_txn->mt_next_pgno);
      env->me_txn->mt_end_pgno = env->me_txn0->mt_end_pgno = size_pgno;
    }
#ifdef USE_VALGRIND
    if (prev_mapsize != env->me_mapsize || prev_mapaddr != env->me_map) {
      VALGRIND_DISCARD(env->me_valgrind_handle);
      env->me_valgrind_handle = 0;
      if (env->me_mapsize)
        env->me_valgrind_handle =
            VALGRIND_CREATE_BLOCK(env->me_map, env->me_mapsize, "mdbx");
    }
#endif
  } else {
    if (rc != MDBX_RESULT_TRUE) {
      mdbx_error("failed resize datafile/mapping: "
                 "present %" PRIuPTR " -> %" PRIuPTR ", "
                 "limit %" PRIuPTR " -> %" PRIuPTR ", errcode %d",
                 env->me_dbgeo.now, size_bytes, env->me_dbgeo.upper,
                 limit_bytes, rc);
    } else {
      mdbx_notice("unable resize datafile/mapping: "
                  "present %" PRIuPTR " -> %" PRIuPTR ", "
                  "limit %" PRIuPTR " -> %" PRIuPTR ", errcode %d",
                  env->me_dbgeo.now, size_bytes, env->me_dbgeo.upper,
                  limit_bytes, rc);
    }
    if (!env->me_dxb_mmap.address) {
      env->me_flags |= MDBX_FATAL_ERROR;
      if (env->me_txn)
        env->me_txn->mt_flags |= MDBX_TXN_ERROR;
      rc = MDBX_PANIC;
    }
  }

#if defined(_WIN32) || defined(_WIN64)
  int err = MDBX_SUCCESS;
  mdbx_srwlock_ReleaseExclusive(&env->me_remap_guard);
  if (suspended) {
    err = mdbx_resume_threads_after_remap(suspended);
    if (suspended != &array_onstack)
      mdbx_free(suspended);
  }
#else
  int err = mdbx_fastmutex_release(&env->me_remap_guard);
#endif /* Windows */
  if (err != MDBX_SUCCESS) {
    mdbx_fatal("failed resume-after-remap: errcode %d", err);
    return MDBX_PANIC;
  }
  return rc;
}

/* Allocate page numbers and memory for writing.  Maintain me_last_reclaimed,
 * me_reclaimed_pglist and mt_next_pgno.  Set MDBX_TXN_ERROR on failure.
 *
 * If there are free pages available from older transactions, they
 * are re-used first. Otherwise allocate a new page at mt_next_pgno.
 * Do not modify the freedB, just merge freeDB records into me_reclaimed_pglist
 * and move me_last_reclaimed to say which records were consumed.  Only this
 * function can create me_reclaimed_pglist and move
 * me_last_reclaimed/mt_next_pgno.
 *
 * [in] mc    cursor A cursor handle identifying the transaction and
 *            database for which we are allocating.
 * [in] num   the number of pages to allocate.
 * [out] mp   Address of the allocated page(s). Requests for multiple pages
 *            will always be satisfied by a single contiguous chunk of memory.
 *
 * Returns 0 on success, non-zero on failure.*/

#define MDBX_ALLOC_CACHE 1
#define MDBX_ALLOC_GC 2
#define MDBX_ALLOC_NEW 4
#define MDBX_ALLOC_KICK 8
#define MDBX_ALLOC_ALL                                                         \
  (MDBX_ALLOC_CACHE | MDBX_ALLOC_GC | MDBX_ALLOC_NEW | MDBX_ALLOC_KICK)

static int mdbx_page_alloc(MDBX_cursor *mc, unsigned num, MDBX_page **mp,
                           int flags) {
  int rc;
  MDBX_txn *txn = mc->mc_txn;
  MDBX_env *env = txn->mt_env;
  MDBX_page *np;

  if (likely(flags & MDBX_ALLOC_GC)) {
    flags |= env->me_flags & (MDBX_COALESCE | MDBX_LIFORECLAIM);
    if (unlikely(mc->mc_flags & C_RECLAIMING)) {
      /* If mc is updating the freeDB, then the befree-list cannot play
       * catch-up with itself by growing while trying to save it. */
      flags &=
          ~(MDBX_ALLOC_GC | MDBX_ALLOC_KICK | MDBX_COALESCE | MDBX_LIFORECLAIM);
    } else if (unlikely(txn->mt_dbs[FREE_DBI].md_entries == 0)) {
      /* avoid (recursive) search inside empty tree and while tree is updating,
       * https://github.com/leo-yuriev/libmdbx/issues/31 */
      flags &= ~MDBX_ALLOC_GC;
    }
  }

  if (likely(flags & MDBX_ALLOC_CACHE)) {
    /* If there are any loose pages, just use them */
    mdbx_assert(env, mp && num);
    if (likely(num == 1 && txn->mt_loose_pages)) {
      np = txn->mt_loose_pages;
      txn->mt_loose_pages = NEXT_LOOSE_PAGE(np);
      txn->mt_loose_count--;
      mdbx_debug("db %d use loose page %" PRIaPGNO, DDBI(mc), np->mp_pgno);
      ASAN_UNPOISON_MEMORY_REGION(np, env->me_psize);
      mdbx_tassert(txn, np->mp_pgno < txn->mt_next_pgno);
      mdbx_ensure(env, np->mp_pgno >= NUM_METAS);
      *mp = np;
      return MDBX_SUCCESS;
    }
  }

  mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  pgno_t pgno, *repg_list = env->me_reclaimed_pglist;
  unsigned repg_pos = 0, repg_len = repg_list ? MDBX_PNL_SIZE(repg_list) : 0;
  txnid_t oldest = 0, last = 0;
  const unsigned wanna_range = num - 1;

  while (1) { /* oom-kick retry loop */
    /* If our dirty list is already full, we can't do anything */
    if (unlikely(txn->mt_dirtyroom == 0)) {
      rc = MDBX_TXN_FULL;
      goto fail;
    }

    MDBX_cursor recur;
    for (MDBX_cursor_op op = MDBX_FIRST;;
         op = (flags & MDBX_LIFORECLAIM) ? MDBX_PREV : MDBX_NEXT) {
      MDBX_val key, data;

      /* Seek a big enough contiguous page range.
       * Prefer pages with lower pgno. */
      mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
      if (likely(flags & MDBX_ALLOC_CACHE) && repg_len > wanna_range &&
          (!(flags & MDBX_COALESCE) || op == MDBX_FIRST)) {
#if MDBX_PNL_ASCENDING
        for (repg_pos = 1; repg_pos <= repg_len - wanna_range; ++repg_pos) {
          pgno = repg_list[repg_pos];
          if (likely(repg_list[repg_pos + wanna_range - 1] ==
                     pgno + wanna_range - 1))
            goto done;
        }
#else
        repg_pos = repg_len;
        do {
          pgno = repg_list[repg_pos];
          if (likely(repg_list[repg_pos - wanna_range] == pgno + wanna_range))
            goto done;
        } while (--repg_pos > wanna_range);
#endif /* MDBX_PNL sort-order */
      }

      if (op == MDBX_FIRST) { /* 1st iteration, setup cursor, etc */
        if (unlikely(!(flags & MDBX_ALLOC_GC)))
          break /* reclaiming is prohibited for now */;

        /* Prepare to fetch more and coalesce */
        oldest = (flags & MDBX_LIFORECLAIM) ? mdbx_find_oldest(txn)
                                            : *env->me_oldest;
        rc = mdbx_cursor_init(&recur, txn, FREE_DBI);
        if (unlikely(rc != MDBX_SUCCESS))
          goto fail;
        if (flags & MDBX_LIFORECLAIM) {
          /* Begin from oldest reader if any */
          if (oldest > 2) {
            last = oldest - 1;
            op = MDBX_SET_RANGE;
          }
        } else if (env->me_last_reclaimed) {
          /* Continue lookup from env->me_last_reclaimed to oldest reader */
          last = env->me_last_reclaimed;
          op = MDBX_SET_RANGE;
        }

        key.iov_base = &last;
        key.iov_len = sizeof(last);
      }

      if (!(flags & MDBX_LIFORECLAIM)) {
        /* Do not try fetch more if the record will be too recent */
        if (op != MDBX_FIRST && ++last >= oldest) {
          oldest = mdbx_find_oldest(txn);
          if (oldest <= last)
            break;
        }
      }

      rc = mdbx_cursor_get(&recur, &key, NULL, op);
      if (rc == MDBX_NOTFOUND && (flags & MDBX_LIFORECLAIM)) {
        if (op == MDBX_SET_RANGE)
          continue;
        if (oldest < mdbx_find_oldest(txn)) {
          oldest = *env->me_oldest;
          last = oldest - 1;
          key.iov_base = &last;
          key.iov_len = sizeof(last);
          op = MDBX_SET_RANGE;
          rc = mdbx_cursor_get(&recur, &key, NULL, op);
        }
      }
      if (unlikely(rc)) {
        if (rc == MDBX_NOTFOUND)
          break;
        goto fail;
      }

      last = *(txnid_t *)key.iov_base;
      if (oldest <= last) {
        oldest = mdbx_find_oldest(txn);
        if (oldest <= last) {
          if (flags & MDBX_LIFORECLAIM)
            continue;
          break;
        }
      }

      if (flags & MDBX_LIFORECLAIM) {
        /* skip IDs of records that already reclaimed */
        if (txn->mt_lifo_reclaimed) {
          unsigned i;
          for (i = (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed); i > 0; --i)
            if (txn->mt_lifo_reclaimed[i] == last)
              break;
          if (i)
            continue;
        }
      }

      /* Reading next FreeDB record */
      np = recur.mc_pg[recur.mc_top];
      MDBX_node *leaf = NODEPTR(np, recur.mc_ki[recur.mc_top]);
      if (unlikely((rc = mdbx_node_read(&recur, leaf, &data)) != MDBX_SUCCESS))
        goto fail;

      if ((flags & MDBX_LIFORECLAIM) && !txn->mt_lifo_reclaimed) {
        txn->mt_lifo_reclaimed = mdbx_txl_alloc();
        if (unlikely(!txn->mt_lifo_reclaimed)) {
          rc = MDBX_ENOMEM;
          goto fail;
        }
      }

      /* Append PNL from FreeDB record to me_reclaimed_pglist */
      mdbx_cassert(mc, (mc->mc_flags & C_GCFREEZE) == 0);
      pgno_t *re_pnl = (pgno_t *)data.iov_base;
      mdbx_tassert(txn, data.iov_len >= MDBX_PNL_SIZEOF(re_pnl));
      mdbx_tassert(txn, mdbx_pnl_check(re_pnl, false));
      repg_pos = MDBX_PNL_SIZE(re_pnl);
      if (!repg_list) {
        if (unlikely(!(env->me_reclaimed_pglist = repg_list =
                           mdbx_pnl_alloc(repg_pos)))) {
          rc = MDBX_ENOMEM;
          goto fail;
        }
      } else {
        if (unlikely(
                (rc = mdbx_pnl_need(&env->me_reclaimed_pglist, repg_pos)) != 0))
          goto fail;
        repg_list = env->me_reclaimed_pglist;
      }

      /* Remember ID of FreeDB record */
      if (flags & MDBX_LIFORECLAIM) {
        if ((rc = mdbx_txl_append(&txn->mt_lifo_reclaimed, last)) != 0)
          goto fail;
      }
      env->me_last_reclaimed = last;

      if (mdbx_debug_enabled(MDBX_DBG_EXTRA)) {
        mdbx_debug_extra("PNL read txn %" PRIaTXN " root %" PRIaPGNO
                         " num %u, PNL",
                         last, txn->mt_dbs[FREE_DBI].md_root, repg_pos);
        unsigned i;
        for (i = repg_pos; i; i--)
          mdbx_debug_extra_print(" %" PRIaPGNO, re_pnl[i]);
        mdbx_debug_extra_print("\n");
      }

      /* Merge in descending sorted order */
      mdbx_pnl_xmerge(repg_list, re_pnl);
      repg_len = MDBX_PNL_SIZE(repg_list);
      if (unlikely((flags & MDBX_ALLOC_CACHE) == 0)) {
        /* Done for a kick-reclaim mode, actually no page needed */
        return MDBX_SUCCESS;
      }

      mdbx_tassert(txn,
                   repg_len == 0 || repg_list[repg_len] < txn->mt_next_pgno);
      if (repg_len) {
        /* Refund suitable pages into "unallocated" space */
        pgno_t tail = txn->mt_next_pgno;
        pgno_t *const begin = repg_list + 1;
        pgno_t *const end = begin + repg_len;
        pgno_t *higest;
#if MDBX_PNL_ASCENDING
        for (higest = end; --higest >= begin;) {
#else
        for (higest = begin; higest < end; ++higest) {
#endif /* MDBX_PNL sort-order */
          mdbx_tassert(txn, *higest >= NUM_METAS && *higest < tail);
          if (*higest != tail - 1)
            break;
          tail -= 1;
        }
        if (tail != txn->mt_next_pgno) {
#if MDBX_PNL_ASCENDING
          repg_len = (unsigned)(higest + 1 - begin);
#else
          repg_len -= (unsigned)(higest - begin);
          for (pgno_t *move = begin; higest < end; ++move, ++higest)
            *move = *higest;
#endif /* MDBX_PNL sort-order */
          MDBX_PNL_SIZE(repg_list) = repg_len;
          mdbx_info("refunded %" PRIaPGNO " pages: %" PRIaPGNO " -> %" PRIaPGNO,
                    tail - txn->mt_next_pgno, tail, txn->mt_next_pgno);
          txn->mt_next_pgno = tail;
          mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
        }
      }

      /* Don't try to coalesce too much. */
      if (unlikely(repg_len > MDBX_DPL_TXNFULL / 4))
        break;
      if (repg_len /* current size */ >= env->me_maxgc_ov1page ||
          repg_pos /* prev size */ >= env->me_maxgc_ov1page / 2)
        flags &= ~MDBX_COALESCE;
    }

    if ((flags & (MDBX_COALESCE | MDBX_ALLOC_CACHE)) ==
            (MDBX_COALESCE | MDBX_ALLOC_CACHE) &&
        repg_len > wanna_range) {
#if MDBX_PNL_ASCENDING
      for (repg_pos = 1; repg_pos <= repg_len - wanna_range; ++repg_pos) {
        pgno = repg_list[repg_pos];
        if (likely(repg_list[repg_pos + wanna_range - 1] ==
                   pgno + wanna_range - 1))
          goto done;
      }
#else
      repg_pos = repg_len;
      do {
        pgno = repg_list[repg_pos];
        if (likely(repg_list[repg_pos - wanna_range] == pgno + wanna_range))
          goto done;
      } while (--repg_pos > wanna_range);
#endif /* MDBX_PNL sort-order */
    }

    /* Use new pages from the map when nothing suitable in the freeDB */
    repg_pos = 0;
    pgno = txn->mt_next_pgno;
    rc = MDBX_MAP_FULL;
    const pgno_t next = pgno_add(pgno, num);
    if (likely(next <= txn->mt_end_pgno)) {
      rc = MDBX_NOTFOUND;
      if (likely(flags & MDBX_ALLOC_NEW))
        goto done;
    }

    const MDBX_meta *head = mdbx_meta_head(env);
    if ((flags & MDBX_ALLOC_GC) &&
        ((flags & MDBX_ALLOC_KICK) || rc == MDBX_MAP_FULL)) {
      MDBX_meta *steady = mdbx_meta_steady(env);

      if (oldest == mdbx_meta_txnid_stable(env, steady) &&
          !META_IS_STEADY(head) && META_IS_STEADY(steady)) {
        /* LY: Here an oom was happened:
         *  - all pages had allocated;
         *  - reclaiming was stopped at the last steady-sync;
         *  - the head-sync is weak.
         * Now we need make a sync to resume reclaiming. If both
         * MDBX_NOSYNC and MDBX_MAPASYNC flags are set, then assume that
         * utterly no-sync write mode was requested. In such case
         * don't make a steady-sync, but only a legacy-mode checkpoint,
         * just for resume reclaiming only, not for data consistency. */

        mdbx_debug("kick-gc: head %" PRIaTXN "-%s, tail %" PRIaTXN
                   "-%s, oldest %" PRIaTXN,
                   mdbx_meta_txnid_stable(env, head), mdbx_durable_str(head),
                   mdbx_meta_txnid_stable(env, steady),
                   mdbx_durable_str(steady), oldest);

        const unsigned syncflags = F_ISSET(env->me_flags, MDBX_UTTERLY_NOSYNC)
                                       ? env->me_flags
                                       : env->me_flags & MDBX_WRITEMAP;
        MDBX_meta meta = *head;
        if (mdbx_sync_locked(env, syncflags, &meta) == MDBX_SUCCESS) {
          txnid_t snap = mdbx_find_oldest(txn);
          if (snap > oldest)
            continue;
        }
      }

      if (rc == MDBX_MAP_FULL && oldest < txn->mt_txnid - 1) {
        if (mdbx_oomkick(env, oldest) > oldest)
          continue;
      }
    }

    if (rc == MDBX_MAP_FULL && next < head->mm_geo.upper) {
      mdbx_assert(env, next > txn->mt_end_pgno);
      pgno_t aligned = pgno_align2os_pgno(
          env, pgno_add(next, head->mm_geo.grow - next % head->mm_geo.grow));

      if (aligned > head->mm_geo.upper)
        aligned = head->mm_geo.upper;
      mdbx_assert(env, aligned > txn->mt_end_pgno);

      mdbx_info("try growth datafile to %" PRIaPGNO " pages (+%" PRIaPGNO ")",
                aligned, aligned - txn->mt_end_pgno);
      rc = mdbx_mapresize(env, aligned, head->mm_geo.upper);
      if (rc == MDBX_SUCCESS) {
        mdbx_tassert(env->me_txn, txn->mt_end_pgno >= next);
        if (!mp)
          return rc;
        goto done;
      }

      mdbx_warning("unable growth datafile to %" PRIaPGNO "pages (+%" PRIaPGNO
                   "), errcode %d",
                   aligned, aligned - txn->mt_end_pgno, rc);
    }

  fail:
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    if (mp) {
      *mp = NULL;
      txn->mt_flags |= MDBX_TXN_ERROR;
    }
    mdbx_assert(env, rc != MDBX_SUCCESS);
    return rc;
  }

done:
  mdbx_tassert(txn, mp && num);
  mdbx_ensure(env, pgno >= NUM_METAS);
  if (env->me_flags & MDBX_WRITEMAP) {
    np = pgno2page(env, pgno);
    /* LY: reset no-access flag from mdbx_kill_page() */
    VALGRIND_MAKE_MEM_UNDEFINED(np, pgno2bytes(env, num));
    ASAN_UNPOISON_MEMORY_REGION(np, pgno2bytes(env, num));
  } else {
    if (unlikely(!(np = mdbx_page_malloc(txn, num)))) {
      rc = MDBX_ENOMEM;
      goto fail;
    }
  }

  if (repg_pos) {
    mdbx_cassert(mc, (mc->mc_flags & C_GCFREEZE) == 0);
    mdbx_tassert(txn, pgno < txn->mt_next_pgno);
    mdbx_tassert(txn, pgno == repg_list[repg_pos]);
    /* Cutoff allocated pages from me_reclaimed_pglist */
    MDBX_PNL_SIZE(repg_list) = repg_len -= num;
    for (unsigned i = repg_pos - num; i < repg_len;)
      repg_list[++i] = repg_list[++repg_pos];
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  } else {
    txn->mt_next_pgno = pgno + num;
    mdbx_assert(env, txn->mt_next_pgno <= txn->mt_end_pgno);
  }

  if (env->me_flags & MDBX_PAGEPERTURB)
    memset(np, 0x71 /* 'q', 113 */, pgno2bytes(env, num));
  VALGRIND_MAKE_MEM_UNDEFINED(np, pgno2bytes(env, num));

  np->mp_pgno = pgno;
  np->mp_leaf2_ksize = 0;
  np->mp_flags = 0;
  np->mp_pages = num;
  rc = mdbx_page_dirty(txn, np);
  if (unlikely(rc != MDBX_SUCCESS))
    goto fail;
  *mp = np;

  mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  return MDBX_SUCCESS;
}

/* Copy the used portions of a non-overflow page.
 * [in] dst page to copy into
 * [in] src page to copy from
 * [in] psize size of a page */
__hot static void mdbx_page_copy(MDBX_page *dst, MDBX_page *src,
                                 unsigned psize) {
  STATIC_ASSERT(UINT16_MAX > MAX_PAGESIZE - PAGEHDRSZ);
  STATIC_ASSERT(MIN_PAGESIZE > PAGEHDRSZ + NODESIZE * 42);
  enum { Align = sizeof(pgno_t) };
  indx_t upper = src->mp_upper, lower = src->mp_lower, unused = upper - lower;

  /* If page isn't full, just copy the used portion. Adjust
   * alignment so memcpy may copy words instead of bytes. */
  if ((unused &= -Align) && !IS_LEAF2(src)) {
    upper = (upper + PAGEHDRSZ) & -Align;
    memcpy(dst, src, (lower + PAGEHDRSZ + (Align - 1)) & -Align);
    memcpy((pgno_t *)((char *)dst + upper), (pgno_t *)((char *)src + upper),
           psize - upper);
  } else {
    memcpy(dst, src, psize - unused);
  }
}

/* Pull a page off the txn's spill list, if present.
 *
 * If a page being referenced was spilled to disk in this txn, bring
 * it back and make it dirty/writable again.
 *
 * [in] txn   the transaction handle.
 * [in] mp    the page being referenced. It must not be dirty.
 * [out] ret  the writable page, if any.
 *            ret is unchanged if mp wasn't spilled. */
__hot static int __must_check_result mdbx_page_unspill(MDBX_txn *txn,
                                                       MDBX_page *mp,
                                                       MDBX_page **ret) {
  MDBX_env *env = txn->mt_env;
  const MDBX_txn *tx2;
  unsigned x;
  pgno_t pgno = mp->mp_pgno, pn = pgno << 1;

  for (tx2 = txn; tx2; tx2 = tx2->mt_parent) {
    if (!tx2->mt_spill_pages)
      continue;
    x = mdbx_pnl_search(tx2->mt_spill_pages, pn);
    if (x <= MDBX_PNL_SIZE(tx2->mt_spill_pages) &&
        tx2->mt_spill_pages[x] == pn) {
      MDBX_page *np;
      int num;
      if (txn->mt_dirtyroom == 0)
        return MDBX_TXN_FULL;
      num = IS_OVERFLOW(mp) ? mp->mp_pages : 1;
      if (env->me_flags & MDBX_WRITEMAP) {
        np = mp;
      } else {
        np = mdbx_page_malloc(txn, num);
        if (unlikely(!np))
          return MDBX_ENOMEM;
        if (unlikely(num > 1))
          memcpy(np, mp, pgno2bytes(env, num));
        else
          mdbx_page_copy(np, mp, env->me_psize);
      }
      mdbx_debug("unspill page %" PRIaPGNO, mp->mp_pgno);
      if (tx2 == txn) {
        /* If in current txn, this page is no longer spilled.
         * If it happens to be the last page, truncate the spill list.
         * Otherwise mark it as deleted by setting the LSB. */
        if (x == MDBX_PNL_SIZE(txn->mt_spill_pages))
          MDBX_PNL_SIZE(txn->mt_spill_pages)--;
        else
          txn->mt_spill_pages[x] |= 1;
      } /* otherwise, if belonging to a parent txn, the
         * page remains spilled until child commits */

      int rc = mdbx_page_dirty(txn, np);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;

      np->mp_flags |= P_DIRTY;
      *ret = np;
      break;
    }
  }
  return MDBX_SUCCESS;
}

/* Touch a page: make it dirty and re-insert into tree with updated pgno.
 * Set MDBX_TXN_ERROR on failure.
 *
 * [in] mc  cursor pointing to the page to be touched
 *
 * Returns 0 on success, non-zero on failure. */
__hot static int mdbx_page_touch(MDBX_cursor *mc) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top], *np;
  MDBX_txn *txn = mc->mc_txn;
  MDBX_cursor *m2, *m3;
  pgno_t pgno;
  int rc;

  mdbx_cassert(mc, !IS_OVERFLOW(mp));
  if (!F_ISSET(mp->mp_flags, P_DIRTY)) {
    if (txn->mt_flags & MDBX_TXN_SPILLS) {
      np = NULL;
      rc = mdbx_page_unspill(txn, mp, &np);
      if (unlikely(rc))
        goto fail;
      if (likely(np))
        goto done;
    }

    if (unlikely((rc = mdbx_pnl_need(&txn->mt_befree_pages, 1)) ||
                 (rc = mdbx_page_alloc(mc, 1, &np, MDBX_ALLOC_ALL))))
      goto fail;
    pgno = np->mp_pgno;
    mdbx_debug("touched db %d page %" PRIaPGNO " -> %" PRIaPGNO, DDBI(mc),
               mp->mp_pgno, pgno);
    mdbx_cassert(mc, mp->mp_pgno != pgno);
    mdbx_pnl_xappend(txn->mt_befree_pages, mp->mp_pgno);
    /* Update the parent page, if any, to point to the new page */
    if (mc->mc_top) {
      MDBX_page *parent = mc->mc_pg[mc->mc_top - 1];
      MDBX_node *node = NODEPTR(parent, mc->mc_ki[mc->mc_top - 1]);
      SETPGNO(node, pgno);
    } else {
      mc->mc_db->md_root = pgno;
    }
  } else if (txn->mt_parent && !IS_SUBP(mp)) {
    mdbx_tassert(txn, (txn->mt_env->me_flags & MDBX_WRITEMAP) == 0);
    MDBX_DP *dl = txn->mt_rw_dirtylist;
    pgno = mp->mp_pgno;
    /* If txn has a parent, make sure the page is in our dirty list. */
    if (dl->length) {
      unsigned x = mdbx_dpl_search(dl, pgno);
      if (x <= dl->length && dl[x].pgno == pgno) {
        if (unlikely(mp != dl[x].ptr)) { /* bad cursor? */
          mdbx_error("wrong page 0x%p #%" PRIaPGNO
                     " in the dirtylist[%d], expecting %p",
                     dl[x].ptr, pgno, x, mp);
          mc->mc_flags &= ~(C_INITIALIZED | C_EOF);
          rc = MDBX_PROBLEM;
          goto fail;
        }
        return MDBX_SUCCESS;
      }
    }

    mdbx_debug("clone db %d page %" PRIaPGNO, DDBI(mc), mp->mp_pgno);
    mdbx_cassert(mc, dl->length <= MDBX_DPL_TXNFULL);
    /* No - copy it */
    np = mdbx_page_malloc(txn, 1);
    if (unlikely(!np)) {
      rc = MDBX_ENOMEM;
      goto fail;
    }
    rc = mdbx_dpl_insert(dl, pgno, np);
    if (unlikely(rc)) {
      mdbx_dpage_free(txn->mt_env, np, 1);
      goto fail;
    }
  } else {
    return MDBX_SUCCESS;
  }

  mdbx_page_copy(np, mp, txn->mt_env->me_psize);
  np->mp_pgno = pgno;
  np->mp_flags |= P_DIRTY;

done:
  /* Adjust cursors pointing to mp */
  mc->mc_pg[mc->mc_top] = np;
  m2 = txn->mt_cursors[mc->mc_dbi];
  if (mc->mc_flags & C_SUB) {
    for (; m2; m2 = m2->mc_next) {
      m3 = &m2->mc_xcursor->mx_cursor;
      if (m3->mc_snum < mc->mc_snum)
        continue;
      if (m3->mc_pg[mc->mc_top] == mp)
        m3->mc_pg[mc->mc_top] = np;
    }
  } else {
    for (; m2; m2 = m2->mc_next) {
      if (m2->mc_snum < mc->mc_snum)
        continue;
      if (m2 == mc)
        continue;
      if (m2->mc_pg[mc->mc_top] == mp) {
        m2->mc_pg[mc->mc_top] = np;
        if (XCURSOR_INITED(m2) && IS_LEAF(np))
          XCURSOR_REFRESH(m2, np, m2->mc_ki[mc->mc_top]);
      }
    }
  }
  return MDBX_SUCCESS;

fail:
  txn->mt_flags |= MDBX_TXN_ERROR;
  return rc;
}

__cold static int mdbx_env_sync_ex(MDBX_env *env, int force, int nonblock) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  unsigned flags = env->me_flags & ~MDBX_NOMETASYNC;
  if (unlikely(flags & (MDBX_RDONLY | MDBX_FATAL_ERROR)))
    return MDBX_EACCESS;

  const bool outside_txn =
      (!env->me_txn0 || env->me_txn0->mt_owner != mdbx_thread_self());

  if (outside_txn) {
    int rc = mdbx_txn_lock(env, nonblock);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  const MDBX_meta *head = mdbx_meta_head(env);
  pgno_t unsynced_pages = *env->me_unsynced_pages;
  if (!META_IS_STEADY(head) || unsynced_pages) {
    const pgno_t autosync_threshold = *env->me_autosync_threshold;
    const uint64_t unsynced_timeout = *env->me_unsynced_timeout;
    if (force || (autosync_threshold && unsynced_pages >= autosync_threshold) ||
        (unsynced_timeout && mdbx_osal_monotime() >= unsynced_timeout))
      flags &= MDBX_WRITEMAP /* clear flags for full steady sync */;

    if (outside_txn) {
      if (unsynced_pages > /* FIXME: define threshold */ 16 &&
          (flags & (MDBX_NOSYNC | MDBX_MAPASYNC)) == 0) {
        mdbx_assert(env, ((flags ^ env->me_flags) & MDBX_WRITEMAP) == 0);
        const size_t usedbytes = pgno_align2os_bytes(env, head->mm_geo.next);

        mdbx_txn_unlock(env);

        /* LY: pre-sync without holding lock to reduce latency for writer(s) */
        int rc = (flags & MDBX_WRITEMAP)
                     ? mdbx_msync(&env->me_dxb_mmap, 0, usedbytes, false)
                     : mdbx_filesync(env->me_fd, MDBX_SYNC_DATA);
        if (unlikely(rc != MDBX_SUCCESS))
          return rc;

        rc = mdbx_txn_lock(env, nonblock);
        if (unlikely(rc != MDBX_SUCCESS))
          return rc;

        /* LY: head and unsynced_pages may be changed. */
        head = mdbx_meta_head(env);
        unsynced_pages = *env->me_unsynced_pages;
      }
      env->me_txn0->mt_txnid = meta_txnid(env, head, false);
      mdbx_find_oldest(env->me_txn0);
    }

    if (!META_IS_STEADY(head) ||
        ((flags & (MDBX_NOSYNC | MDBX_MAPASYNC)) == 0 && unsynced_pages)) {
      mdbx_debug("meta-head %" PRIaPGNO ", %s, sync_pending %" PRIaPGNO,
                 container_of(head, MDBX_page, mp_data)->mp_pgno,
                 mdbx_durable_str(head), unsynced_pages);
      MDBX_meta meta = *head;
      int rc = mdbx_sync_locked(env, flags | MDBX_SHRINK_ALLOWED, &meta);
      if (unlikely(rc != MDBX_SUCCESS)) {
        if (outside_txn)
          mdbx_txn_unlock(env);
        return rc;
      }
    }
  }

  if (outside_txn)
    mdbx_txn_unlock(env);
  return MDBX_SUCCESS;
}

__cold int mdbx_env_sync(MDBX_env *env, int force) {
  return mdbx_env_sync_ex(env, force, false);
}

/* Back up parent txn's cursors, then grab the originals for tracking */
static int mdbx_cursor_shadow(MDBX_txn *src, MDBX_txn *dst) {
  MDBX_cursor *mc, *bk;
  MDBX_xcursor *mx;
  size_t size;
  int i;

  for (i = src->mt_numdbs; --i >= 0;) {
    if ((mc = src->mt_cursors[i]) != NULL) {
      size = sizeof(MDBX_cursor);
      if (mc->mc_xcursor)
        size += sizeof(MDBX_xcursor);
      for (; mc; mc = bk->mc_next) {
        bk = mdbx_malloc(size);
        if (unlikely(!bk))
          return MDBX_ENOMEM;
        *bk = *mc;
        mc->mc_backup = bk;
        mc->mc_db = &dst->mt_dbs[i];
        /* Kill pointers into src to reduce abuse: The
         * user may not use mc until dst ends. But we need a valid
         * txn pointer here for cursor fixups to keep working. */
        mc->mc_txn = dst;
        mc->mc_dbflag = &dst->mt_dbflags[i];
        if ((mx = mc->mc_xcursor) != NULL) {
          *(MDBX_xcursor *)(bk + 1) = *mx;
          mx->mx_cursor.mc_txn = dst;
        }
        mc->mc_next = dst->mt_cursors[i];
        dst->mt_cursors[i] = mc;
      }
    }
  }
  return MDBX_SUCCESS;
}

/* Close this write txn's cursors, give parent txn's cursors back to parent.
 *
 * [in] txn     the transaction handle.
 * [in] merge   true to keep changes to parent cursors, false to revert.
 *
 * Returns 0 on success, non-zero on failure. */
static void mdbx_cursors_eot(MDBX_txn *txn, unsigned merge) {
  MDBX_cursor **cursors = txn->mt_cursors, *mc, *next, *bk;
  MDBX_xcursor *mx;
  int i;

  for (i = txn->mt_numdbs; --i >= 0;) {
    for (mc = cursors[i]; mc; mc = next) {
      unsigned stage = mc->mc_signature;
      mdbx_ensure(NULL,
                  stage == MDBX_MC_SIGNATURE || stage == MDBX_MC_WAIT4EOT);
      next = mc->mc_next;
      if ((bk = mc->mc_backup) != NULL) {
        if (merge) {
          /* Commit changes to parent txn */
          mc->mc_next = bk->mc_next;
          mc->mc_backup = bk->mc_backup;
          mc->mc_txn = bk->mc_txn;
          mc->mc_db = bk->mc_db;
          mc->mc_dbflag = bk->mc_dbflag;
          if ((mx = mc->mc_xcursor) != NULL)
            mx->mx_cursor.mc_txn = bk->mc_txn;
        } else {
          /* Abort nested txn */
          *mc = *bk;
          if ((mx = mc->mc_xcursor) != NULL)
            *mx = *(MDBX_xcursor *)(bk + 1);
        }
        bk->mc_signature = 0;
        mdbx_free(bk);
      }
      if (stage == MDBX_MC_WAIT4EOT) {
        mc->mc_signature = 0;
        mdbx_free(mc);
      } else {
        mc->mc_signature = MDBX_MC_READY4CLOSE;
        mc->mc_flags = 0 /* reset C_UNTRACK */;
      }
    }
    cursors[i] = NULL;
  }
}

/* Common code for mdbx_txn_begin() and mdbx_txn_renew(). */
static int mdbx_txn_renew0(MDBX_txn *txn, unsigned flags) {
  MDBX_env *env = txn->mt_env;
  int rc;

#if MDBX_TXN_CHECKPID
  if (unlikely(env->me_pid != mdbx_getpid())) {
    env->me_flags |= MDBX_FATAL_ERROR;
    return MDBX_PANIC;
  }
#endif /* MDBX_TXN_CHECKPID */

  STATIC_ASSERT(sizeof(MDBX_reader) == 32);
#ifdef MDBX_OSAL_LOCK
  STATIC_ASSERT(offsetof(MDBX_lockinfo, mti_wmutex) % MDBX_CACHELINE_SIZE == 0);
  STATIC_ASSERT(offsetof(MDBX_lockinfo, mti_rmutex) % MDBX_CACHELINE_SIZE == 0);
#else
  STATIC_ASSERT(
      offsetof(MDBX_lockinfo, mti_oldest_reader) % MDBX_CACHELINE_SIZE == 0);
  STATIC_ASSERT(offsetof(MDBX_lockinfo, mti_numreaders) % MDBX_CACHELINE_SIZE ==
                0);
#endif
  STATIC_ASSERT(offsetof(MDBX_lockinfo, mti_readers) % MDBX_CACHELINE_SIZE ==
                0);

  pgno_t upper_limit_pgno = 0;
  if (flags & MDBX_TXN_RDONLY) {
    txn->mt_flags = MDBX_TXN_RDONLY;
    MDBX_reader *r = txn->mt_ro_reader;
    if (likely(env->me_flags & MDBX_ENV_TXKEY)) {
      mdbx_assert(env, !(env->me_flags & MDBX_NOTLS));
      r = mdbx_thread_rthc_get(env->me_txkey);
      if (likely(r)) {
        mdbx_assert(env, r->mr_pid == env->me_pid);
        mdbx_assert(env, r->mr_tid == mdbx_thread_self());
      }
    } else {
      mdbx_assert(env, !env->me_lck || (env->me_flags & MDBX_NOTLS));
    }

    if (likely(r)) {
      if (unlikely(r->mr_pid != env->me_pid || r->mr_txnid != ~(txnid_t)0))
        return MDBX_BAD_RSLOT;
    } else if (env->me_lck) {
      unsigned slot, nreaders;
      const mdbx_tid_t tid = mdbx_thread_self();
      mdbx_assert(env, env->me_lck->mti_magic_and_version == MDBX_LOCK_MAGIC);
      mdbx_assert(env, env->me_lck->mti_os_and_format == MDBX_LOCK_FORMAT);

      rc = mdbx_rdt_lock(env);
      if (unlikely(MDBX_IS_ERROR(rc)))
        return rc;
      if (unlikely(env->me_flags & MDBX_FATAL_ERROR)) {
        mdbx_rdt_unlock(env);
        return MDBX_PANIC;
      }
#if defined(_WIN32) || defined(_WIN64)
      if (unlikely(!env->me_map)) {
        mdbx_rdt_unlock(env);
        return MDBX_EPERM;
      }
#endif /* Windows */
      rc = MDBX_SUCCESS;

      if (unlikely(env->me_live_reader != env->me_pid)) {
        rc = mdbx_rpid_set(env);
        if (unlikely(rc != MDBX_SUCCESS)) {
          mdbx_rdt_unlock(env);
          return rc;
        }
        env->me_live_reader = env->me_pid;
      }

      while (1) {
        nreaders = env->me_lck->mti_numreaders;
        for (slot = 0; slot < nreaders; slot++)
          if (env->me_lck->mti_readers[slot].mr_pid == 0)
            break;

        if (likely(slot < env->me_maxreaders))
          break;

        rc = mdbx_reader_check0(env, true, NULL);
        if (rc != MDBX_RESULT_TRUE) {
          mdbx_rdt_unlock(env);
          return (rc == MDBX_SUCCESS) ? MDBX_READERS_FULL : rc;
        }
      }

      r = &env->me_lck->mti_readers[slot];
      /* Claim the reader slot, carefully since other code
       * uses the reader table un-mutexed: First reset the
       * slot, next publish it in lck->mti_numreaders.  After
       * that, it is safe for mdbx_env_close() to touch it.
       * When it will be closed, we can finally claim it. */
      r->mr_pid = 0;
      r->mr_txnid = ~(txnid_t)0;
      r->mr_tid = tid;
      mdbx_flush_noncoherent_cpu_writeback();
      if (slot == nreaders)
        env->me_lck->mti_numreaders = ++nreaders;
      r->mr_pid = env->me_pid;
      mdbx_rdt_unlock(env);

      if (likely(env->me_flags & MDBX_ENV_TXKEY)) {
        mdbx_assert(env, env->me_live_reader == env->me_pid);
        mdbx_thread_rthc_set(env->me_txkey, r);
      }
    }

    while (1) {
      MDBX_meta *const meta = mdbx_meta_head(env);
      mdbx_jitter4testing(false);
      const txnid_t snap = mdbx_meta_txnid_fluid(env, meta);
      mdbx_jitter4testing(false);
      if (likely(r)) {
        r->mr_snapshot_pages_used = meta->mm_geo.next;
        r->mr_txnid = snap;
        mdbx_jitter4testing(false);
        mdbx_assert(env, r->mr_pid == mdbx_getpid());
        mdbx_assert(env, r->mr_tid == mdbx_thread_self());
        mdbx_assert(env, r->mr_txnid == snap);
        mdbx_compiler_barrier();
        env->me_lck->mti_readers_refresh_flag = true;
        mdbx_flush_noncoherent_cpu_writeback();
      }
      mdbx_jitter4testing(true);

      /* Snap the state from current meta-head */
      txn->mt_txnid = snap;
      txn->mt_next_pgno = meta->mm_geo.next;
      txn->mt_end_pgno = meta->mm_geo.now;
      upper_limit_pgno = meta->mm_geo.upper;
      memcpy(txn->mt_dbs, meta->mm_dbs, CORE_DBS * sizeof(MDBX_db));
      txn->mt_canary = meta->mm_canary;

      /* LY: Retry on a race, ITS#7970. */
      mdbx_compiler_barrier();
      if (likely(meta == mdbx_meta_head(env) &&
                 snap == mdbx_meta_txnid_fluid(env, meta) &&
                 snap >= *env->me_oldest)) {
        mdbx_jitter4testing(false);
        break;
      }
    }

    if (unlikely(txn->mt_txnid == 0)) {
      mdbx_error("environment corrupted by died writer, must shutdown!");
      rc = MDBX_WANNA_RECOVERY;
      goto bailout;
    }
    mdbx_assert(env, txn->mt_txnid >= *env->me_oldest);
    txn->mt_ro_reader = r;
    txn->mt_dbxs = env->me_dbxs; /* mostly static anyway */
    mdbx_ensure(env, txn->mt_txnid >=
                         /* paranoia is appropriate here */ *env->me_oldest);
  } else {
    /* Not yet touching txn == env->me_txn0, it may be active */
    mdbx_jitter4testing(false);
    rc = mdbx_txn_lock(env, F_ISSET(flags, MDBX_TRYTXN));
    if (unlikely(rc))
      return rc;
    if (unlikely(env->me_flags & MDBX_FATAL_ERROR)) {
      mdbx_txn_unlock(env);
      return MDBX_PANIC;
    }
#if defined(_WIN32) || defined(_WIN64)
    if (unlikely(!env->me_map)) {
      mdbx_txn_unlock(env);
      return MDBX_EPERM;
    }
#endif /* Windows */

    mdbx_jitter4testing(false);
    MDBX_meta *meta = mdbx_meta_head(env);
    mdbx_jitter4testing(false);
    txn->mt_canary = meta->mm_canary;
    const txnid_t snap = mdbx_meta_txnid_stable(env, meta);
    txn->mt_txnid = snap + 1;
    if (unlikely(txn->mt_txnid < snap)) {
      mdbx_debug("txnid overflow!");
      rc = MDBX_TXN_FULL;
      goto bailout;
    }

    txn->mt_flags = flags;
    txn->mt_child = NULL;
    txn->mt_loose_pages = NULL;
    txn->mt_loose_count = 0;
    txn->mt_dirtyroom = MDBX_DPL_TXNFULL;
    txn->mt_rw_dirtylist = env->me_dirtylist;
    txn->mt_rw_dirtylist->length = 0;
    txn->mt_befree_pages = env->me_free_pgs;
    MDBX_PNL_SIZE(txn->mt_befree_pages) = 0;
    txn->mt_spill_pages = NULL;
    if (txn->mt_lifo_reclaimed)
      MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) = 0;
    env->me_txn = txn;
    memcpy(txn->mt_dbiseqs, env->me_dbiseqs, env->me_maxdbs * sizeof(unsigned));
    /* Copy the DB info and flags */
    memcpy(txn->mt_dbs, meta->mm_dbs, CORE_DBS * sizeof(MDBX_db));
    /* Moved to here to avoid a data race in read TXNs */
    txn->mt_next_pgno = meta->mm_geo.next;
    txn->mt_end_pgno = meta->mm_geo.now;
    upper_limit_pgno = meta->mm_geo.upper;
  }

  /* Setup db info */
  txn->mt_numdbs = env->me_numdbs;
  mdbx_compiler_barrier();
  for (unsigned i = CORE_DBS; i < txn->mt_numdbs; i++) {
    unsigned x = env->me_dbflags[i];
    txn->mt_dbs[i].md_flags = x & PERSISTENT_FLAGS;
    txn->mt_dbflags[i] =
        (x & MDBX_VALID) ? DB_VALID | DB_USRVALID | DB_STALE : 0;
  }
  txn->mt_dbflags[MAIN_DBI] = DB_VALID | DB_USRVALID;
  txn->mt_dbflags[FREE_DBI] = DB_VALID;

  if (unlikely(env->me_flags & MDBX_FATAL_ERROR)) {
    mdbx_warning("environment had fatal error, must shutdown!");
    rc = MDBX_PANIC;
  } else {
    const size_t size = pgno2bytes(env, txn->mt_end_pgno);
    if (unlikely(size > env->me_mapsize)) {
      if (upper_limit_pgno > MAX_PAGENO ||
          bytes2pgno(env, pgno2bytes(env, upper_limit_pgno)) !=
              upper_limit_pgno) {
        rc = MDBX_MAP_RESIZED;
        goto bailout;
      }
      rc = mdbx_mapresize(env, txn->mt_end_pgno, upper_limit_pgno);
      if (rc != MDBX_SUCCESS) {
        if (rc == MDBX_RESULT_TRUE)
          rc = MDBX_MAP_RESIZED;
        goto bailout;
      }
    }
    txn->mt_owner = mdbx_thread_self();
#if defined(_WIN32) || defined(_WIN64)
    if ((txn->mt_flags & MDBX_TXN_RDONLY) != 0 && size > env->me_dbgeo.lower &&
        env->me_dbgeo.shrink) {
      txn->mt_flags |= MDBX_SHRINK_ALLOWED;
      mdbx_srwlock_AcquireShared(&env->me_remap_guard);
    }
#endif
    return MDBX_SUCCESS;
  }
bailout:
  mdbx_tassert(txn, rc != MDBX_SUCCESS);
  mdbx_txn_end(txn, MDBX_END_SLOT | MDBX_END_FAIL_BEGIN);
  return rc;
}

int mdbx_txn_renew(MDBX_txn *txn) {
  int rc;

  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely((txn->mt_flags & MDBX_TXN_RDONLY) == 0))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_owner != 0))
    return MDBX_THREAD_MISMATCH;

  rc = mdbx_txn_renew0(txn, MDBX_TXN_RDONLY);
  if (rc == MDBX_SUCCESS) {
    txn->mt_owner = mdbx_thread_self();
    mdbx_debug("renew txn %" PRIaTXN "%c %p on env %p, root page %" PRIaPGNO
               "/%" PRIaPGNO,
               txn->mt_txnid, (txn->mt_flags & MDBX_TXN_RDONLY) ? 'r' : 'w',
               (void *)txn, (void *)txn->mt_env, txn->mt_dbs[MAIN_DBI].md_root,
               txn->mt_dbs[FREE_DBI].md_root);
  }
  return rc;
}

int mdbx_txn_begin(MDBX_env *env, MDBX_txn *parent, unsigned flags,
                   MDBX_txn **ret) {
  MDBX_txn *txn;
  MDBX_ntxn *ntxn;
  int rc;
  unsigned size, tsize;

  if (unlikely(!env || !ret))
    return MDBX_EINVAL;

  *ret = NULL;
  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

#if MDBX_TXN_CHECKPID
  if (unlikely(env->me_pid != mdbx_getpid()))
    env->me_flags |= MDBX_FATAL_ERROR;
#endif /* MDBX_TXN_CHECKPID */

  if (unlikely(env->me_flags & MDBX_FATAL_ERROR))
    return MDBX_PANIC;

#if !defined(_WIN32) && !defined(_WIN64)
  /* Don't check env->me_map until lock to avoid race with re-mapping for
   * shrinking */
  if (unlikely(!env->me_map))
    return MDBX_EPERM;
#endif /* Windows */

  flags &= MDBX_TXN_BEGIN_FLAGS;
  flags |= env->me_flags & MDBX_WRITEMAP;

  if (unlikely(env->me_flags & MDBX_RDONLY &
               ~flags)) /* write txn in RDONLY env */
    return MDBX_EACCESS;

  if (parent) {
    if (unlikely(parent->mt_signature != MDBX_MT_SIGNATURE))
      return MDBX_EBADSIGN;

    if (unlikely(parent->mt_owner != mdbx_thread_self()))
      return MDBX_THREAD_MISMATCH;

#if defined(_WIN32) || defined(_WIN64)
    if (unlikely(!env->me_map))
      return MDBX_EPERM;
#endif /* Windows */

    /* Nested transactions: Max 1 child, write txns only, no writemap */
    flags |= parent->mt_flags;
    if (unlikely(flags & (MDBX_RDONLY | MDBX_WRITEMAP | MDBX_TXN_BLOCKED)))
      return (parent->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EINVAL : MDBX_BAD_TXN;

    /* Child txns save MDBX_pgstate and use own copy of cursors */
    size = env->me_maxdbs * (sizeof(MDBX_db) + sizeof(MDBX_cursor *) + 1);
    size += tsize = sizeof(MDBX_ntxn);
  } else if (flags & MDBX_RDONLY) {
    if (env->me_txn0 && unlikely(env->me_txn0->mt_owner == mdbx_thread_self()))
      return MDBX_BUSY;
    size = env->me_maxdbs * (sizeof(MDBX_db) + 1);
    size += tsize = sizeof(MDBX_txn);
  } else {
    /* Reuse preallocated write txn. However, do not touch it until
     * mdbx_txn_renew0() succeeds, since it currently may be active. */
    txn = env->me_txn0;
    if (unlikely(txn->mt_owner == mdbx_thread_self()))
      return MDBX_BUSY;
    goto renew;
  }
  if (unlikely((txn = mdbx_malloc(size)) == NULL)) {
    mdbx_debug("calloc: %s", "failed");
    return MDBX_ENOMEM;
  }
  memset(txn, 0, tsize);
  txn->mt_dbxs = env->me_dbxs; /* static */
  txn->mt_dbs = (MDBX_db *)((char *)txn + tsize);
  txn->mt_dbflags = (uint8_t *)txn + size - env->me_maxdbs;
  txn->mt_flags = flags;
  txn->mt_env = env;

  if (parent) {
    unsigned i;
    txn->mt_cursors = (MDBX_cursor **)(txn->mt_dbs + env->me_maxdbs);
    txn->mt_dbiseqs = parent->mt_dbiseqs;
    txn->mt_rw_dirtylist =
        mdbx_malloc(sizeof(MDBX_DP) * (MDBX_DPL_TXNFULL + 1));
    if (!txn->mt_rw_dirtylist ||
        !(txn->mt_befree_pages = mdbx_pnl_alloc(MDBX_PNL_INITIAL))) {
      mdbx_free(txn->mt_rw_dirtylist);
      mdbx_free(txn);
      return MDBX_ENOMEM;
    }
    txn->mt_txnid = parent->mt_txnid;
    txn->mt_dirtyroom = parent->mt_dirtyroom;
    txn->mt_rw_dirtylist->length = 0;
    txn->mt_spill_pages = NULL;
    txn->mt_next_pgno = parent->mt_next_pgno;
    txn->mt_end_pgno = parent->mt_end_pgno;
    parent->mt_flags |= MDBX_TXN_HAS_CHILD;
    parent->mt_child = txn;
    txn->mt_parent = parent;
    txn->mt_numdbs = parent->mt_numdbs;
    memcpy(txn->mt_dbs, parent->mt_dbs, txn->mt_numdbs * sizeof(MDBX_db));
    /* Copy parent's mt_dbflags, but clear DB_NEW */
    for (i = 0; i < txn->mt_numdbs; i++)
      txn->mt_dbflags[i] = parent->mt_dbflags[i] & ~(DB_FRESH | DB_CREAT);
    rc = 0;
    ntxn = (MDBX_ntxn *)txn;
    ntxn->mnt_pgstate =
        env->me_pgstate; /* save parent me_reclaimed_pglist & co */
    if (env->me_reclaimed_pglist) {
      size = MDBX_PNL_SIZEOF(env->me_reclaimed_pglist);
      env->me_reclaimed_pglist =
          mdbx_pnl_alloc(MDBX_PNL_SIZE(env->me_reclaimed_pglist));
      if (likely(env->me_reclaimed_pglist))
        memcpy(env->me_reclaimed_pglist, ntxn->mnt_pgstate.mf_reclaimed_pglist,
               size);
      else
        rc = MDBX_ENOMEM;
    }
    if (likely(!rc))
      rc = mdbx_cursor_shadow(parent, txn);
    if (unlikely(rc))
      mdbx_txn_end(txn, MDBX_END_FAIL_BEGINCHILD);
  } else { /* MDBX_RDONLY */
    txn->mt_dbiseqs = env->me_dbiseqs;
  renew:
    rc = mdbx_txn_renew0(txn, flags);
  }

  if (unlikely(rc != MDBX_SUCCESS)) {
    if (txn != env->me_txn0)
      mdbx_free(txn);
  } else {
    mdbx_assert(env, (txn->mt_flags & ~(MDBX_TXN_RDONLY | MDBX_TXN_WRITEMAP |
                                        MDBX_SHRINK_ALLOWED)) == 0);
    txn->mt_signature = MDBX_MT_SIGNATURE;
    *ret = txn;
    mdbx_debug("begin txn %" PRIaTXN "%c %p on env %p, root page %" PRIaPGNO
               "/%" PRIaPGNO,
               txn->mt_txnid, (flags & MDBX_RDONLY) ? 'r' : 'w', (void *)txn,
               (void *)env, txn->mt_dbs[MAIN_DBI].md_root,
               txn->mt_dbs[FREE_DBI].md_root);
  }

  return rc;
}

MDBX_env *mdbx_txn_env(MDBX_txn *txn) {
  if (unlikely(!txn || txn->mt_signature != MDBX_MT_SIGNATURE))
    return NULL;
  return txn->mt_env;
}

uint64_t mdbx_txn_id(MDBX_txn *txn) {
  if (unlikely(!txn || txn->mt_signature != MDBX_MT_SIGNATURE))
    return ~(txnid_t)0;
  return txn->mt_txnid;
}

int mdbx_txn_flags(MDBX_txn *txn) {
  if (unlikely(!txn || txn->mt_signature != MDBX_MT_SIGNATURE))
    return -1;

  return txn->mt_flags;
}

/* Export or close DBI handles opened in this txn. */
static void mdbx_dbis_update(MDBX_txn *txn, int keep) {
  MDBX_dbi n = txn->mt_numdbs;
  if (n) {
    bool locked = false;
    MDBX_env *env = txn->mt_env;
    uint8_t *tdbflags = txn->mt_dbflags;

    for (unsigned i = n; --i >= CORE_DBS;) {
      if (likely((tdbflags[i] & DB_CREAT) == 0))
        continue;
      if (!locked) {
        mdbx_ensure(env,
                    mdbx_fastmutex_acquire(&env->me_dbi_lock) == MDBX_SUCCESS);
        locked = true;
      }
      if (keep) {
        env->me_dbflags[i] = txn->mt_dbs[i].md_flags | MDBX_VALID;
        mdbx_compiler_barrier();
        if (env->me_numdbs <= i)
          env->me_numdbs = i + 1;
      } else {
        char *ptr = env->me_dbxs[i].md_name.iov_base;
        if (ptr) {
          env->me_dbxs[i].md_name.iov_len = 0;
          mdbx_compiler_barrier();
          mdbx_assert(env, env->me_dbflags[i] == 0);
          env->me_dbiseqs[i]++;
          env->me_dbxs[i].md_name.iov_base = NULL;
          mdbx_free(ptr);
        }
      }
    }

    if (unlikely(locked))
      mdbx_ensure(env,
                  mdbx_fastmutex_release(&env->me_dbi_lock) == MDBX_SUCCESS);
  }
}

/* End a transaction, except successful commit of a nested transaction.
 * May be called twice for readonly txns: First reset it, then abort.
 * [in] txn   the transaction handle to end
 * [in] mode  why and how to end the transaction */
static int mdbx_txn_end(MDBX_txn *txn, unsigned mode) {
  MDBX_env *env = txn->mt_env;
  static const char *const names[] = MDBX_END_NAMES;

#if MDBX_TXN_CHECKPID
  if (unlikely(txn->mt_env->me_pid != mdbx_getpid())) {
    env->me_flags |= MDBX_FATAL_ERROR;
    return MDBX_PANIC;
  }
#endif /* MDBX_TXN_CHECKPID */

  mdbx_debug("%s txn %" PRIaTXN "%c %p on mdbenv %p, root page %" PRIaPGNO
             "/%" PRIaPGNO,
             names[mode & MDBX_END_OPMASK], txn->mt_txnid,
             (txn->mt_flags & MDBX_TXN_RDONLY) ? 'r' : 'w', (void *)txn,
             (void *)env, txn->mt_dbs[MAIN_DBI].md_root,
             txn->mt_dbs[FREE_DBI].md_root);

  mdbx_ensure(env, txn->mt_txnid >=
                       /* paranoia is appropriate here */ *env->me_oldest);
  if (F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)) {
#if defined(_WIN32) || defined(_WIN64)
    if (txn->mt_flags & MDBX_SHRINK_ALLOWED)
      mdbx_srwlock_ReleaseShared(&env->me_remap_guard);
#endif
    if (txn->mt_ro_reader) {
      mdbx_ensure(env, /* paranoia is appropriate here */
                  txn->mt_txnid == txn->mt_ro_reader->mr_txnid &&
                      txn->mt_ro_reader->mr_txnid >=
                          env->me_lck->mti_oldest_reader);
      txn->mt_ro_reader->mr_snapshot_pages_used = 0;
      txn->mt_ro_reader->mr_txnid = ~(txnid_t)0;
      mdbx_memory_barrier();
      env->me_lck->mti_readers_refresh_flag = true;
      if (mode & MDBX_END_SLOT) {
        if ((env->me_flags & MDBX_ENV_TXKEY) == 0)
          txn->mt_ro_reader->mr_pid = 0;
        txn->mt_ro_reader = NULL;
      }
    }
    mdbx_flush_noncoherent_cpu_writeback();
    txn->mt_numdbs = 0; /* prevent further DBI activity */
    txn->mt_flags = MDBX_TXN_RDONLY | MDBX_TXN_FINISHED;
    txn->mt_owner = 0;
  } else if (!F_ISSET(txn->mt_flags, MDBX_TXN_FINISHED)) {
    /* Export or close DBI handles created in this txn */
    mdbx_dbis_update(txn, mode & MDBX_END_UPDATE);

    pgno_t *pghead = env->me_reclaimed_pglist;

    if (!(mode & MDBX_END_EOTDONE)) /* !(already closed cursors) */
      mdbx_cursors_eot(txn, 0);
    if (!(env->me_flags & MDBX_WRITEMAP)) {
      mdbx_dlist_free(txn);
    }

    if (txn->mt_lifo_reclaimed) {
      MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) = 0;
      if (txn != env->me_txn0) {
        mdbx_txl_free(txn->mt_lifo_reclaimed);
        txn->mt_lifo_reclaimed = NULL;
      }
    }
    txn->mt_numdbs = 0;
    txn->mt_flags = MDBX_TXN_FINISHED;
    txn->mt_owner = 0;

    if (!txn->mt_parent) {
      mdbx_pnl_shrink(&txn->mt_befree_pages);
      env->me_free_pgs = txn->mt_befree_pages;
      /* me_pgstate: */
      env->me_reclaimed_pglist = NULL;
      env->me_last_reclaimed = 0;

      env->me_txn = NULL;

      /* The writer mutex was locked in mdbx_txn_begin. */
      mdbx_txn_unlock(env);
    } else {
      txn->mt_parent->mt_child = NULL;
      txn->mt_parent->mt_flags &= ~MDBX_TXN_HAS_CHILD;
      env->me_pgstate = ((MDBX_ntxn *)txn)->mnt_pgstate;
      mdbx_pnl_free(txn->mt_befree_pages);
      mdbx_pnl_free(txn->mt_spill_pages);
      mdbx_free(txn->mt_rw_dirtylist);
    }

    mdbx_pnl_free(pghead);
  }

  mdbx_assert(env, txn == env->me_txn0 || txn->mt_owner == 0);
  if ((mode & MDBX_END_FREE) != 0 && txn != env->me_txn0) {
    txn->mt_signature = 0;
    mdbx_free(txn);
  }

  return MDBX_SUCCESS;
}

int mdbx_txn_reset(MDBX_txn *txn) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  /* This call is only valid for read-only txns */
  if (unlikely(!(txn->mt_flags & MDBX_TXN_RDONLY)))
    return MDBX_EINVAL;

  /* LY: don't close DBI-handles in MDBX mode */
  int rc = mdbx_txn_end(txn, MDBX_END_RESET | MDBX_END_UPDATE);
  if (rc == MDBX_SUCCESS) {
    mdbx_tassert(txn, txn->mt_signature == MDBX_MT_SIGNATURE);
    mdbx_tassert(txn, txn->mt_owner == 0);
  }
  return rc;
}

int mdbx_txn_abort(MDBX_txn *txn) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY))
    /* LY: don't close DBI-handles in MDBX mode */
    return mdbx_txn_end(txn, MDBX_END_ABORT | MDBX_END_UPDATE | MDBX_END_SLOT |
                                 MDBX_END_FREE);

  if (txn->mt_child)
    mdbx_txn_abort(txn->mt_child);

  return mdbx_txn_end(txn, MDBX_END_ABORT | MDBX_END_SLOT | MDBX_END_FREE);
}

static __inline int mdbx_backlog_size(MDBX_txn *txn) {
  int reclaimed_and_loose =
      txn->mt_env->me_reclaimed_pglist
          ? MDBX_PNL_SIZE(txn->mt_env->me_reclaimed_pglist) +
                txn->mt_loose_count
          : 0;
  return reclaimed_and_loose;
}

static __inline int mdbx_backlog_extragap(MDBX_env *env) {
  /* LY: extra page(s) for b-tree rebalancing */
  return (env->me_flags & MDBX_LIFORECLAIM) ? 2 : 1;
}

/* LY: Prepare a backlog of pages to modify FreeDB itself,
 * while reclaiming is prohibited. It should be enough to prevent search
 * in mdbx_page_alloc() during a deleting, when freeDB tree is unbalanced. */
static int mdbx_prep_backlog(MDBX_txn *txn, MDBX_cursor *mc) {
  /* LY: extra page(s) for b-tree rebalancing */
  const int extra =
      mdbx_backlog_extragap(txn->mt_env) +
      MDBX_PNL_SIZEOF(txn->mt_befree_pages) / txn->mt_env->me_maxkey_limit;

  if (mdbx_backlog_size(txn) < mc->mc_db->md_depth + extra) {
    mc->mc_flags &= ~C_RECLAIMING;
    int rc = mdbx_cursor_touch(mc);
    if (unlikely(rc))
      return rc;

    while (unlikely(mdbx_backlog_size(txn) < extra)) {
      rc = mdbx_page_alloc(mc, 1, NULL, MDBX_ALLOC_GC);
      if (unlikely(rc)) {
        if (rc != MDBX_NOTFOUND)
          return rc;
        break;
      }
    }
    mc->mc_flags |= C_RECLAIMING;
  }

  return MDBX_SUCCESS;
}

static void mdbx_prep_backlog_data(MDBX_txn *txn, MDBX_cursor *mc,
                                   size_t bytes) {
  const int wanna =
      (int)OVPAGES(txn->mt_env, bytes) + mdbx_backlog_extragap(txn->mt_env);
  if (unlikely(wanna > mdbx_backlog_size(txn))) {
    mc->mc_flags &= ~C_RECLAIMING;
    do {
      if (mdbx_page_alloc(mc, 1, NULL, MDBX_ALLOC_GC) != MDBX_SUCCESS)
        break;
    } while (wanna > mdbx_backlog_size(txn));
    mc->mc_flags |= C_RECLAIMING;
  }
}

/* Count all the pages in each DB and in the freelist and make sure
 * it matches the actual number of pages being used.
 * All named DBs must be open for a correct count. */
static __cold int mdbx_audit(MDBX_txn *txn, unsigned befree_stored) {
  MDBX_val key, data;

  const pgno_t pending =
      (txn->mt_flags & MDBX_RDONLY)
          ? 0
          : txn->mt_loose_count +
                (txn->mt_env->me_reclaimed_pglist
                     ? MDBX_PNL_SIZE(txn->mt_env->me_reclaimed_pglist)
                     : 0) +
                (txn->mt_befree_pages
                     ? MDBX_PNL_SIZE(txn->mt_befree_pages) - befree_stored
                     : 0);

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, FREE_DBI);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  pgno_t freecount = 0;
  while ((rc = mdbx_cursor_get(&cx.outer, &key, &data, MDBX_NEXT)) == 0)
    freecount += *(pgno_t *)data.iov_base;
  mdbx_tassert(txn, rc == MDBX_NOTFOUND);

  pgno_t count = 0;
  for (MDBX_dbi i = FREE_DBI; i <= MAIN_DBI; i++) {
    if (!(txn->mt_dbflags[i] & DB_VALID))
      continue;
    rc = mdbx_cursor_init(&cx.outer, txn, i);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    if (txn->mt_dbs[i].md_root == P_INVALID)
      continue;
    count += txn->mt_dbs[i].md_branch_pages + txn->mt_dbs[i].md_leaf_pages +
             txn->mt_dbs[i].md_overflow_pages;

    rc = mdbx_page_search(&cx.outer, NULL, MDBX_PS_FIRST);
    while (rc == MDBX_SUCCESS) {
      MDBX_page *mp = cx.outer.mc_pg[cx.outer.mc_top];
      for (unsigned j = 0; j < NUMKEYS(mp); j++) {
        MDBX_node *leaf = NODEPTR(mp, j);
        if ((leaf->mn_flags & (F_DUPDATA | F_SUBDATA)) == F_SUBDATA) {
          MDBX_db db_copy, *db;
          memcpy(db = &db_copy, NODEDATA(leaf), sizeof(db_copy));
          if ((txn->mt_flags & MDBX_RDONLY) == 0) {
            for (MDBX_dbi k = txn->mt_numdbs; --k > MAIN_DBI;) {
              if ((txn->mt_dbflags[k] & MDBX_TBL_DIRTY) &&
                  /* txn->mt_dbxs[k].md_name.iov_len > 0 && */
                  NODEKSZ(leaf) == txn->mt_dbxs[k].md_name.iov_len &&
                  memcmp(NODEKEY(leaf), txn->mt_dbxs[k].md_name.iov_base,
                         NODEKSZ(leaf)) == 0) {
                db = txn->mt_dbs + k;
                break;
              }
            }
          }
          count +=
              db->md_branch_pages + db->md_leaf_pages + db->md_overflow_pages;
        }
      }
      rc = mdbx_cursor_sibling(&cx.outer, 1);
    }
    mdbx_tassert(txn, rc == MDBX_NOTFOUND);
  }

  if (pending + freecount + count + NUM_METAS == txn->mt_next_pgno)
    return MDBX_SUCCESS;

  if ((txn->mt_flags & MDBX_RDONLY) == 0)
    mdbx_error("audit @%" PRIaTXN ": %u(pending) = %u(loose-count) + "
               "%u(reclaimed-list) + %u(befree-pending) - %u(befree-stored)",
               txn->mt_txnid, pending, txn->mt_loose_count,
               txn->mt_env->me_reclaimed_pglist
                   ? MDBX_PNL_SIZE(txn->mt_env->me_reclaimed_pglist)
                   : 0,
               txn->mt_befree_pages ? MDBX_PNL_SIZE(txn->mt_befree_pages) : 0,
               befree_stored);
  mdbx_error("audit @%" PRIaTXN ": %" PRIaPGNO "(pending) + %" PRIaPGNO
             "(free) + %" PRIaPGNO "(count) = %" PRIaPGNO
             "(total) <> %" PRIaPGNO "(next-pgno)",
             txn->mt_txnid, pending, freecount, count + NUM_METAS,
             pending + freecount + count + NUM_METAS, txn->mt_next_pgno);
  return MDBX_PROBLEM;
}

/* Cleanup reclaimed GC records, than save the befree-list as of this
 * transaction to GC (aka freeDB). This recursive changes the reclaimed-list
 * loose-list and befree-list. Keep trying until it stabilizes. */
static int mdbx_update_gc(MDBX_txn *txn) {
  /* env->me_reclaimed_pglist[] can grow and shrink during this call.
   * env->me_last_reclaimed and txn->mt_befree_pages[] can only grow.
   * Page numbers cannot disappear from txn->mt_befree_pages[]. */
  MDBX_env *const env = txn->mt_env;
  const bool lifo = (env->me_flags & MDBX_LIFORECLAIM) != 0;
  const char *dbg_prefix_mode = lifo ? "    lifo" : "    fifo";
  (void)dbg_prefix_mode;
  mdbx_trace("\n>>> @%" PRIaTXN, txn->mt_txnid);

  unsigned befree_stored = 0, loop = 0;
  MDBX_cursor mc;
  int rc = mdbx_cursor_init(&mc, txn, FREE_DBI);
  if (unlikely(rc != MDBX_SUCCESS))
    goto bailout_notracking;

  mc.mc_flags |= C_RECLAIMING;
  mc.mc_next = txn->mt_cursors[FREE_DBI];
  txn->mt_cursors[FREE_DBI] = &mc;

retry:
  mdbx_trace(" >> restart");
  mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  if (unlikely(/* paranoia */ ++loop > 42)) {
    mdbx_error("too more loops %u, bailout", loop);
    rc = MDBX_PROBLEM;
    goto bailout;
  }

  unsigned settled = 0, cleaned_gc_slot = 0, reused_gc_slot = 0,
           filled_gc_slot = ~0u;
  txnid_t cleaned_gc_id = 0, head_gc_id = env->me_last_reclaimed;
  while (1) {
    /* Come back here after each Put() in case befree-list changed */
    MDBX_val key, data;
    mdbx_trace(" >> continue");

    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    if (txn->mt_lifo_reclaimed) {
      if (cleaned_gc_slot < MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)) {
        settled = 0;
        cleaned_gc_slot = 0;
        reused_gc_slot = 0;
        filled_gc_slot = ~0u;
        /* LY: cleanup reclaimed records. */
        do {
          cleaned_gc_id = txn->mt_lifo_reclaimed[++cleaned_gc_slot];
          mdbx_tassert(txn,
                       cleaned_gc_slot > 0 && cleaned_gc_id < *env->me_oldest);
          key.iov_base = &cleaned_gc_id;
          key.iov_len = sizeof(cleaned_gc_id);
          rc = mdbx_cursor_get(&mc, &key, NULL, MDBX_SET);
          if (rc == MDBX_NOTFOUND)
            continue;
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
          rc = mdbx_prep_backlog(txn, &mc);
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
          mdbx_tassert(txn, cleaned_gc_id < *env->me_oldest);
          mdbx_trace("%s.cleanup-reclaimed-id [%u]%" PRIaTXN, dbg_prefix_mode,
                     cleaned_gc_slot, cleaned_gc_id);
          rc = mdbx_cursor_del(&mc, 0);
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
        } while (cleaned_gc_slot < MDBX_PNL_SIZE(txn->mt_lifo_reclaimed));
        mdbx_txl_sort(txn->mt_lifo_reclaimed);
        head_gc_id = MDBX_PNL_LAST(txn->mt_lifo_reclaimed);
      }
    } else {
      /* If using records from freeDB which we have not yet deleted,
       * now delete them and any we reserved for me_reclaimed_pglist. */
      while (cleaned_gc_id < env->me_last_reclaimed) {
        rc = mdbx_cursor_first(&mc, &key, NULL);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
        rc = mdbx_prep_backlog(txn, &mc);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
        cleaned_gc_id = head_gc_id = *(txnid_t *)key.iov_base;
        mdbx_tassert(txn, cleaned_gc_id <= env->me_last_reclaimed);
        mdbx_tassert(txn, cleaned_gc_id < *env->me_oldest);
        mdbx_trace("%s.cleanup-reclaimed-id %" PRIaTXN, dbg_prefix_mode,
                   cleaned_gc_id);
        rc = mdbx_cursor_del(&mc, 0);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
        settled = 0;
      }
    }

    // handle loose pages - put ones into the reclaimed- or befree-list
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    if (mdbx_audit_enabled()) {
      rc = mdbx_audit(txn, befree_stored);
      if (unlikely(rc != MDBX_SUCCESS))
        goto bailout;
    }
    if (txn->mt_loose_pages) {
      /* Return loose page numbers to me_reclaimed_pglist,
       * though usually none are left at this point.
       * The pages themselves remain in dirtylist. */
      if (unlikely(!env->me_reclaimed_pglist) && !txn->mt_lifo_reclaimed &&
          env->me_last_reclaimed < 1) {
        /* Put loose page numbers in mt_befree_pages,
         * since unable to return them to me_reclaimed_pglist. */
        if (unlikely((rc = mdbx_pnl_need(&txn->mt_befree_pages,
                                         txn->mt_loose_count)) != 0))
          goto bailout;
        for (MDBX_page *mp = txn->mt_loose_pages; mp; mp = NEXT_LOOSE_PAGE(mp))
          mdbx_pnl_xappend(txn->mt_befree_pages, mp->mp_pgno);
        mdbx_trace("%s: append %u loose-pages to befree-pages", dbg_prefix_mode,
                   txn->mt_loose_count);
      } else {
        /* Room for loose pages + temp PNL with same */
        if (likely(env->me_reclaimed_pglist != NULL)) {
          rc = mdbx_pnl_need(&env->me_reclaimed_pglist,
                             2 * txn->mt_loose_count + 2);
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
          MDBX_PNL loose = env->me_reclaimed_pglist +
                           MDBX_PNL_ALLOCLEN(env->me_reclaimed_pglist) -
                           txn->mt_loose_count - 1;
          unsigned count = 0;
          for (MDBX_page *mp = txn->mt_loose_pages; mp;
               mp = NEXT_LOOSE_PAGE(mp))
            loose[++count] = mp->mp_pgno;
          MDBX_PNL_SIZE(loose) = count;
          mdbx_pnl_sort(loose);
          mdbx_pnl_xmerge(env->me_reclaimed_pglist, loose);
        } else {
          env->me_reclaimed_pglist = mdbx_pnl_alloc(txn->mt_loose_count);
          if (unlikely(env->me_reclaimed_pglist == NULL)) {
            rc = MDBX_ENOMEM;
            goto bailout;
          }
          for (MDBX_page *mp = txn->mt_loose_pages; mp;
               mp = NEXT_LOOSE_PAGE(mp))
            mdbx_pnl_xappend(env->me_reclaimed_pglist, mp->mp_pgno);
          mdbx_pnl_sort(env->me_reclaimed_pglist);
        }
        mdbx_trace("%s: append %u loose-pages to reclaimed-pages",
                   dbg_prefix_mode, txn->mt_loose_count);
      }

      // filter-out list of dirty-pages from loose-pages
      MDBX_DPL dl = txn->mt_rw_dirtylist;
      mdbx_dpl_sort(dl);
      unsigned left = dl->length;
      for (MDBX_page *mp = txn->mt_loose_pages; mp;) {
        mdbx_tassert(txn, mp->mp_pgno < txn->mt_next_pgno);
        mdbx_ensure(env, mp->mp_pgno >= NUM_METAS);

        if (left > 0) {
          const unsigned i = mdbx_dpl_search(dl, mp->mp_pgno);
          if (i <= dl->length && dl[i].pgno == mp->mp_pgno) {
            mdbx_tassert(txn, i > 0 && dl[i].ptr != dl);
            dl[i].ptr = dl /* mark for deletion */;
          }
          left -= 1;
        }

        MDBX_page *dp = mp;
        mp = NEXT_LOOSE_PAGE(mp);
        if ((env->me_flags & MDBX_WRITEMAP) == 0)
          mdbx_dpage_free(env, dp, 1);
      }

      if (left > 0) {
        MDBX_DPL r, w, end = dl + dl->length;
        for (r = w = dl + 1; r <= end; r++) {
          if (r->ptr != dl) {
            if (r != w)
              *w = *r;
            ++w;
          }
        }
        mdbx_tassert(txn, w - dl == (int)left + 1);
      }

      if (left != dl->length)
        mdbx_trace("%s: filtered-out loose-pages from %u -> %u dirty-pages",
                   dbg_prefix_mode, dl->length, left);
      dl->length = left;

      txn->mt_loose_pages = NULL;
      txn->mt_loose_count = 0;
      if (mdbx_audit_enabled()) {
        rc = mdbx_audit(txn, befree_stored);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
      }
    }

    // handle reclaimed pages - return suitable into unallocated space
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    if (env->me_reclaimed_pglist) {
      pgno_t tail = txn->mt_next_pgno;
      pgno_t *const begin = MDBX_PNL_BEGIN(env->me_reclaimed_pglist);
      pgno_t *const end = MDBX_PNL_END(env->me_reclaimed_pglist);
      pgno_t *higest;
#if MDBX_PNL_ASCENDING
      for (higest = end; --higest >= begin;) {
#else
      for (higest = begin; higest < end; ++higest) {
#endif /* MDBX_PNL sort-order */
        mdbx_tassert(txn, *higest >= NUM_METAS && *higest < tail);
        if (*higest != tail - 1)
          break;
        tail -= 1;
      }
      if (tail != txn->mt_next_pgno) {
#if MDBX_PNL_ASCENDING
        MDBX_PNL_SIZE(env->me_reclaimed_pglist) =
            (unsigned)(higest + 1 - begin);
#else
        MDBX_PNL_SIZE(env->me_reclaimed_pglist) -= (unsigned)(higest - begin);
        for (pgno_t *move = begin; higest < end; ++move, ++higest)
          *move = *higest;
#endif /* MDBX_PNL sort-order */
        mdbx_info(
            "%s.refunded %" PRIaPGNO " pages: %" PRIaPGNO " -> %" PRIaPGNO,
            dbg_prefix_mode, txn->mt_next_pgno - tail, tail, txn->mt_next_pgno);
        txn->mt_next_pgno = tail;
        mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
        if (mdbx_audit_enabled()) {
          rc = mdbx_audit(txn, befree_stored);
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
        }
      }
    }

    // handle befree-list - store ones into singe gc-record
    if (befree_stored < MDBX_PNL_SIZE(txn->mt_befree_pages)) {
      if (unlikely(!befree_stored)) {
        /* Make sure last page of freeDB is touched and on befree-list */
        mc.mc_flags &= ~C_RECLAIMING;
        rc = mdbx_page_search(&mc, NULL, MDBX_PS_LAST | MDBX_PS_MODIFY);
        mc.mc_flags |= C_RECLAIMING;
        if (unlikely(rc != MDBX_SUCCESS) && rc != MDBX_NOTFOUND)
          goto bailout;
      }
      /* Write to last page of freeDB */
      key.iov_len = sizeof(txn->mt_txnid);
      key.iov_base = &txn->mt_txnid;
      do {
        data.iov_len = MDBX_PNL_SIZEOF(txn->mt_befree_pages);
        mdbx_prep_backlog_data(txn, &mc, data.iov_len);
        rc = mdbx_cursor_put(&mc, &key, &data, MDBX_RESERVE);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
        /* Retry if mt_befree_pages[] grew during the Put() */
      } while (data.iov_len < MDBX_PNL_SIZEOF(txn->mt_befree_pages));

      befree_stored = (unsigned)MDBX_PNL_SIZE(txn->mt_befree_pages);
      mdbx_pnl_sort(txn->mt_befree_pages);
      memcpy(data.iov_base, txn->mt_befree_pages, data.iov_len);

      mdbx_trace("%s.put-befree #%u @ %" PRIaTXN, dbg_prefix_mode,
                 befree_stored, txn->mt_txnid);

      if (mdbx_debug_enabled(MDBX_DBG_EXTRA)) {
        unsigned i = befree_stored;
        mdbx_debug_extra("PNL write txn %" PRIaTXN " root %" PRIaPGNO
                         " num %u, PNL",
                         txn->mt_txnid, txn->mt_dbs[FREE_DBI].md_root, i);
        for (; i; i--)
          mdbx_debug_extra_print(" %" PRIaPGNO, txn->mt_befree_pages[i]);
        mdbx_debug_extra_print("\n");
      }
      continue;
    }

    // handle reclaimed and loost pages - merge and store both into gc
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    mdbx_tassert(txn, txn->mt_loose_count == 0);

    mdbx_trace(" >> reserving");
    if (mdbx_audit_enabled()) {
      rc = mdbx_audit(txn, befree_stored);
      if (unlikely(rc != MDBX_SUCCESS))
        goto bailout;
    }
    const unsigned amount =
        env->me_reclaimed_pglist ? MDBX_PNL_SIZE(env->me_reclaimed_pglist) : 0;
    const unsigned left = amount - settled;
    mdbx_trace("%s: amount %u, settled %d, left %d, lifo-reclaimed-slots %u, "
               "reused-gc-slots %u",
               dbg_prefix_mode, amount, settled, (int)left,
               txn->mt_lifo_reclaimed
                   ? (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)
                   : 0,
               reused_gc_slot);
    if (0 >= (int)left)
      break;

    if (unlikely(head_gc_id == 0)) {
      head_gc_id = mdbx_find_oldest(txn) - 1;
      if (txn->mt_lifo_reclaimed == NULL) {
        rc = mdbx_cursor_get(&mc, &key, NULL, MDBX_FIRST);
        if (unlikely(rc != MDBX_SUCCESS)) {
          if (rc != MDBX_NOTFOUND)
            goto bailout;
        } else if (unlikely(key.iov_len != sizeof(txnid_t))) {
          rc = MDBX_CORRUPTED;
          goto bailout;
        } else {
          txnid_t first_txn;
          memcpy(&first_txn, key.iov_base, sizeof(txnid_t));
          if (head_gc_id >= first_txn)
            head_gc_id = first_txn - 1;
        }
      }
    }

    const unsigned prefer_max_scatter = 257;
    txnid_t reservation_gc_id;
    if (lifo) {
      mdbx_tassert(txn, txn->mt_lifo_reclaimed != NULL);
      if (unlikely(!txn->mt_lifo_reclaimed)) {
        txn->mt_lifo_reclaimed = mdbx_txl_alloc();
        if (unlikely(!txn->mt_lifo_reclaimed)) {
          rc = MDBX_ENOMEM;
          goto bailout;
        }
      }

      if (head_gc_id > 1 &&
          MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) < prefer_max_scatter &&
          left > ((unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) -
                  reused_gc_slot) *
                     env->me_maxgc_ov1page) {
        /* LY: need just a txn-id for save page list. */
        mc.mc_flags &= ~C_RECLAIMING;
        rc = mdbx_page_alloc(&mc, 0, NULL, MDBX_ALLOC_GC | MDBX_ALLOC_KICK);
        mc.mc_flags |= C_RECLAIMING;
        if (likely(rc == MDBX_SUCCESS)) {
          /* LY: ok, reclaimed from freedb. */
          mdbx_trace("%s: took @%" PRIaTXN " from GC, continue",
                     dbg_prefix_mode, MDBX_PNL_LAST(txn->mt_lifo_reclaimed));
          continue;
        }
        if (unlikely(rc != MDBX_NOTFOUND))
          /* LY: other troubles... */
          goto bailout;

        /* LY: freedb is empty, will look any free txn-id in high2low order. */
        do {
          --head_gc_id;
          mdbx_assert(env,
                      MDBX_PNL_IS_EMPTY(txn->mt_lifo_reclaimed) ||
                          MDBX_PNL_LAST(txn->mt_lifo_reclaimed) > head_gc_id);
          rc = mdbx_txl_append(&txn->mt_lifo_reclaimed, head_gc_id);
          if (unlikely(rc != MDBX_SUCCESS))
            goto bailout;
          cleaned_gc_slot += 1 /* mark GC cleanup is not needed. */;

          mdbx_trace("%s: append @%" PRIaTXN
                     " to lifo-reclaimed, cleaned-gc-slot = %u",
                     dbg_prefix_mode, head_gc_id, cleaned_gc_slot);
        } while (head_gc_id > 1 &&
                 MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) < prefer_max_scatter &&
                 left > ((unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) -
                         reused_gc_slot) *
                            env->me_maxgc_ov1page);
      }

      if ((unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) <= reused_gc_slot) {
        mdbx_notice("** restart: reserve depleted (reused_gc_slot %u >= "
                    "lifo_reclaimed %u" PRIaTXN,
                    reused_gc_slot,
                    (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed));
        goto retry;
      }
      const unsigned i =
          (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) - reused_gc_slot;
      mdbx_tassert(txn, i > 0 && i <= MDBX_PNL_SIZE(txn->mt_lifo_reclaimed));
      reservation_gc_id = txn->mt_lifo_reclaimed[i];
      mdbx_trace("%s: take @%" PRIaTXN " from lifo-reclaimed[%u]",
                 dbg_prefix_mode, reservation_gc_id, i);
    } else {
      mdbx_tassert(txn, txn->mt_lifo_reclaimed == NULL);
      reservation_gc_id = head_gc_id--;
      mdbx_trace("%s: take @%" PRIaTXN " from head-gc-id", dbg_prefix_mode,
                 reservation_gc_id);
    }
    ++reused_gc_slot;

    unsigned chunk = left;
    if (unlikely(chunk > env->me_maxgc_ov1page)) {
      const unsigned avail_gs_slots =
          txn->mt_lifo_reclaimed
              ? (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) -
                    reused_gc_slot + 1
              : (head_gc_id < INT16_MAX) ? (unsigned)head_gc_id : INT16_MAX;
      if (avail_gs_slots > 1) {
        if (chunk < env->me_maxgc_ov1page * 2)
          chunk /= 2;
        else {
          const unsigned threshold =
              env->me_maxgc_ov1page * ((avail_gs_slots < prefer_max_scatter)
                                           ? avail_gs_slots
                                           : prefer_max_scatter);
          if (left < threshold)
            chunk = env->me_maxgc_ov1page;
          else {
            const unsigned tail = left - threshold + env->me_maxgc_ov1page + 1;
            unsigned span = 1;
            unsigned avail = (unsigned)((pgno2bytes(env, span) - PAGEHDRSZ) /
                                        sizeof(pgno_t)) /*- 1 + span */;
            if (tail > avail) {
              for (unsigned i = amount - span; i > 0; --i) {
                if (MDBX_PNL_ASCENDING
                        ? (env->me_reclaimed_pglist[i] + span)
                        : (env->me_reclaimed_pglist[i] - span) ==
                              env->me_reclaimed_pglist[i + span]) {
                  span += 1;
                  avail = (unsigned)((pgno2bytes(env, span) - PAGEHDRSZ) /
                                     sizeof(pgno_t)) -
                          1 + span;
                  if (avail >= tail)
                    break;
                }
              }
            }

            chunk = (avail >= tail) ? tail - span
                                    : (avail_gs_slots > 3 &&
                                       reused_gc_slot < prefer_max_scatter - 3)
                                          ? avail - span
                                          : tail;
          }
        }
      }
    }
    mdbx_tassert(txn, chunk > 0);

    mdbx_trace("%s: head_gc_id %" PRIaTXN ", reused_gc_slot %u, reservation-id "
               "%" PRIaTXN,
               dbg_prefix_mode, head_gc_id, reused_gc_slot, reservation_gc_id);

    mdbx_trace("%s: chunk %u, gc-per-ovpage %u", dbg_prefix_mode, chunk,
               env->me_maxgc_ov1page);

    mdbx_tassert(txn, reservation_gc_id < *env->me_oldest);
    if (unlikely(reservation_gc_id < 1 ||
                 reservation_gc_id >= *env->me_oldest)) {
      /* LY: not any txn in the past of freedb. */
      rc = MDBX_PROBLEM;
      goto bailout;
    }

    key.iov_len = sizeof(reservation_gc_id);
    key.iov_base = &reservation_gc_id;
    data.iov_len = (chunk + 1) * sizeof(pgno_t);
    mdbx_trace("%s.reserve: %u [%u...%u] @%" PRIaTXN, dbg_prefix_mode, chunk,
               settled + 1, settled + chunk + 1, reservation_gc_id);
    mdbx_prep_backlog_data(txn, &mc, data.iov_len);
    rc = mdbx_cursor_put(&mc, &key, &data, MDBX_RESERVE | MDBX_NOOVERWRITE);
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    if (unlikely(rc != MDBX_SUCCESS))
      goto bailout;

    /* PNL is initially empty, zero out at least the length */
    memset(data.iov_base, 0, sizeof(pgno_t));
    settled += chunk;
    mdbx_trace("%s.settled %u (+%u), continue", dbg_prefix_mode, settled,
               chunk);

    if (txn->mt_lifo_reclaimed &&
        unlikely(amount < MDBX_PNL_SIZE(env->me_reclaimed_pglist))) {
      mdbx_notice("** restart: reclaimed-list growth %u -> %u", amount,
                  (unsigned)MDBX_PNL_SIZE(env->me_reclaimed_pglist));
      goto retry;
    }

    continue;
  }

  mdbx_tassert(
      txn,
      cleaned_gc_slot ==
          (txn->mt_lifo_reclaimed ? MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) : 0));

  mdbx_trace(" >> filling");
  /* Fill in the reserved records */
  filled_gc_slot =
      txn->mt_lifo_reclaimed
          ? (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) - reused_gc_slot
          : reused_gc_slot;
  rc = MDBX_SUCCESS;
  mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  if (env->me_reclaimed_pglist && MDBX_PNL_SIZE(env->me_reclaimed_pglist)) {
    MDBX_val key, data;
    key.iov_len = data.iov_len = 0; /* avoid MSVC warning */
    key.iov_base = data.iov_base = NULL;

    const unsigned amount = MDBX_PNL_SIZE(env->me_reclaimed_pglist);
    unsigned left = amount;
    if (txn->mt_lifo_reclaimed == nullptr) {
      mdbx_tassert(txn, lifo == 0);
      rc = mdbx_cursor_first(&mc, &key, &data);
      if (unlikely(rc != MDBX_SUCCESS))
        goto bailout;
    } else {
      mdbx_tassert(txn, lifo != 0);
    }

    while (1) {
      txnid_t fill_gc_id;
      mdbx_trace("%s: left %u of %u", dbg_prefix_mode, left,
                 (unsigned)MDBX_PNL_SIZE(env->me_reclaimed_pglist));
      if (txn->mt_lifo_reclaimed == nullptr) {
        mdbx_tassert(txn, lifo == 0);
        fill_gc_id = *(txnid_t *)key.iov_base;
        if (filled_gc_slot-- == 0 || fill_gc_id > env->me_last_reclaimed) {
          mdbx_notice(
              "** restart: reserve depleted (filled_slot %u, fill_id %" PRIaTXN
              " > last_reclaimed %" PRIaTXN,
              filled_gc_slot, fill_gc_id, env->me_last_reclaimed);
          goto retry;
        }
      } else {
        mdbx_tassert(txn, lifo != 0);
        if (++filled_gc_slot >
            (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)) {
          mdbx_notice("** restart: reserve depleted (filled_gc_slot %u > "
                      "lifo_reclaimed %u" PRIaTXN,
                      filled_gc_slot,
                      (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed));
          goto retry;
        }
        fill_gc_id = txn->mt_lifo_reclaimed[filled_gc_slot];
        mdbx_trace("%s.seek-reservation @%" PRIaTXN " at lifo_reclaimed[%u]",
                   dbg_prefix_mode, fill_gc_id, filled_gc_slot);
        key.iov_base = &fill_gc_id;
        key.iov_len = sizeof(fill_gc_id);
        rc = mdbx_cursor_get(&mc, &key, &data, MDBX_SET_KEY);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
      }
      mdbx_tassert(txn, cleaned_gc_slot ==
                            (txn->mt_lifo_reclaimed
                                 ? MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)
                                 : 0));
      mdbx_tassert(txn, fill_gc_id > 0 && fill_gc_id < *env->me_oldest);
      key.iov_base = &fill_gc_id;
      key.iov_len = sizeof(fill_gc_id);

      mdbx_tassert(txn, data.iov_len >= sizeof(pgno_t) * 2);
      mc.mc_flags |= C_GCFREEZE;
      unsigned chunk = (unsigned)(data.iov_len / sizeof(pgno_t)) - 1;
      if (unlikely(chunk > left)) {
        mdbx_trace("%s: chunk %u > left %u, @%" PRIaTXN, dbg_prefix_mode, chunk,
                   left, fill_gc_id);
        if ((loop < 5 && chunk - left > loop / 2) ||
            chunk - left > env->me_maxgc_ov1page) {
          data.iov_len = (left + 1) * sizeof(pgno_t);
          if (loop < 7)
            mc.mc_flags &= ~C_GCFREEZE;
        }
        chunk = left;
      }
      rc = mdbx_cursor_put(&mc, &key, &data, MDBX_CURRENT | MDBX_RESERVE);
      mc.mc_flags &= ~C_GCFREEZE;
      if (unlikely(rc != MDBX_SUCCESS))
        goto bailout;

      if (unlikely(txn->mt_loose_count ||
                   amount != MDBX_PNL_SIZE(env->me_reclaimed_pglist))) {
        memset(data.iov_base, 0, sizeof(pgno_t));
        mdbx_notice("** restart: reclaimed-list changed (%u -> %u, %u)", amount,
                    MDBX_PNL_SIZE(env->me_reclaimed_pglist),
                    txn->mt_loose_count);
        goto retry;
      }
      if (unlikely(txn->mt_lifo_reclaimed
                       ? cleaned_gc_slot < MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)
                       : cleaned_gc_id < env->me_last_reclaimed)) {
        memset(data.iov_base, 0, sizeof(pgno_t));
        mdbx_notice("** restart: reclaimed-slots changed");
        goto retry;
      }

      pgno_t *dst = data.iov_base;
      *dst++ = chunk;
      pgno_t *src = MDBX_PNL_BEGIN(env->me_reclaimed_pglist) + left - chunk;
      memcpy(dst, src, chunk * sizeof(pgno_t));
      pgno_t *from = src, *to = src + chunk;
      mdbx_trace("%s.fill: %u [ %u:%" PRIaPGNO "...%u:%" PRIaPGNO
                 "] @%" PRIaTXN,
                 dbg_prefix_mode, chunk,
                 (unsigned)(from - env->me_reclaimed_pglist), from[0],
                 (unsigned)(to - env->me_reclaimed_pglist), to[-1], fill_gc_id);

      left -= chunk;
      if (mdbx_audit_enabled()) {
        rc = mdbx_audit(txn, befree_stored + amount - left);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
      }
      if (left == 0) {
        rc = MDBX_SUCCESS;
        break;
      }

      if (txn->mt_lifo_reclaimed == nullptr) {
        mdbx_tassert(txn, lifo == 0);
        rc = mdbx_cursor_next(&mc, &key, &data, MDBX_NEXT);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
      } else {
        mdbx_tassert(txn, lifo != 0);
      }
    }
  }

  mdbx_tassert(txn, rc == MDBX_SUCCESS);
  if (unlikely(txn->mt_loose_count != 0 ||
               filled_gc_slot !=
                   (txn->mt_lifo_reclaimed
                        ? (unsigned)MDBX_PNL_SIZE(txn->mt_lifo_reclaimed)
                        : 0))) {
    mdbx_notice("** restart: reserve excess (filled-slot %u, loose-count %u)",
                filled_gc_slot, txn->mt_loose_count);
    goto retry;
  }

  mdbx_tassert(txn,
               txn->mt_lifo_reclaimed == NULL ||
                   cleaned_gc_slot == MDBX_PNL_SIZE(txn->mt_lifo_reclaimed));

bailout:
  txn->mt_cursors[FREE_DBI] = mc.mc_next;

bailout_notracking:
  if (txn->mt_lifo_reclaimed) {
    MDBX_PNL_SIZE(txn->mt_lifo_reclaimed) = 0;
    if (txn != env->me_txn0) {
      mdbx_txl_free(txn->mt_lifo_reclaimed);
      txn->mt_lifo_reclaimed = NULL;
    }
  }

  mdbx_trace("<<< %u loops, rc = %d", loop, rc);
  return rc;
}

/* Flush (some) dirty pages to the map, after clearing their dirty flag.
 * [in] txn   the transaction that's being committed
 * [in] keep  number of initial pages in dirtylist to keep dirty.
 * Returns 0 on success, non-zero on failure. */
static int mdbx_page_flush(MDBX_txn *txn, pgno_t keep) {
  MDBX_env *env = txn->mt_env;
  MDBX_DPL dl = txn->mt_rw_dirtylist;
  unsigned i, j, pagecount = dl->length;
  int rc;
  size_t size = 0, pos = 0;
  pgno_t pgno = 0;
  MDBX_page *dp = NULL;
  struct iovec iov[MDBX_COMMIT_PAGES];
  intptr_t wpos = 0, wsize = 0;
  size_t next_pos = 1; /* impossible pos, so pos != next_pos */
  int n = 0;

  j = i = keep;

  if (env->me_flags & MDBX_WRITEMAP) {
    /* Clear dirty flags */
    while (++i <= pagecount) {
      dp = dl[i].ptr;
      /* Don't flush this page yet */
      if (dp->mp_flags & (P_LOOSE | P_KEEP)) {
        dp->mp_flags &= ~P_KEEP;
        dl[++j] = dl[i];
        continue;
      }
      dp->mp_flags &= ~P_DIRTY;
      dp->mp_validator = 0 /* TODO */;
      *env->me_unsynced_pages += IS_OVERFLOW(dp) ? dp->mp_pages : 1;
    }
    goto done;
  }

  /* Write the pages */
  for (;;) {
    if (++i <= pagecount) {
      dp = dl[i].ptr;
      /* Don't flush this page yet */
      if (dp->mp_flags & (P_LOOSE | P_KEEP)) {
        dp->mp_flags &= ~P_KEEP;
        dl[i].pgno = 0;
        continue;
      }
      pgno = dl[i].pgno;
      mdbx_tassert(txn, pgno >= MIN_PAGENO);
      /* clear dirty flag */
      dp->mp_flags &= ~P_DIRTY;
      dp->mp_validator = 0 /* TODO */;
      pos = pgno2bytes(env, pgno);
      const unsigned npages = IS_OVERFLOW(dp) ? dp->mp_pages : 1;
      *env->me_unsynced_pages += npages;
      size = pgno2bytes(env, npages);
    }
    /* Write up to MDBX_COMMIT_PAGES dirty pages at a time. */
    if (pos != next_pos || n == MDBX_COMMIT_PAGES || wsize + size > MAX_WRITE) {
      if (n) {
        /* Write previous page(s) */
        rc = mdbx_pwritev(env->me_fd, iov, n, wpos, wsize);
        if (unlikely(rc != MDBX_SUCCESS)) {
          mdbx_debug("Write error: %s", mdbx_strerror(rc));
          return rc;
        }

#if MDBX_CPU_CACHE_MMAP_NONCOHERENT
#if defined(__linux__) || defined(__gnu_linux__)
        if (linux_kernel_version >= 0x02060b00)
        /* Linux kernels older than version 2.6.11 ignore the addr and nbytes
         * arguments, making this function fairly expensive. Therefore, the
         * whole cache is always flushed. */
#endif /* Linux */
          mdbx_invalidate_mmap_noncoherent_cache(env->me_map + wpos, wsize);
#endif /* MDBX_CPU_CACHE_MMAP_NONCOHERENT */

        n = 0;
      }
      if (i > pagecount)
        break;
      wpos = pos;
      wsize = 0;
    }
    mdbx_debug("committing page %" PRIaPGNO, pgno);
    next_pos = pos + size;
    iov[n].iov_len = size;
    iov[n].iov_base = (char *)dp;
    wsize += size;
    n++;
  }

#if MDBX_CPU_CACHE_MMAP_NONCOHERENT &&                                         \
    (defined(__linux__) || defined(__gnu_linux__))
  if (linux_kernel_version < 0x02060b00) {
    /* Linux kernels older than version 2.6.11 ignore the addr and nbytes
     * arguments, making this function fairly expensive. Therefore, the whole
     * cache is always flushed. */
    mdbx_invalidate_mmap_noncoherent_cache(env->me_map,
                                           pgno2bytes(env, txn->mt_next_pgno));
  }
#endif /* MDBX_CPU_CACHE_MMAP_NONCOHERENT && Linux */

  for (i = keep; ++i <= pagecount;) {
    dp = dl[i].ptr;
    /* This is a page we skipped above */
    if (!dl[i].pgno) {
      dl[++j] = dl[i];
      dl[j].pgno = dp->mp_pgno;
      continue;
    }
    mdbx_dpage_free(env, dp, IS_OVERFLOW(dp) ? dp->mp_pages : 1);
  }

done:
  i--;
  txn->mt_dirtyroom += i - j;
  dl->length = j;
  return MDBX_SUCCESS;
}

/* Check for misused dbi handles */
#define TXN_DBI_CHANGED(txn, dbi)                                              \
  ((txn)->mt_dbiseqs[dbi] != (txn)->mt_env->me_dbiseqs[dbi])

/* Import DBI which opened after txn started into context */
static __cold bool mdbx_txn_import_dbi(MDBX_txn *txn, MDBX_dbi dbi) {
  MDBX_env *env = txn->mt_env;
  if (dbi < CORE_DBS || dbi >= env->me_numdbs)
    return false;

  mdbx_ensure(env, mdbx_fastmutex_acquire(&env->me_dbi_lock) == MDBX_SUCCESS);
  const unsigned snap_numdbs = env->me_numdbs;
  mdbx_compiler_barrier();
  for (unsigned i = CORE_DBS; i < snap_numdbs; ++i) {
    if (i >= txn->mt_numdbs)
      txn->mt_dbflags[i] = 0;
    if (!(txn->mt_dbflags[i] & DB_USRVALID) &&
        (env->me_dbflags[i] & MDBX_VALID)) {
      txn->mt_dbs[i].md_flags = env->me_dbflags[i] & PERSISTENT_FLAGS;
      txn->mt_dbflags[i] = DB_VALID | DB_USRVALID | DB_STALE;
      mdbx_tassert(txn, txn->mt_dbxs[i].md_cmp != NULL);
    }
  }
  txn->mt_numdbs = snap_numdbs;

  mdbx_ensure(env, mdbx_fastmutex_release(&env->me_dbi_lock) == MDBX_SUCCESS);
  return txn->mt_dbflags[dbi] & DB_USRVALID;
}

/* Check txn and dbi arguments to a function */
static __inline bool TXN_DBI_EXIST(MDBX_txn *txn, MDBX_dbi dbi,
                                   unsigned validity) {
  if (likely(dbi < txn->mt_numdbs && (txn->mt_dbflags[dbi] & validity)))
    return true;

  return mdbx_txn_import_dbi(txn, dbi);
}

int mdbx_txn_commit(MDBX_txn *txn) {
  int rc;

  if (unlikely(txn == NULL))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  MDBX_env *env = txn->mt_env;
#if MDBX_TXN_CHECKPID
  if (unlikely(env->me_pid != mdbx_getpid())) {
    env->me_flags |= MDBX_FATAL_ERROR;
    return MDBX_PANIC;
  }
#endif /* MDBX_TXN_CHECKPID */

  if (txn->mt_child) {
    rc = mdbx_txn_commit(txn->mt_child);
    txn->mt_child = NULL;
    if (unlikely(rc != MDBX_SUCCESS))
      goto fail;
  }

  /* mdbx_txn_end() mode for a commit which writes nothing */
  unsigned end_mode =
      MDBX_END_EMPTY_COMMIT | MDBX_END_UPDATE | MDBX_END_SLOT | MDBX_END_FREE;
  if (unlikely(F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)))
    goto done;

  if (unlikely(txn->mt_flags & (MDBX_TXN_FINISHED | MDBX_TXN_ERROR))) {
    mdbx_debug("error flag is set, can't commit");
    if (txn->mt_parent)
      txn->mt_parent->mt_flags |= MDBX_TXN_ERROR;
    rc = MDBX_BAD_TXN;
    goto fail;
  }

  if (txn->mt_parent) {
    MDBX_txn *parent = txn->mt_parent;
    MDBX_page **lp;
    MDBX_DPL dst, src;
    MDBX_PNL pspill;
    unsigned i, x, y, len, ps_len;

    /* Append our reclaim list to parent's */
    if (txn->mt_lifo_reclaimed) {
      if (parent->mt_lifo_reclaimed) {
        rc = mdbx_txl_append_list(&parent->mt_lifo_reclaimed,
                                  txn->mt_lifo_reclaimed);
        if (unlikely(rc != MDBX_SUCCESS))
          goto fail;
        mdbx_txl_free(txn->mt_lifo_reclaimed);
      } else
        parent->mt_lifo_reclaimed = txn->mt_lifo_reclaimed;
      txn->mt_lifo_reclaimed = NULL;
    }

    /* Append our free list to parent's */
    rc = mdbx_pnl_append_list(&parent->mt_befree_pages, txn->mt_befree_pages);
    if (unlikely(rc != MDBX_SUCCESS))
      goto fail;
    mdbx_pnl_free(txn->mt_befree_pages);
    /* Failures after this must either undo the changes
     * to the parent or set MDBX_TXN_ERROR in the parent. */

    parent->mt_next_pgno = txn->mt_next_pgno;
    parent->mt_end_pgno = txn->mt_end_pgno;
    parent->mt_flags = txn->mt_flags;

    /* Merge our cursors into parent's and close them */
    mdbx_cursors_eot(txn, 1);

    /* Update parent's DB table. */
    memcpy(parent->mt_dbs, txn->mt_dbs, txn->mt_numdbs * sizeof(MDBX_db));
    parent->mt_numdbs = txn->mt_numdbs;
    parent->mt_dbflags[FREE_DBI] = txn->mt_dbflags[FREE_DBI];
    parent->mt_dbflags[MAIN_DBI] = txn->mt_dbflags[MAIN_DBI];
    for (i = CORE_DBS; i < txn->mt_numdbs; i++) {
      /* preserve parent's DB_NEW status */
      parent->mt_dbflags[i] =
          txn->mt_dbflags[i] | (parent->mt_dbflags[i] & (DB_CREAT | DB_FRESH));
    }

    dst = parent->mt_rw_dirtylist;
    src = txn->mt_rw_dirtylist;
    /* Remove anything in our dirty list from parent's spill list */
    if ((pspill = parent->mt_spill_pages) && (ps_len = MDBX_PNL_SIZE(pspill))) {
      x = y = ps_len;
      MDBX_PNL_SIZE(pspill) = ~(pgno_t)0;
      /* Mark our dirty pages as deleted in parent spill list */
      for (i = 0, len = src->length; ++i <= len;) {
        pgno_t pn = src[i].pgno << 1;
        while (pn > pspill[x])
          x--;
        if (pn == pspill[x]) {
          pspill[x] = 1;
          y = --x;
        }
      }
      /* Squash deleted pagenums if we deleted any */
      for (x = y; ++x <= ps_len;)
        if ((pspill[x] & 1) == 0)
          pspill[++y] = pspill[x];
      MDBX_PNL_SIZE(pspill) = y;
    }

    /* Remove anything in our spill list from parent's dirty list */
    if (txn->mt_spill_pages && MDBX_PNL_SIZE(txn->mt_spill_pages)) {
      for (i = 1; i <= MDBX_PNL_SIZE(txn->mt_spill_pages); i++) {
        pgno_t pn = txn->mt_spill_pages[i];
        if (pn & 1)
          continue; /* deleted spillpg */
        pn >>= 1;
        y = mdbx_dpl_search(dst, pn);
        if (y <= dst->length && dst[y].pgno == pn) {
          mdbx_free(dst[y].ptr);
          while (y < dst->length) {
            dst[y] = dst[y + 1];
            y++;
          }
          dst->length--;
        }
      }
    }

    /* Find len = length of merging our dirty list with parent's */
    x = dst->length;
    dst->length = 0; /* simplify loops */
    if (parent->mt_parent) {
      len = x + src->length;
      y = mdbx_dpl_search(src, dst[x].pgno + 1) - 1;
      for (i = x; y && i; y--) {
        pgno_t yp = src[y].pgno;
        while (yp < dst[i].pgno)
          i--;
        if (yp == dst[i].pgno) {
          i--;
          len--;
        }
      }
    } else { /* Simplify the above for single-ancestor case */
      len = MDBX_DPL_TXNFULL - txn->mt_dirtyroom;
    }
    /* Merge our dirty list with parent's */
    y = src->length;
    for (i = len; y; dst[i--] = src[y--]) {
      pgno_t yp = src[y].pgno;
      while (yp < dst[x].pgno)
        dst[i--] = dst[x--];
      if (yp == dst[x].pgno)
        mdbx_free(dst[x--].ptr);
    }
    mdbx_tassert(txn, i == x);
    dst->length = len;
    mdbx_free(txn->mt_rw_dirtylist);
    parent->mt_dirtyroom = txn->mt_dirtyroom;
    if (txn->mt_spill_pages) {
      if (parent->mt_spill_pages) {
        /* TODO: Prevent failure here, so parent does not fail */
        rc = mdbx_pnl_append_list(&parent->mt_spill_pages, txn->mt_spill_pages);
        if (unlikely(rc != MDBX_SUCCESS))
          parent->mt_flags |= MDBX_TXN_ERROR;
        mdbx_pnl_free(txn->mt_spill_pages);
        mdbx_pnl_sort(parent->mt_spill_pages);
      } else {
        parent->mt_spill_pages = txn->mt_spill_pages;
      }
    }

    /* Append our loose page list to parent's */
    for (lp = &parent->mt_loose_pages; *lp; lp = &NEXT_LOOSE_PAGE(*lp))
      ;
    *lp = txn->mt_loose_pages;
    parent->mt_loose_count += txn->mt_loose_count;

    parent->mt_child = NULL;
    mdbx_pnl_free(((MDBX_ntxn *)txn)->mnt_pgstate.mf_reclaimed_pglist);
    txn->mt_signature = 0;
    mdbx_free(txn);
    return rc;
  }

  if (unlikely(txn != env->me_txn)) {
    mdbx_debug("attempt to commit unknown transaction");
    rc = MDBX_EINVAL;
    goto fail;
  }

  mdbx_cursors_eot(txn, 0);
  end_mode |= MDBX_END_EOTDONE;

  if (txn->mt_rw_dirtylist->length == 0 &&
      !(txn->mt_flags & (MDBX_TXN_DIRTY | MDBX_TXN_SPILLS)))
    goto done;

  mdbx_debug("committing txn %" PRIaTXN " %p on mdbenv %p, root page %" PRIaPGNO
             "/%" PRIaPGNO,
             txn->mt_txnid, (void *)txn, (void *)env,
             txn->mt_dbs[MAIN_DBI].md_root, txn->mt_dbs[FREE_DBI].md_root);

  /* Update DB root pointers */
  if (txn->mt_numdbs > CORE_DBS) {
    MDBX_cursor mc;
    MDBX_dbi i;
    MDBX_val data;
    data.iov_len = sizeof(MDBX_db);

    rc = mdbx_cursor_init(&mc, txn, MAIN_DBI);
    if (unlikely(rc != MDBX_SUCCESS))
      goto fail;
    for (i = CORE_DBS; i < txn->mt_numdbs; i++) {
      if (txn->mt_dbflags[i] & DB_DIRTY) {
        if (unlikely(TXN_DBI_CHANGED(txn, i))) {
          rc = MDBX_BAD_DBI;
          goto fail;
        }
        data.iov_base = &txn->mt_dbs[i];
        WITH_CURSOR_TRACKING(mc,
                             rc = mdbx_cursor_put(&mc, &txn->mt_dbxs[i].md_name,
                                                  &data, F_SUBDATA));
        if (unlikely(rc != MDBX_SUCCESS))
          goto fail;
      }
    }
  }

  rc = mdbx_update_gc(txn);
  if (unlikely(rc != MDBX_SUCCESS))
    goto fail;

  mdbx_pnl_free(env->me_reclaimed_pglist);
  env->me_reclaimed_pglist = NULL;
  mdbx_pnl_shrink(&txn->mt_befree_pages);

  if (mdbx_audit_enabled()) {
    rc = mdbx_audit(txn, 0);
    if (unlikely(rc != MDBX_SUCCESS))
      goto fail;
  }

  rc = mdbx_page_flush(txn, 0);
  if (likely(rc == MDBX_SUCCESS)) {
    MDBX_meta meta, *head = mdbx_meta_head(env);

    meta.mm_magic_and_version = head->mm_magic_and_version;
    meta.mm_extra_flags = head->mm_extra_flags;
    meta.mm_validator_id = head->mm_validator_id;
    meta.mm_extra_pagehdr = head->mm_extra_pagehdr;

    meta.mm_geo = head->mm_geo;
    meta.mm_geo.next = txn->mt_next_pgno;
    meta.mm_geo.now = txn->mt_end_pgno;
    meta.mm_dbs[FREE_DBI] = txn->mt_dbs[FREE_DBI];
    meta.mm_dbs[MAIN_DBI] = txn->mt_dbs[MAIN_DBI];
    meta.mm_canary = txn->mt_canary;
    mdbx_meta_set_txnid(env, &meta, txn->mt_txnid);

    rc = mdbx_sync_locked(
        env, env->me_flags | txn->mt_flags | MDBX_SHRINK_ALLOWED, &meta);
  }
  if (unlikely(rc != MDBX_SUCCESS)) {
    env->me_flags |= MDBX_FATAL_ERROR;
    goto fail;
  }

  if (likely(env->me_lck))
    env->me_lck->mti_readers_refresh_flag = false;
  end_mode = MDBX_END_COMMITTED | MDBX_END_UPDATE | MDBX_END_EOTDONE;

done:
  return mdbx_txn_end(txn, end_mode);

fail:
  mdbx_txn_abort(txn);
  return rc;
}

/* Read the environment parameters of a DB environment
 * before mapping it into memory. */
static int __cold mdbx_read_header(MDBX_env *env, MDBX_meta *meta,
                                   uint64_t *filesize) {
  STATIC_ASSERT(offsetof(MDBX_page, mp_meta) == PAGEHDRSZ);

  int rc = mdbx_filesize(env->me_fd, filesize);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  memset(meta, 0, sizeof(MDBX_meta));
  meta->mm_datasync_sign = MDBX_DATASIGN_WEAK;
  rc = MDBX_CORRUPTED;

  /* Read twice all meta pages so we can find the latest one. */
  unsigned loop_limit = NUM_METAS * 2;
  for (unsigned loop_count = 0; loop_count < loop_limit; ++loop_count) {
    MDBX_page page;

    /* We don't know the page size on first time.
     * So, just guess it. */
    unsigned guess_pagesize = meta->mm_psize;
    if (guess_pagesize == 0)
      guess_pagesize =
          (loop_count > NUM_METAS) ? env->me_psize : env->me_os_psize;

    const unsigned meta_number = loop_count % NUM_METAS;
    const unsigned offset = guess_pagesize * meta_number;

    unsigned retryleft = 42;
    while (1) {
      mdbx_trace("reading meta[%d]: offset %u, bytes %u, retry-left %u",
                 meta_number, offset, (unsigned)sizeof(page), retryleft);
      int err = mdbx_pread(env->me_fd, &page, sizeof(page), offset);
      if (err != MDBX_SUCCESS) {
        mdbx_error("read meta[%u,%u]: %i, %s", offset, (unsigned)sizeof(page),
                   err, mdbx_strerror(err));
        return err;
      }

      MDBX_page again;
      err = mdbx_pread(env->me_fd, &again, sizeof(again), offset);
      if (err != MDBX_SUCCESS) {
        mdbx_error("read meta[%u,%u]: %i, %s", offset, (unsigned)sizeof(again),
                   err, mdbx_strerror(err));
        return err;
      }

      if (memcmp(&page, &again, sizeof(page)) == 0 || --retryleft == 0)
        break;

      mdbx_info("meta[%u] was updated, re-read it", meta_number);
    }

    if (page.mp_meta.mm_magic_and_version != MDBX_DATA_MAGIC &&
        page.mp_meta.mm_magic_and_version != MDBX_DATA_MAGIC_DEVEL) {
      mdbx_error("meta[%u] has invalid magic/version %" PRIx64, meta_number,
                 page.mp_meta.mm_magic_and_version);
      return ((page.mp_meta.mm_magic_and_version >> 8) != MDBX_MAGIC)
                 ? MDBX_INVALID
                 : MDBX_VERSION_MISMATCH;
    }

    if (page.mp_pgno != meta_number) {
      mdbx_error("meta[%u] has invalid pageno %" PRIaPGNO, meta_number,
                 page.mp_pgno);
      return MDBX_INVALID;
    }

    if (page.mp_flags != P_META) {
      mdbx_error("page #%u not a meta-page", meta_number);
      return MDBX_INVALID;
    }

    if (!retryleft) {
      mdbx_error("meta[%u] is too volatile, skip it", meta_number);
      continue;
    }

    /* LY: check pagesize */
    if (!mdbx_is_power2(page.mp_meta.mm_psize) ||
        page.mp_meta.mm_psize < MIN_PAGESIZE ||
        page.mp_meta.mm_psize > MAX_PAGESIZE) {
      mdbx_notice("meta[%u] has invalid pagesize (%u), skip it", meta_number,
                  page.mp_meta.mm_psize);
      rc = mdbx_is_power2(page.mp_meta.mm_psize) ? MDBX_VERSION_MISMATCH
                                                 : MDBX_INVALID;
      continue;
    }

    if (meta_number == 0 && guess_pagesize != page.mp_meta.mm_psize) {
      meta->mm_psize = page.mp_meta.mm_psize;
      mdbx_info("meta[%u] took pagesize %u", meta_number,
                page.mp_meta.mm_psize);
    }

    if (page.mp_meta.mm_txnid_a != page.mp_meta.mm_txnid_b) {
      mdbx_warning("meta[%u] not completely updated, skip it", meta_number);
      continue;
    }

    /* LY: check signature as a checksum */
    if (META_IS_STEADY(&page.mp_meta) &&
        page.mp_meta.mm_datasync_sign != mdbx_meta_sign(&page.mp_meta)) {
      mdbx_notice("meta[%u] has invalid steady-checksum (0x%" PRIx64
                  " != 0x%" PRIx64 "), skip it",
                  meta_number, page.mp_meta.mm_datasync_sign,
                  mdbx_meta_sign(&page.mp_meta));
      continue;
    }

    mdbx_debug("read meta%" PRIaPGNO " = root %" PRIaPGNO "/%" PRIaPGNO
               ", geo %" PRIaPGNO "/%" PRIaPGNO "-%" PRIaPGNO "/%" PRIaPGNO
               " +%u -%u, txn_id %" PRIaTXN ", %s",
               page.mp_pgno, page.mp_meta.mm_dbs[MAIN_DBI].md_root,
               page.mp_meta.mm_dbs[FREE_DBI].md_root, page.mp_meta.mm_geo.lower,
               page.mp_meta.mm_geo.next, page.mp_meta.mm_geo.now,
               page.mp_meta.mm_geo.upper, page.mp_meta.mm_geo.grow,
               page.mp_meta.mm_geo.shrink, page.mp_meta.mm_txnid_a,
               mdbx_durable_str(&page.mp_meta));

    /* LY: check min-pages value */
    if (page.mp_meta.mm_geo.lower < MIN_PAGENO ||
        page.mp_meta.mm_geo.lower > MAX_PAGENO) {
      mdbx_notice("meta[%u] has invalid min-pages (%" PRIaPGNO "), skip it",
                  meta_number, page.mp_meta.mm_geo.lower);
      rc = MDBX_INVALID;
      continue;
    }

    /* LY: check max-pages value */
    if (page.mp_meta.mm_geo.upper < MIN_PAGENO ||
        page.mp_meta.mm_geo.upper > MAX_PAGENO ||
        page.mp_meta.mm_geo.upper < page.mp_meta.mm_geo.lower) {
      mdbx_notice("meta[%u] has invalid max-pages (%" PRIaPGNO "), skip it",
                  meta_number, page.mp_meta.mm_geo.upper);
      rc = MDBX_INVALID;
      continue;
    }

    /* LY: check last_pgno */
    if (page.mp_meta.mm_geo.next < MIN_PAGENO ||
        page.mp_meta.mm_geo.next - 1 > MAX_PAGENO) {
      mdbx_notice("meta[%u] has invalid next-pageno (%" PRIaPGNO "), skip it",
                  meta_number, page.mp_meta.mm_geo.next);
      rc = MDBX_CORRUPTED;
      continue;
    }

    /* LY: check filesize & used_bytes */
    const uint64_t used_bytes =
        page.mp_meta.mm_geo.next * (uint64_t)page.mp_meta.mm_psize;
    if (used_bytes > *filesize) {
      /* Here could be a race with DB-shrinking performed by other process */
      rc = mdbx_filesize(env->me_fd, filesize);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      if (used_bytes > *filesize) {
        mdbx_notice("meta[%u] used-bytes (%" PRIu64
                    ") beyond filesize (%" PRIu64 "), skip it",
                    meta_number, used_bytes, *filesize);
        rc = MDBX_CORRUPTED;
        continue;
      }
    }

    /* LY: check mapsize limits */
    const uint64_t mapsize_min =
        page.mp_meta.mm_geo.lower * (uint64_t)page.mp_meta.mm_psize;
    STATIC_ASSERT(MAX_MAPSIZE < PTRDIFF_MAX - MAX_PAGESIZE);
    STATIC_ASSERT(MIN_MAPSIZE < MAX_MAPSIZE);
    if (mapsize_min < MIN_MAPSIZE || mapsize_min > MAX_MAPSIZE) {
      mdbx_notice("meta[%u] has invalid min-mapsize (%" PRIu64 "), skip it",
                  meta_number, mapsize_min);
      rc = MDBX_VERSION_MISMATCH;
      continue;
    }

    const uint64_t mapsize_max =
        page.mp_meta.mm_geo.upper * (uint64_t)page.mp_meta.mm_psize;
    STATIC_ASSERT(MIN_MAPSIZE < MAX_MAPSIZE);
    if (mapsize_max > MAX_MAPSIZE ||
        MAX_PAGENO < mdbx_roundup2((size_t)mapsize_max, env->me_os_psize) /
                         (size_t)page.mp_meta.mm_psize) {
      if (page.mp_meta.mm_geo.next - 1 > MAX_PAGENO ||
          used_bytes > MAX_MAPSIZE) {
        mdbx_notice("meta[%u] has too large max-mapsize (%" PRIu64 "), skip it",
                    meta_number, mapsize_max);
        rc = MDBX_TOO_LARGE;
        continue;
      }

      /* allow to open large DB from a 32-bit environment */
      mdbx_notice("meta[%u] has too large max-mapsize (%" PRIu64 "), "
                  "but size of used space still acceptable (%" PRIu64 ")",
                  meta_number, mapsize_max, used_bytes);
      page.mp_meta.mm_geo.upper = (pgno_t)(MAX_MAPSIZE / page.mp_meta.mm_psize);
    }

    /* LY: check and silently put mm_geo.now into [geo.lower...geo.upper].
     *
     * Copy-with-compaction by previous version of libmdbx could produce DB-file
     * less than meta.geo.lower bound, in case actual filling is low or no data
     * at all. This is not a problem as there is no damage or loss of data.
     * Therefore it is better not to consider such situation as an error, but
     * silently correct it. */
    if (page.mp_meta.mm_geo.now < page.mp_meta.mm_geo.lower)
      page.mp_meta.mm_geo.now = page.mp_meta.mm_geo.lower;
    if (page.mp_meta.mm_geo.now > page.mp_meta.mm_geo.upper)
      page.mp_meta.mm_geo.now = page.mp_meta.mm_geo.upper;

    if (page.mp_meta.mm_geo.next > page.mp_meta.mm_geo.now) {
      mdbx_notice("meta[%u] next-pageno (%" PRIaPGNO
                  ") is beyond end-pgno (%" PRIaPGNO "), skip it",
                  meta_number, page.mp_meta.mm_geo.next,
                  page.mp_meta.mm_geo.now);
      rc = MDBX_CORRUPTED;
      continue;
    }

    /* LY: FreeDB root */
    if (page.mp_meta.mm_dbs[FREE_DBI].md_root == P_INVALID) {
      if (page.mp_meta.mm_dbs[FREE_DBI].md_branch_pages ||
          page.mp_meta.mm_dbs[FREE_DBI].md_depth ||
          page.mp_meta.mm_dbs[FREE_DBI].md_entries ||
          page.mp_meta.mm_dbs[FREE_DBI].md_leaf_pages ||
          page.mp_meta.mm_dbs[FREE_DBI].md_overflow_pages) {
        mdbx_notice("meta[%u] has false-empty freedb, skip it", meta_number);
        rc = MDBX_CORRUPTED;
        continue;
      }
    } else if (page.mp_meta.mm_dbs[FREE_DBI].md_root >=
               page.mp_meta.mm_geo.next) {
      mdbx_notice("meta[%u] has invalid freedb-root %" PRIaPGNO ", skip it",
                  meta_number, page.mp_meta.mm_dbs[FREE_DBI].md_root);
      rc = MDBX_CORRUPTED;
      continue;
    }

    /* LY: MainDB root */
    if (page.mp_meta.mm_dbs[MAIN_DBI].md_root == P_INVALID) {
      if (page.mp_meta.mm_dbs[MAIN_DBI].md_branch_pages ||
          page.mp_meta.mm_dbs[MAIN_DBI].md_depth ||
          page.mp_meta.mm_dbs[MAIN_DBI].md_entries ||
          page.mp_meta.mm_dbs[MAIN_DBI].md_leaf_pages ||
          page.mp_meta.mm_dbs[MAIN_DBI].md_overflow_pages) {
        mdbx_notice("meta[%u] has false-empty maindb", meta_number);
        rc = MDBX_CORRUPTED;
        continue;
      }
    } else if (page.mp_meta.mm_dbs[MAIN_DBI].md_root >=
               page.mp_meta.mm_geo.next) {
      mdbx_notice("meta[%u] has invalid maindb-root %" PRIaPGNO ", skip it",
                  meta_number, page.mp_meta.mm_dbs[MAIN_DBI].md_root);
      rc = MDBX_CORRUPTED;
      continue;
    }

    if (page.mp_meta.mm_txnid_a == 0) {
      mdbx_warning("meta[%u] has zero txnid, skip it", meta_number);
      continue;
    }

    if (mdbx_meta_ot(prefer_noweak, env, meta, &page.mp_meta)) {
      *meta = page.mp_meta;
      if (META_IS_WEAK(meta))
        loop_limit += 1; /* LY: should re-read to hush race with update */
      mdbx_info("latch meta[%u]", meta_number);
    }
  }

  if (META_IS_WEAK(meta)) {
    mdbx_error("no usable meta-pages, database is corrupted");
    return rc;
  }

  return MDBX_SUCCESS;
}

static MDBX_page *__cold mdbx_meta_model(const MDBX_env *env, MDBX_page *model,
                                         unsigned num) {

  mdbx_ensure(env, mdbx_is_power2(env->me_psize));
  mdbx_ensure(env, env->me_psize >= MIN_PAGESIZE);
  mdbx_ensure(env, env->me_psize <= MAX_PAGESIZE);
  mdbx_ensure(env, env->me_dbgeo.lower >= MIN_MAPSIZE);
  mdbx_ensure(env, env->me_dbgeo.upper <= MAX_MAPSIZE);
  mdbx_ensure(env, env->me_dbgeo.now >= env->me_dbgeo.lower);
  mdbx_ensure(env, env->me_dbgeo.now <= env->me_dbgeo.upper);

  memset(model, 0, sizeof(*model));
  model->mp_pgno = num;
  model->mp_flags = P_META;
  model->mp_meta.mm_magic_and_version = MDBX_DATA_MAGIC;

  model->mp_meta.mm_geo.lower = bytes2pgno(env, env->me_dbgeo.lower);
  model->mp_meta.mm_geo.upper = bytes2pgno(env, env->me_dbgeo.upper);
  model->mp_meta.mm_geo.grow = (uint16_t)bytes2pgno(env, env->me_dbgeo.grow);
  model->mp_meta.mm_geo.shrink =
      (uint16_t)bytes2pgno(env, env->me_dbgeo.shrink);
  model->mp_meta.mm_geo.now = bytes2pgno(env, env->me_dbgeo.now);
  model->mp_meta.mm_geo.next = NUM_METAS;

  mdbx_ensure(env, model->mp_meta.mm_geo.lower >= MIN_PAGENO);
  mdbx_ensure(env, model->mp_meta.mm_geo.upper <= MAX_PAGENO);
  mdbx_ensure(env, model->mp_meta.mm_geo.now >= model->mp_meta.mm_geo.lower);
  mdbx_ensure(env, model->mp_meta.mm_geo.now <= model->mp_meta.mm_geo.upper);
  mdbx_ensure(env, model->mp_meta.mm_geo.next >= MIN_PAGENO);
  mdbx_ensure(env, model->mp_meta.mm_geo.next <= model->mp_meta.mm_geo.now);
  mdbx_ensure(env, model->mp_meta.mm_geo.grow ==
                       bytes2pgno(env, env->me_dbgeo.grow));
  mdbx_ensure(env, model->mp_meta.mm_geo.shrink ==
                       bytes2pgno(env, env->me_dbgeo.shrink));

  model->mp_meta.mm_psize = env->me_psize;
  model->mp_meta.mm_flags = (uint16_t)env->me_flags;
  model->mp_meta.mm_flags |=
      MDBX_INTEGERKEY; /* this is mm_dbs[FREE_DBI].md_flags */
  model->mp_meta.mm_dbs[FREE_DBI].md_root = P_INVALID;
  model->mp_meta.mm_dbs[MAIN_DBI].md_root = P_INVALID;
  mdbx_meta_set_txnid(env, &model->mp_meta, MIN_TXNID + num);
  model->mp_meta.mm_datasync_sign = mdbx_meta_sign(&model->mp_meta);
  return (MDBX_page *)((uint8_t *)model + env->me_psize);
}

/* Fill in most of the zeroed meta-pages for an empty database environment.
 * Return pointer to recenly (head) meta-page. */
static MDBX_page *__cold mdbx_init_metas(const MDBX_env *env, void *buffer) {
  MDBX_page *page0 = (MDBX_page *)buffer;
  MDBX_page *page1 = mdbx_meta_model(env, page0, 0);
  MDBX_page *page2 = mdbx_meta_model(env, page1, 1);
  mdbx_meta_model(env, page2, 2);
  mdbx_assert(env, !mdbx_meta_eq(env, &page0->mp_meta, &page1->mp_meta));
  mdbx_assert(env, !mdbx_meta_eq(env, &page1->mp_meta, &page2->mp_meta));
  mdbx_assert(env, !mdbx_meta_eq(env, &page2->mp_meta, &page0->mp_meta));
  return page2;
}

static int mdbx_sync_locked(MDBX_env *env, unsigned flags,
                            MDBX_meta *const pending) {
  mdbx_assert(env, ((env->me_flags ^ flags) & MDBX_WRITEMAP) == 0);
  MDBX_meta *const meta0 = METAPAGE(env, 0);
  MDBX_meta *const meta1 = METAPAGE(env, 1);
  MDBX_meta *const meta2 = METAPAGE(env, 2);
  MDBX_meta *const head = mdbx_meta_head(env);

  mdbx_assert(env, mdbx_meta_eq_mask(env) == 0);
  mdbx_assert(env,
              pending < METAPAGE(env, 0) || pending > METAPAGE(env, NUM_METAS));
  mdbx_assert(env, (env->me_flags & (MDBX_RDONLY | MDBX_FATAL_ERROR)) == 0);
  mdbx_assert(env, !META_IS_STEADY(head) || *env->me_unsynced_pages != 0);
  mdbx_assert(env, pending->mm_geo.next <= pending->mm_geo.now);

  if (flags & (MDBX_NOSYNC | MDBX_MAPASYNC)) {
    /* Check auto-sync conditions */
    const pgno_t autosync_threshold = *env->me_autosync_threshold;
    const uint64_t unsynced_timeout = *env->me_unsynced_timeout;
    if ((autosync_threshold && *env->me_unsynced_pages >= autosync_threshold) ||
        (unsynced_timeout && mdbx_osal_monotime() >= unsynced_timeout))
      flags &= MDBX_WRITEMAP | MDBX_SHRINK_ALLOWED; /* force steady */
  }

  /* LY: check conditions to shrink datafile */
  const pgno_t backlog_gap =
      pending->mm_dbs[FREE_DBI].md_depth + mdbx_backlog_extragap(env);
  pgno_t shrink = 0;
  if ((flags & MDBX_SHRINK_ALLOWED) && pending->mm_geo.shrink &&
      pending->mm_geo.now - pending->mm_geo.next >
          pending->mm_geo.shrink + backlog_gap) {
    const pgno_t largest = mdbx_find_largest(
        env, (head->mm_geo.next > pending->mm_geo.next) ? head->mm_geo.next
                                                        : pending->mm_geo.next);
    if (pending->mm_geo.now > largest &&
        pending->mm_geo.now - largest > pending->mm_geo.shrink + backlog_gap) {
      const pgno_t aligner =
          pending->mm_geo.grow ? pending->mm_geo.grow : pending->mm_geo.shrink;
      const pgno_t with_backlog_gap = largest + backlog_gap;
      const pgno_t aligned = pgno_align2os_pgno(
          env, with_backlog_gap + aligner - with_backlog_gap % aligner);
      const pgno_t bottom =
          (aligned > pending->mm_geo.lower) ? aligned : pending->mm_geo.lower;
      if (pending->mm_geo.now > bottom) {
        flags &= MDBX_WRITEMAP | MDBX_SHRINK_ALLOWED; /* force steady */
        shrink = pending->mm_geo.now - bottom;
        pending->mm_geo.now = bottom;
        if (mdbx_meta_txnid_stable(env, head) == pending->mm_txnid_a)
          mdbx_meta_set_txnid(env, pending, pending->mm_txnid_a + 1);
      }
    }
  }

  /* LY: step#1 - sync previously written/updated data-pages */
  int rc = *env->me_unsynced_pages ? MDBX_RESULT_TRUE /* carry non-steady */
                                   : MDBX_RESULT_FALSE /* carry steady */;
  if (rc != MDBX_RESULT_FALSE && (flags & MDBX_NOSYNC) == 0) {
    mdbx_assert(env, ((flags ^ env->me_flags) & MDBX_WRITEMAP) == 0);
    MDBX_meta *const recent_steady_meta = mdbx_meta_steady(env);
    if (flags & MDBX_WRITEMAP) {
      const size_t usedbytes = pgno_align2os_bytes(env, pending->mm_geo.next);
      rc = mdbx_msync(&env->me_dxb_mmap, 0, usedbytes, flags & MDBX_MAPASYNC);
      if (unlikely(rc != MDBX_SUCCESS))
        goto fail;
      rc = MDBX_RESULT_TRUE /* carry non-steady */;
      if ((flags & MDBX_MAPASYNC) == 0) {
        if (unlikely(pending->mm_geo.next > recent_steady_meta->mm_geo.now)) {
          rc = mdbx_filesync(env->me_fd, MDBX_SYNC_SIZE);
          if (unlikely(rc != MDBX_SUCCESS))
            goto fail;
        }
        rc = MDBX_RESULT_FALSE /* carry steady */;
      }
    } else {
      rc = mdbx_filesync(env->me_fd,
                         (pending->mm_geo.next > recent_steady_meta->mm_geo.now)
                             ? MDBX_SYNC_DATA | MDBX_SYNC_SIZE
                             : MDBX_SYNC_DATA);
      if (unlikely(rc != MDBX_SUCCESS))
        goto fail;
    }
  }

  /* Steady or Weak */
  if (rc == MDBX_RESULT_FALSE /* carry steady */) {
    pending->mm_datasync_sign = mdbx_meta_sign(pending);
    *env->me_unsynced_pages = 0;
    *env->me_unsynced_timeout = 0;
  } else {
    assert(rc == MDBX_RESULT_TRUE /* carry non-steady */);
    const uint64_t autosync_period = *env->me_autosync_period;
    if (autosync_period && *env->me_unsynced_timeout == 0)
      *env->me_unsynced_timeout = mdbx_osal_monotime() + autosync_period;
    pending->mm_datasync_sign =
        (flags & MDBX_UTTERLY_NOSYNC) == MDBX_UTTERLY_NOSYNC
            ? MDBX_DATASIGN_NONE
            : MDBX_DATASIGN_WEAK;
  }

  MDBX_meta *target = nullptr;
  if (mdbx_meta_txnid_stable(env, head) == pending->mm_txnid_a) {
    mdbx_assert(env, memcmp(&head->mm_dbs, &pending->mm_dbs,
                            sizeof(head->mm_dbs)) == 0);
    mdbx_assert(env, memcmp(&head->mm_canary, &pending->mm_canary,
                            sizeof(head->mm_canary)) == 0);
    mdbx_assert(env, memcmp(&head->mm_geo, &pending->mm_geo,
                            sizeof(pending->mm_geo)) == 0);
    if (!META_IS_STEADY(head) && META_IS_STEADY(pending))
      target = head;
    else {
      mdbx_ensure(env, mdbx_meta_eq(env, head, pending));
      mdbx_debug("skip update meta");
      return MDBX_SUCCESS;
    }
  } else if (head == meta0)
    target = mdbx_meta_ancient(prefer_steady, env, meta1, meta2);
  else if (head == meta1)
    target = mdbx_meta_ancient(prefer_steady, env, meta0, meta2);
  else {
    mdbx_assert(env, head == meta2);
    target = mdbx_meta_ancient(prefer_steady, env, meta0, meta1);
  }

  /* LY: step#2 - update meta-page. */
  mdbx_debug("writing meta%" PRIaPGNO " = root %" PRIaPGNO "/%" PRIaPGNO
             ", geo %" PRIaPGNO "/%" PRIaPGNO "-%" PRIaPGNO "/%" PRIaPGNO
             " +%u -%u, txn_id %" PRIaTXN ", %s",
             container_of(target, MDBX_page, mp_data)->mp_pgno,
             pending->mm_dbs[MAIN_DBI].md_root,
             pending->mm_dbs[FREE_DBI].md_root, pending->mm_geo.lower,
             pending->mm_geo.next, pending->mm_geo.now, pending->mm_geo.upper,
             pending->mm_geo.grow, pending->mm_geo.shrink, pending->mm_txnid_a,
             mdbx_durable_str(pending));

  mdbx_debug("meta0: %s, %s, txn_id %" PRIaTXN ", root %" PRIaPGNO
             "/%" PRIaPGNO,
             (meta0 == head) ? "head" : (meta0 == target) ? "tail" : "stay",
             mdbx_durable_str(meta0), mdbx_meta_txnid_fluid(env, meta0),
             meta0->mm_dbs[MAIN_DBI].md_root, meta0->mm_dbs[FREE_DBI].md_root);
  mdbx_debug("meta1: %s, %s, txn_id %" PRIaTXN ", root %" PRIaPGNO
             "/%" PRIaPGNO,
             (meta1 == head) ? "head" : (meta1 == target) ? "tail" : "stay",
             mdbx_durable_str(meta1), mdbx_meta_txnid_fluid(env, meta1),
             meta1->mm_dbs[MAIN_DBI].md_root, meta1->mm_dbs[FREE_DBI].md_root);
  mdbx_debug("meta2: %s, %s, txn_id %" PRIaTXN ", root %" PRIaPGNO
             "/%" PRIaPGNO,
             (meta2 == head) ? "head" : (meta2 == target) ? "tail" : "stay",
             mdbx_durable_str(meta2), mdbx_meta_txnid_fluid(env, meta2),
             meta2->mm_dbs[MAIN_DBI].md_root, meta2->mm_dbs[FREE_DBI].md_root);

  mdbx_assert(env, !mdbx_meta_eq(env, pending, meta0));
  mdbx_assert(env, !mdbx_meta_eq(env, pending, meta1));
  mdbx_assert(env, !mdbx_meta_eq(env, pending, meta2));

  mdbx_assert(env, ((env->me_flags ^ flags) & MDBX_WRITEMAP) == 0);
  mdbx_ensure(env, target == head || mdbx_meta_txnid_stable(env, target) <
                                         pending->mm_txnid_a);
  if (env->me_flags & MDBX_WRITEMAP) {
    mdbx_jitter4testing(true);
    if (likely(target != head)) {
      /* LY: 'invalidate' the meta. */
      target->mm_datasync_sign = MDBX_DATASIGN_WEAK;
      mdbx_meta_update_begin(env, target, pending->mm_txnid_a);
#ifndef NDEBUG
      /* debug: provoke failure to catch a violators, but don't touch mm_psize
       * and mm_flags to allow readers catch actual pagesize. */
      uint8_t *provoke_begin = (uint8_t *)&target->mm_dbs[FREE_DBI].md_root;
      uint8_t *provoke_end = (uint8_t *)&target->mm_datasync_sign;
      memset(provoke_begin, 0xCC, provoke_end - provoke_begin);
      mdbx_jitter4testing(false);
#endif

      /* LY: update info */
      target->mm_geo = pending->mm_geo;
      target->mm_dbs[FREE_DBI] = pending->mm_dbs[FREE_DBI];
      target->mm_dbs[MAIN_DBI] = pending->mm_dbs[MAIN_DBI];
      target->mm_canary = pending->mm_canary;
      mdbx_jitter4testing(true);
      mdbx_flush_noncoherent_cpu_writeback();

      /* LY: 'commit' the meta */
      mdbx_meta_update_end(env, target, pending->mm_txnid_b);
      mdbx_jitter4testing(true);
    } else {
      /* dangerous case (target == head), only mm_datasync_sign could
       * me updated, check assertions once again */
      mdbx_ensure(env,
                  mdbx_meta_txnid_stable(env, head) == pending->mm_txnid_a &&
                      !META_IS_STEADY(head) && META_IS_STEADY(pending));
      mdbx_ensure(env, memcmp(&head->mm_geo, &pending->mm_geo,
                              sizeof(head->mm_geo)) == 0);
      mdbx_ensure(env, memcmp(&head->mm_dbs, &pending->mm_dbs,
                              sizeof(head->mm_dbs)) == 0);
      mdbx_ensure(env, memcmp(&head->mm_canary, &pending->mm_canary,
                              sizeof(head->mm_canary)) == 0);
    }
    target->mm_datasync_sign = pending->mm_datasync_sign;
    mdbx_flush_noncoherent_cpu_writeback();
    mdbx_jitter4testing(true);
  } else {
    rc = mdbx_pwrite(env->me_fd, pending, sizeof(MDBX_meta),
                     (uint8_t *)target - env->me_map);
    if (unlikely(rc != MDBX_SUCCESS)) {
    undo:
      mdbx_debug("write failed, disk error?");
      /* On a failure, the pagecache still contains the new data.
       * Try write some old data back, to prevent it from being used. */
      mdbx_pwrite(env->me_fd, (void *)target, sizeof(MDBX_meta),
                  (uint8_t *)target - env->me_map);
      goto fail;
    }
    mdbx_invalidate_mmap_noncoherent_cache(target, sizeof(MDBX_meta));
  }

  /* LY: step#3 - sync meta-pages. */
  mdbx_assert(env, ((env->me_flags ^ flags) & MDBX_WRITEMAP) == 0);
  if ((flags & (MDBX_NOSYNC | MDBX_NOMETASYNC)) == 0) {
    mdbx_assert(env, ((flags ^ env->me_flags) & MDBX_WRITEMAP) == 0);
    if (flags & MDBX_WRITEMAP) {
      const size_t offset =
          ((uint8_t *)container_of(head, MDBX_page, mp_meta)) -
          env->me_dxb_mmap.dxb;
      const size_t paged_offset = offset & ~(env->me_os_psize - 1);
      const size_t paged_length = mdbx_roundup2(
          env->me_psize + offset - paged_offset, env->me_os_psize);
      rc = mdbx_msync(&env->me_dxb_mmap, paged_offset, paged_length,
                      flags & MDBX_MAPASYNC);
      if (unlikely(rc != MDBX_SUCCESS))
        goto fail;
    } else {
      rc = mdbx_filesync(env->me_fd, MDBX_SYNC_DATA | MDBX_SYNC_IODQ);
      if (rc != MDBX_SUCCESS)
        goto undo;
    }
  }

  /* LY: shrink datafile if needed */
  if (unlikely(shrink)) {
    mdbx_info("shrink to %" PRIaPGNO " pages (-%" PRIaPGNO ")",
              pending->mm_geo.now, shrink);
    rc = mdbx_mapresize(env, pending->mm_geo.now, pending->mm_geo.upper);
    if (MDBX_IS_ERROR(rc))
      goto fail;
  }

  return MDBX_SUCCESS;

fail:
  env->me_flags |= MDBX_FATAL_ERROR;
  return rc;
}

int __cold mdbx_env_get_maxkeysize(MDBX_env *env) {
  if (!env || env->me_signature != MDBX_ME_SIGNATURE || !env->me_maxkey_limit)
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;
  return env->me_maxkey_limit;
}

#define mdbx_nodemax(pagesize)                                                 \
  (((((pagesize)-PAGEHDRSZ) / MDBX_MINKEYS) & ~(uintptr_t)1) - sizeof(indx_t))

#define mdbx_maxkey(nodemax) (((nodemax)-NODESIZE - sizeof(MDBX_db)) / 2)

#define mdbx_maxgc_ov1page(pagesize)                                           \
  (((pagesize)-PAGEHDRSZ) / sizeof(pgno_t) - 1)

static void __cold mdbx_setup_pagesize(MDBX_env *env, const size_t pagesize) {
  STATIC_ASSERT(PTRDIFF_MAX > MAX_MAPSIZE);
  STATIC_ASSERT(MIN_PAGESIZE > sizeof(MDBX_page));
  mdbx_ensure(env, mdbx_is_power2(pagesize));
  mdbx_ensure(env, pagesize >= MIN_PAGESIZE);
  mdbx_ensure(env, pagesize <= MAX_PAGESIZE);
  env->me_psize = (unsigned)pagesize;

  STATIC_ASSERT(mdbx_maxgc_ov1page(MIN_PAGESIZE) > 42);
  STATIC_ASSERT(mdbx_maxgc_ov1page(MAX_PAGESIZE) < MDBX_DPL_TXNFULL);
  const intptr_t maxgc_ov1page = (pagesize - PAGEHDRSZ) / sizeof(pgno_t) - 1;
  mdbx_ensure(env,
              maxgc_ov1page > 42 && maxgc_ov1page < (intptr_t)MDBX_DPL_TXNFULL);
  env->me_maxgc_ov1page = (unsigned)maxgc_ov1page;

  STATIC_ASSERT(mdbx_nodemax(MIN_PAGESIZE) > 42);
  STATIC_ASSERT(mdbx_nodemax(MAX_PAGESIZE) < UINT16_MAX);
  const intptr_t nodemax = mdbx_nodemax(pagesize);
  mdbx_ensure(env, nodemax > 42 && nodemax < UINT16_MAX && nodemax % 2 == 0);
  env->me_nodemax = (unsigned)nodemax;

  STATIC_ASSERT(mdbx_maxkey(MIN_PAGESIZE) > 42);
  STATIC_ASSERT(mdbx_maxkey(MIN_PAGESIZE) < MIN_PAGESIZE);
  STATIC_ASSERT(mdbx_maxkey(MAX_PAGESIZE) > 42);
  STATIC_ASSERT(mdbx_maxkey(MAX_PAGESIZE) < MAX_PAGESIZE);
  const intptr_t maxkey_limit = mdbx_maxkey(env->me_nodemax);
  mdbx_ensure(env, maxkey_limit > 42 && (size_t)maxkey_limit < pagesize &&
                       maxkey_limit % 2 == 0);
  env->me_maxkey_limit = (unsigned)maxkey_limit;

  env->me_psize2log = mdbx_log2(pagesize);
  mdbx_assert(env, pgno2bytes(env, 1) == pagesize);
  mdbx_assert(env, bytes2pgno(env, pagesize + pagesize) == 2);
}

int __cold mdbx_env_create(MDBX_env **penv) {
  MDBX_env *env = mdbx_calloc(1, sizeof(MDBX_env));
  if (!env)
    return MDBX_ENOMEM;

  env->me_maxreaders = DEFAULT_READERS;
  env->me_maxdbs = env->me_numdbs = CORE_DBS;
  env->me_fd = INVALID_HANDLE_VALUE;
  env->me_lfd = INVALID_HANDLE_VALUE;
  env->me_pid = mdbx_getpid();

  int rc;
  const size_t os_psize = mdbx_syspagesize();
  if (!mdbx_is_power2(os_psize) || os_psize < MIN_PAGESIZE) {
    mdbx_error("unsuitable system pagesize %" PRIuPTR, os_psize);
    rc = MDBX_INCOMPATIBLE;
    goto bailout;
  }
  env->me_os_psize = (unsigned)os_psize;
  mdbx_setup_pagesize(env, env->me_os_psize);

  rc = mdbx_fastmutex_init(&env->me_dbi_lock);
  if (unlikely(rc != MDBX_SUCCESS))
    goto bailout;

#if defined(_WIN32) || defined(_WIN64)
  mdbx_srwlock_Init(&env->me_remap_guard);
  InitializeCriticalSection(&env->me_windowsbug_lock);
#else
  rc = mdbx_fastmutex_init(&env->me_remap_guard);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_fastmutex_destroy(&env->me_dbi_lock);
    goto bailout;
  }
  rc = mdbx_fastmutex_init(&env->me_lckless_stub.wmutex);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_fastmutex_destroy(&env->me_remap_guard);
    mdbx_fastmutex_destroy(&env->me_dbi_lock);
    goto bailout;
  }
#endif /* Windows */

  VALGRIND_CREATE_MEMPOOL(env, 0, 0);
  env->me_signature = MDBX_ME_SIGNATURE;
  *penv = env;
  return MDBX_SUCCESS;

bailout:
  mdbx_free(env);
  *penv = nullptr;
  return rc;
}

static int __cold mdbx_env_map(MDBX_env *env, size_t usedsize) {
  int rc = mdbx_mmap(env->me_flags, &env->me_dxb_mmap, env->me_dbgeo.now,
                     env->me_dbgeo.upper);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

#ifdef MADV_DONTFORK
  if (madvise(env->me_map, env->me_mapsize, MADV_DONTFORK))
    return errno;
#endif

#ifdef MADV_NOHUGEPAGE
  (void)madvise(env->me_map, env->me_mapsize, MADV_NOHUGEPAGE);
#endif

#if defined(MADV_DODUMP) && defined(MADV_DONTDUMP)
  const size_t meta_length = pgno2bytes(env, NUM_METAS);
  (void)madvise(env->me_map, meta_length, MADV_DODUMP);
  if (!(env->me_flags & MDBX_PAGEPERTURB))
    (void)madvise(env->me_map + meta_length, env->me_mapsize - meta_length,
                  MADV_DONTDUMP);
#endif

#ifdef MADV_REMOVE
  if (usedsize && (env->me_flags & MDBX_WRITEMAP)) {
    (void)madvise(env->me_map + usedsize, env->me_mapsize - usedsize,
                  MADV_REMOVE);
  }
#else
  (void)usedsize;
#endif

#if defined(MADV_RANDOM) && defined(MADV_WILLNEED)
  /* Turn on/off readahead. It's harmful when the DB is larger than RAM. */
  if (madvise(env->me_map, env->me_mapsize,
              (env->me_flags & MDBX_NORDAHEAD) ? MADV_RANDOM : MADV_WILLNEED))
    return errno;
#endif

#ifdef USE_VALGRIND
  env->me_valgrind_handle =
      VALGRIND_CREATE_BLOCK(env->me_map, env->me_mapsize, "mdbx");
#endif

  return MDBX_SUCCESS;
}

__cold LIBMDBX_API int
mdbx_env_set_geometry(MDBX_env *env, intptr_t size_lower, intptr_t size_now,
                      intptr_t size_upper, intptr_t growth_step,
                      intptr_t shrink_threshold, intptr_t pagesize) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

#if MDBX_TXN_CHECKPID
  if (unlikely(env->me_pid != mdbx_getpid()))
    env->me_flags |= MDBX_FATAL_ERROR;
#endif /* MDBX_TXN_CHECKPID */

  if (unlikely(env->me_flags & MDBX_FATAL_ERROR))
    return MDBX_PANIC;

  const bool inside_txn =
      (env->me_txn0 && env->me_txn0->mt_owner == mdbx_thread_self());

#if MDBX_DEBUG
  if (growth_step < 0)
    growth_step = 1;
  if (shrink_threshold < 0)
    shrink_threshold = 1;
#endif

  bool need_unlock = false;
  int rc = MDBX_PROBLEM;
  if (env->me_map) {
    /* env already mapped */
    if (!env->me_lck || (env->me_flags & MDBX_RDONLY))
      return MDBX_EACCESS;

    if (!inside_txn) {
      int err = mdbx_txn_lock(env, false);
      if (unlikely(err != MDBX_SUCCESS))
        return err;
      need_unlock = true;
    }
    MDBX_meta *head = mdbx_meta_head(env);
    if (!inside_txn) {
      env->me_txn0->mt_txnid = meta_txnid(env, head, false);
      mdbx_find_oldest(env->me_txn0);
    }

    if (pagesize < 0)
      pagesize = env->me_psize;
    if (pagesize != (intptr_t)env->me_psize) {
      rc = MDBX_EINVAL;
      goto bailout;
    }

    if (size_lower < 0)
      size_lower = pgno2bytes(env, head->mm_geo.lower);
    if (size_now < 0)
      size_now = pgno2bytes(env, head->mm_geo.now);
    if (size_upper < 0)
      size_upper = pgno2bytes(env, head->mm_geo.upper);
    if (growth_step < 0)
      growth_step = pgno2bytes(env, head->mm_geo.grow);
    if (shrink_threshold < 0)
      shrink_threshold = pgno2bytes(env, head->mm_geo.shrink);

    const size_t usedbytes =
        pgno2bytes(env, mdbx_find_largest(env, head->mm_geo.next));
    if ((size_t)size_upper < usedbytes) {
      rc = MDBX_MAP_FULL;
      goto bailout;
    }
    if ((size_t)size_now < usedbytes)
      size_now = usedbytes;
  } else {
    /* env NOT yet mapped */
    if (unlikely(inside_txn))
      return MDBX_PANIC;

    if (pagesize < 0) {
      pagesize = env->me_os_psize;
      if ((uintptr_t)pagesize > MAX_PAGESIZE)
        pagesize = MAX_PAGESIZE;
      mdbx_assert(env, (uintptr_t)pagesize >= MIN_PAGESIZE);
    }
  }

  if (pagesize == 0)
    pagesize = MIN_PAGESIZE;
  else if (pagesize == INTPTR_MAX)
    pagesize = MAX_PAGESIZE;

  if (pagesize < (intptr_t)MIN_PAGESIZE || pagesize > (intptr_t)MAX_PAGESIZE ||
      !mdbx_is_power2(pagesize)) {
    rc = MDBX_EINVAL;
    goto bailout;
  }

  if (size_lower <= 0) {
    size_lower = MIN_MAPSIZE;
    if (MIN_MAPSIZE / pagesize < MIN_PAGENO)
      size_lower = MIN_PAGENO * pagesize;
  }

  if (size_now <= 0) {
    size_now = DEFAULT_MAPSIZE;
    if (size_now < size_lower)
      size_now = size_lower;
    if (size_upper >= size_lower && size_now > size_upper)
      size_now = size_upper;
  }

  if (size_upper <= 0) {
    if ((size_t)size_now >= MAX_MAPSIZE / 2)
      size_upper = MAX_MAPSIZE;
    else if (MAX_MAPSIZE != MAX_MAPSIZE32 &&
             (size_t)size_now >= MAX_MAPSIZE32 / 2 &&
             (size_t)size_now <= MAX_MAPSIZE32 / 4 * 3)
      size_upper = MAX_MAPSIZE32;
    else {
      size_upper = size_now + size_now;
      if ((size_t)size_upper < DEFAULT_MAPSIZE * 2)
        size_upper = DEFAULT_MAPSIZE * 2;
    }
    if ((size_t)size_upper / pagesize > MAX_PAGENO)
      size_upper = pagesize * MAX_PAGENO;
  }

  if (unlikely(size_lower < (intptr_t)MIN_MAPSIZE || size_lower > size_upper)) {
    rc = MDBX_EINVAL;
    goto bailout;
  }

  if ((uint64_t)size_lower / pagesize < MIN_PAGENO) {
    rc = MDBX_EINVAL;
    goto bailout;
  }

  if (unlikely((size_t)size_upper > MAX_MAPSIZE ||
               (uint64_t)size_upper / pagesize > MAX_PAGENO)) {
    rc = MDBX_TOO_LARGE;
    goto bailout;
  }

  size_lower = mdbx_roundup2(size_lower, env->me_os_psize);
  size_upper = mdbx_roundup2(size_upper, env->me_os_psize);
  size_now = mdbx_roundup2(size_now, env->me_os_psize);

  /* LY: подбираем значение size_upper:
   *  - кратное размеру системной страницы
   *  - без нарушения MAX_MAPSIZE или MAX_PAGENO */
  while (unlikely((size_t)size_upper > MAX_MAPSIZE ||
                  (uint64_t)size_upper / pagesize > MAX_PAGENO)) {
    if ((size_t)size_upper < env->me_os_psize + MIN_MAPSIZE ||
        (size_t)size_upper < env->me_os_psize * (MIN_PAGENO + 1)) {
      /* паранойа на случай переполнения при невероятных значениях */
      rc = MDBX_EINVAL;
      goto bailout;
    }
    size_upper -= env->me_os_psize;
    if ((size_t)size_upper < (size_t)size_lower)
      size_lower = size_upper;
  }
  mdbx_assert(env, (size_upper - size_lower) % env->me_os_psize == 0);

  if (size_now < size_lower)
    size_now = size_lower;
  if (size_now > size_upper)
    size_now = size_upper;

  if (growth_step < 0) {
    growth_step = ((size_t)(size_upper - size_lower)) / 42;
    if (growth_step > size_lower)
      growth_step = size_lower;
    if (growth_step < 65536)
      growth_step = 65536;
    if ((size_t)growth_step > MEGABYTE * 16)
      growth_step = MEGABYTE * 16;
  }
  growth_step = mdbx_roundup2(growth_step, env->me_os_psize);
  if (bytes2pgno(env, growth_step) > UINT16_MAX)
    growth_step = pgno2bytes(env, UINT16_MAX);

  if (shrink_threshold < 0) {
    shrink_threshold = growth_step + growth_step;
    if (shrink_threshold < growth_step)
      shrink_threshold = growth_step;
  }
  shrink_threshold = mdbx_roundup2(shrink_threshold, env->me_os_psize);
  if (bytes2pgno(env, shrink_threshold) > UINT16_MAX)
    shrink_threshold = pgno2bytes(env, UINT16_MAX);

  /* save user's geo-params for future open/create */
  env->me_dbgeo.lower = size_lower;
  env->me_dbgeo.now = size_now;
  env->me_dbgeo.upper = size_upper;
  env->me_dbgeo.grow = growth_step;
  env->me_dbgeo.shrink = shrink_threshold;
  rc = MDBX_SUCCESS;

  if (env->me_map) {
    /* apply new params */
    mdbx_assert(env, pagesize == (intptr_t)env->me_psize);

    MDBX_meta *head = mdbx_meta_head(env);
    MDBX_meta meta = *head;
    meta.mm_geo.lower = bytes2pgno(env, env->me_dbgeo.lower);
    meta.mm_geo.now = bytes2pgno(env, env->me_dbgeo.now);
    meta.mm_geo.upper = bytes2pgno(env, env->me_dbgeo.upper);
    meta.mm_geo.grow = (uint16_t)bytes2pgno(env, env->me_dbgeo.grow);
    meta.mm_geo.shrink = (uint16_t)bytes2pgno(env, env->me_dbgeo.shrink);

    mdbx_assert(env, env->me_dbgeo.lower >= MIN_MAPSIZE);
    mdbx_assert(env, meta.mm_geo.lower >= MIN_PAGENO);
    mdbx_assert(env, env->me_dbgeo.upper <= MAX_MAPSIZE);
    mdbx_assert(env, meta.mm_geo.upper <= MAX_PAGENO);
    mdbx_assert(env, meta.mm_geo.now >= meta.mm_geo.next);
    mdbx_assert(env, env->me_dbgeo.upper >= env->me_dbgeo.lower);
    mdbx_assert(env, meta.mm_geo.upper >= meta.mm_geo.now);
    mdbx_assert(env, meta.mm_geo.now >= meta.mm_geo.lower);
    mdbx_assert(env, meta.mm_geo.grow == bytes2pgno(env, env->me_dbgeo.grow));
    mdbx_assert(env,
                meta.mm_geo.shrink == bytes2pgno(env, env->me_dbgeo.shrink));

    if (memcmp(&meta.mm_geo, &head->mm_geo, sizeof(meta.mm_geo))) {

#if defined(_WIN32) || defined(_WIN64)
      /* Was DB shrinking disabled before and now it will be enabled? */
      if (meta.mm_geo.lower < meta.mm_geo.upper && meta.mm_geo.shrink &&
          !(head->mm_geo.lower < head->mm_geo.upper && head->mm_geo.shrink)) {
        rc = mdbx_rdt_lock(env);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;

        /* Check if there are any reading threads that do not use the SRWL */
        const mdbx_pid_t CurrentTid = GetCurrentThreadId();
        const MDBX_reader *const begin = env->me_lck->mti_readers;
        const MDBX_reader *const end = begin + env->me_lck->mti_numreaders;
        for (const MDBX_reader *reader = begin; reader < end; ++reader) {
          if (reader->mr_pid == env->me_pid && reader->mr_tid &&
              reader->mr_tid != CurrentTid) {
            /* At least one thread may don't use SRWL */
            rc = MDBX_EPERM;
            break;
          }
        }

        mdbx_rdt_unlock(env);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
      }
#endif

      if (meta.mm_geo.now != head->mm_geo.now ||
          meta.mm_geo.upper != head->mm_geo.upper) {
        rc = mdbx_mapresize(env, meta.mm_geo.now, meta.mm_geo.upper);
        if (unlikely(rc != MDBX_SUCCESS))
          goto bailout;
        head = /* base address could be changed */ mdbx_meta_head(env);
      }
      *env->me_unsynced_pages += 1;
      mdbx_meta_set_txnid(env, &meta, mdbx_meta_txnid_stable(env, head) + 1);
      rc = mdbx_sync_locked(env, env->me_flags, &meta);
    }
  } else if (pagesize != (intptr_t)env->me_psize) {
    mdbx_setup_pagesize(env, pagesize);
  }

bailout:
  if (need_unlock)
    mdbx_txn_unlock(env);
  return rc;
}

int __cold mdbx_env_set_mapsize(MDBX_env *env, size_t size) {
  return mdbx_env_set_geometry(env, -1, size, -1, -1, -1, -1);
}

int __cold mdbx_env_set_maxdbs(MDBX_env *env, MDBX_dbi dbs) {
  if (unlikely(dbs > MAX_DBI))
    return MDBX_EINVAL;

  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(env->me_map))
    return MDBX_EPERM;

  env->me_maxdbs = dbs + CORE_DBS;
  return MDBX_SUCCESS;
}

int __cold mdbx_env_set_maxreaders(MDBX_env *env, unsigned readers) {
  if (unlikely(readers < 1 || readers > INT16_MAX))
    return MDBX_EINVAL;

  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(env->me_map))
    return MDBX_EPERM;

  env->me_maxreaders = readers;
  return MDBX_SUCCESS;
}

int __cold mdbx_env_get_maxreaders(MDBX_env *env, unsigned *readers) {
  if (!env || !readers)
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  *readers = env->me_maxreaders;
  return MDBX_SUCCESS;
}

/* Further setup required for opening an MDBX environment */
static int __cold mdbx_setup_dxb(MDBX_env *env, const int lck_rc) {
  uint64_t filesize_before_mmap;
  MDBX_meta meta;
  int rc = MDBX_RESULT_FALSE;
  int err = mdbx_read_header(env, &meta, &filesize_before_mmap);
  if (unlikely(err != MDBX_SUCCESS)) {
    if (lck_rc != /* lck exclusive */ MDBX_RESULT_TRUE || err != MDBX_ENODATA ||
        (env->me_flags & MDBX_RDONLY) != 0)
      return err;

    mdbx_debug("create new database");
    rc = /* new database */ MDBX_RESULT_TRUE;

    if (!env->me_dbgeo.now) {
      /* set defaults if not configured */
      err = mdbx_env_set_mapsize(env, DEFAULT_MAPSIZE);
      if (unlikely(err != MDBX_SUCCESS))
        return err;
    }

    void *buffer = mdbx_calloc(NUM_METAS, env->me_psize);
    if (!buffer)
      return MDBX_ENOMEM;

    meta = mdbx_init_metas(env, buffer)->mp_meta;
    err = mdbx_pwrite(env->me_fd, buffer, env->me_psize * NUM_METAS, 0);
    mdbx_free(buffer);
    if (unlikely(err != MDBX_SUCCESS))
      return err;

    err = mdbx_ftruncate(env->me_fd, filesize_before_mmap = env->me_dbgeo.now);
    if (unlikely(err != MDBX_SUCCESS))
      return err;

#ifndef NDEBUG /* just for checking */
    err = mdbx_read_header(env, &meta, &filesize_before_mmap);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
#endif
  }

  mdbx_info("header: root %" PRIaPGNO "/%" PRIaPGNO ", geo %" PRIaPGNO
            "/%" PRIaPGNO "-%" PRIaPGNO "/%" PRIaPGNO
            " +%u -%u, txn_id %" PRIaTXN ", %s",
            meta.mm_dbs[MAIN_DBI].md_root, meta.mm_dbs[FREE_DBI].md_root,
            meta.mm_geo.lower, meta.mm_geo.next, meta.mm_geo.now,
            meta.mm_geo.upper, meta.mm_geo.grow, meta.mm_geo.shrink,
            meta.mm_txnid_a, mdbx_durable_str(&meta));

  mdbx_setup_pagesize(env, meta.mm_psize);
  const size_t used_bytes = pgno2bytes(env, meta.mm_geo.next);
  if ((env->me_flags & MDBX_RDONLY) /* readonly */
      || lck_rc != MDBX_RESULT_TRUE /* not exclusive */) {
    /* use present params from db */
    err = mdbx_env_set_geometry(
        env, meta.mm_geo.lower * (uint64_t)meta.mm_psize,
        meta.mm_geo.now * (uint64_t)meta.mm_psize,
        meta.mm_geo.upper * (uint64_t)meta.mm_psize,
        meta.mm_geo.grow * (uint64_t)meta.mm_psize,
        meta.mm_geo.shrink * (uint64_t)meta.mm_psize, meta.mm_psize);
    if (unlikely(err != MDBX_SUCCESS)) {
      mdbx_error("could not use present dbsize-params from db");
      return MDBX_INCOMPATIBLE;
    }
  } else if (env->me_dbgeo.now) {
    /* silently growth to last used page */
    if (env->me_dbgeo.now < used_bytes)
      env->me_dbgeo.now = used_bytes;
    if (env->me_dbgeo.upper < used_bytes)
      env->me_dbgeo.upper = used_bytes;

    /* apply preconfigured params, but only if substantial changes:
     *  - upper or lower limit changes
     *  - shrink threshold or growth step
     * But ignore just chagne just a 'now/current' size. */
    if (bytes_align2os_bytes(env, env->me_dbgeo.upper) !=
            pgno_align2os_bytes(env, meta.mm_geo.upper) ||
        bytes_align2os_bytes(env, env->me_dbgeo.lower) !=
            pgno_align2os_bytes(env, meta.mm_geo.lower) ||
        bytes_align2os_bytes(env, env->me_dbgeo.shrink) !=
            pgno_align2os_bytes(env, meta.mm_geo.shrink) ||
        bytes_align2os_bytes(env, env->me_dbgeo.grow) !=
            pgno_align2os_bytes(env, meta.mm_geo.grow)) {

      if (env->me_dbgeo.shrink && env->me_dbgeo.now > used_bytes)
        /* pre-shrink if enabled */
        env->me_dbgeo.now = used_bytes + env->me_dbgeo.shrink -
                            used_bytes % env->me_dbgeo.shrink;

      err = mdbx_env_set_geometry(env, env->me_dbgeo.lower, env->me_dbgeo.now,
                                  env->me_dbgeo.upper, env->me_dbgeo.grow,
                                  env->me_dbgeo.shrink, meta.mm_psize);
      if (unlikely(err != MDBX_SUCCESS)) {
        mdbx_error("could not apply preconfigured dbsize-params to db");
        return MDBX_INCOMPATIBLE;
      }

      /* update meta fields */
      meta.mm_geo.now = bytes2pgno(env, env->me_dbgeo.now);
      meta.mm_geo.lower = bytes2pgno(env, env->me_dbgeo.lower);
      meta.mm_geo.upper = bytes2pgno(env, env->me_dbgeo.upper);
      meta.mm_geo.grow = (uint16_t)bytes2pgno(env, env->me_dbgeo.grow);
      meta.mm_geo.shrink = (uint16_t)bytes2pgno(env, env->me_dbgeo.shrink);

      mdbx_info("amended: root %" PRIaPGNO "/%" PRIaPGNO ", geo %" PRIaPGNO
                "/%" PRIaPGNO "-%" PRIaPGNO "/%" PRIaPGNO
                " +%u -%u, txn_id %" PRIaTXN ", %s",
                meta.mm_dbs[MAIN_DBI].md_root, meta.mm_dbs[FREE_DBI].md_root,
                meta.mm_geo.lower, meta.mm_geo.next, meta.mm_geo.now,
                meta.mm_geo.upper, meta.mm_geo.grow, meta.mm_geo.shrink,
                meta.mm_txnid_a, mdbx_durable_str(&meta));
    }
    mdbx_ensure(env, meta.mm_geo.now >= meta.mm_geo.next);
  } else {
    /* geo-params not pre-configured by user,
     * get current values from a meta. */
    env->me_dbgeo.now = pgno2bytes(env, meta.mm_geo.now);
    env->me_dbgeo.lower = pgno2bytes(env, meta.mm_geo.lower);
    env->me_dbgeo.upper = pgno2bytes(env, meta.mm_geo.upper);
    env->me_dbgeo.grow = pgno2bytes(env, meta.mm_geo.grow);
    env->me_dbgeo.shrink = pgno2bytes(env, meta.mm_geo.shrink);
  }

  const size_t expected_bytes =
      mdbx_roundup2(pgno2bytes(env, meta.mm_geo.now), env->me_os_psize);
  mdbx_ensure(env, expected_bytes >= used_bytes);
  if (filesize_before_mmap != expected_bytes) {
    if (lck_rc != /* lck exclusive */ MDBX_RESULT_TRUE) {
      mdbx_info("filesize mismatch (expect %" PRIuPTR "/%" PRIaPGNO
                ", have %" PRIu64 "/%" PRIaPGNO "), "
                "assume collision in non-exclusive mode",
                expected_bytes, bytes2pgno(env, expected_bytes),
                filesize_before_mmap,
                bytes2pgno(env, (size_t)filesize_before_mmap));
    } else {
      mdbx_notice("filesize mismatch (expect %" PRIuSIZE "/%" PRIaPGNO
                  ", have %" PRIu64 "/%" PRIaPGNO ")",
                  expected_bytes, bytes2pgno(env, expected_bytes),
                  filesize_before_mmap,
                  bytes2pgno(env, (size_t)filesize_before_mmap));
      if (filesize_before_mmap < used_bytes) {
        mdbx_error("last-page beyond end-of-file (last %" PRIaPGNO
                   ", have %" PRIaPGNO ")",
                   meta.mm_geo.next,
                   bytes2pgno(env, (size_t)filesize_before_mmap));
        return MDBX_CORRUPTED;
      }

      if (env->me_flags & MDBX_RDONLY) {
        if (filesize_before_mmap % env->me_os_psize) {
          mdbx_error("filesize should be rounded-up to system page");
          return MDBX_WANNA_RECOVERY;
        }
        mdbx_notice("ignore filesize mismatch in readonly-mode");
      } else {
        mdbx_info("resize datafile to %" PRIuSIZE " bytes, %" PRIaPGNO " pages",
                  expected_bytes, bytes2pgno(env, expected_bytes));
        err = mdbx_ftruncate(env->me_fd, expected_bytes);
        if (unlikely(err != MDBX_SUCCESS)) {
          mdbx_error("error %d, while resize datafile to %" PRIuSIZE
                     " bytes, %" PRIaPGNO " pages",
                     rc, expected_bytes, bytes2pgno(env, expected_bytes));
          return err;
        }
        filesize_before_mmap = expected_bytes;
      }
    }
  }

  err = mdbx_env_map(env, (lck_rc != /* lck exclusive */ MDBX_RESULT_TRUE)
                              ? 0
                              : expected_bytes);
  if (err != MDBX_SUCCESS)
    return err;

  const unsigned meta_clash_mask = mdbx_meta_eq_mask(env);
  if (meta_clash_mask) {
    mdbx_error("meta-pages are clashed: mask 0x%d", meta_clash_mask);
    return MDBX_WANNA_RECOVERY;
  }

  while (1) {
    MDBX_meta *head = mdbx_meta_head(env);
    const txnid_t head_txnid = mdbx_meta_txnid_fluid(env, head);
    if (head_txnid == meta.mm_txnid_a)
      break;

    if (lck_rc == /* lck exclusive */ MDBX_RESULT_TRUE) {
      mdbx_assert(env, META_IS_STEADY(&meta) && !META_IS_STEADY(head));
      if (env->me_flags & MDBX_RDONLY) {
        mdbx_error("rollback needed: (from head %" PRIaTXN
                   " to steady %" PRIaTXN "), but unable in read-only mode",
                   head_txnid, meta.mm_txnid_a);
        return MDBX_WANNA_RECOVERY /* LY: could not recovery/rollback */;
      }

      const MDBX_meta *const meta0 = METAPAGE(env, 0);
      const MDBX_meta *const meta1 = METAPAGE(env, 1);
      const MDBX_meta *const meta2 = METAPAGE(env, 2);
      txnid_t undo_txnid = 0;
      while (
          (head != meta0 && mdbx_meta_txnid_fluid(env, meta0) == undo_txnid) ||
          (head != meta1 && mdbx_meta_txnid_fluid(env, meta1) == undo_txnid) ||
          (head != meta2 && mdbx_meta_txnid_fluid(env, meta2) == undo_txnid))
        undo_txnid += 1;
      if (unlikely(undo_txnid >= meta.mm_txnid_a)) {
        mdbx_fatal("rollback failed: no suitable txnid (0,1,2) < %" PRIaTXN,
                   meta.mm_txnid_a);
        return MDBX_PANIC /* LY: could not recovery/rollback */;
      }

      /* LY: rollback weak checkpoint */
      mdbx_trace("rollback: from %" PRIaTXN ", to %" PRIaTXN " as %" PRIaTXN,
                 head_txnid, meta.mm_txnid_a, undo_txnid);
      mdbx_ensure(env, head_txnid == mdbx_meta_txnid_stable(env, head));

      if (env->me_flags & MDBX_WRITEMAP) {
        head->mm_txnid_a = undo_txnid;
        head->mm_datasync_sign = MDBX_DATASIGN_WEAK;
        head->mm_txnid_b = undo_txnid;
        const size_t offset =
            ((uint8_t *)container_of(head, MDBX_page, mp_meta)) -
            env->me_dxb_mmap.dxb;
        const size_t paged_offset = offset & ~(env->me_os_psize - 1);
        const size_t paged_length = mdbx_roundup2(
            env->me_psize + offset - paged_offset, env->me_os_psize);
        err = mdbx_msync(&env->me_dxb_mmap, paged_offset, paged_length, false);
      } else {
        MDBX_meta rollback = *head;
        mdbx_meta_set_txnid(env, &rollback, undo_txnid);
        rollback.mm_datasync_sign = MDBX_DATASIGN_WEAK;
        err = mdbx_pwrite(env->me_fd, &rollback, sizeof(MDBX_meta),
                          (uint8_t *)head - (uint8_t *)env->me_map);
      }
      if (err)
        return err;

      mdbx_invalidate_mmap_noncoherent_cache(env->me_map,
                                             pgno2bytes(env, NUM_METAS));
      mdbx_ensure(env, undo_txnid == mdbx_meta_txnid_fluid(env, head));
      mdbx_ensure(env, 0 == mdbx_meta_eq_mask(env));
      continue;
    }

    if (!env->me_lck) {
      /* LY: without-lck (read-only) mode, so it is imposible that other
       * process made weak checkpoint. */
      mdbx_error("without-lck, unable recovery/rollback");
      return MDBX_WANNA_RECOVERY;
    }

    /* LY: assume just have a collision with other running process,
     *     or someone make a weak checkpoint */
    mdbx_info("assume collision or online weak checkpoint");
    break;
  }

  const MDBX_meta *head = mdbx_meta_head(env);
  if (lck_rc == /* lck exclusive */ MDBX_RESULT_TRUE) {
    /* re-check file size after mmap */
    uint64_t filesize_after_mmap;
    err = mdbx_filesize(env->me_fd, &filesize_after_mmap);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
    if (filesize_after_mmap != expected_bytes) {
      if (filesize_after_mmap != filesize_before_mmap)
        mdbx_info("datafile resized by system to %" PRIu64 " bytes",
                  filesize_after_mmap);
      if (filesize_after_mmap % env->me_os_psize ||
          filesize_after_mmap > env->me_dbgeo.upper ||
          filesize_after_mmap < used_bytes) {
        mdbx_info("unacceptable/unexpected  datafile size %" PRIu64,
                  filesize_after_mmap);
        return MDBX_PROBLEM;
      }
      if ((env->me_flags & MDBX_RDONLY) == 0) {
        meta.mm_geo.now =
            bytes2pgno(env, env->me_dbgeo.now = (size_t)filesize_after_mmap);
        mdbx_info("update meta-geo to filesize %" PRIuPTR " bytes, %" PRIaPGNO
                  " pages",
                  env->me_dbgeo.now, meta.mm_geo.now);
      }
    }

    if (memcmp(&meta.mm_geo, &head->mm_geo, sizeof(meta.mm_geo))) {
      const txnid_t txnid = mdbx_meta_txnid_stable(env, head);
      mdbx_info("updating meta.geo: "
                "from l%" PRIaPGNO "-n%" PRIaPGNO "-u%" PRIaPGNO
                "/s%u-g%u (txn#%" PRIaTXN "), "
                "to l%" PRIaPGNO "-n%" PRIaPGNO "-u%" PRIaPGNO
                "/s%u-g%u (txn#%" PRIaTXN ")",
                head->mm_geo.lower, head->mm_geo.now, head->mm_geo.upper,
                head->mm_geo.shrink, head->mm_geo.grow, txnid,
                meta.mm_geo.lower, meta.mm_geo.now, meta.mm_geo.upper,
                meta.mm_geo.shrink, meta.mm_geo.grow, txnid + 1);

      mdbx_ensure(env, mdbx_meta_eq(env, &meta, head));
      mdbx_meta_set_txnid(env, &meta, txnid + 1);
      *env->me_unsynced_pages += 1;
      err = mdbx_sync_locked(env, env->me_flags | MDBX_SHRINK_ALLOWED, &meta);
      if (err) {
        mdbx_info("error %d, while updating meta.geo: "
                  "from l%" PRIaPGNO "-n%" PRIaPGNO "-u%" PRIaPGNO
                  "/s%u-g%u (txn#%" PRIaTXN "), "
                  "to l%" PRIaPGNO "-n%" PRIaPGNO "-u%" PRIaPGNO
                  "/s%u-g%u (txn#%" PRIaTXN ")",
                  err, head->mm_geo.lower, head->mm_geo.now, head->mm_geo.upper,
                  head->mm_geo.shrink, head->mm_geo.grow, txnid,
                  meta.mm_geo.lower, meta.mm_geo.now, meta.mm_geo.upper,
                  meta.mm_geo.shrink, meta.mm_geo.grow, txnid + 1);
        return err;
      }
    }
  }

  return rc;
}

/****************************************************************************/

/* Open and/or initialize the lock region for the environment. */
static int __cold mdbx_setup_lck(MDBX_env *env, char *lck_pathname,
                                 mode_t mode) {
  mdbx_assert(env, env->me_fd != INVALID_HANDLE_VALUE);
  mdbx_assert(env, env->me_lfd == INVALID_HANDLE_VALUE);

  int err = mdbx_openfile(lck_pathname, O_RDWR | O_CREAT, mode, &env->me_lfd,
                          (env->me_flags & MDBX_EXCLUSIVE) ? true : false);
  if (err != MDBX_SUCCESS) {
    if (!(err == MDBX_ENOFILE && (env->me_flags & MDBX_EXCLUSIVE)) &&
        !(err == MDBX_EROFS && (env->me_flags & MDBX_RDONLY)))
      return err;

    /* LY: without-lck mode (e.g. exclusive or on read-only filesystem) */
    env->me_lfd = INVALID_HANDLE_VALUE;
    const int rc = mdbx_lck_seize(env);
    if (MDBX_IS_ERROR(rc))
      return rc;

    env->me_oldest = &env->me_lckless_stub.oldest;
    env->me_unsynced_timeout = &env->me_lckless_stub.unsynced_timeout;
    env->me_autosync_period = &env->me_lckless_stub.autosync_period;
    env->me_unsynced_pages = &env->me_lckless_stub.autosync_pending;
    env->me_autosync_threshold = &env->me_lckless_stub.autosync_threshold;
    env->me_maxreaders = UINT_MAX;
#ifdef MDBX_OSAL_LOCK
    env->me_wmutex = &env->me_lckless_stub.wmutex;
#endif
    mdbx_debug("lck-setup:%s%s%s", " lck-less",
               (env->me_flags & MDBX_RDONLY) ? " readonly" : "",
               (rc == MDBX_RESULT_TRUE) ? " exclusive" : " cooperative");
    return rc;
  }

  /* Try to get exclusive lock. If we succeed, then
   * nobody is using the lock region and we should initialize it. */
  const int rc = mdbx_lck_seize(env);
  if (MDBX_IS_ERROR(rc))
    return rc;

  mdbx_debug("lck-setup:%s%s%s", " with-lck",
             (env->me_flags & MDBX_RDONLY) ? " readonly" : "",
             (rc == MDBX_RESULT_TRUE) ? " exclusive" : " cooperative");

  uint64_t size;
  err = mdbx_filesize(env->me_lfd, &size);
  if (unlikely(err != MDBX_SUCCESS))
    return err;

  if (rc == MDBX_RESULT_TRUE) {
    uint64_t wanna = mdbx_roundup2(
        (env->me_maxreaders - 1) * sizeof(MDBX_reader) + sizeof(MDBX_lockinfo),
        env->me_os_psize);
#ifndef NDEBUG
    err = mdbx_ftruncate(env->me_lfd, size = 0);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
#endif
    mdbx_jitter4testing(false);

    if (size != wanna) {
      err = mdbx_ftruncate(env->me_lfd, wanna);
      if (unlikely(err != MDBX_SUCCESS))
        return err;
      size = wanna;
    }
  } else {
    if (env->me_flags & MDBX_EXCLUSIVE)
      return MDBX_BUSY;
    if (size > PTRDIFF_MAX || (size & (env->me_os_psize - 1)) ||
        size < env->me_os_psize) {
      mdbx_notice("lck-file has invalid size %" PRIu64 " bytes", size);
      return MDBX_PROBLEM;
    }
  }

  const size_t maxreaders =
      ((size_t)size - sizeof(MDBX_lockinfo)) / sizeof(MDBX_reader) + 1;
  if (maxreaders > UINT16_MAX) {
    mdbx_error("lck-size too big (up to %" PRIuPTR " readers)", maxreaders);
    return MDBX_PROBLEM;
  }
  env->me_maxreaders = (unsigned)maxreaders;

  err = mdbx_mmap(MDBX_WRITEMAP, &env->me_lck_mmap, (size_t)size, (size_t)size);
  if (unlikely(err != MDBX_SUCCESS))
    return err;

#ifdef MADV_DODUMP
  (void)madvise(env->me_lck, size, MADV_DODUMP);
#endif

#ifdef MADV_DONTFORK
  if (madvise(env->me_lck, size, MADV_DONTFORK) < 0)
    return errno;
#endif

#ifdef MADV_WILLNEED
  if (madvise(env->me_lck, size, MADV_WILLNEED) < 0)
    return errno;
#endif

#ifdef MADV_RANDOM
  if (madvise(env->me_lck, size, MADV_RANDOM) < 0)
    return errno;
#endif

  if (rc == MDBX_RESULT_TRUE) {
    /* LY: exlcusive mode, init lck */
    memset(env->me_lck, 0, (size_t)size);
    err = mdbx_lck_init(env);
    if (err)
      return err;

    env->me_lck->mti_magic_and_version = MDBX_LOCK_MAGIC;
    env->me_lck->mti_os_and_format = MDBX_LOCK_FORMAT;
  } else {
    if (env->me_lck->mti_magic_and_version != MDBX_LOCK_MAGIC) {
      mdbx_error("lock region has invalid magic/version");
      return ((env->me_lck->mti_magic_and_version >> 8) != MDBX_MAGIC)
                 ? MDBX_INVALID
                 : MDBX_VERSION_MISMATCH;
    }
    if (env->me_lck->mti_os_and_format != MDBX_LOCK_FORMAT) {
      mdbx_error("lock region has os/format 0x%" PRIx32 ", expected 0x%" PRIx32,
                 env->me_lck->mti_os_and_format, MDBX_LOCK_FORMAT);
      return MDBX_VERSION_MISMATCH;
    }
  }

  mdbx_assert(env, !MDBX_IS_ERROR(rc));
  env->me_oldest = &env->me_lck->mti_oldest_reader;
  env->me_unsynced_timeout = &env->me_lck->mti_unsynced_timeout;
  env->me_autosync_period = &env->me_lck->mti_autosync_period;
  env->me_unsynced_pages = &env->me_lck->mti_unsynced_pages;
  env->me_autosync_threshold = &env->me_lck->mti_autosync_threshold;
#ifdef MDBX_OSAL_LOCK
  env->me_wmutex = &env->me_lck->mti_wmutex;
#endif
  return rc;
}

/* Only a subset of the mdbx_env flags can be changed
 * at runtime. Changing other flags requires closing the
 * environment and re-opening it with the new flags. */
#define CHANGEABLE                                                             \
  (MDBX_NOSYNC | MDBX_NOMETASYNC | MDBX_MAPASYNC | MDBX_NOMEMINIT |            \
   MDBX_COALESCE | MDBX_PAGEPERTURB)
#define CHANGELESS                                                             \
  (MDBX_NOSUBDIR | MDBX_RDONLY | MDBX_WRITEMAP | MDBX_NOTLS | MDBX_NORDAHEAD | \
   MDBX_LIFORECLAIM | MDBX_EXCLUSIVE)

#if VALID_FLAGS & PERSISTENT_FLAGS & (CHANGEABLE | CHANGELESS)
#error "Persistent DB flags & env flags overlap, but both go in mm_flags"
#endif

int __cold mdbx_env_open(MDBX_env *env, const char *path, unsigned flags,
                         mode_t mode) {
  if (unlikely(!env || !path))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (flags & ~(CHANGEABLE | CHANGELESS))
    return MDBX_EINVAL;

  if (env->me_fd != INVALID_HANDLE_VALUE ||
      (env->me_flags & MDBX_ENV_ACTIVE) != 0)
    return MDBX_EPERM;

  size_t len_full, len = strlen(path);
  if (flags & MDBX_NOSUBDIR) {
    len_full = len + sizeof(MDBX_LOCK_SUFFIX) + len + 1;
  } else {
    len_full = len + sizeof(MDBX_LOCKNAME) + len + sizeof(MDBX_DATANAME);
  }
  char *lck_pathname = mdbx_malloc(len_full);
  if (!lck_pathname)
    return MDBX_ENOMEM;

  char *dxb_pathname;
  if (flags & MDBX_NOSUBDIR) {
    dxb_pathname = lck_pathname + len + sizeof(MDBX_LOCK_SUFFIX);
    sprintf(lck_pathname, "%s" MDBX_LOCK_SUFFIX, path);
    strcpy(dxb_pathname, path);
  } else {
    dxb_pathname = lck_pathname + len + sizeof(MDBX_LOCKNAME);
    sprintf(lck_pathname, "%s" MDBX_LOCKNAME, path);
    sprintf(dxb_pathname, "%s" MDBX_DATANAME, path);
  }

  int rc = MDBX_SUCCESS;
  flags |= env->me_flags;
  if (flags & MDBX_RDONLY) {
    /* LY: silently ignore irrelevant flags when
     * we're only getting read access */
    flags &= ~(MDBX_WRITEMAP | MDBX_MAPASYNC | MDBX_NOSYNC | MDBX_NOMETASYNC |
               MDBX_COALESCE | MDBX_LIFORECLAIM | MDBX_NOMEMINIT);
  } else {
    if (!((env->me_free_pgs = mdbx_pnl_alloc(MDBX_PNL_INITIAL)) &&
          (env->me_dirtylist =
               mdbx_calloc(MDBX_DPL_TXNFULL + 1, sizeof(MDBX_DP)))))
      rc = MDBX_ENOMEM;
  }

  const uint32_t saved_me_flags = env->me_flags;
  env->me_flags = (flags & ~MDBX_FATAL_ERROR) | MDBX_ENV_ACTIVE;
  if (rc)
    goto bailout;

  env->me_path = mdbx_strdup(path);
  env->me_dbxs = mdbx_calloc(env->me_maxdbs, sizeof(MDBX_dbx));
  env->me_dbflags = mdbx_calloc(env->me_maxdbs, sizeof(env->me_dbflags[0]));
  env->me_dbiseqs = mdbx_calloc(env->me_maxdbs, sizeof(env->me_dbiseqs[0]));
  if (!(env->me_dbxs && env->me_path && env->me_dbflags && env->me_dbiseqs)) {
    rc = MDBX_ENOMEM;
    goto bailout;
  }
  env->me_dbxs[FREE_DBI].md_cmp = mdbx_cmp_int_ai; /* aligned MDBX_INTEGERKEY */

  int oflags;
  if (F_ISSET(flags, MDBX_RDONLY))
    oflags = O_RDONLY;
  else if (mode != 0)
    oflags = O_RDWR | O_CREAT;
  else
    oflags = O_RDWR;

  rc = mdbx_openfile(dxb_pathname, oflags, mode, &env->me_fd,
                     (env->me_flags & MDBX_EXCLUSIVE) ? true : false);
  if (rc != MDBX_SUCCESS)
    goto bailout;

  const int lck_rc = mdbx_setup_lck(env, lck_pathname, mode);
  if (MDBX_IS_ERROR(lck_rc)) {
    rc = lck_rc;
    goto bailout;
  }

  const int dxb_rc = mdbx_setup_dxb(env, lck_rc);
  if (MDBX_IS_ERROR(dxb_rc)) {
    rc = dxb_rc;
    goto bailout;
  }

  mdbx_debug("opened dbenv %p", (void *)env);
  if (env->me_lck) {
    const unsigned mode_flags =
        MDBX_WRITEMAP | MDBX_NOSYNC | MDBX_NOMETASYNC | MDBX_MAPASYNC;
    if (lck_rc == MDBX_RESULT_TRUE) {
      env->me_lck->mti_envmode = env->me_flags & (mode_flags | MDBX_RDONLY);
      if ((env->me_flags & MDBX_EXCLUSIVE) == 0) {
        /* LY: downgrade lock only if exclusive access not requested.
         *     in case exclusive==1, just leave value as is. */
        rc = mdbx_lck_downgrade(env, true);
        mdbx_debug("lck-downgrade-full: rc %i ", rc);
      } else {
        rc = mdbx_lck_downgrade(env, false);
        mdbx_debug("lck-downgrade-partial: rc %i ", rc);
      }
      if (rc != MDBX_SUCCESS)
        goto bailout;
    } else {
      if ((env->me_flags & MDBX_RDONLY) == 0) {
        while (env->me_lck->mti_envmode == MDBX_RDONLY) {
          if (mdbx_atomic_compare_and_swap32(&env->me_lck->mti_envmode,
                                             MDBX_RDONLY,
                                             env->me_flags & mode_flags))
            break;
          /* TODO: yield/relax cpu */
        }
        if ((env->me_lck->mti_envmode ^ env->me_flags) & mode_flags) {
          mdbx_error("current mode/flags incompatible with requested");
          rc = MDBX_INCOMPATIBLE;
          goto bailout;
        }
      }
    }

    if ((env->me_flags & MDBX_NOTLS) == 0) {
      rc = mdbx_rthc_alloc(&env->me_txkey, &env->me_lck->mti_readers[0],
                           &env->me_lck->mti_readers[env->me_maxreaders]);
      if (unlikely(rc != MDBX_SUCCESS))
        goto bailout;
      env->me_flags |= MDBX_ENV_TXKEY;
    }
  }

  if ((flags & MDBX_RDONLY) == 0) {
    MDBX_txn *txn;
    int tsize = sizeof(MDBX_txn),
        size =
            tsize + env->me_maxdbs * (sizeof(MDBX_db) + sizeof(MDBX_cursor *) +
                                      sizeof(unsigned) + 1);
    if ((env->me_pbuf = mdbx_calloc(2, env->me_psize)) &&
        (txn = mdbx_calloc(1, size))) {
      txn->mt_dbs = (MDBX_db *)((char *)txn + tsize);
      txn->mt_cursors = (MDBX_cursor **)(txn->mt_dbs + env->me_maxdbs);
      txn->mt_dbiseqs = (unsigned *)(txn->mt_cursors + env->me_maxdbs);
      txn->mt_dbflags = (uint8_t *)(txn->mt_dbiseqs + env->me_maxdbs);
      txn->mt_env = env;
      txn->mt_dbxs = env->me_dbxs;
      txn->mt_flags = MDBX_TXN_FINISHED;
      env->me_txn0 = txn;
    } else {
      rc = MDBX_ENOMEM;
    }
  }

#if MDBX_DEBUG
  if (rc == MDBX_SUCCESS) {
    MDBX_meta *meta = mdbx_meta_head(env);
    MDBX_db *db = &meta->mm_dbs[MAIN_DBI];

    mdbx_debug("opened database version %u, pagesize %u",
               (uint8_t)meta->mm_magic_and_version, env->me_psize);
    mdbx_debug("using meta page %" PRIaPGNO ", txn %" PRIaTXN,
               container_of(meta, MDBX_page, mp_data)->mp_pgno,
               mdbx_meta_txnid_fluid(env, meta));
    mdbx_debug("depth: %u", db->md_depth);
    mdbx_debug("entries: %" PRIu64, db->md_entries);
    mdbx_debug("branch pages: %" PRIaPGNO, db->md_branch_pages);
    mdbx_debug("leaf pages: %" PRIaPGNO, db->md_leaf_pages);
    mdbx_debug("overflow pages: %" PRIaPGNO, db->md_overflow_pages);
    mdbx_debug("root: %" PRIaPGNO, db->md_root);
  }
#endif

bailout:
  if (rc) {
    mdbx_env_close0(env);
    env->me_flags = saved_me_flags | MDBX_FATAL_ERROR;
  }
  mdbx_free(lck_pathname);
  return rc;
}

/* Destroy resources from mdbx_env_open(), clear our readers & DBIs */
static void __cold mdbx_env_close0(MDBX_env *env) {
  if (!(env->me_flags & MDBX_ENV_ACTIVE))
    return;
  env->me_flags &= ~MDBX_ENV_ACTIVE;

  /* Doing this here since me_dbxs may not exist during mdbx_env_close */
  if (env->me_dbxs) {
    for (unsigned i = env->me_maxdbs; --i >= CORE_DBS;)
      mdbx_free(env->me_dbxs[i].md_name.iov_base);
    mdbx_free(env->me_dbxs);
  }

  mdbx_free(env->me_pbuf);
  mdbx_free(env->me_dbiseqs);
  mdbx_free(env->me_dbflags);
  mdbx_free(env->me_path);
  mdbx_free(env->me_dirtylist);
  if (env->me_txn0) {
    mdbx_txl_free(env->me_txn0->mt_lifo_reclaimed);
    mdbx_free(env->me_txn0);
  }
  mdbx_pnl_free(env->me_free_pgs);

  if (env->me_flags & MDBX_ENV_TXKEY)
    mdbx_rthc_remove(env->me_txkey);
  if (env->me_live_reader)
    (void)mdbx_rpid_clear(env);

  if (env->me_map) {
    mdbx_munmap(&env->me_dxb_mmap);
#ifdef USE_VALGRIND
    VALGRIND_DISCARD(env->me_valgrind_handle);
    env->me_valgrind_handle = -1;
#endif
  }
  if (env->me_fd != INVALID_HANDLE_VALUE) {
    (void)mdbx_closefile(env->me_fd);
    env->me_fd = INVALID_HANDLE_VALUE;
  }

  if (env->me_lck)
    mdbx_munmap(&env->me_lck_mmap);
  env->me_oldest = nullptr;
  env->me_unsynced_timeout = nullptr;
  env->me_autosync_period = nullptr;
  env->me_unsynced_pages = nullptr;
  env->me_autosync_threshold = nullptr;

  mdbx_lck_destroy(env);
  if (env->me_lfd != INVALID_HANDLE_VALUE) {
    (void)mdbx_closefile(env->me_lfd);
    env->me_lfd = INVALID_HANDLE_VALUE;
  }
  env->me_flags = 0;
}

int __cold mdbx_env_close_ex(MDBX_env *env, int dont_sync) {
  MDBX_page *dp;
  int rc = MDBX_SUCCESS;

  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if ((env->me_flags & (MDBX_RDONLY | MDBX_FATAL_ERROR)) == 0) {
    if (env->me_txn0 && env->me_txn0->mt_owner &&
        env->me_txn0->mt_owner != mdbx_thread_self())
      return MDBX_BUSY;
    if (!dont_sync) {
#if defined(_WIN32) || defined(_WIN64)
      /* On windows, without blocking is impossible to determine whether another
       * process is running a writing transaction or not.
       * Because in the "owner died" condition kernel don't release
       * file lock immediately. */
      rc = mdbx_env_sync_ex(env, true, false);
#else
      rc = mdbx_env_sync_ex(env, true, true);
      rc = (rc == MDBX_BUSY || rc == EAGAIN || rc == EACCES || rc == EBUSY ||
            rc == EWOULDBLOCK)
               ? MDBX_SUCCESS
               : rc;
#endif
    }
  }

  VALGRIND_DESTROY_MEMPOOL(env);
  while ((dp = env->me_dpages) != NULL) {
    ASAN_UNPOISON_MEMORY_REGION(&dp->mp_next, sizeof(dp->mp_next));
    VALGRIND_MAKE_MEM_DEFINED(&dp->mp_next, sizeof(dp->mp_next));
    env->me_dpages = dp->mp_next;
    mdbx_free(dp);
  }

  mdbx_env_close0(env);
  mdbx_ensure(env, mdbx_fastmutex_destroy(&env->me_dbi_lock) == MDBX_SUCCESS);
#if defined(_WIN32) || defined(_WIN64)
  /* me_remap_guard don't have destructor (Slim Reader/Writer Lock) */
  DeleteCriticalSection(&env->me_windowsbug_lock);
#else
  mdbx_ensure(env,
              mdbx_fastmutex_destroy(&env->me_remap_guard) == MDBX_SUCCESS);
#endif /* Windows */

#ifdef MDBX_OSAL_LOCK
  mdbx_ensure(env, mdbx_fastmutex_destroy(&env->me_lckless_stub.wmutex) ==
                       MDBX_SUCCESS);
#endif

  env->me_pid = 0;
  env->me_signature = 0;
  mdbx_free(env);

  return rc;
}

__cold int mdbx_env_close(MDBX_env *env) {
  return mdbx_env_close_ex(env, false);
}

/* Compare two items pointing at aligned unsigned int's. */
static int __hot mdbx_cmp_int_ai(const MDBX_val *a, const MDBX_val *b) {
  mdbx_assert(NULL, a->iov_len == b->iov_len);
  mdbx_assert(NULL, 0 == (uintptr_t)a->iov_base % sizeof(int) &&
                        0 == (uintptr_t)b->iov_base % sizeof(int));
  switch (a->iov_len) {
  case 4:
    return mdbx_cmp2int(*(uint32_t *)a->iov_base, *(uint32_t *)b->iov_base);
  case 8:
    return mdbx_cmp2int(*(uint64_t *)a->iov_base, *(uint64_t *)b->iov_base);
  default:
    mdbx_assert_fail(NULL, "invalid size for INTEGERKEY/INTEGERDUP", mdbx_func_,
                     __LINE__);
    return 0;
  }
}

/* Compare two items pointing at 2-byte aligned unsigned int's. */
static int __hot mdbx_cmp_int_a2(const MDBX_val *a, const MDBX_val *b) {
  mdbx_assert(NULL, a->iov_len == b->iov_len);
  mdbx_assert(NULL, 0 == (uintptr_t)a->iov_base % sizeof(uint16_t) &&
                        0 == (uintptr_t)b->iov_base % sizeof(uint16_t));
#if UNALIGNED_OK
  switch (a->iov_len) {
  case 4:
    return mdbx_cmp2int(*(uint32_t *)a->iov_base, *(uint32_t *)b->iov_base);
  case 8:
    return mdbx_cmp2int(*(uint64_t *)a->iov_base, *(uint64_t *)b->iov_base);
  default:
    mdbx_assert_fail(NULL, "invalid size for INTEGERKEY/INTEGERDUP", mdbx_func_,
                     __LINE__);
    return 0;
  }
#else
  mdbx_assert(NULL, 0 == a->iov_len % sizeof(uint16_t));
  {
    int diff;
    const uint16_t *pa, *pb, *end;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    end = (const uint16_t *)a->iov_base;
    pa = (const uint16_t *)((char *)a->iov_base + a->iov_len);
    pb = (const uint16_t *)((char *)b->iov_base + a->iov_len);
    do {
      diff = *--pa - *--pb;
#else  /* __BYTE_ORDER__ */
    end = (const uint16_t *)((char *)a->iov_base + a->iov_len);
    pa = (const uint16_t *)a->iov_base;
    pb = (const uint16_t *)b->iov_base;
    do {
      diff = *pa++ - *pb++;
#endif /* __BYTE_ORDER__ */
      if (likely(diff != 0))
        break;
    } while (pa != end);
    return diff;
  }
#endif /* UNALIGNED_OK */
}

/* Compare two items pointing at unsigneds of unknown alignment.
 *
 * This is also set as MDBX_INTEGERDUP|MDBX_DUPFIXED's MDBX_dbx.md_dcmp. */
static int __hot mdbx_cmp_int_ua(const MDBX_val *a, const MDBX_val *b) {
  mdbx_assert(NULL, a->iov_len == b->iov_len);
#if UNALIGNED_OK
  switch (a->iov_len) {
  case 4:
    return mdbx_cmp2int(*(uint32_t *)a->iov_base, *(uint32_t *)b->iov_base);
  case 8:
    return mdbx_cmp2int(*(uint64_t *)a->iov_base, *(uint64_t *)b->iov_base);
  default:
    mdbx_assert_fail(NULL, "invalid size for INTEGERKEY/INTEGERDUP", mdbx_func_,
                     __LINE__);
    return 0;
  }
#else
  mdbx_assert(NULL, a->iov_len == sizeof(int) || a->iov_len == sizeof(size_t));
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  {
    int diff;
    const uint8_t *pa, *pb;

    pa = (const uint8_t *)a->iov_base + a->iov_len;
    pb = (const uint8_t *)b->iov_base + a->iov_len;

    do {
      diff = *--pa - *--pb;
      if (likely(diff != 0))
        break;
    } while (pa != a->iov_base);
    return diff;
  }
#else  /* __BYTE_ORDER__ */
  return memcmp(a->iov_base, b->iov_base, a->iov_len);
#endif /* __BYTE_ORDER__ */
#endif /* UNALIGNED_OK */
}

/* Compare two items lexically */
static int __hot mdbx_cmp_memn(const MDBX_val *a, const MDBX_val *b) {
/* LY: assumes that length of keys are NOT equal for most cases,
 * if no then branch-prediction should mitigate the problem */
#if 0
  /* LY: without branch instructions on x86,
   * but isn't best for equal length of keys */
  int diff_len = mdbx_cmp2int(a->iov_len, b->iov_len);
#else
  /* LY: best when length of keys are equal,
   * but got a branch-penalty otherwise */
  if (likely(a->iov_len == b->iov_len))
    return memcmp(a->iov_base, b->iov_base, a->iov_len);
  int diff_len = (a->iov_len < b->iov_len) ? -1 : 1;
#endif
  size_t shortest = (a->iov_len < b->iov_len) ? a->iov_len : b->iov_len;
  int diff_data = memcmp(a->iov_base, b->iov_base, shortest);
  return likely(diff_data) ? diff_data : diff_len;
}

/* Compare two items in reverse byte order */
static int __hot mdbx_cmp_memnr(const MDBX_val *a, const MDBX_val *b) {
  const uint8_t *pa, *pb, *end;

  pa = (const uint8_t *)a->iov_base + a->iov_len;
  pb = (const uint8_t *)b->iov_base + b->iov_len;
  size_t minlen = (a->iov_len < b->iov_len) ? a->iov_len : b->iov_len;
  end = pa - minlen;

  while (pa != end) {
    int diff = *--pa - *--pb;
    if (likely(diff))
      return diff;
  }
  return mdbx_cmp2int(a->iov_len, b->iov_len);
}

/* Search for key within a page, using binary search.
 * Returns the smallest entry larger or equal to the key.
 * If exactp is non-null, stores whether the found entry was an exact match
 * in *exactp (1 or 0).
 * Updates the cursor index with the index of the found entry.
 * If no entry larger or equal to the key is found, returns NULL. */
static MDBX_node *__hot mdbx_node_search(MDBX_cursor *mc, MDBX_val *key,
                                         int *exactp) {
  int low, high;
  int rc = 0;
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  MDBX_node *node = NULL;
  MDBX_val nodekey;
  MDBX_cmp_func *cmp;
  DKBUF;

  const unsigned nkeys = NUMKEYS(mp);

  mdbx_debug("searching %u keys in %s %spage %" PRIaPGNO, nkeys,
             IS_LEAF(mp) ? "leaf" : "branch", IS_SUBP(mp) ? "sub-" : "",
             mp->mp_pgno);

  low = IS_LEAF(mp) ? 0 : 1;
  high = nkeys - 1;
  cmp = mc->mc_dbx->md_cmp;

  /* Branch pages have no data, so if using integer keys,
   * alignment is guaranteed. Use faster mdbx_cmp_int_ai.
   */
  if (cmp == mdbx_cmp_int_a2 && IS_BRANCH(mp))
    cmp = mdbx_cmp_int_ai;

  unsigned i = 0;
  if (IS_LEAF2(mp)) {
    nodekey.iov_len = mc->mc_db->md_xsize;
    node = NODEPTR(mp, 0); /* fake */
    while (low <= high) {
      i = (low + high) >> 1;
      nodekey.iov_base = LEAF2KEY(mp, i, nodekey.iov_len);
      rc = cmp(key, &nodekey);
      mdbx_debug("found leaf index %u [%s], rc = %i", i, DKEY(&nodekey), rc);
      if (rc == 0)
        break;
      if (rc > 0)
        low = i + 1;
      else
        high = i - 1;
    }
  } else {
    while (low <= high) {
      i = (low + high) >> 1;

      node = NODEPTR(mp, i);
      nodekey.iov_len = NODEKSZ(node);
      nodekey.iov_base = NODEKEY(node);

      rc = cmp(key, &nodekey);
      if (IS_LEAF(mp))
        mdbx_debug("found leaf index %u [%s], rc = %i", i, DKEY(&nodekey), rc);
      else
        mdbx_debug("found branch index %u [%s -> %" PRIaPGNO "], rc = %i", i,
                   DKEY(&nodekey), NODEPGNO(node), rc);
      if (rc == 0)
        break;
      if (rc > 0)
        low = i + 1;
      else
        high = i - 1;
    }
  }

  if (rc > 0) /* Found entry is less than the key. */
    i++;      /* Skip to get the smallest entry larger than key. */

  if (exactp)
    *exactp = (rc == 0 && nkeys > 0);
  /* store the key index */
  mdbx_cassert(mc, i <= UINT16_MAX);
  mc->mc_ki[mc->mc_top] = (indx_t)i;
  if (i >= nkeys)
    /* There is no entry larger or equal to the key. */
    return NULL;

  /* nodeptr is fake for LEAF2 */
  return IS_LEAF2(mp) ? node : NODEPTR(mp, i);
}

#if 0 /* unused for now */
static void mdbx_cursor_adjust(MDBX_cursor *mc, func) {
  MDBX_cursor *m2;

  for (m2 = mc->mc_txn->mt_cursors[mc->mc_dbi]; m2; m2 = m2->mc_next) {
    if (m2->mc_pg[m2->mc_top] == mc->mc_pg[mc->mc_top]) {
      func(mc, m2);
    }
  }
}
#endif

/* Pop a page off the top of the cursor's stack. */
static void mdbx_cursor_pop(MDBX_cursor *mc) {
  if (mc->mc_snum) {
    mdbx_debug("popped page %" PRIaPGNO " off db %d cursor %p",
               mc->mc_pg[mc->mc_top]->mp_pgno, DDBI(mc), (void *)mc);

    mc->mc_snum--;
    if (mc->mc_snum) {
      mc->mc_top--;
    } else {
      mc->mc_flags &= ~C_INITIALIZED;
    }
  }
}

/* Push a page onto the top of the cursor's stack.
 * Set MDBX_TXN_ERROR on failure. */
static int mdbx_cursor_push(MDBX_cursor *mc, MDBX_page *mp) {
  mdbx_debug("pushing page %" PRIaPGNO " on db %d cursor %p", mp->mp_pgno,
             DDBI(mc), (void *)mc);

  if (unlikely(mc->mc_snum >= CURSOR_STACK)) {
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_CURSOR_FULL;
  }

  mdbx_cassert(mc, mc->mc_snum < UINT16_MAX);
  mc->mc_top = mc->mc_snum++;
  mc->mc_pg[mc->mc_top] = mp;
  mc->mc_ki[mc->mc_top] = 0;

  return MDBX_SUCCESS;
}

/* Find the address of the page corresponding to a given page number.
 * Set MDBX_TXN_ERROR on failure.
 *
 * [in] mc    the cursor accessing the page.
 * [in] pgno  the page number for the page to retrieve.
 * [out] ret  address of a pointer where the page's address will be
 *            stored.
 * [out] lvl  dirtylist inheritance level of found page. 1=current txn,
 *            0=mapped page.
 *
 * Returns 0 on success, non-zero on failure. */
__hot static int mdbx_page_get(MDBX_cursor *mc, pgno_t pgno, MDBX_page **ret,
                               int *lvl) {
  MDBX_txn *txn = mc->mc_txn;
  MDBX_env *env = txn->mt_env;
  MDBX_page *p = NULL;
  int level;

  if (!(txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_WRITEMAP))) {
    MDBX_txn *tx2 = txn;
    level = 1;
    do {
      MDBX_DPL dl = tx2->mt_rw_dirtylist;
      /* Spilled pages were dirtied in this txn and flushed
       * because the dirty list got full. Bring this page
       * back in from the map (but don't unspill it here,
       * leave that unless page_touch happens again). */
      if (tx2->mt_spill_pages) {
        pgno_t pn = pgno << 1;
        unsigned x = mdbx_pnl_search(tx2->mt_spill_pages, pn);
        if (x <= MDBX_PNL_SIZE(tx2->mt_spill_pages) &&
            tx2->mt_spill_pages[x] == pn)
          goto mapped;
      }
      if (dl->length) {
        unsigned y = mdbx_dpl_search(dl, pgno);
        if (y <= dl->length && dl[y].pgno == pgno) {
          p = dl[y].ptr;
          goto done;
        }
      }
      level++;
    } while ((tx2 = tx2->mt_parent) != NULL);
  }

  if (unlikely(pgno >= txn->mt_next_pgno)) {
    mdbx_debug("page %" PRIaPGNO " not found", pgno);
    txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_PAGE_NOTFOUND;
  }
  level = 0;

mapped:
  p = pgno2page(env, pgno);

done:
  if (unlikely(p->mp_pgno != pgno)) {
    mdbx_error("mismatch pgno %" PRIaPGNO " (actual) != %" PRIaPGNO
               " (expected)",
               p->mp_pgno, pgno);
    return MDBX_CORRUPTED;
  }

  if (unlikely(p->mp_upper < p->mp_lower ||
               PAGEHDRSZ + p->mp_upper > env->me_psize) &&
      !IS_OVERFLOW(p)) {
    mdbx_error("invalid page lower(%u)/upper(%u), pg-limit %u", p->mp_lower,
               p->mp_upper, env->me_psize - PAGEHDRSZ);
    return MDBX_CORRUPTED;
  }
  /* TODO: more checks here, including p->mp_validator */

  *ret = p;
  if (lvl)
    *lvl = level;
  return MDBX_SUCCESS;
}

/* Finish mdbx_page_search() / mdbx_page_search_lowest().
 * The cursor is at the root page, set up the rest of it. */
__hot static int mdbx_page_search_root(MDBX_cursor *mc, MDBX_val *key,
                                       int flags) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  int rc;
  DKBUF;

  while (IS_BRANCH(mp)) {
    MDBX_node *node;
    indx_t i;

    mdbx_debug("branch page %" PRIaPGNO " has %u keys", mp->mp_pgno,
               NUMKEYS(mp));
    /* Don't assert on branch pages in the FreeDB. We can get here
     * while in the process of rebalancing a FreeDB branch page; we must
     * let that proceed. ITS#8336 */
    mdbx_cassert(mc, !mc->mc_dbi || NUMKEYS(mp) > 1);
    mdbx_debug("found index 0 to page %" PRIaPGNO, NODEPGNO(NODEPTR(mp, 0)));

    if (flags & (MDBX_PS_FIRST | MDBX_PS_LAST)) {
      i = 0;
      if (flags & MDBX_PS_LAST) {
        i = NUMKEYS(mp) - 1;
        /* if already init'd, see if we're already in right place */
        if (mc->mc_flags & C_INITIALIZED) {
          if (mc->mc_ki[mc->mc_top] == i) {
            mc->mc_top = mc->mc_snum++;
            mp = mc->mc_pg[mc->mc_top];
            goto ready;
          }
        }
      }
    } else {
      int exact;
      node = mdbx_node_search(mc, key, &exact);
      if (node == NULL)
        i = NUMKEYS(mp) - 1;
      else {
        i = mc->mc_ki[mc->mc_top];
        if (!exact) {
          mdbx_cassert(mc, i > 0);
          i--;
        }
      }
      mdbx_debug("following index %u for key [%s]", i, DKEY(key));
    }

    mdbx_cassert(mc, i < NUMKEYS(mp));
    node = NODEPTR(mp, i);

    if (unlikely((rc = mdbx_page_get(mc, NODEPGNO(node), &mp, NULL)) != 0))
      return rc;

    mc->mc_ki[mc->mc_top] = i;
    if (unlikely(rc = mdbx_cursor_push(mc, mp)))
      return rc;

  ready:
    if (flags & MDBX_PS_MODIFY) {
      if (unlikely((rc = mdbx_page_touch(mc)) != 0))
        return rc;
      mp = mc->mc_pg[mc->mc_top];
    }
  }

  if (unlikely(!IS_LEAF(mp))) {
    mdbx_debug("internal error, index points to a page with 0x%02x flags!?",
               mp->mp_flags);
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_CORRUPTED;
  }

  mdbx_debug("found leaf page %" PRIaPGNO " for key [%s]", mp->mp_pgno,
             DKEY(key));
  mc->mc_flags |= C_INITIALIZED;
  mc->mc_flags &= ~C_EOF;

  return MDBX_SUCCESS;
}

/* Search for the lowest key under the current branch page.
 * This just bypasses a NUMKEYS check in the current page
 * before calling mdbx_page_search_root(), because the callers
 * are all in situations where the current page is known to
 * be underfilled. */
__hot static int mdbx_page_search_lowest(MDBX_cursor *mc) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  mdbx_cassert(mc, IS_BRANCH(mp));
  MDBX_node *node = NODEPTR(mp, 0);
  int rc;

  if (unlikely((rc = mdbx_page_get(mc, NODEPGNO(node), &mp, NULL)) != 0))
    return rc;

  mc->mc_ki[mc->mc_top] = 0;
  if (unlikely(rc = mdbx_cursor_push(mc, mp)))
    return rc;
  return mdbx_page_search_root(mc, NULL, MDBX_PS_FIRST);
}

/* Search for the page a given key should be in.
 * Push it and its parent pages on the cursor stack.
 *
 * [in,out] mc  the cursor for this operation.
 * [in] key     the key to search for, or NULL for first/last page.
 * [in] flags   If MDBX_PS_MODIFY is set, visited pages in the DB
 *              are touched (updated with new page numbers).
 *              If MDBX_PS_FIRST or MDBX_PS_LAST is set, find first or last
 * leaf.
 *              This is used by mdbx_cursor_first() and mdbx_cursor_last().
 *              If MDBX_PS_ROOTONLY set, just fetch root node, no further
 *              lookups.
 *
 * Returns 0 on success, non-zero on failure. */
__hot static int mdbx_page_search(MDBX_cursor *mc, MDBX_val *key, int flags) {
  int rc;
  pgno_t root;

  /* Make sure the txn is still viable, then find the root from
   * the txn's db table and set it as the root of the cursor's stack. */
  if (unlikely(mc->mc_txn->mt_flags & MDBX_TXN_BLOCKED)) {
    mdbx_debug("transaction has failed, must abort");
    return MDBX_BAD_TXN;
  }

  /* Make sure we're using an up-to-date root */
  if (unlikely(*mc->mc_dbflag & DB_STALE)) {
    MDBX_cursor mc2;
    if (unlikely(TXN_DBI_CHANGED(mc->mc_txn, mc->mc_dbi)))
      return MDBX_BAD_DBI;
    rc = mdbx_cursor_init(&mc2, mc->mc_txn, MAIN_DBI);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    rc = mdbx_page_search(&mc2, &mc->mc_dbx->md_name, 0);
    if (unlikely(rc != MDBX_SUCCESS))
      return (rc == MDBX_NOTFOUND) ? MDBX_BAD_DBI : rc;
    {
      MDBX_val data;
      int exact = 0;
      MDBX_node *leaf = mdbx_node_search(&mc2, &mc->mc_dbx->md_name, &exact);
      if (!exact)
        return MDBX_BAD_DBI;
      if (unlikely((leaf->mn_flags & (F_DUPDATA | F_SUBDATA)) != F_SUBDATA))
        return MDBX_INCOMPATIBLE; /* not a named DB */
      rc = mdbx_node_read(&mc2, leaf, &data);
      if (rc)
        return rc;

      uint16_t md_flags;
      memcpy(&md_flags, ((char *)data.iov_base + offsetof(MDBX_db, md_flags)),
             sizeof(uint16_t));
      /* The txn may not know this DBI, or another process may
       * have dropped and recreated the DB with other flags. */
      if (unlikely((mc->mc_db->md_flags & PERSISTENT_FLAGS) != md_flags))
        return MDBX_INCOMPATIBLE;
      memcpy(mc->mc_db, data.iov_base, sizeof(MDBX_db));
    }
    *mc->mc_dbflag &= ~DB_STALE;
  }
  root = mc->mc_db->md_root;

  if (unlikely(root == P_INVALID)) { /* Tree is empty. */
    mdbx_debug("tree is empty");
    return MDBX_NOTFOUND;
  }

  mdbx_cassert(mc, root >= NUM_METAS);
  if (!mc->mc_pg[0] || mc->mc_pg[0]->mp_pgno != root)
    if (unlikely((rc = mdbx_page_get(mc, root, &mc->mc_pg[0], NULL)) != 0))
      return rc;

  mc->mc_snum = 1;
  mc->mc_top = 0;

  mdbx_debug("db %d root page %" PRIaPGNO " has flags 0x%X", DDBI(mc), root,
             mc->mc_pg[0]->mp_flags);

  if (flags & MDBX_PS_MODIFY) {
    if (unlikely(rc = mdbx_page_touch(mc)))
      return rc;
  }

  if (flags & MDBX_PS_ROOTONLY)
    return MDBX_SUCCESS;

  return mdbx_page_search_root(mc, key, flags);
}

static int mdbx_ovpage_free(MDBX_cursor *mc, MDBX_page *mp) {
  MDBX_txn *txn = mc->mc_txn;
  pgno_t pg = mp->mp_pgno;
  unsigned x = 0, ovpages = mp->mp_pages;
  MDBX_env *env = txn->mt_env;
  MDBX_PNL sl = txn->mt_spill_pages;
  pgno_t pn = pg << 1;
  int rc;

  mdbx_cassert(mc, (mc->mc_flags & C_GCFREEZE) == 0);
  mdbx_cassert(mc, IS_OVERFLOW(mp));
  mdbx_debug("free ov page %" PRIaPGNO " (%u)", pg, ovpages);

  if (mdbx_audit_enabled() && env->me_reclaimed_pglist) {
    mdbx_cassert(mc, mdbx_pnl_check(env->me_reclaimed_pglist, true));
    const unsigned a = mdbx_pnl_search(env->me_reclaimed_pglist, pg);
    mdbx_cassert(mc, a > MDBX_PNL_SIZE(env->me_reclaimed_pglist) ||
                         env->me_reclaimed_pglist[a] != pg);
    if (a <= MDBX_PNL_SIZE(env->me_reclaimed_pglist) &&
        unlikely(env->me_reclaimed_pglist[a] == pg))
      return MDBX_PROBLEM;

    if (ovpages > 1) {
      const unsigned b =
          mdbx_pnl_search(env->me_reclaimed_pglist, pg + ovpages - 1);
      mdbx_cassert(mc, a == b);
      if (unlikely(a != b))
        return MDBX_PROBLEM;
    }
  }

  /* If the page is dirty or on the spill list we just acquired it,
   * so we should give it back to our current free list, if any.
   * Otherwise put it onto the list of pages we freed in this txn.
   *
   * Won't create me_reclaimed_pglist: me_last_reclaimed must be inited along
   * with it.
   * Unsupported in nested txns: They would need to hide the page
   * range in ancestor txns' dirty and spilled lists. */
  if (env->me_reclaimed_pglist && !txn->mt_parent &&
      (IS_DIRTY(mp) ||
       (sl && (x = mdbx_pnl_search(sl, pn)) <= MDBX_PNL_SIZE(sl) &&
        sl[x] == pn))) {
    unsigned i, j;
    pgno_t *mop;
    MDBX_DP *dl, ix, iy;
    rc = mdbx_pnl_need(&env->me_reclaimed_pglist, ovpages);
    if (unlikely(rc))
      return rc;

    if (!IS_DIRTY(mp)) {
      /* This page is no longer spilled */
      if (x == MDBX_PNL_SIZE(sl))
        MDBX_PNL_SIZE(sl)--;
      else
        sl[x] |= 1;
      goto release;
    }
    /* Remove from dirty list */
    dl = txn->mt_rw_dirtylist;
    x = dl->length--;
    for (ix = dl[x]; ix.ptr != mp; ix = iy) {
      if (likely(x > 1)) {
        x--;
        iy = dl[x];
        dl[x] = ix;
      } else {
        mdbx_cassert(mc, x > 1);
        mdbx_error("not found page 0x%p #%" PRIaPGNO " in the dirtylist", mp,
                   mp->mp_pgno);
        j = dl->length += 1;
        dl[j] = ix; /* Unsorted. OK when MDBX_TXN_ERROR. */
        txn->mt_flags |= MDBX_TXN_ERROR;
        return MDBX_PROBLEM;
      }
    }
    txn->mt_dirtyroom++;
    if (!(env->me_flags & MDBX_WRITEMAP))
      mdbx_dpage_free(env, mp, IS_OVERFLOW(mp) ? mp->mp_pages : 1);
  release:
    /* Insert in me_reclaimed_pglist */
    mop = env->me_reclaimed_pglist;
    j = MDBX_PNL_SIZE(mop) + ovpages;
    for (i = MDBX_PNL_SIZE(mop); i && MDBX_PNL_DISORDERED(mop[i], pg);)
      mop[j--] = mop[i--];
    MDBX_PNL_SIZE(mop) += ovpages;

    pgno_t n = MDBX_PNL_ASCENDING ? pg + ovpages : pg;
    while (j > i)
      mop[j--] = MDBX_PNL_ASCENDING ? --n : n++;
    mdbx_tassert(txn, mdbx_pnl_check(env->me_reclaimed_pglist, true));
  } else {
    rc = mdbx_pnl_append_range(&txn->mt_befree_pages, pg, ovpages);
    if (unlikely(rc))
      return rc;
  }

  mc->mc_db->md_overflow_pages -= ovpages;
  if (unlikely(mc->mc_flags & C_SUB)) {
    MDBX_db *outer = mdbx_outer_db(mc);
    outer->md_overflow_pages -= ovpages;
  }
  return 0;
}

/* Return the data associated with a given node.
 *
 * [in] mc      The cursor for this operation.
 * [in] leaf    The node being read.
 * [out] data   Updated to point to the node's data.
 *
 * Returns 0 on success, non-zero on failure. */
static __inline int mdbx_node_read(MDBX_cursor *mc, MDBX_node *leaf,
                                   MDBX_val *data) {
  MDBX_page *omp; /* overflow page */
  pgno_t pgno;
  int rc;

  if (!F_ISSET(leaf->mn_flags, F_BIGDATA)) {
    data->iov_len = NODEDSZ(leaf);
    data->iov_base = NODEDATA(leaf);
    return MDBX_SUCCESS;
  }

  /* Read overflow data. */
  data->iov_len = NODEDSZ(leaf);
  memcpy(&pgno, NODEDATA(leaf), sizeof(pgno));
  if (unlikely((rc = mdbx_page_get(mc, pgno, &omp, NULL)) != 0)) {
    mdbx_debug("read overflow page %" PRIaPGNO " failed", pgno);
    return rc;
  }
  data->iov_base = PAGEDATA(omp);

  return MDBX_SUCCESS;
}

int mdbx_get(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data) {
  int exact = 0;
  DKBUF;

  mdbx_debug("===> get db %u key [%s]", dbi, DKEY(key));

  if (unlikely(!key || !data || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  return mdbx_cursor_set(&cx.outer, key, data, MDBX_SET, &exact);
}

int mdbx_get2(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data) {
  int exact = 0;
  DKBUF;

  mdbx_debug("===> get db %u key [%s]", dbi, DKEY(key));

  if (unlikely(!key || !data || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  const int op =
      (txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT) ? MDBX_GET_BOTH : MDBX_SET_KEY;
  rc = mdbx_cursor_set(&cx.outer, key, data, op, &exact);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  return exact ? MDBX_SUCCESS : MDBX_RESULT_TRUE;
}

/* Find a sibling for a page.
 * Replaces the page at the top of the cursor's stack with the specified
 * sibling, if one exists.
 *
 * [in] mc          The cursor for this operation.
 * [in] move_right  Non-zero if the right sibling is requested,
 *                  otherwise the left sibling.
 *
 * Returns 0 on success, non-zero on failure. */
static int mdbx_cursor_sibling(MDBX_cursor *mc, int move_right) {
  int rc;
  MDBX_node *indx;
  MDBX_page *mp;

  if (unlikely(mc->mc_snum < 2)) {
    return MDBX_NOTFOUND; /* root has no siblings */
  }

  mdbx_cursor_pop(mc);
  mdbx_debug("parent page is page %" PRIaPGNO ", index %u",
             mc->mc_pg[mc->mc_top]->mp_pgno, mc->mc_ki[mc->mc_top]);

  if (move_right
          ? (mc->mc_ki[mc->mc_top] + 1u >= NUMKEYS(mc->mc_pg[mc->mc_top]))
          : (mc->mc_ki[mc->mc_top] == 0)) {
    mdbx_debug("no more keys left, moving to %s sibling",
               move_right ? "right" : "left");
    if (unlikely((rc = mdbx_cursor_sibling(mc, move_right)) != MDBX_SUCCESS)) {
      /* undo cursor_pop before returning */
      mc->mc_top++;
      mc->mc_snum++;
      return rc;
    }
  } else {
    if (move_right)
      mc->mc_ki[mc->mc_top]++;
    else
      mc->mc_ki[mc->mc_top]--;
    mdbx_debug("just moving to %s index key %u", move_right ? "right" : "left",
               mc->mc_ki[mc->mc_top]);
  }
  mdbx_cassert(mc, IS_BRANCH(mc->mc_pg[mc->mc_top]));

  indx = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
  if (unlikely((rc = mdbx_page_get(mc, NODEPGNO(indx), &mp, NULL)) != 0)) {
    /* mc will be inconsistent if caller does mc_snum++ as above */
    mc->mc_flags &= ~(C_INITIALIZED | C_EOF);
    return rc;
  }

  rc = mdbx_cursor_push(mc, mp);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  if (!move_right)
    mc->mc_ki[mc->mc_top] = NUMKEYS(mp) - 1;

  return MDBX_SUCCESS;
}

/* Move the cursor to the next data item. */
static int mdbx_cursor_next(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                            MDBX_cursor_op op) {
  MDBX_page *mp;
  MDBX_node *leaf;
  int rc;

  if ((mc->mc_flags & C_DEL) && op == MDBX_NEXT_DUP)
    return MDBX_NOTFOUND;

  if (!(mc->mc_flags & C_INITIALIZED))
    return mdbx_cursor_first(mc, key, data);

  mp = mc->mc_pg[mc->mc_top];
  if (mc->mc_flags & C_EOF) {
    if (mc->mc_ki[mc->mc_top] + 1u >= NUMKEYS(mp))
      return MDBX_NOTFOUND;
    mc->mc_flags ^= C_EOF;
  }

  if (mc->mc_db->md_flags & MDBX_DUPSORT) {
    leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      if (op == MDBX_NEXT || op == MDBX_NEXT_DUP) {
        rc =
            mdbx_cursor_next(&mc->mc_xcursor->mx_cursor, data, NULL, MDBX_NEXT);
        if (op != MDBX_NEXT || rc != MDBX_NOTFOUND) {
          if (likely(rc == MDBX_SUCCESS))
            MDBX_GET_MAYNULL_KEYPTR(leaf, key);
          return rc;
        }
      }
    } else {
      mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);
      if (op == MDBX_NEXT_DUP)
        return MDBX_NOTFOUND;
    }
  }

  mdbx_debug("cursor_next: top page is %" PRIaPGNO " in cursor %p", mp->mp_pgno,
             (void *)mc);
  if (mc->mc_flags & C_DEL) {
    mc->mc_flags ^= C_DEL;
    goto skip;
  }

  if (mc->mc_ki[mc->mc_top] + 1u >= NUMKEYS(mp)) {
    mdbx_debug("=====> move to next sibling page");
    if (unlikely((rc = mdbx_cursor_sibling(mc, 1)) != MDBX_SUCCESS)) {
      mc->mc_flags |= C_EOF;
      return rc;
    }
    mp = mc->mc_pg[mc->mc_top];
    mdbx_debug("next page is %" PRIaPGNO ", key index %u", mp->mp_pgno,
               mc->mc_ki[mc->mc_top]);
  } else
    mc->mc_ki[mc->mc_top]++;

skip:
  mdbx_debug("==> cursor points to page %" PRIaPGNO
             " with %u keys, key index %u",
             mp->mp_pgno, NUMKEYS(mp), mc->mc_ki[mc->mc_top]);

  if (IS_LEAF2(mp)) {
    if (likely(key)) {
      key->iov_len = mc->mc_db->md_xsize;
      key->iov_base = LEAF2KEY(mp, mc->mc_ki[mc->mc_top], key->iov_len);
    }
    return MDBX_SUCCESS;
  }

  mdbx_cassert(mc, IS_LEAF(mp));
  leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);

  if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
    rc = mdbx_xcursor_init1(mc, leaf);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  if (data) {
    if (unlikely((rc = mdbx_node_read(mc, leaf, data)) != MDBX_SUCCESS))
      return rc;

    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      rc = mdbx_cursor_first(&mc->mc_xcursor->mx_cursor, data, NULL);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
  }

  MDBX_GET_MAYNULL_KEYPTR(leaf, key);
  return MDBX_SUCCESS;
}

/* Move the cursor to the previous data item. */
static int mdbx_cursor_prev(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                            MDBX_cursor_op op) {
  MDBX_page *mp;
  MDBX_node *leaf;
  int rc;

  if ((mc->mc_flags & C_DEL) && op == MDBX_PREV_DUP)
    return MDBX_NOTFOUND;

  if (!(mc->mc_flags & C_INITIALIZED)) {
    rc = mdbx_cursor_last(mc, key, data);
    if (unlikely(rc))
      return rc;
    mc->mc_ki[mc->mc_top]++;
  }

  mp = mc->mc_pg[mc->mc_top];
  if ((mc->mc_db->md_flags & MDBX_DUPSORT) &&
      mc->mc_ki[mc->mc_top] < NUMKEYS(mp)) {
    leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      if (op == MDBX_PREV || op == MDBX_PREV_DUP) {
        rc =
            mdbx_cursor_prev(&mc->mc_xcursor->mx_cursor, data, NULL, MDBX_PREV);
        if (op != MDBX_PREV || rc != MDBX_NOTFOUND) {
          if (likely(rc == MDBX_SUCCESS)) {
            MDBX_GET_MAYNULL_KEYPTR(leaf, key);
            mc->mc_flags &= ~C_EOF;
          }
          return rc;
        }
      }
    } else {
      mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);
      if (op == MDBX_PREV_DUP)
        return MDBX_NOTFOUND;
    }
  }

  mdbx_debug("cursor_prev: top page is %" PRIaPGNO " in cursor %p", mp->mp_pgno,
             (void *)mc);

  mc->mc_flags &= ~(C_EOF | C_DEL);

  if (mc->mc_ki[mc->mc_top] == 0) {
    mdbx_debug("=====> move to prev sibling page");
    if ((rc = mdbx_cursor_sibling(mc, 0)) != MDBX_SUCCESS) {
      return rc;
    }
    mp = mc->mc_pg[mc->mc_top];
    mc->mc_ki[mc->mc_top] = NUMKEYS(mp) - 1;
    mdbx_debug("prev page is %" PRIaPGNO ", key index %u", mp->mp_pgno,
               mc->mc_ki[mc->mc_top]);
  } else
    mc->mc_ki[mc->mc_top]--;

  mdbx_debug("==> cursor points to page %" PRIaPGNO
             " with %u keys, key index %u",
             mp->mp_pgno, NUMKEYS(mp), mc->mc_ki[mc->mc_top]);

  if (IS_LEAF2(mp)) {
    if (likely(key)) {
      key->iov_len = mc->mc_db->md_xsize;
      key->iov_base = LEAF2KEY(mp, mc->mc_ki[mc->mc_top], key->iov_len);
    }
    return MDBX_SUCCESS;
  }

  mdbx_cassert(mc, IS_LEAF(mp));
  leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);

  if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
    rc = mdbx_xcursor_init1(mc, leaf);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  if (data) {
    if (unlikely((rc = mdbx_node_read(mc, leaf, data)) != MDBX_SUCCESS))
      return rc;

    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      rc = mdbx_cursor_last(&mc->mc_xcursor->mx_cursor, data, NULL);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
  }

  MDBX_GET_MAYNULL_KEYPTR(leaf, key);
  return MDBX_SUCCESS;
}

/* Set the cursor on a specific data item. */
__hot static int mdbx_cursor_set(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                                 MDBX_cursor_op op, int *exactp) {
  int rc;
  MDBX_page *mp;
  MDBX_node *leaf = NULL;
  DKBUF;

  if ((mc->mc_db->md_flags & MDBX_INTEGERKEY) &&
      unlikely(key->iov_len != sizeof(uint32_t) &&
               key->iov_len != sizeof(uint64_t))) {
    mdbx_cassert(mc, !"key-size is invalid for MDBX_INTEGERKEY");
    return MDBX_BAD_VALSIZE;
  }

  if (mc->mc_xcursor)
    mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);

  /* See if we're already on the right page */
  if (mc->mc_flags & C_INITIALIZED) {
    MDBX_val nodekey;

    mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]));
    mp = mc->mc_pg[mc->mc_top];
    if (!NUMKEYS(mp)) {
      mc->mc_ki[mc->mc_top] = 0;
      return MDBX_NOTFOUND;
    }
    if (IS_LEAF2(mp)) {
      nodekey.iov_len = mc->mc_db->md_xsize;
      nodekey.iov_base = LEAF2KEY(mp, 0, nodekey.iov_len);
    } else {
      leaf = NODEPTR(mp, 0);
      MDBX_GET_KEYVALUE(leaf, nodekey);
    }
    rc = mc->mc_dbx->md_cmp(key, &nodekey);
    if (unlikely(rc == 0)) {
      /* Probably happens rarely, but first node on the page
       * was the one we wanted. */
      mc->mc_ki[mc->mc_top] = 0;
      if (exactp)
        *exactp = 1;
      goto set1;
    }
    if (rc > 0) {
      const unsigned nkeys = NUMKEYS(mp);
      unsigned i;
      if (nkeys > 1) {
        if (IS_LEAF2(mp)) {
          nodekey.iov_base = LEAF2KEY(mp, nkeys - 1, nodekey.iov_len);
        } else {
          leaf = NODEPTR(mp, nkeys - 1);
          MDBX_GET_KEYVALUE(leaf, nodekey);
        }
        rc = mc->mc_dbx->md_cmp(key, &nodekey);
        if (rc == 0) {
          /* last node was the one we wanted */
          mdbx_cassert(mc, nkeys >= 1 && nkeys <= UINT16_MAX + 1);
          mc->mc_ki[mc->mc_top] = (indx_t)(nkeys - 1);
          if (exactp)
            *exactp = 1;
          goto set1;
        }
        if (rc < 0) {
          if (mc->mc_ki[mc->mc_top] < NUMKEYS(mp)) {
            /* This is definitely the right page, skip search_page */
            if (IS_LEAF2(mp)) {
              nodekey.iov_base =
                  LEAF2KEY(mp, mc->mc_ki[mc->mc_top], nodekey.iov_len);
            } else {
              leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
              MDBX_GET_KEYVALUE(leaf, nodekey);
            }
            rc = mc->mc_dbx->md_cmp(key, &nodekey);
            if (rc == 0) {
              /* current node was the one we wanted */
              if (exactp)
                *exactp = 1;
              goto set1;
            }
          }
          rc = 0;
          mc->mc_flags &= ~C_EOF;
          goto set2;
        }
      }
      /* If any parents have right-sibs, search.
       * Otherwise, there's nothing further. */
      for (i = 0; i < mc->mc_top; i++)
        if (mc->mc_ki[i] < NUMKEYS(mc->mc_pg[i]) - 1)
          break;
      if (i == mc->mc_top) {
        /* There are no other pages */
        mdbx_cassert(mc, nkeys <= UINT16_MAX);
        mc->mc_ki[mc->mc_top] = (uint16_t)nkeys;
        return MDBX_NOTFOUND;
      }
    }
    if (!mc->mc_top) {
      /* There are no other pages */
      mc->mc_ki[mc->mc_top] = 0;
      if (op == MDBX_SET_RANGE && !exactp) {
        rc = 0;
        goto set1;
      } else
        return MDBX_NOTFOUND;
    }
  } else {
    mc->mc_pg[0] = 0;
  }

  rc = mdbx_page_search(mc, key, 0);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  mp = mc->mc_pg[mc->mc_top];
  mdbx_cassert(mc, IS_LEAF(mp));

set2:
  leaf = mdbx_node_search(mc, key, exactp);
  if (exactp != NULL && !*exactp) {
    /* MDBX_SET specified and not an exact match. */
    return MDBX_NOTFOUND;
  }

  if (leaf == NULL) {
    mdbx_debug("===> inexact leaf not found, goto sibling");
    if (unlikely((rc = mdbx_cursor_sibling(mc, 1)) != MDBX_SUCCESS)) {
      mc->mc_flags |= C_EOF;
      return rc; /* no entries matched */
    }
    mp = mc->mc_pg[mc->mc_top];
    mdbx_cassert(mc, IS_LEAF(mp));
    leaf = NODEPTR(mp, 0);
  }

set1:
  mc->mc_flags |= C_INITIALIZED;
  mc->mc_flags &= ~C_EOF;

  if (IS_LEAF2(mp)) {
    if (op == MDBX_SET_RANGE || op == MDBX_SET_KEY) {
      key->iov_len = mc->mc_db->md_xsize;
      key->iov_base = LEAF2KEY(mp, mc->mc_ki[mc->mc_top], key->iov_len);
    }
    return MDBX_SUCCESS;
  }

  if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
    rc = mdbx_xcursor_init1(mc, leaf);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  if (likely(data)) {
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      if (op == MDBX_SET || op == MDBX_SET_KEY || op == MDBX_SET_RANGE) {
        rc = mdbx_cursor_first(&mc->mc_xcursor->mx_cursor, data, NULL);
      } else {
        int ex2, *ex2p;
        if (op == MDBX_GET_BOTH) {
          ex2p = &ex2;
          ex2 = 0;
        } else {
          ex2p = NULL;
        }
        rc = mdbx_cursor_set(&mc->mc_xcursor->mx_cursor, data, NULL,
                             MDBX_SET_RANGE, ex2p);
        if (unlikely(rc != MDBX_SUCCESS))
          return rc;
      }
    } else if (op == MDBX_GET_BOTH || op == MDBX_GET_BOTH_RANGE) {
      MDBX_val olddata;
      if (unlikely((rc = mdbx_node_read(mc, leaf, &olddata)) != MDBX_SUCCESS))
        return rc;
      if (unlikely(mc->mc_dbx->md_dcmp == NULL))
        return MDBX_EINVAL;
      rc = mc->mc_dbx->md_dcmp(data, &olddata);
      if (rc) {
        if (op != MDBX_GET_BOTH_RANGE || rc > 0)
          return MDBX_NOTFOUND;
        rc = 0;
      }
      *data = olddata;
    } else {
      if (mc->mc_xcursor)
        mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);
      if (unlikely((rc = mdbx_node_read(mc, leaf, data)) != MDBX_SUCCESS))
        return rc;
    }
  }

  /* The key already matches in all other cases */
  if (op == MDBX_SET_RANGE || op == MDBX_SET_KEY)
    MDBX_GET_MAYNULL_KEYPTR(leaf, key);

  mdbx_debug("==> cursor placed on key [%s], data [%s]", DKEY(key), DVAL(data));
  return rc;
}

/* Move the cursor to the first item in the database. */
static int mdbx_cursor_first(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data) {
  int rc;
  MDBX_node *leaf;

  if (mc->mc_xcursor)
    mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);

  if (!(mc->mc_flags & C_INITIALIZED) || mc->mc_top) {
    rc = mdbx_page_search(mc, NULL, MDBX_PS_FIRST);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]));

  leaf = NODEPTR(mc->mc_pg[mc->mc_top], 0);
  mc->mc_flags |= C_INITIALIZED;
  mc->mc_flags &= ~C_EOF;

  mc->mc_ki[mc->mc_top] = 0;

  if (IS_LEAF2(mc->mc_pg[mc->mc_top])) {
    key->iov_len = mc->mc_db->md_xsize;
    key->iov_base = LEAF2KEY(mc->mc_pg[mc->mc_top], 0, key->iov_len);
    return MDBX_SUCCESS;
  }

  if (likely(data)) {
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      rc = mdbx_xcursor_init1(mc, leaf);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      rc = mdbx_cursor_first(&mc->mc_xcursor->mx_cursor, data, NULL);
      if (unlikely(rc))
        return rc;
    } else {
      if (unlikely((rc = mdbx_node_read(mc, leaf, data)) != MDBX_SUCCESS))
        return rc;
    }
  }
  MDBX_GET_MAYNULL_KEYPTR(leaf, key);
  return MDBX_SUCCESS;
}

/* Move the cursor to the last item in the database. */
static int mdbx_cursor_last(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data) {
  int rc;
  MDBX_node *leaf;

  if (mc->mc_xcursor)
    mc->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);

  if (likely((mc->mc_flags & (C_EOF | C_DEL)) != C_EOF)) {
    if (!(mc->mc_flags & C_INITIALIZED) || mc->mc_top) {
      rc = mdbx_page_search(mc, NULL, MDBX_PS_LAST);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
    mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]));
  }

  mc->mc_ki[mc->mc_top] = NUMKEYS(mc->mc_pg[mc->mc_top]) - 1;
  mc->mc_flags |= C_INITIALIZED | C_EOF;
  leaf = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);

  if (IS_LEAF2(mc->mc_pg[mc->mc_top])) {
    key->iov_len = mc->mc_db->md_xsize;
    key->iov_base =
        LEAF2KEY(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top], key->iov_len);
    return MDBX_SUCCESS;
  }

  if (likely(data)) {
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      rc = mdbx_xcursor_init1(mc, leaf);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      rc = mdbx_cursor_last(&mc->mc_xcursor->mx_cursor, data, NULL);
      if (unlikely(rc))
        return rc;
    } else {
      if (unlikely((rc = mdbx_node_read(mc, leaf, data)) != MDBX_SUCCESS))
        return rc;
    }
  }

  MDBX_GET_MAYNULL_KEYPTR(leaf, key);
  return MDBX_SUCCESS;
}

int mdbx_cursor_get(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                    MDBX_cursor_op op) {
  int rc;
  int exact = 0;
  int (*mfunc)(MDBX_cursor * mc, MDBX_val * key, MDBX_val * data);

  if (unlikely(mc == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(mc->mc_txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  switch (op) {
  case MDBX_GET_CURRENT: {
    if (unlikely(!(mc->mc_flags & C_INITIALIZED)))
      return MDBX_EINVAL;
    MDBX_page *mp = mc->mc_pg[mc->mc_top];
    const unsigned nkeys = NUMKEYS(mp);
    if (mc->mc_ki[mc->mc_top] >= nkeys) {
      mdbx_cassert(mc, nkeys <= UINT16_MAX);
      mc->mc_ki[mc->mc_top] = (uint16_t)nkeys;
      return MDBX_NOTFOUND;
    }
    mdbx_cassert(mc, nkeys > 0);

    rc = MDBX_SUCCESS;
    if (IS_LEAF2(mp)) {
      key->iov_len = mc->mc_db->md_xsize;
      key->iov_base = LEAF2KEY(mp, mc->mc_ki[mc->mc_top], key->iov_len);
    } else {
      MDBX_node *leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
      MDBX_GET_MAYNULL_KEYPTR(leaf, key);
      if (data) {
        if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
          if (unlikely(!(mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED))) {
            rc = mdbx_xcursor_init1(mc, leaf);
            if (unlikely(rc != MDBX_SUCCESS))
              return rc;
            rc = mdbx_cursor_first(&mc->mc_xcursor->mx_cursor, data, NULL);
            if (unlikely(rc))
              return rc;
          }
          rc = mdbx_cursor_get(&mc->mc_xcursor->mx_cursor, data, NULL,
                               MDBX_GET_CURRENT);
        } else {
          rc = mdbx_node_read(mc, leaf, data);
        }
        if (unlikely(rc))
          return rc;
      }
    }
    break;
  }
  case MDBX_GET_BOTH:
  case MDBX_GET_BOTH_RANGE:
    if (unlikely(data == NULL))
      return MDBX_EINVAL;
    if (unlikely(mc->mc_xcursor == NULL))
      return MDBX_INCOMPATIBLE;
  /* FALLTHRU */
  case MDBX_SET:
  case MDBX_SET_KEY:
  case MDBX_SET_RANGE:
    if (unlikely(key == NULL))
      return MDBX_EINVAL;
    rc = mdbx_cursor_set(mc, key, data, op,
                         op == MDBX_SET_RANGE ? NULL : &exact);
    break;
  case MDBX_GET_MULTIPLE:
    if (unlikely(data == NULL || !(mc->mc_flags & C_INITIALIZED)))
      return MDBX_EINVAL;
    if (unlikely(!(mc->mc_db->md_flags & MDBX_DUPFIXED)))
      return MDBX_INCOMPATIBLE;
    rc = MDBX_SUCCESS;
    if (!(mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED) ||
        (mc->mc_xcursor->mx_cursor.mc_flags & C_EOF))
      break;
    goto fetchm;
  case MDBX_NEXT_MULTIPLE:
    if (unlikely(data == NULL))
      return MDBX_EINVAL;
    if (unlikely(!(mc->mc_db->md_flags & MDBX_DUPFIXED)))
      return MDBX_INCOMPATIBLE;
    rc = mdbx_cursor_next(mc, key, data, MDBX_NEXT_DUP);
    if (rc == MDBX_SUCCESS) {
      if (mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED) {
        MDBX_cursor *mx;
      fetchm:
        mx = &mc->mc_xcursor->mx_cursor;
        data->iov_len = NUMKEYS(mx->mc_pg[mx->mc_top]) * mx->mc_db->md_xsize;
        data->iov_base = PAGEDATA(mx->mc_pg[mx->mc_top]);
        mx->mc_ki[mx->mc_top] = NUMKEYS(mx->mc_pg[mx->mc_top]) - 1;
      } else {
        rc = MDBX_NOTFOUND;
      }
    }
    break;
  case MDBX_PREV_MULTIPLE:
    if (data == NULL)
      return MDBX_EINVAL;
    if (!(mc->mc_db->md_flags & MDBX_DUPFIXED))
      return MDBX_INCOMPATIBLE;
    rc = MDBX_SUCCESS;
    if (!(mc->mc_flags & C_INITIALIZED))
      rc = mdbx_cursor_last(mc, key, data);
    if (rc == MDBX_SUCCESS) {
      MDBX_cursor *mx = &mc->mc_xcursor->mx_cursor;
      if (mx->mc_flags & C_INITIALIZED) {
        rc = mdbx_cursor_sibling(mx, 0);
        if (rc == MDBX_SUCCESS)
          goto fetchm;
      } else {
        rc = MDBX_NOTFOUND;
      }
    }
    break;
  case MDBX_NEXT:
  case MDBX_NEXT_DUP:
  case MDBX_NEXT_NODUP:
    rc = mdbx_cursor_next(mc, key, data, op);
    break;
  case MDBX_PREV:
  case MDBX_PREV_DUP:
  case MDBX_PREV_NODUP:
    rc = mdbx_cursor_prev(mc, key, data, op);
    break;
  case MDBX_FIRST:
    rc = mdbx_cursor_first(mc, key, data);
    break;
  case MDBX_FIRST_DUP:
    mfunc = mdbx_cursor_first;
  mmove:
    if (unlikely(data == NULL || !(mc->mc_flags & C_INITIALIZED)))
      return MDBX_EINVAL;
    if (unlikely(mc->mc_xcursor == NULL))
      return MDBX_INCOMPATIBLE;
    if (mc->mc_ki[mc->mc_top] >= NUMKEYS(mc->mc_pg[mc->mc_top])) {
      mc->mc_ki[mc->mc_top] = NUMKEYS(mc->mc_pg[mc->mc_top]);
      return MDBX_NOTFOUND;
    }
    {
      MDBX_node *leaf = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
      if (!F_ISSET(leaf->mn_flags, F_DUPDATA)) {
        MDBX_GET_MAYNULL_KEYPTR(leaf, key);
        rc = mdbx_node_read(mc, leaf, data);
        break;
      }
    }
    if (unlikely(!(mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED)))
      return MDBX_EINVAL;
    rc = mfunc(&mc->mc_xcursor->mx_cursor, data, NULL);
    break;
  case MDBX_LAST:
    rc = mdbx_cursor_last(mc, key, data);
    break;
  case MDBX_LAST_DUP:
    mfunc = mdbx_cursor_last;
    goto mmove;
  default:
    mdbx_debug("unhandled/unimplemented cursor operation %u", op);
    return MDBX_EINVAL;
  }

  mc->mc_flags &= ~C_DEL;
  return rc;
}

/* Touch all the pages in the cursor stack. Set mc_top.
 * Makes sure all the pages are writable, before attempting a write operation.
 * [in] mc The cursor to operate on. */
static int mdbx_cursor_touch(MDBX_cursor *mc) {
  int rc = MDBX_SUCCESS;

  if (mc->mc_dbi >= CORE_DBS &&
      (*mc->mc_dbflag & (DB_DIRTY | DB_DUPDATA)) == 0) {
    mdbx_cassert(mc, (mc->mc_flags & C_RECLAIMING) == 0);
    /* Touch DB record of named DB */
    MDBX_cursor_couple cx;
    if (TXN_DBI_CHANGED(mc->mc_txn, mc->mc_dbi))
      return MDBX_BAD_DBI;
    rc = mdbx_cursor_init(&cx.outer, mc->mc_txn, MAIN_DBI);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    rc = mdbx_page_search(&cx.outer, &mc->mc_dbx->md_name, MDBX_PS_MODIFY);
    if (unlikely(rc))
      return rc;
    *mc->mc_dbflag |= DB_DIRTY;
  }
  mc->mc_top = 0;
  if (mc->mc_snum) {
    do {
      rc = mdbx_page_touch(mc);
    } while (!rc && ++(mc->mc_top) < mc->mc_snum);
    mc->mc_top = mc->mc_snum - 1;
  }
  return rc;
}

int mdbx_cursor_put(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                    unsigned flags) {
  MDBX_env *env;
  MDBX_page *fp, *sub_root = NULL;
  uint16_t fp_flags;
  MDBX_val xdata, *rdata, dkey, olddata;
  MDBX_db dummy;
  unsigned mcount = 0, dcount = 0, nospill;
  size_t nsize;
  int rc = MDBX_SUCCESS, rc2;
  unsigned nflags;
  DKBUF;

  if (unlikely(mc == NULL || key == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  env = mc->mc_txn->mt_env;

  /* Check this first so counter will always be zero on any early failures. */
  if (flags & MDBX_MULTIPLE) {
    if (unlikely(!F_ISSET(mc->mc_db->md_flags, MDBX_DUPFIXED)))
      return MDBX_INCOMPATIBLE;
    if (unlikely(data[1].iov_len >= INT_MAX))
      return MDBX_EINVAL;
    dcount = (unsigned)data[1].iov_len;
    data[1].iov_len = 0;
  }

  if (flags & MDBX_RESERVE) {
    if (unlikely(mc->mc_db->md_flags & (MDBX_DUPSORT | MDBX_REVERSEDUP)))
      return MDBX_INCOMPATIBLE;
    data->iov_base = nullptr;
  }

  nospill = flags & MDBX_NOSPILL;
  flags &= ~MDBX_NOSPILL;

  if (unlikely(mc->mc_txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (mc->mc_txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS
                                                    : MDBX_BAD_TXN;

  if (unlikely(key->iov_len > env->me_maxkey_limit))
    return MDBX_BAD_VALSIZE;

  if (unlikely(data->iov_len > ((mc->mc_db->md_flags & MDBX_DUPSORT)
                                    ? env->me_maxkey_limit
                                    : MDBX_MAXDATASIZE)))
    return MDBX_BAD_VALSIZE;

  if ((mc->mc_db->md_flags & MDBX_INTEGERKEY) &&
      unlikely(key->iov_len != sizeof(uint32_t) &&
               key->iov_len != sizeof(uint64_t))) {
    mdbx_cassert(mc, !"key-size is invalid for MDBX_INTEGERKEY");
    return MDBX_BAD_VALSIZE;
  }

  if ((mc->mc_db->md_flags & MDBX_INTEGERDUP) &&
      unlikely(data->iov_len != sizeof(uint32_t) &&
               data->iov_len != sizeof(uint64_t))) {
    mdbx_cassert(mc, !"data-size is invalid MDBX_INTEGERDUP");
    return MDBX_BAD_VALSIZE;
  }

  mdbx_debug("==> put db %d key [%s], size %" PRIuPTR
             ", data [%s] size %" PRIuPTR,
             DDBI(mc), DKEY(key), key ? key->iov_len : 0,
             DVAL((flags & MDBX_RESERVE) ? nullptr : data), data->iov_len);

  int dupdata_flag = 0;
  if ((flags & MDBX_CURRENT) != 0 && (mc->mc_flags & C_SUB) == 0) {
    /* Опция MDBX_CURRENT означает, что запрошено обновление текущей записи,
     * на которой сейчас стоит курсор. Проверяем что переданный ключ совпадает
     * со значением в текущей позиции курсора.
     * Здесь проще вызвать mdbx_cursor_get(), так как для обслуживания таблиц
     * с MDBX_DUPSORT также требуется текущий размер данных. */
    MDBX_val current_key, current_data;
    rc = mdbx_cursor_get(mc, &current_key, &current_data, MDBX_GET_CURRENT);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    if (mc->mc_dbx->md_cmp(key, &current_key) != 0)
      return MDBX_EKEYMISMATCH;

    if (F_ISSET(mc->mc_db->md_flags, MDBX_DUPSORT)) {
      MDBX_node *leaf = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
      if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
        mdbx_cassert(mc,
                     mc->mc_xcursor != NULL &&
                         (mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED));
        /* Если за ключом более одного значения, либо если размер данных
         * отличается, то вместо inplace обновления требуется удаление и
         * последующая вставка. */
        if (mc->mc_xcursor->mx_db.md_entries > 1 ||
            current_data.iov_len != data->iov_len) {
          rc = mdbx_cursor_del(mc, 0);
          if (rc != MDBX_SUCCESS)
            return rc;
          flags -= MDBX_CURRENT;
        }
      }
    }
  }

  if (mc->mc_db->md_root == P_INVALID) {
    /* new database, cursor has nothing to point to */
    mc->mc_snum = 0;
    mc->mc_top = 0;
    mc->mc_flags &= ~C_INITIALIZED;
    rc = MDBX_NO_ROOT;
  } else if ((flags & MDBX_CURRENT) == 0) {
    int exact = 0;
    MDBX_val d2;
    if (flags & MDBX_APPEND) {
      MDBX_val k2;
      rc = mdbx_cursor_last(mc, &k2, &d2);
      if (rc == 0) {
        rc = mc->mc_dbx->md_cmp(key, &k2);
        if (rc > 0) {
          rc = MDBX_NOTFOUND;
          mc->mc_ki[mc->mc_top]++;
        } else if (unlikely(rc < 0 || (flags & MDBX_APPENDDUP) == 0)) {
          /* new key is <= last key */
          rc = MDBX_EKEYMISMATCH;
        }
      }
    } else {
      rc = mdbx_cursor_set(mc, key, &d2, MDBX_SET, &exact);
    }
    if ((flags & MDBX_NOOVERWRITE) &&
        (rc == MDBX_SUCCESS || rc == MDBX_EKEYMISMATCH)) {
      mdbx_debug("duplicate key [%s]", DKEY(key));
      *data = d2;
      return MDBX_KEYEXIST;
    }
    if (rc && unlikely(rc != MDBX_NOTFOUND))
      return rc;
  }

  mc->mc_flags &= ~C_DEL;

  /* Cursor is positioned, check for room in the dirty list */
  if (!nospill) {
    if (flags & MDBX_MULTIPLE) {
      rdata = &xdata;
      xdata.iov_len = data->iov_len * dcount;
    } else {
      rdata = data;
    }
    if (unlikely(rc2 = mdbx_page_spill(mc, key, rdata)))
      return rc2;
  }

  if (rc == MDBX_NO_ROOT) {
    MDBX_page *np;
    /* new database, write a root leaf page */
    mdbx_debug("allocating new root leaf page");
    if (unlikely(rc2 = mdbx_page_new(mc, P_LEAF, 1, &np))) {
      return rc2;
    }
    rc2 = mdbx_cursor_push(mc, np);
    if (unlikely(rc2 != MDBX_SUCCESS))
      return rc2;
    mc->mc_db->md_root = np->mp_pgno;
    mc->mc_db->md_depth++;
    *mc->mc_dbflag |= DB_DIRTY;
    if ((mc->mc_db->md_flags & (MDBX_DUPSORT | MDBX_DUPFIXED)) == MDBX_DUPFIXED)
      np->mp_flags |= P_LEAF2;
    mc->mc_flags |= C_INITIALIZED;
  } else {
    /* make sure all cursor pages are writable */
    rc2 = mdbx_cursor_touch(mc);
    if (unlikely(rc2))
      return rc2;
  }

  bool insert_key, insert_data, do_sub = false;
  insert_key = insert_data = (rc != MDBX_SUCCESS);
  if (insert_key) {
    /* The key does not exist */
    mdbx_debug("inserting key at index %i", mc->mc_ki[mc->mc_top]);
    if ((mc->mc_db->md_flags & MDBX_DUPSORT) &&
        LEAFSIZE(key, data) > env->me_nodemax) {
      /* Too big for a node, insert in sub-DB.  Set up an empty
       * "old sub-page" for prep_subDB to expand to a full page. */
      fp_flags = P_LEAF | P_DIRTY;
      fp = env->me_pbuf;
      fp->mp_leaf2_ksize = (uint16_t)data->iov_len; /* used if MDBX_DUPFIXED */
      fp->mp_lower = fp->mp_upper = 0;
      olddata.iov_len = PAGEHDRSZ;
      goto prep_subDB;
    }
  } else {
    /* there's only a key anyway, so this is a no-op */
    if (IS_LEAF2(mc->mc_pg[mc->mc_top])) {
      char *ptr;
      unsigned ksize = mc->mc_db->md_xsize;
      if (key->iov_len != ksize)
        return MDBX_BAD_VALSIZE;
      ptr = LEAF2KEY(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top], ksize);
      memcpy(ptr, key->iov_base, ksize);
    fix_parent:
      /* if overwriting slot 0 of leaf, need to
       * update branch key if there is a parent page */
      if (mc->mc_top && !mc->mc_ki[mc->mc_top]) {
        unsigned dtop = 1;
        mc->mc_top--;
        /* slot 0 is always an empty key, find real slot */
        while (mc->mc_top && !mc->mc_ki[mc->mc_top]) {
          mc->mc_top--;
          dtop++;
        }
        if (mc->mc_ki[mc->mc_top])
          rc2 = mdbx_update_key(mc, key);
        else
          rc2 = MDBX_SUCCESS;
        mdbx_cassert(mc, mc->mc_top + dtop < UINT16_MAX);
        mc->mc_top += (uint16_t)dtop;
        if (rc2)
          return rc2;
      }

      if (mdbx_audit_enabled()) {
        int err = mdbx_cursor_check(mc, false);
        if (unlikely(err != MDBX_SUCCESS))
          return err;
      }
      return MDBX_SUCCESS;
    }

  more:;
    MDBX_node *leaf = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
    olddata.iov_len = NODEDSZ(leaf);
    olddata.iov_base = NODEDATA(leaf);

    /* DB has dups? */
    if (F_ISSET(mc->mc_db->md_flags, MDBX_DUPSORT)) {
      /* Prepare (sub-)page/sub-DB to accept the new item, if needed.
       * fp: old sub-page or a header faking it.
       * mp: new (sub-)page.  offset: growth in page size.
       * xdata: node data with new page or DB. */
      unsigned i, offset = 0;
      MDBX_page *mp = fp = xdata.iov_base = env->me_pbuf;
      mp->mp_pgno = mc->mc_pg[mc->mc_top]->mp_pgno;

      /* Was a single item before, must convert now */
      if (!F_ISSET(leaf->mn_flags, F_DUPDATA)) {

        /* does data match? */
        if (!mc->mc_dbx->md_dcmp(data, &olddata)) {
          if (unlikely(flags & (MDBX_NODUPDATA | MDBX_APPENDDUP)))
            return MDBX_KEYEXIST;
          /* overwrite it */
          goto current;
        }

        /* Just overwrite the current item */
        if (flags & MDBX_CURRENT)
          goto current;

        /* Back up original data item */
        dupdata_flag = 1;
        dkey.iov_len = olddata.iov_len;
        dkey.iov_base = memcpy(fp + 1, olddata.iov_base, olddata.iov_len);

        /* Make sub-page header for the dup items, with dummy body */
        fp->mp_flags = P_LEAF | P_DIRTY | P_SUBP;
        fp->mp_lower = 0;
        xdata.iov_len = PAGEHDRSZ + dkey.iov_len + data->iov_len;
        if (mc->mc_db->md_flags & MDBX_DUPFIXED) {
          fp->mp_flags |= P_LEAF2;
          fp->mp_leaf2_ksize = (uint16_t)data->iov_len;
          xdata.iov_len += 2 * data->iov_len; /* leave space for 2 more */
        } else {
          xdata.iov_len += 2 * (sizeof(indx_t) + NODESIZE) +
                           (dkey.iov_len & 1) + (data->iov_len & 1);
        }
        fp->mp_upper = (uint16_t)(xdata.iov_len - PAGEHDRSZ);
        olddata.iov_len = xdata.iov_len; /* pretend olddata is fp */
      } else if (leaf->mn_flags & F_SUBDATA) {
        /* Data is on sub-DB, just store it */
        flags |= F_DUPDATA | F_SUBDATA;
        goto put_sub;
      } else {
        /* Data is on sub-page */
        fp = olddata.iov_base;
        switch (flags) {
        default:
          if (!(mc->mc_db->md_flags & MDBX_DUPFIXED)) {
            offset = EVEN(NODESIZE + sizeof(indx_t) + data->iov_len);
            break;
          }
          offset = fp->mp_leaf2_ksize;
          if (SIZELEFT(fp) < offset) {
            offset *= 4; /* space for 4 more */
            break;
          }
          /* FALLTHRU: Big enough MDBX_DUPFIXED sub-page */
          __fallthrough;
        case MDBX_CURRENT | MDBX_NODUPDATA:
        case MDBX_CURRENT:
          fp->mp_flags |= P_DIRTY;
          fp->mp_pgno = mp->mp_pgno;
          mc->mc_xcursor->mx_cursor.mc_pg[0] = fp;
          flags |= F_DUPDATA;
          goto put_sub;
        }
        xdata.iov_len = olddata.iov_len + offset;
      }

      fp_flags = fp->mp_flags;
      if (NODESIZE + NODEKSZ(leaf) + xdata.iov_len > env->me_nodemax) {
        /* Too big for a sub-page, convert to sub-DB */
        fp_flags &= ~P_SUBP;
      prep_subDB:
        dummy.md_xsize = 0;
        dummy.md_flags = 0;
        if (mc->mc_db->md_flags & MDBX_DUPFIXED) {
          fp_flags |= P_LEAF2;
          dummy.md_xsize = fp->mp_leaf2_ksize;
          dummy.md_flags = MDBX_DUPFIXED;
          if (mc->mc_db->md_flags & MDBX_INTEGERDUP)
            dummy.md_flags |= MDBX_INTEGERKEY;
        }
        dummy.md_depth = 1;
        dummy.md_branch_pages = 0;
        dummy.md_leaf_pages = 1;
        dummy.md_overflow_pages = 0;
        dummy.md_entries = NUMKEYS(fp);
        xdata.iov_len = sizeof(MDBX_db);
        xdata.iov_base = &dummy;
        if ((rc = mdbx_page_alloc(mc, 1, &mp, MDBX_ALLOC_ALL)))
          return rc;
        mc->mc_db->md_leaf_pages += 1;
        mdbx_cassert(mc, env->me_psize > olddata.iov_len);
        offset = env->me_psize - (unsigned)olddata.iov_len;
        flags |= F_DUPDATA | F_SUBDATA;
        dummy.md_root = mp->mp_pgno;
        dummy.md_seq = dummy.md_merkle = 0;
        sub_root = mp;
      }
      if (mp != fp) {
        mp->mp_flags = fp_flags | P_DIRTY;
        mp->mp_leaf2_ksize = fp->mp_leaf2_ksize;
        mp->mp_lower = fp->mp_lower;
        mdbx_cassert(mc, fp->mp_upper + offset <= UINT16_MAX);
        mp->mp_upper = (indx_t)(fp->mp_upper + offset);
        if (unlikely(fp_flags & P_LEAF2)) {
          memcpy(PAGEDATA(mp), PAGEDATA(fp), NUMKEYS(fp) * fp->mp_leaf2_ksize);
        } else {
          memcpy((char *)mp + mp->mp_upper + PAGEHDRSZ,
                 (char *)fp + fp->mp_upper + PAGEHDRSZ,
                 olddata.iov_len - fp->mp_upper - PAGEHDRSZ);
          memcpy((char *)(&mp->mp_ptrs), (char *)(&fp->mp_ptrs),
                 NUMKEYS(fp) * sizeof(mp->mp_ptrs[0]));
          for (i = 0; i < NUMKEYS(fp); i++) {
            mdbx_cassert(mc, mp->mp_ptrs[i] + offset <= UINT16_MAX);
            mp->mp_ptrs[i] += (indx_t)offset;
          }
        }
      }

      rdata = &xdata;
      flags |= F_DUPDATA;
      do_sub = true;
      if (!insert_key)
        mdbx_node_del(mc, 0);
      goto new_sub;
    }
  current:
    /* MDBX passes F_SUBDATA in 'flags' to write a DB record */
    if (unlikely((leaf->mn_flags ^ flags) & F_SUBDATA))
      return MDBX_INCOMPATIBLE;
    /* overflow page overwrites need special handling */
    if (F_ISSET(leaf->mn_flags, F_BIGDATA)) {
      MDBX_page *omp;
      pgno_t pg;
      int level, ovpages,
          dpages = (LEAFSIZE(key, data) > env->me_nodemax)
                       ? OVPAGES(env, data->iov_len)
                       : 0;

      memcpy(&pg, olddata.iov_base, sizeof(pg));
      if (unlikely((rc2 = mdbx_page_get(mc, pg, &omp, &level)) != 0))
        return rc2;
      ovpages = omp->mp_pages;

      /* Is the ov page large enough? */
      if (unlikely(mc->mc_flags & C_GCFREEZE)
              ? ovpages >= dpages
              : ovpages ==
                    /* LY: add configurable threshold to keep reserve space */
                    dpages) {
        if (!IS_DIRTY(omp) && (level || (env->me_flags & MDBX_WRITEMAP))) {
          rc = mdbx_page_unspill(mc->mc_txn, omp, &omp);
          if (unlikely(rc))
            return rc;
          level = 0; /* dirty in this txn or clean */
        }
        /* Is it dirty? */
        if (IS_DIRTY(omp)) {
          /* yes, overwrite it. Note in this case we don't
           * bother to try shrinking the page if the new data
           * is smaller than the overflow threshold. */
          if (unlikely(level > 1)) {
            /* It is writable only in a parent txn */
            MDBX_page *np = mdbx_page_malloc(mc->mc_txn, ovpages);
            if (unlikely(!np))
              return MDBX_ENOMEM;
            /* Note - this page is already counted in parent's dirtyroom */
            rc2 = mdbx_dpl_insert(mc->mc_txn->mt_rw_dirtylist, pg, np);
            if (unlikely(rc2 != MDBX_SUCCESS)) {
              rc = rc2;
              mdbx_dpage_free(env, np, ovpages);
              goto fail;
            }

            /* Currently we make the page look as with put() in the
             * parent txn, in case the user peeks at MDBX_RESERVEd
             * or unused parts. Some users treat ovpages specially. */
            const size_t whole = pgno2bytes(env, ovpages);
            /* Skip the part where MDBX will put *data.
             * Copy end of page, adjusting alignment so
             * compiler may copy words instead of bytes. */
            const size_t off =
                (PAGEHDRSZ + data->iov_len) & -(intptr_t)sizeof(size_t);
            memcpy((size_t *)((char *)np + off), (size_t *)((char *)omp + off),
                   whole - off);
            memcpy(np, omp, PAGEHDRSZ); /* Copy header of page */
            omp = np;
          }
          SETDSZ(leaf, data->iov_len);
          if (F_ISSET(flags, MDBX_RESERVE))
            data->iov_base = PAGEDATA(omp);
          else
            memcpy(PAGEDATA(omp), data->iov_base, data->iov_len);

          if (mdbx_audit_enabled()) {
            int err = mdbx_cursor_check(mc, false);
            if (unlikely(err != MDBX_SUCCESS))
              return err;
          }
          return MDBX_SUCCESS;
        }
      }
      if ((rc2 = mdbx_ovpage_free(mc, omp)) != MDBX_SUCCESS)
        return rc2;
    } else if (data->iov_len == olddata.iov_len) {
      mdbx_cassert(mc, EVEN(key->iov_len) == EVEN(leaf->mn_ksize));
      /* same size, just replace it. Note that we could
       * also reuse this node if the new data is smaller,
       * but instead we opt to shrink the node in that case. */
      if (F_ISSET(flags, MDBX_RESERVE))
        data->iov_base = olddata.iov_base;
      else if (!(mc->mc_flags & C_SUB))
        memcpy(olddata.iov_base, data->iov_base, data->iov_len);
      else {
        mdbx_cassert(mc, NUMKEYS(mc->mc_pg[mc->mc_top]) == 1);
        mdbx_cassert(mc, PAGETYPE(mc->mc_pg[mc->mc_top]) == P_LEAF);
        mdbx_cassert(mc, NODEDSZ(leaf) == 0);
        mdbx_cassert(mc, leaf->mn_flags == 0);
        mdbx_cassert(mc, key->iov_len < UINT16_MAX);
        leaf->mn_ksize = (uint16_t)key->iov_len;
        memcpy(NODEKEY(leaf), key->iov_base, key->iov_len);
        mdbx_cassert(mc, (char *)NODEKEY(leaf) + NODEDSZ(leaf) <
                             (char *)(mc->mc_pg[mc->mc_top]) + env->me_psize);
        goto fix_parent;
      }

      if (mdbx_audit_enabled()) {
        int err = mdbx_cursor_check(mc, false);
        if (unlikely(err != MDBX_SUCCESS))
          return err;
      }
      return MDBX_SUCCESS;
    }
    mdbx_node_del(mc, 0);
  }

  rdata = data;

new_sub:
  nflags = flags & NODE_ADD_FLAGS;
  nsize = IS_LEAF2(mc->mc_pg[mc->mc_top]) ? key->iov_len
                                          : mdbx_leaf_size(env, key, rdata);
  if (SIZELEFT(mc->mc_pg[mc->mc_top]) < nsize) {
    if ((flags & (F_DUPDATA | F_SUBDATA)) == F_DUPDATA)
      nflags &= ~MDBX_APPEND; /* sub-page may need room to grow */
    if (!insert_key)
      nflags |= MDBX_SPLIT_REPLACE;
    rc = mdbx_page_split(mc, key, rdata, P_INVALID, nflags);
    if (rc == MDBX_SUCCESS && mdbx_audit_enabled())
      rc = mdbx_cursor_check(mc, false);
  } else {
    /* There is room already in this leaf page. */
    if (IS_LEAF2(mc->mc_pg[mc->mc_top])) {
      mdbx_cassert(mc, (nflags & (F_BIGDATA | F_SUBDATA | F_DUPDATA)) == 0 &&
                           rdata->iov_len == 0);
      rc = mdbx_node_add_leaf2(mc, mc->mc_ki[mc->mc_top], key);
    } else
      rc = mdbx_node_add_leaf(mc, mc->mc_ki[mc->mc_top], key, rdata, nflags);
    if (likely(rc == 0)) {
      /* Adjust other cursors pointing to mp */
      MDBX_cursor *m2, *m3;
      MDBX_dbi dbi = mc->mc_dbi;
      unsigned i = mc->mc_top;
      MDBX_page *mp = mc->mc_pg[i];

      for (m2 = mc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
        m3 = (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
        if (m3 == mc || m3->mc_snum < mc->mc_snum || m3->mc_pg[i] != mp)
          continue;
        if (m3->mc_ki[i] >= mc->mc_ki[i] && insert_key) {
          m3->mc_ki[i]++;
        }
        if (XCURSOR_INITED(m3))
          XCURSOR_REFRESH(m3, mp, m3->mc_ki[i]);
      }
    }
  }

  if (likely(rc == MDBX_SUCCESS)) {
    /* Now store the actual data in the child DB. Note that we're
     * storing the user data in the keys field, so there are strict
     * size limits on dupdata. The actual data fields of the child
     * DB are all zero size. */
    if (do_sub) {
      int xflags;
      size_t ecount;
    put_sub:
      xdata.iov_len = 0;
      xdata.iov_base = "";
      MDBX_node *leaf = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
      if (flags & MDBX_CURRENT) {
        xflags = (flags & MDBX_NODUPDATA)
                     ? MDBX_CURRENT | MDBX_NOOVERWRITE | MDBX_NOSPILL
                     : MDBX_CURRENT | MDBX_NOSPILL;
      } else {
        rc2 = mdbx_xcursor_init1(mc, leaf);
        if (unlikely(rc2 != MDBX_SUCCESS))
          return rc2;
        xflags = (flags & MDBX_NODUPDATA) ? MDBX_NOOVERWRITE | MDBX_NOSPILL
                                          : MDBX_NOSPILL;
      }
      if (sub_root)
        mc->mc_xcursor->mx_cursor.mc_pg[0] = sub_root;
      /* converted, write the original data first */
      if (dupdata_flag) {
        rc = mdbx_cursor_put(&mc->mc_xcursor->mx_cursor, &dkey, &xdata, xflags);
        if (unlikely(rc))
          goto bad_sub;
        /* we've done our job */
        dkey.iov_len = 0;
      }
      if (!(leaf->mn_flags & F_SUBDATA) || sub_root) {
        /* Adjust other cursors pointing to mp */
        MDBX_cursor *m2;
        MDBX_xcursor *mx = mc->mc_xcursor;
        unsigned i = mc->mc_top;
        MDBX_page *mp = mc->mc_pg[i];
        const int nkeys = NUMKEYS(mp);

        for (m2 = mc->mc_txn->mt_cursors[mc->mc_dbi]; m2; m2 = m2->mc_next) {
          if (m2 == mc || m2->mc_snum < mc->mc_snum)
            continue;
          if (!(m2->mc_flags & C_INITIALIZED))
            continue;
          if (m2->mc_pg[i] == mp) {
            if (m2->mc_ki[i] == mc->mc_ki[i]) {
              rc2 = mdbx_xcursor_init2(m2, mx, dupdata_flag);
              if (unlikely(rc2 != MDBX_SUCCESS))
                return rc2;
            } else if (!insert_key && m2->mc_ki[i] < nkeys) {
              XCURSOR_REFRESH(m2, mp, m2->mc_ki[i]);
            }
          }
        }
      }
      mdbx_cassert(mc, mc->mc_xcursor->mx_db.md_entries < PTRDIFF_MAX);
      ecount = (size_t)mc->mc_xcursor->mx_db.md_entries;
      if (flags & MDBX_APPENDDUP)
        xflags |= MDBX_APPEND;
      rc = mdbx_cursor_put(&mc->mc_xcursor->mx_cursor, data, &xdata, xflags);
      if (flags & F_SUBDATA) {
        void *db = NODEDATA(leaf);
        memcpy(db, &mc->mc_xcursor->mx_db, sizeof(MDBX_db));
      }
      insert_data = (ecount != (size_t)mc->mc_xcursor->mx_db.md_entries);
    }
    /* Increment count unless we just replaced an existing item. */
    if (insert_data)
      mc->mc_db->md_entries++;
    if (insert_key) {
      /* Invalidate txn if we created an empty sub-DB */
      if (unlikely(rc))
        goto bad_sub;
      /* If we succeeded and the key didn't exist before,
       * make sure the cursor is marked valid. */
      mc->mc_flags |= C_INITIALIZED;
    }
    if (flags & MDBX_MULTIPLE) {
      if (!rc) {
        mcount++;
        /* let caller know how many succeeded, if any */
        data[1].iov_len = mcount;
        if (mcount < dcount) {
          data[0].iov_base = (char *)data[0].iov_base + data[0].iov_len;
          insert_key = insert_data = false;
          goto more;
        }
      }
    }
    if (rc == MDBX_SUCCESS && mdbx_audit_enabled())
      rc = mdbx_cursor_check(mc, false);
    return rc;
  bad_sub:
    if (unlikely(rc == MDBX_KEYEXIST))
      mdbx_error("unexpected %s", "MDBX_KEYEXIST");
    /* should not happen, we deleted that item */
    rc = MDBX_PROBLEM;
  }
fail:
  mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
  return rc;
}

int mdbx_cursor_del(MDBX_cursor *mc, unsigned flags) {
  MDBX_node *leaf;
  MDBX_page *mp;
  int rc;

  if (unlikely(!mc))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(mc->mc_txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (mc->mc_txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS
                                                    : MDBX_BAD_TXN;

  if (unlikely(!(mc->mc_flags & C_INITIALIZED)))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_ki[mc->mc_top] >= NUMKEYS(mc->mc_pg[mc->mc_top])))
    return MDBX_NOTFOUND;

  if (unlikely(!(flags & MDBX_NOSPILL) &&
               (rc = mdbx_page_spill(mc, NULL, NULL))))
    return rc;

  rc = mdbx_cursor_touch(mc);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  mp = mc->mc_pg[mc->mc_top];
  if (IS_LEAF2(mp))
    goto del_key;
  leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);

  if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
    if (flags & MDBX_NODUPDATA) {
      /* mdbx_cursor_del0() will subtract the final entry */
      mc->mc_db->md_entries -= mc->mc_xcursor->mx_db.md_entries - 1;
      mc->mc_xcursor->mx_cursor.mc_flags &= ~C_INITIALIZED;
    } else {
      if (!F_ISSET(leaf->mn_flags, F_SUBDATA)) {
        mc->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(leaf);
      }
      rc = mdbx_cursor_del(&mc->mc_xcursor->mx_cursor, MDBX_NOSPILL);
      if (unlikely(rc))
        return rc;
      /* If sub-DB still has entries, we're done */
      if (mc->mc_xcursor->mx_db.md_entries) {
        if (leaf->mn_flags & F_SUBDATA) {
          /* update subDB info */
          void *db = NODEDATA(leaf);
          memcpy(db, &mc->mc_xcursor->mx_db, sizeof(MDBX_db));
        } else {
          MDBX_cursor *m2;
          /* shrink fake page */
          mdbx_node_shrink(mp, mc->mc_ki[mc->mc_top]);
          leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
          mc->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(leaf);
          /* fix other sub-DB cursors pointed at fake pages on this page */
          for (m2 = mc->mc_txn->mt_cursors[mc->mc_dbi]; m2; m2 = m2->mc_next) {
            if (m2 == mc || m2->mc_snum < mc->mc_snum)
              continue;
            if (!(m2->mc_flags & C_INITIALIZED))
              continue;
            if (m2->mc_pg[mc->mc_top] == mp) {
              MDBX_node *n2 = leaf;
              if (m2->mc_ki[mc->mc_top] >= NUMKEYS(mp))
                continue;
              if (m2->mc_ki[mc->mc_top] != mc->mc_ki[mc->mc_top]) {
                n2 = NODEPTR(mp, m2->mc_ki[mc->mc_top]);
                if (n2->mn_flags & F_SUBDATA)
                  continue;
              }
              m2->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(n2);
            }
          }
        }
        mc->mc_db->md_entries--;
        mdbx_cassert(mc, mc->mc_db->md_entries > 0 && mc->mc_db->md_depth > 0 &&
                             mc->mc_db->md_root != P_INVALID);
        return rc;
      } else {
        mc->mc_xcursor->mx_cursor.mc_flags &= ~C_INITIALIZED;
      }
      /* otherwise fall thru and delete the sub-DB */
    }

    if (leaf->mn_flags & F_SUBDATA) {
      /* add all the child DB's pages to the free list */
      mc->mc_db->md_branch_pages -= mc->mc_xcursor->mx_db.md_branch_pages;
      mc->mc_db->md_leaf_pages -= mc->mc_xcursor->mx_db.md_leaf_pages;
      mc->mc_db->md_overflow_pages -= mc->mc_xcursor->mx_db.md_overflow_pages;
      rc = mdbx_drop0(&mc->mc_xcursor->mx_cursor, 0);
      if (unlikely(rc))
        goto fail;
    }
  }
  /* MDBX passes F_SUBDATA in 'flags' to delete a DB record */
  else if (unlikely((leaf->mn_flags ^ flags) & F_SUBDATA)) {
    rc = MDBX_INCOMPATIBLE;
    goto fail;
  }

  /* add overflow pages to free list */
  if (F_ISSET(leaf->mn_flags, F_BIGDATA)) {
    MDBX_page *omp;
    pgno_t pg;

    memcpy(&pg, NODEDATA(leaf), sizeof(pg));
    if (unlikely((rc = mdbx_page_get(mc, pg, &omp, NULL)) ||
                 (rc = mdbx_ovpage_free(mc, omp))))
      goto fail;
  }

del_key:
  return mdbx_cursor_del0(mc);

fail:
  mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
  return rc;
}

/* Allocate and initialize new pages for a database.
 * Set MDBX_TXN_ERROR on failure.
 *
 * [in] mc a  cursor on the database being added to.
 * [in] flags flags defining what type of page is being allocated.
 * [in] num   the number of pages to allocate. This is usually 1,
 *            unless allocating overflow pages for a large record.
 * [out] mp   Address of a page, or NULL on failure.
 *
 * Returns 0 on success, non-zero on failure. */
static int mdbx_page_new(MDBX_cursor *mc, unsigned flags, unsigned num,
                         MDBX_page **mp) {
  MDBX_page *np;
  int rc;

  if (unlikely((rc = mdbx_page_alloc(mc, num, &np, MDBX_ALLOC_ALL))))
    return rc;
  mdbx_debug("allocated new page #%" PRIaPGNO ", size %u", np->mp_pgno,
             mc->mc_txn->mt_env->me_psize);
  np->mp_flags = (uint16_t)(flags | P_DIRTY);
  np->mp_lower = 0;
  np->mp_upper = (indx_t)(mc->mc_txn->mt_env->me_psize - PAGEHDRSZ);

  if (IS_BRANCH(np))
    mc->mc_db->md_branch_pages++;
  else if (IS_LEAF(np))
    mc->mc_db->md_leaf_pages++;
  else {
    mdbx_cassert(mc, IS_OVERFLOW(np));
    mc->mc_db->md_overflow_pages += num;
    np->mp_pages = num;
  }

  if (unlikely(mc->mc_flags & C_SUB)) {
    MDBX_db *outer = mdbx_outer_db(mc);
    if (IS_BRANCH(np))
      outer->md_branch_pages++;
    else if (IS_LEAF(np))
      outer->md_leaf_pages++;
    else {
      mdbx_cassert(mc, IS_OVERFLOW(np));
      outer->md_overflow_pages += num;
    }
  }

  *mp = np;
  return MDBX_SUCCESS;
}

/* Calculate the size of a leaf node.
 *
 * The size depends on the environment's page size; if a data item
 * is too large it will be put onto an overflow page and the node
 * size will only include the key and not the data. Sizes are always
 * rounded up to an even number of bytes, to guarantee 2-byte alignment
 * of the MDBX_node headers.
 *
 * [in] env   The environment handle.
 * [in] key   The key for the node.
 * [in] data  The data for the node.
 *
 * Returns The number of bytes needed to store the node. */
static __inline size_t mdbx_leaf_size(MDBX_env *env, const MDBX_val *key,
                                      const MDBX_val *data) {
  size_t sz = LEAFSIZE(key, data);
  if (sz > env->me_nodemax) {
    /* put on overflow page */
    sz = sz - data->iov_len + sizeof(pgno_t);
  }

  return EVEN(sz) + sizeof(indx_t);
}

/* Calculate the size of a branch node.
 *
 * The size should depend on the environment's page size but since
 * we currently don't support spilling large keys onto overflow
 * pages, it's simply the size of the MDBX_node header plus the
 * size of the key. Sizes are always rounded up to an even number
 * of bytes, to guarantee 2-byte alignment of the MDBX_node headers.
 *
 * [in] env The environment handle.
 * [in] key The key for the node.
 *
 * Returns The number of bytes needed to store the node. */
static __inline size_t mdbx_branch_size(MDBX_env *env, const MDBX_val *key) {
  size_t sz = INDXSIZE(key);
  if (unlikely(sz > env->me_nodemax)) {
    /* put on overflow page */
    /* not implemented */
    mdbx_assert_fail(env, "INDXSIZE(key) <= env->me_nodemax", __FUNCTION__,
                     __LINE__);
    sz = sz - key->iov_len + sizeof(pgno_t);
  }

  return EVEN(sz) + sizeof(indx_t);
}

static int __must_check_result mdbx_node_add_leaf2(MDBX_cursor *mc,
                                                   unsigned indx,
                                                   const MDBX_val *key) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  DKBUF;
  mdbx_debug("add to leaf2-%spage %" PRIaPGNO " index %i, "
             " key size %" PRIuPTR " [%s]",
             IS_SUBP(mp) ? "sub-" : "", mp->mp_pgno, indx,
             key ? key->iov_len : 0, DKEY(key));

  mdbx_cassert(mc, key);
  mdbx_cassert(mc, PAGETYPE(mp) == (P_LEAF | P_LEAF2));
  const unsigned ksize = mc->mc_db->md_xsize;
  mdbx_cassert(mc, ksize == key->iov_len);

  const int room = SIZELEFT(mp);
  mdbx_cassert(mc, room >= (int)ksize);
  if (unlikely(room < (int)ksize)) {
  bailout:
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_PAGE_FULL;
  }

  char *const ptr = LEAF2KEY(mp, indx, ksize);
  mdbx_cassert(mc, NUMKEYS(mp) >= indx);
  if (unlikely(NUMKEYS(mp) < indx))
    goto bailout;

  const unsigned diff = NUMKEYS(mp) - indx;
  if (likely(diff > 0))
    /* Move higher keys up one slot. */
    memmove(ptr + ksize, ptr, diff * ksize);
  /* insert new key */
  memcpy(ptr, key->iov_base, ksize);

  /* Just using these for counting */
  mdbx_cassert(mc, UINT16_MAX - mp->mp_lower >= (int)sizeof(indx_t));
  mp->mp_lower += sizeof(indx_t);
  mdbx_cassert(mc, mp->mp_upper >= ksize - sizeof(indx_t));
  mp->mp_upper -= (indx_t)(ksize - sizeof(indx_t));

  mdbx_cassert(mc,
               mp->mp_upper >= mp->mp_lower &&
                   PAGEHDRSZ + mp->mp_upper <= mc->mc_txn->mt_env->me_psize);
  return MDBX_SUCCESS;
}

static int __must_check_result mdbx_node_add_branch(MDBX_cursor *mc,
                                                    unsigned indx,
                                                    const MDBX_val *key,
                                                    pgno_t pgno) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  DKBUF;
  mdbx_debug("add to branch-%spage %" PRIaPGNO " index %i, node-pgno %" PRIaPGNO
             " key size %" PRIuPTR " [%s]",
             IS_SUBP(mp) ? "sub-" : "", mp->mp_pgno, indx, pgno,
             key ? key->iov_len : 0, DKEY(key));

  mdbx_cassert(mc, PAGETYPE(mp) == P_BRANCH);
  STATIC_ASSERT(NODESIZE % 2 == 0);

  const size_t room = SIZELEFT(mp);
  const size_t node_size =
      likely(key != NULL) ? NODESIZE + EVEN(key->iov_len) : NODESIZE;
  mdbx_cassert(mc, mdbx_branch_size(mc->mc_txn->mt_env, key) ==
                       node_size + sizeof(indx_t));
  mdbx_cassert(mc, room >= node_size + sizeof(indx_t));
  if (unlikely(room < node_size + sizeof(indx_t))) {
  bailout:
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_PAGE_FULL;
  }

  const unsigned numkeys = NUMKEYS(mp);
  mdbx_cassert(mc, numkeys >= indx);
  if (unlikely(numkeys < indx))
    goto bailout;

  /* Move higher pointers up one slot. */
  for (unsigned i = numkeys; i > indx; --i)
    mp->mp_ptrs[i] = mp->mp_ptrs[i - 1];

  /* Adjust free space offsets. */
  const size_t ofs = mp->mp_upper - node_size;
  mdbx_cassert(mc, ofs >= mp->mp_lower + sizeof(indx_t));
  mdbx_cassert(mc, ofs <= UINT16_MAX);
  mp->mp_ptrs[indx] = (uint16_t)ofs;
  mp->mp_upper = (uint16_t)ofs;
  mp->mp_lower += sizeof(indx_t);

  /* Write the node data. */
  MDBX_node *node = NODEPTR(mp, indx);
  SETPGNO(node, pgno);
  node->mn_ksize = 0;
  node->mn_flags = 0;
  if (likely(key != NULL)) {
    node->mn_ksize = (uint16_t)key->iov_len;
    memcpy(NODEKEY(node), key->iov_base, key->iov_len);
  }

  mdbx_cassert(mc,
               mp->mp_upper >= mp->mp_lower &&
                   PAGEHDRSZ + mp->mp_upper <= mc->mc_txn->mt_env->me_psize);
  return MDBX_SUCCESS;
}

static int __must_check_result mdbx_node_add_leaf(MDBX_cursor *mc,
                                                  unsigned indx,
                                                  const MDBX_val *key,
                                                  MDBX_val *data,
                                                  unsigned flags) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  DKBUF;
  mdbx_debug("add to leaf-%spage %" PRIaPGNO " index %i, data size %" PRIuPTR
             " key size %" PRIuPTR " [%s]",
             IS_SUBP(mp) ? "sub-" : "", mp->mp_pgno, indx,
             data ? data->iov_len : 0, key ? key->iov_len : 0, DKEY(key));
  mdbx_cassert(mc, key != NULL && data != NULL);
  mdbx_cassert(mc, PAGETYPE(mp) == P_LEAF);
  MDBX_page *largepage = NULL;

  const size_t room = SIZELEFT(mp);
  size_t node_size = NODESIZE + key->iov_len;
  if (unlikely(flags & F_BIGDATA)) {
    /* Data already on overflow page. */
    STATIC_ASSERT(sizeof(pgno_t) % 2 == 0);
    node_size += sizeof(pgno_t);
  } else if (unlikely(node_size + data->iov_len >
                      mc->mc_txn->mt_env->me_nodemax)) {
    const pgno_t ovpages = OVPAGES(mc->mc_txn->mt_env, data->iov_len);
    /* Put data on overflow page. */
    mdbx_debug("data size is %" PRIuPTR ", node would be %" PRIuPTR
               ", put data on %u-overflow page(s)",
               data->iov_len, node_size + data->iov_len, ovpages);
    int rc = mdbx_page_new(mc, P_OVERFLOW, ovpages, &largepage);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    mdbx_debug("allocated overflow page %" PRIaPGNO, largepage->mp_pgno);
    flags |= F_BIGDATA;
    node_size += sizeof(pgno_t);
    mdbx_cassert(mc, mdbx_leaf_size(mc->mc_txn->mt_env, key, data) ==
                         EVEN(node_size) + sizeof(indx_t));
  } else {
    node_size += data->iov_len;
    mdbx_cassert(mc, mdbx_leaf_size(mc->mc_txn->mt_env, key, data) ==
                         EVEN(node_size) + sizeof(indx_t));
  }

  node_size = EVEN(node_size);
  mdbx_cassert(mc, room >= node_size + sizeof(indx_t));
  if (unlikely(room < node_size + sizeof(indx_t))) {
  bailout:
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_PAGE_FULL;
  }

  const unsigned numkeys = NUMKEYS(mp);
  mdbx_cassert(mc, numkeys >= indx);
  if (unlikely(numkeys < indx))
    goto bailout;

  /* Move higher pointers up one slot. */
  for (unsigned i = numkeys; i > indx; --i)
    mp->mp_ptrs[i] = mp->mp_ptrs[i - 1];

  /* Adjust free space offsets. */
  const size_t ofs = mp->mp_upper - node_size;
  mdbx_cassert(mc, ofs >= mp->mp_lower + sizeof(indx_t));
  mdbx_cassert(mc, ofs <= UINT16_MAX);
  mp->mp_ptrs[indx] = (uint16_t)ofs;
  mp->mp_upper = (uint16_t)ofs;
  mp->mp_lower += sizeof(indx_t);

  /* Write the node data. */
  MDBX_node *node = NODEPTR(mp, indx);
  node->mn_ksize = (uint16_t)key->iov_len;
  node->mn_flags = (uint16_t)flags;
  SETDSZ(node, data->iov_len);
  memcpy(NODEKEY(node), key->iov_base, key->iov_len);

  void *nodedata = NODEDATA(node);
  if (likely(largepage == NULL)) {
    if (unlikely(flags & F_BIGDATA))
      memcpy(nodedata, data->iov_base, sizeof(pgno_t));
    else if (unlikely(flags & MDBX_RESERVE))
      data->iov_base = nodedata;
    else if (likely(nodedata != data->iov_base))
      memcpy(nodedata, data->iov_base, data->iov_len);
  } else {
    memcpy(nodedata, &largepage->mp_pgno, sizeof(pgno_t));
    nodedata = PAGEDATA(largepage);
    if (unlikely(flags & MDBX_RESERVE))
      data->iov_base = nodedata;
    else if (likely(nodedata != data->iov_base))
      memcpy(nodedata, data->iov_base, data->iov_len);
  }

  mdbx_cassert(mc,
               mp->mp_upper >= mp->mp_lower &&
                   PAGEHDRSZ + mp->mp_upper <= mc->mc_txn->mt_env->me_psize);
  return MDBX_SUCCESS;
}

/* Delete the specified node from a page.
 * [in] mc Cursor pointing to the node to delete.
 * [in] ksize The size of a node. Only used if the page is
 * part of a MDBX_DUPFIXED database. */
static void mdbx_node_del(MDBX_cursor *mc, size_t ksize) {
  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  indx_t indx = mc->mc_ki[mc->mc_top];
  indx_t i, j, numkeys, ptr;
  MDBX_node *node;
  char *base;

  mdbx_debug("delete node %u on %s page %" PRIaPGNO, indx,
             IS_LEAF(mp) ? "leaf" : "branch", mp->mp_pgno);
  numkeys = NUMKEYS(mp);
  mdbx_cassert(mc, indx < numkeys);

  if (IS_LEAF2(mp)) {
    mdbx_cassert(mc, ksize >= sizeof(indx_t));
    unsigned diff = numkeys - 1 - indx;
    base = LEAF2KEY(mp, indx, ksize);
    if (diff)
      memmove(base, base + ksize, diff * ksize);
    mdbx_cassert(mc, mp->mp_lower >= sizeof(indx_t));
    mp->mp_lower -= sizeof(indx_t);
    mdbx_cassert(mc,
                 (size_t)UINT16_MAX - mp->mp_upper >= ksize - sizeof(indx_t));
    mp->mp_upper += (indx_t)(ksize - sizeof(indx_t));
    return;
  }

  node = NODEPTR(mp, indx);
  size_t sz = NODESIZE + node->mn_ksize;
  if (IS_LEAF(mp)) {
    if (F_ISSET(node->mn_flags, F_BIGDATA))
      sz += sizeof(pgno_t);
    else
      sz += NODEDSZ(node);
  }
  sz = EVEN(sz);

  ptr = mp->mp_ptrs[indx];
  for (i = j = 0; i < numkeys; i++) {
    if (i != indx) {
      mp->mp_ptrs[j] = mp->mp_ptrs[i];
      if (mp->mp_ptrs[i] < ptr) {
        mdbx_cassert(mc, (size_t)UINT16_MAX - mp->mp_ptrs[j] >= sz);
        mp->mp_ptrs[j] += (indx_t)sz;
      }
      j++;
    }
  }

  base = (char *)mp + mp->mp_upper + PAGEHDRSZ;
  memmove(base + sz, base, ptr - mp->mp_upper);

  mdbx_cassert(mc, mp->mp_lower >= sizeof(indx_t));
  mp->mp_lower -= sizeof(indx_t);
  mdbx_cassert(mc, (size_t)UINT16_MAX - mp->mp_upper >= sz);
  mp->mp_upper += (indx_t)sz;
}

/* Compact the main page after deleting a node on a subpage.
 * [in] mp The main page to operate on.
 * [in] indx The index of the subpage on the main page. */
static void mdbx_node_shrink(MDBX_page *mp, unsigned indx) {
  MDBX_node *node;
  MDBX_page *sp, *xp;
  char *base;
  size_t nsize, delta, len, ptr;
  int i;

  node = NODEPTR(mp, indx);
  sp = (MDBX_page *)NODEDATA(node);
  delta = SIZELEFT(sp);
  assert(delta > 0);

  /* Prepare to shift upward, set len = length(subpage part to shift) */
  if (IS_LEAF2(sp)) {
    delta &= /* do not make the node uneven-sized */ ~1u;
    if (unlikely(delta) == 0)
      return;
    nsize = NODEDSZ(node) - delta;
    assert(nsize % 1 == 0);
    len = nsize;
  } else {
    xp = (MDBX_page *)((char *)sp + delta); /* destination subpage */
    for (i = NUMKEYS(sp); --i >= 0;) {
      assert(sp->mp_ptrs[i] >= delta);
      xp->mp_ptrs[i] = (indx_t)(sp->mp_ptrs[i] - delta);
    }
    nsize = NODEDSZ(node) - delta;
    len = PAGEHDRSZ;
  }
  sp->mp_upper = sp->mp_lower;
  sp->mp_pgno = mp->mp_pgno;
  SETDSZ(node, nsize);

  /* Shift <lower nodes...initial part of subpage> upward */
  base = (char *)mp + mp->mp_upper + PAGEHDRSZ;
  memmove(base + delta, base, (char *)sp + len - base);

  ptr = mp->mp_ptrs[indx];
  for (i = NUMKEYS(mp); --i >= 0;) {
    if (mp->mp_ptrs[i] <= ptr) {
      assert((size_t)UINT16_MAX - mp->mp_ptrs[i] >= delta);
      mp->mp_ptrs[i] += (indx_t)delta;
    }
  }
  assert((size_t)UINT16_MAX - mp->mp_upper >= delta);
  mp->mp_upper += (indx_t)delta;
}

/* Initial setup of a sorted-dups cursor.
 *
 * Sorted duplicates are implemented as a sub-database for the given key.
 * The duplicate data items are actually keys of the sub-database.
 * Operations on the duplicate data items are performed using a sub-cursor
 * initialized when the sub-database is first accessed. This function does
 * the preliminary setup of the sub-cursor, filling in the fields that
 * depend only on the parent DB.
 *
 * [in] mc The main cursor whose sorted-dups cursor is to be initialized. */
static int mdbx_xcursor_init0(MDBX_cursor *mc) {
  MDBX_xcursor *mx = mc->mc_xcursor;
  if (unlikely(mx == nullptr))
    return MDBX_CORRUPTED;

  mx->mx_cursor.mc_xcursor = NULL;
  mx->mx_cursor.mc_txn = mc->mc_txn;
  mx->mx_cursor.mc_db = &mx->mx_db;
  mx->mx_cursor.mc_dbx = &mx->mx_dbx;
  mx->mx_cursor.mc_dbi = mc->mc_dbi;
  mx->mx_cursor.mc_dbflag = &mx->mx_dbflag;
  mx->mx_cursor.mc_snum = 0;
  mx->mx_cursor.mc_top = 0;
  mx->mx_cursor.mc_flags = C_SUB;
  mx->mx_dbx.md_name.iov_len = 0;
  mx->mx_dbx.md_name.iov_base = NULL;
  mx->mx_dbx.md_cmp = mc->mc_dbx->md_dcmp;
  mx->mx_dbx.md_dcmp = NULL;
  return MDBX_SUCCESS;
}

/* Final setup of a sorted-dups cursor.
 * Sets up the fields that depend on the data from the main cursor.
 * [in] mc The main cursor whose sorted-dups cursor is to be initialized.
 * [in] node The data containing the MDBX_db record for the sorted-dup database.
 */
static int mdbx_xcursor_init1(MDBX_cursor *mc, MDBX_node *node) {
  MDBX_xcursor *mx = mc->mc_xcursor;
  if (unlikely(mx == nullptr))
    return MDBX_CORRUPTED;

  if (node->mn_flags & F_SUBDATA) {
    memcpy(&mx->mx_db, NODEDATA(node), sizeof(MDBX_db));
    mx->mx_cursor.mc_pg[0] = 0;
    mx->mx_cursor.mc_snum = 0;
    mx->mx_cursor.mc_top = 0;
    mx->mx_cursor.mc_flags = C_SUB;
  } else {
    MDBX_page *fp = NODEDATA(node);
    mx->mx_db.md_xsize = 0;
    mx->mx_db.md_flags = 0;
    mx->mx_db.md_depth = 1;
    mx->mx_db.md_branch_pages = 0;
    mx->mx_db.md_leaf_pages = 1;
    mx->mx_db.md_overflow_pages = 0;
    mx->mx_db.md_entries = NUMKEYS(fp);
    mx->mx_db.md_root = fp->mp_pgno;
    mx->mx_cursor.mc_snum = 1;
    mx->mx_cursor.mc_top = 0;
    mx->mx_cursor.mc_flags = C_INITIALIZED | C_SUB;
    mx->mx_cursor.mc_pg[0] = fp;
    mx->mx_cursor.mc_ki[0] = 0;
    if (mc->mc_db->md_flags & MDBX_DUPFIXED) {
      mx->mx_db.md_flags = MDBX_DUPFIXED;
      mx->mx_db.md_xsize = fp->mp_leaf2_ksize;
      if (mc->mc_db->md_flags & MDBX_INTEGERDUP)
        mx->mx_db.md_flags |= MDBX_INTEGERKEY;
    }
  }
  mdbx_debug("Sub-db -%u root page %" PRIaPGNO, mx->mx_cursor.mc_dbi,
             mx->mx_db.md_root);
  mx->mx_dbflag = DB_VALID | DB_USRVALID | DB_DUPDATA;
  /* FIXME: #if UINT_MAX < SIZE_MAX
          if (mx->mx_dbx.md_cmp == mdbx_cmp_int && mx->mx_db.md_pad ==
  sizeof(size_t))
                  mx->mx_dbx.md_cmp = mdbx_cmp_clong;
  #endif */

  return MDBX_SUCCESS;
}

/* Fixup a sorted-dups cursor due to underlying update.
 * Sets up some fields that depend on the data from the main cursor.
 * Almost the same as init1, but skips initialization steps if the
 * xcursor had already been used.
 * [in] mc The main cursor whose sorted-dups cursor is to be fixed up.
 * [in] src_mx The xcursor of an up-to-date cursor.
 * [in] new_dupdata True if converting from a non-F_DUPDATA item. */
static int mdbx_xcursor_init2(MDBX_cursor *mc, MDBX_xcursor *src_mx,
                              int new_dupdata) {
  MDBX_xcursor *mx = mc->mc_xcursor;
  if (unlikely(mx == nullptr))
    return MDBX_CORRUPTED;

  if (new_dupdata) {
    mx->mx_cursor.mc_snum = 1;
    mx->mx_cursor.mc_top = 0;
    mx->mx_cursor.mc_flags |= C_INITIALIZED;
    mx->mx_cursor.mc_ki[0] = 0;
    mx->mx_dbflag = DB_VALID | DB_USRVALID | DB_DUPDATA;
    mx->mx_dbx.md_cmp = src_mx->mx_dbx.md_cmp;
  } else if (!(mx->mx_cursor.mc_flags & C_INITIALIZED)) {
    return MDBX_SUCCESS;
  }
  mx->mx_db = src_mx->mx_db;
  mx->mx_cursor.mc_pg[0] = src_mx->mx_cursor.mc_pg[0];
  mdbx_debug("Sub-db -%u root page %" PRIaPGNO, mx->mx_cursor.mc_dbi,
             mx->mx_db.md_root);
  return MDBX_SUCCESS;
}

/* Initialize a cursor for a given transaction and database. */
static int mdbx_cursor_init(MDBX_cursor *mc, MDBX_txn *txn, MDBX_dbi dbi) {
  mc->mc_signature = MDBX_MC_SIGNATURE;
  mc->mc_next = NULL;
  mc->mc_backup = NULL;
  mc->mc_dbi = dbi;
  mc->mc_txn = txn;
  mc->mc_db = &txn->mt_dbs[dbi];
  mc->mc_dbx = &txn->mt_dbxs[dbi];
  mc->mc_dbflag = &txn->mt_dbflags[dbi];
  mc->mc_snum = 0;
  mc->mc_top = 0;
  mc->mc_pg[0] = 0;
  mc->mc_flags = 0;
  mc->mc_ki[0] = 0;
  mc->mc_xcursor = NULL;

  if (txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT) {
    STATIC_ASSERT(offsetof(MDBX_cursor_couple, outer) == 0);
    MDBX_xcursor *mx = &container_of(mc, MDBX_cursor_couple, outer)->inner;
    mdbx_tassert(txn, mx != NULL);
    mx->mx_cursor.mc_signature = MDBX_MC_SIGNATURE;
    mc->mc_xcursor = mx;
    int rc = mdbx_xcursor_init0(mc);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  int rc = MDBX_SUCCESS;
  if (unlikely(*mc->mc_dbflag & DB_STALE)) {
    rc = mdbx_page_search(mc, NULL, MDBX_PS_ROOTONLY);
    rc = (rc != MDBX_NOTFOUND) ? rc : MDBX_SUCCESS;
  }
  return rc;
}

int mdbx_cursor_open(MDBX_txn *txn, MDBX_dbi dbi, MDBX_cursor **ret) {
  if (unlikely(!ret || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_VALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  if (unlikely(dbi == FREE_DBI && !F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)))
    return MDBX_EINVAL;

  const size_t size = (txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT)
                          ? sizeof(MDBX_cursor_couple)
                          : sizeof(MDBX_cursor);

  MDBX_cursor *mc;
  if (likely((mc = mdbx_malloc(size)) != NULL)) {
    int rc = mdbx_cursor_init(mc, txn, dbi);
    if (unlikely(rc != MDBX_SUCCESS)) {
      mdbx_free(mc);
      return rc;
    }
    if (txn->mt_cursors) {
      mc->mc_next = txn->mt_cursors[dbi];
      txn->mt_cursors[dbi] = mc;
      mc->mc_flags |= C_UNTRACK;
    }
  } else {
    return MDBX_ENOMEM;
  }

  *ret = mc;
  return MDBX_SUCCESS;
}

int mdbx_cursor_renew(MDBX_txn *txn, MDBX_cursor *mc) {
  if (unlikely(!mc || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return txn->mt_owner ? MDBX_THREAD_MISMATCH : MDBX_BAD_TXN;

  if (unlikely(txn->mt_flags & (MDBX_TXN_FINISHED | MDBX_TXN_ERROR)))
    return MDBX_BAD_TXN;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE &&
               mc->mc_signature != MDBX_MC_READY4CLOSE))
    return MDBX_EINVAL;

  if (unlikely(!TXN_DBI_EXIST(txn, mc->mc_dbi, DB_VALID)))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_backup))
    return MDBX_EINVAL;

  if (unlikely((mc->mc_flags & C_UNTRACK) || txn->mt_cursors)) {
    MDBX_cursor **prev = &mc->mc_txn->mt_cursors[mc->mc_dbi];
    while (*prev && *prev != mc)
      prev = &(*prev)->mc_next;
    if (*prev == mc)
      *prev = mc->mc_next;
    mc->mc_signature = MDBX_MC_READY4CLOSE;
  }

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  return mdbx_cursor_init(mc, txn, mc->mc_dbi);
}

/* Return the count of duplicate data items for the current key */
int mdbx_cursor_count(MDBX_cursor *mc, size_t *countp) {
  if (unlikely(mc == NULL || countp == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(mc->mc_txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(mc->mc_txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  if (unlikely(!(mc->mc_flags & C_INITIALIZED)))
    return MDBX_EINVAL;

  if (!mc->mc_snum) {
    *countp = 0;
    return MDBX_NOTFOUND;
  }

  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  if ((mc->mc_flags & C_EOF) && mc->mc_ki[mc->mc_top] >= NUMKEYS(mp)) {
    *countp = 0;
    return MDBX_NOTFOUND;
  }

  *countp = 1;
  if (mc->mc_xcursor != NULL) {
    MDBX_node *leaf = NODEPTR(mp, mc->mc_ki[mc->mc_top]);
    if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
      mdbx_cassert(mc, mc->mc_xcursor && (mc->mc_xcursor->mx_cursor.mc_flags &
                                          C_INITIALIZED));
      *countp = unlikely(mc->mc_xcursor->mx_db.md_entries > PTRDIFF_MAX)
                    ? PTRDIFF_MAX
                    : (size_t)mc->mc_xcursor->mx_db.md_entries;
    }
  }
  return MDBX_SUCCESS;
}

void mdbx_cursor_close(MDBX_cursor *mc) {
  if (mc) {
    mdbx_ensure(NULL, mc->mc_signature == MDBX_MC_SIGNATURE ||
                          mc->mc_signature == MDBX_MC_READY4CLOSE);
    if (!mc->mc_backup) {
      /* Remove from txn, if tracked.
       * A read-only txn (!C_UNTRACK) may have been freed already,
       * so do not peek inside it.  Only write txns track cursors. */
      if ((mc->mc_flags & C_UNTRACK) && mc->mc_txn->mt_cursors) {
        MDBX_cursor **prev = &mc->mc_txn->mt_cursors[mc->mc_dbi];
        while (*prev && *prev != mc)
          prev = &(*prev)->mc_next;
        if (*prev == mc)
          *prev = mc->mc_next;
      }
      mc->mc_signature = 0;
      mdbx_free(mc);
    } else {
      /* cursor closed before nested txn ends */
      mdbx_cassert(mc, mc->mc_signature == MDBX_MC_SIGNATURE);
      mc->mc_signature = MDBX_MC_WAIT4EOT;
    }
  }
}

MDBX_txn *mdbx_cursor_txn(MDBX_cursor *mc) {
  if (unlikely(!mc || mc->mc_signature != MDBX_MC_SIGNATURE))
    return NULL;
  MDBX_txn *txn = mc->mc_txn;
  if (unlikely(!txn || txn->mt_signature != MDBX_MT_SIGNATURE))
    return NULL;
  if (unlikely(txn->mt_flags & MDBX_TXN_FINISHED))
    return NULL;
  return txn;
}

MDBX_dbi mdbx_cursor_dbi(MDBX_cursor *mc) {
  if (unlikely(!mc || mc->mc_signature != MDBX_MC_SIGNATURE))
    return UINT_MAX;
  return mc->mc_dbi;
}

/* Replace the key for a branch node with a new key.
 * Set MDBX_TXN_ERROR on failure.
 * [in] mc Cursor pointing to the node to operate on.
 * [in] key The new key to use.
 * Returns 0 on success, non-zero on failure. */
static int mdbx_update_key(MDBX_cursor *mc, const MDBX_val *key) {
  MDBX_page *mp;
  MDBX_node *node;
  char *base;
  size_t len;
  int delta, ksize, oksize;
  indx_t ptr, i, numkeys, indx;
  DKBUF;

  indx = mc->mc_ki[mc->mc_top];
  mp = mc->mc_pg[mc->mc_top];
  node = NODEPTR(mp, indx);
  ptr = mp->mp_ptrs[indx];
  if (MDBX_DEBUG) {
    MDBX_val k2;
    char kbuf2[DKBUF_MAXKEYSIZE * 2 + 1];
    k2.iov_base = NODEKEY(node);
    k2.iov_len = node->mn_ksize;
    mdbx_debug("update key %u (ofs %u) [%s] to [%s] on page %" PRIaPGNO, indx,
               ptr, mdbx_dkey(&k2, kbuf2, sizeof(kbuf2)), DKEY(key),
               mp->mp_pgno);
  }

  /* Sizes must be 2-byte aligned. */
  ksize = EVEN(key->iov_len);
  oksize = EVEN(node->mn_ksize);
  delta = ksize - oksize;

  /* Shift node contents if EVEN(key length) changed. */
  if (delta) {
    if (SIZELEFT(mp) < delta) {
      /* not enough space left, do a delete and split */
      mdbx_debug("Not enough room, delta = %d, splitting...", delta);
      pgno_t pgno = NODEPGNO(node);
      mdbx_node_del(mc, 0);
      int rc = mdbx_page_split(mc, key, NULL, pgno, MDBX_SPLIT_REPLACE);
      if (rc == MDBX_SUCCESS && mdbx_audit_enabled())
        rc = mdbx_cursor_check(mc, true);
      return rc;
    }

    numkeys = NUMKEYS(mp);
    for (i = 0; i < numkeys; i++) {
      if (mp->mp_ptrs[i] <= ptr) {
        mdbx_cassert(mc, mp->mp_ptrs[i] >= delta);
        mp->mp_ptrs[i] -= (indx_t)delta;
      }
    }

    base = (char *)mp + mp->mp_upper + PAGEHDRSZ;
    len = ptr - mp->mp_upper + NODESIZE;
    memmove(base - delta, base, len);
    mdbx_cassert(mc, mp->mp_upper >= delta);
    mp->mp_upper -= (indx_t)delta;

    node = NODEPTR(mp, indx);
  }

  /* But even if no shift was needed, update ksize */
  if (node->mn_ksize != key->iov_len)
    node->mn_ksize = (uint16_t)key->iov_len;

  memcpy(NODEKEY(node), key->iov_base, key->iov_len);
  return MDBX_SUCCESS;
}

static void mdbx_cursor_copy(const MDBX_cursor *csrc, MDBX_cursor *cdst);

/* Move a node from csrc to cdst. */
static int mdbx_node_move(MDBX_cursor *csrc, MDBX_cursor *cdst, int fromleft) {
  int rc;
  DKBUF;

  /* Mark src and dst as dirty. */
  if (unlikely((rc = mdbx_page_touch(csrc)) || (rc = mdbx_page_touch(cdst))))
    return rc;

  MDBX_page *const psrc = csrc->mc_pg[csrc->mc_top];
  MDBX_page *const pdst = cdst->mc_pg[cdst->mc_top];
  mdbx_cassert(csrc, PAGETYPE(psrc) == PAGETYPE(pdst));
  mdbx_cassert(csrc, csrc->mc_dbi == cdst->mc_dbi);
  mdbx_cassert(csrc, csrc->mc_top == cdst->mc_top);
  if (unlikely(PAGETYPE(psrc) != PAGETYPE(pdst))) {
  bailout:
    csrc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
    return MDBX_PROBLEM;
  }

  MDBX_val key4move;
  switch (PAGETYPE(psrc)) {
  case P_BRANCH: {
    const MDBX_node *srcnode = NODEPTR(psrc, csrc->mc_ki[csrc->mc_top]);
    mdbx_cassert(csrc, srcnode->mn_flags == 0);
    const pgno_t srcpg = NODEPGNO(srcnode);
    key4move.iov_len = NODEKSZ(srcnode);
    key4move.iov_base = NODEKEY(srcnode);
    if (csrc->mc_ki[csrc->mc_top] == 0) {
      const uint16_t snum = csrc->mc_snum;
      mdbx_cassert(csrc, snum > 0);
      /* must find the lowest key below src */
      rc = mdbx_page_search_lowest(csrc);
      MDBX_page *psrc2 = csrc->mc_pg[csrc->mc_top];
      if (unlikely(rc))
        return rc;
      mdbx_cassert(csrc, IS_LEAF(psrc2));
      if (unlikely(!IS_LEAF(psrc2)))
        goto bailout;
      if (IS_LEAF2(psrc2)) {
        key4move.iov_len = csrc->mc_db->md_xsize;
        key4move.iov_base = LEAF2KEY(psrc2, 0, key4move.iov_len);
      } else {
        const MDBX_node *s2 = NODEPTR(psrc2, 0);
        key4move.iov_len = NODEKSZ(s2);
        key4move.iov_base = NODEKEY(s2);
      }
      csrc->mc_snum = snum;
      csrc->mc_top = snum - 1;
      csrc->mc_ki[csrc->mc_top] = 0;
      /* paranoia */
      mdbx_cassert(csrc, psrc == csrc->mc_pg[csrc->mc_top]);
      mdbx_cassert(csrc, IS_BRANCH(psrc));
      if (unlikely(!IS_BRANCH(psrc)))
        goto bailout;
    }

    if (cdst->mc_ki[cdst->mc_top] == 0) {
      const uint16_t snum = cdst->mc_snum;
      mdbx_cassert(csrc, snum > 0);
      MDBX_cursor mn;
      mdbx_cursor_copy(cdst, &mn);
      mn.mc_xcursor = NULL;
      /* must find the lowest key below dst */
      rc = mdbx_page_search_lowest(&mn);
      if (unlikely(rc))
        return rc;
      MDBX_page *const pdst2 = mn.mc_pg[mn.mc_top];
      mdbx_cassert(cdst, IS_LEAF(pdst2));
      if (unlikely(!IS_LEAF(pdst2)))
        goto bailout;
      MDBX_val key;
      if (IS_LEAF2(pdst2)) {
        key.iov_len = mn.mc_db->md_xsize;
        key.iov_base = LEAF2KEY(pdst2, 0, key.iov_len);
      } else {
        MDBX_node *s2 = NODEPTR(pdst2, 0);
        key.iov_len = NODEKSZ(s2);
        key.iov_base = NODEKEY(s2);
      }
      mn.mc_snum = snum;
      mn.mc_top = snum - 1;
      mn.mc_ki[mn.mc_top] = 0;
      rc = mdbx_update_key(&mn, &key);
      if (unlikely(rc))
        return rc;
    }

    mdbx_debug("moving %s-node %u [%s] on page %" PRIaPGNO
               " to node %u on page %" PRIaPGNO,
               "branch", csrc->mc_ki[csrc->mc_top], DKEY(&key4move),
               psrc->mp_pgno, cdst->mc_ki[cdst->mc_top], pdst->mp_pgno);
    /* Add the node to the destination page. */
    rc =
        mdbx_node_add_branch(cdst, cdst->mc_ki[cdst->mc_top], &key4move, srcpg);
  } break;

  case P_LEAF: {
    const MDBX_node *srcnode = NODEPTR(psrc, csrc->mc_ki[csrc->mc_top]);
    MDBX_val data;
    data.iov_len = NODEDSZ(srcnode);
    data.iov_base = NODEDATA(srcnode);
    key4move.iov_len = NODEKSZ(srcnode);
    key4move.iov_base = NODEKEY(srcnode);
    mdbx_debug("moving %s-node %u [%s] on page %" PRIaPGNO
               " to node %u on page %" PRIaPGNO,
               "leaf", csrc->mc_ki[csrc->mc_top], DKEY(&key4move),
               psrc->mp_pgno, cdst->mc_ki[cdst->mc_top], pdst->mp_pgno);
    /* Add the node to the destination page. */
    rc = mdbx_node_add_leaf(cdst, cdst->mc_ki[cdst->mc_top], &key4move, &data,
                            srcnode->mn_flags);
  } break;

  case P_LEAF | P_LEAF2: {
    key4move.iov_len = csrc->mc_db->md_xsize;
    key4move.iov_base =
        LEAF2KEY(psrc, csrc->mc_ki[csrc->mc_top], key4move.iov_len);
    mdbx_debug("moving %s-node %u [%s] on page %" PRIaPGNO
               " to node %u on page %" PRIaPGNO,
               "leaf2", csrc->mc_ki[csrc->mc_top], DKEY(&key4move),
               psrc->mp_pgno, cdst->mc_ki[cdst->mc_top], pdst->mp_pgno);
    /* Add the node to the destination page. */
    rc = mdbx_node_add_leaf2(cdst, cdst->mc_ki[cdst->mc_top], &key4move);
  } break;

  default:
    goto bailout;
  }

  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  /* Delete the node from the source page. */
  mdbx_node_del(csrc, key4move.iov_len);

  mdbx_cassert(csrc, psrc == csrc->mc_pg[csrc->mc_top]);
  mdbx_cassert(cdst, pdst == cdst->mc_pg[cdst->mc_top]);
  mdbx_cassert(csrc, PAGETYPE(psrc) == PAGETYPE(pdst));

  {
    /* Adjust other cursors pointing to mp */
    MDBX_cursor *m2, *m3;
    const MDBX_dbi dbi = csrc->mc_dbi;
    mdbx_cassert(csrc, csrc->mc_top == cdst->mc_top);
    if (fromleft) {
      /* If we're adding on the left, bump others up */
      for (m2 = csrc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
        m3 = (csrc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
        if (!(m3->mc_flags & C_INITIALIZED) || m3->mc_top < csrc->mc_top)
          continue;
        if (m3 != cdst && m3->mc_pg[csrc->mc_top] == pdst &&
            m3->mc_ki[csrc->mc_top] >= cdst->mc_ki[csrc->mc_top]) {
          m3->mc_ki[csrc->mc_top]++;
        }
        if (m3 != csrc && m3->mc_pg[csrc->mc_top] == psrc &&
            m3->mc_ki[csrc->mc_top] == csrc->mc_ki[csrc->mc_top]) {
          m3->mc_pg[csrc->mc_top] = pdst;
          m3->mc_ki[csrc->mc_top] = cdst->mc_ki[cdst->mc_top];
          mdbx_cassert(csrc, csrc->mc_top > 0);
          m3->mc_ki[csrc->mc_top - 1]++;
        }
        if (XCURSOR_INITED(m3) && IS_LEAF(psrc))
          XCURSOR_REFRESH(m3, m3->mc_pg[csrc->mc_top], m3->mc_ki[csrc->mc_top]);
      }
    } else {
      /* Adding on the right, bump others down */
      for (m2 = csrc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
        m3 = (csrc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
        if (m3 == csrc)
          continue;
        if (!(m3->mc_flags & C_INITIALIZED) || m3->mc_top < csrc->mc_top)
          continue;
        if (m3->mc_pg[csrc->mc_top] == psrc) {
          if (!m3->mc_ki[csrc->mc_top]) {
            m3->mc_pg[csrc->mc_top] = pdst;
            m3->mc_ki[csrc->mc_top] = cdst->mc_ki[cdst->mc_top];
            mdbx_cassert(csrc, csrc->mc_top > 0);
            m3->mc_ki[csrc->mc_top - 1]--;
          } else {
            m3->mc_ki[csrc->mc_top]--;
          }
          if (XCURSOR_INITED(m3) && IS_LEAF(psrc))
            XCURSOR_REFRESH(m3, m3->mc_pg[csrc->mc_top],
                            m3->mc_ki[csrc->mc_top]);
        }
      }
    }
  }

  /* Update the parent separators. */
  if (csrc->mc_ki[csrc->mc_top] == 0) {
    mdbx_cassert(csrc, csrc->mc_top > 0);
    if (csrc->mc_ki[csrc->mc_top - 1] != 0) {
      MDBX_val key;
      if (IS_LEAF2(psrc)) {
        key.iov_len = psrc->mp_leaf2_ksize;
        key.iov_base = LEAF2KEY(psrc, 0, key.iov_len);
      } else {
        MDBX_node *srcnode = NODEPTR(psrc, 0);
        key.iov_len = NODEKSZ(srcnode);
        key.iov_base = NODEKEY(srcnode);
      }
      mdbx_debug("update separator for source page %" PRIaPGNO " to [%s]",
                 psrc->mp_pgno, DKEY(&key));
      MDBX_cursor mn;
      mdbx_cursor_copy(csrc, &mn);
      mn.mc_xcursor = NULL;
      mdbx_cassert(csrc, mn.mc_snum > 0);
      mn.mc_snum--;
      mn.mc_top--;
      /* We want mdbx_rebalance to find mn when doing fixups */
      WITH_CURSOR_TRACKING(mn, rc = mdbx_update_key(&mn, &key));
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
    if (IS_BRANCH(psrc)) {
      const MDBX_val nullkey = {0, 0};
      const indx_t ix = csrc->mc_ki[csrc->mc_top];
      csrc->mc_ki[csrc->mc_top] = 0;
      rc = mdbx_update_key(csrc, &nullkey);
      csrc->mc_ki[csrc->mc_top] = ix;
      mdbx_cassert(csrc, rc == MDBX_SUCCESS);
    }
  }

  if (cdst->mc_ki[cdst->mc_top] == 0) {
    mdbx_cassert(cdst, cdst->mc_top > 0);
    if (cdst->mc_ki[cdst->mc_top - 1] != 0) {
      MDBX_val key;
      if (IS_LEAF2(pdst)) {
        key.iov_len = pdst->mp_leaf2_ksize;
        key.iov_base = LEAF2KEY(pdst, 0, key.iov_len);
      } else {
        MDBX_node *srcnode = NODEPTR(pdst, 0);
        key.iov_len = NODEKSZ(srcnode);
        key.iov_base = NODEKEY(srcnode);
      }
      mdbx_debug("update separator for destination page %" PRIaPGNO " to [%s]",
                 pdst->mp_pgno, DKEY(&key));
      MDBX_cursor mn;
      mdbx_cursor_copy(cdst, &mn);
      mn.mc_xcursor = NULL;
      mdbx_cassert(cdst, mn.mc_snum > 0);
      mn.mc_snum--;
      mn.mc_top--;
      /* We want mdbx_rebalance to find mn when doing fixups */
      WITH_CURSOR_TRACKING(mn, rc = mdbx_update_key(&mn, &key));
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
    if (IS_BRANCH(pdst)) {
      const MDBX_val nullkey = {0, 0};
      const indx_t ix = cdst->mc_ki[cdst->mc_top];
      cdst->mc_ki[cdst->mc_top] = 0;
      rc = mdbx_update_key(cdst, &nullkey);
      cdst->mc_ki[cdst->mc_top] = ix;
      mdbx_cassert(cdst, rc == MDBX_SUCCESS);
    }
  }

  return MDBX_SUCCESS;
}

/* Merge one page into another.
 *
 * The nodes from the page pointed to by csrc will be copied to the page
 * pointed to by cdst and then the csrc page will be freed.
 *
 * [in] csrc Cursor pointing to the source page.
 * [in] cdst Cursor pointing to the destination page.
 *
 * Returns 0 on success, non-zero on failure. */
static int mdbx_page_merge(MDBX_cursor *csrc, MDBX_cursor *cdst) {
  MDBX_node *srcnode;
  MDBX_val key;
  int rc;

  mdbx_cassert(csrc, csrc != cdst);

  /* Mark dst as dirty. */
  if (unlikely(rc = mdbx_page_touch(cdst)))
    return rc;

  MDBX_page *const psrc = csrc->mc_pg[csrc->mc_top];
  MDBX_page *const pdst = cdst->mc_pg[cdst->mc_top];

  mdbx_debug("merging page %" PRIaPGNO " into %" PRIaPGNO, psrc->mp_pgno,
             pdst->mp_pgno);

  mdbx_cassert(csrc, PAGETYPE(psrc) == PAGETYPE(pdst));
  mdbx_cassert(csrc,
               csrc->mc_dbi == cdst->mc_dbi && csrc->mc_db == cdst->mc_db);
  mdbx_cassert(csrc, csrc->mc_snum > 1); /* can't merge root page */
  mdbx_cassert(cdst, cdst->mc_snum > 1);
  mdbx_cassert(cdst, cdst->mc_snum < cdst->mc_db->md_depth ||
                         IS_LEAF(cdst->mc_pg[cdst->mc_db->md_depth - 1]));
  mdbx_cassert(csrc, csrc->mc_snum < csrc->mc_db->md_depth ||
                         IS_LEAF(csrc->mc_pg[csrc->mc_db->md_depth - 1]));
  const int pagetype = PAGETYPE(psrc);

  /* Move all nodes from src to dst */
  const unsigned nkeys = NUMKEYS(pdst);
  unsigned j = nkeys;
  if (unlikely(pagetype & P_LEAF2)) {
    key.iov_len = csrc->mc_db->md_xsize;
    key.iov_base = PAGEDATA(psrc);
    for (unsigned i = 0; i < NUMKEYS(psrc); i++, j++) {
      rc = mdbx_node_add_leaf2(cdst, j, &key);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      key.iov_base = (char *)key.iov_base + key.iov_len;
    }
  } else {
    for (unsigned i = 0; i < NUMKEYS(psrc); i++, j++) {
      srcnode = NODEPTR(psrc, i);
      if (i == 0 && (pagetype & P_BRANCH)) {
        MDBX_cursor mn;
        mdbx_cursor_copy(csrc, &mn);
        mn.mc_xcursor = NULL;
        /* must find the lowest key below src */
        rc = mdbx_page_search_lowest(&mn);
        if (unlikely(rc))
          return rc;
        if (IS_LEAF2(mn.mc_pg[mn.mc_top])) {
          key.iov_len = mn.mc_db->md_xsize;
          key.iov_base = LEAF2KEY(mn.mc_pg[mn.mc_top], 0, key.iov_len);
        } else {
          MDBX_node *s2 = NODEPTR(mn.mc_pg[mn.mc_top], 0);
          key.iov_len = NODEKSZ(s2);
          key.iov_base = NODEKEY(s2);
        }
      } else {
        key.iov_len = srcnode->mn_ksize;
        key.iov_base = NODEKEY(srcnode);
      }

      if (pagetype & P_LEAF) {
        MDBX_val data;
        data.iov_len = NODEDSZ(srcnode);
        data.iov_base = NODEDATA(srcnode);
        rc = mdbx_node_add_leaf(cdst, j, &key, &data, srcnode->mn_flags);
      } else {
        mdbx_cassert(csrc, srcnode->mn_flags == 0);
        rc = mdbx_node_add_branch(cdst, j, &key, NODEPGNO(srcnode));
      }
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
    }
  }

  mdbx_debug("dst page %" PRIaPGNO " now has %u keys (%.1f%% filled)",
             pdst->mp_pgno, NUMKEYS(pdst),
             PAGEFILL(cdst->mc_txn->mt_env, pdst) / 10.24);

  mdbx_cassert(csrc, psrc == csrc->mc_pg[csrc->mc_top]);
  mdbx_cassert(cdst, pdst == cdst->mc_pg[cdst->mc_top]);

  /* Unlink the src page from parent and add to free list. */
  csrc->mc_top--;
  mdbx_node_del(csrc, 0);
  if (csrc->mc_ki[csrc->mc_top] == 0) {
    const MDBX_val nullkey = {0, 0};
    rc = mdbx_update_key(csrc, &nullkey);
    if (unlikely(rc)) {
      csrc->mc_top++;
      return rc;
    }
  }
  csrc->mc_top++;

  mdbx_cassert(csrc, psrc == csrc->mc_pg[csrc->mc_top]);
  mdbx_cassert(cdst, pdst == cdst->mc_pg[cdst->mc_top]);

  {
    /* Adjust other cursors pointing to mp */
    MDBX_cursor *m2, *m3;
    const MDBX_dbi dbi = csrc->mc_dbi;
    const unsigned top = csrc->mc_top;

    for (m2 = csrc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
      m3 = (csrc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
      if (m3 == csrc || top >= m3->mc_snum)
        continue;
      if (m3->mc_pg[top] == psrc) {
        m3->mc_pg[top] = pdst;
        mdbx_cassert(m3, nkeys + m3->mc_ki[top] <= UINT16_MAX);
        m3->mc_ki[top] += (indx_t)nkeys;
        m3->mc_ki[top - 1] = cdst->mc_ki[top - 1];
      } else if (m3->mc_pg[top - 1] == csrc->mc_pg[top - 1] &&
                 m3->mc_ki[top - 1] > csrc->mc_ki[top - 1]) {
        m3->mc_ki[top - 1]--;
      }
      if (XCURSOR_INITED(m3) && IS_LEAF(psrc))
        XCURSOR_REFRESH(m3, m3->mc_pg[top], m3->mc_ki[top]);
    }
  }

  /* If not operating on FreeDB, allow this page to be reused
   * in this txn. Otherwise just add to free list. */
  rc = mdbx_page_loose(csrc, psrc);
  if (unlikely(rc))
    return rc;

  mdbx_cassert(cdst, cdst->mc_db->md_entries > 0);
  mdbx_cassert(cdst, cdst->mc_snum <= cdst->mc_db->md_depth);
  mdbx_cassert(cdst, cdst->mc_top > 0);
  mdbx_cassert(cdst, cdst->mc_snum == cdst->mc_top + 1);
  MDBX_page *const top_page = cdst->mc_pg[cdst->mc_top];
  const indx_t top_indx = cdst->mc_ki[cdst->mc_top];
  const uint16_t save_snum = cdst->mc_snum;
  const uint16_t save_depth = cdst->mc_db->md_depth;
  mdbx_cursor_pop(cdst);
  rc = mdbx_rebalance(cdst);
  if (unlikely(rc))
    return rc;

  mdbx_cassert(cdst, cdst->mc_db->md_entries > 0);
  mdbx_cassert(cdst, cdst->mc_snum <= cdst->mc_db->md_depth);
  mdbx_cassert(cdst, cdst->mc_snum == cdst->mc_top + 1);

  if (IS_LEAF(cdst->mc_pg[cdst->mc_top])) {
    /* LY: don't touch cursor if top-page is a LEAF */
    mdbx_cassert(cdst, IS_LEAF(cdst->mc_pg[cdst->mc_top]) ||
                           PAGETYPE(cdst->mc_pg[cdst->mc_top]) == pagetype);
    return MDBX_SUCCESS;
  }

  if (pagetype != PAGETYPE(top_page)) {
    /* LY: LEAF-page becomes BRANCH, unable restore cursor's stack */
    goto bailout;
  }

  if (top_page == cdst->mc_pg[cdst->mc_top]) {
    /* LY: don't touch cursor if prev top-page already on the top */
    mdbx_cassert(cdst, cdst->mc_ki[cdst->mc_top] == top_indx);
    mdbx_cassert(cdst, IS_LEAF(cdst->mc_pg[cdst->mc_top]) ||
                           PAGETYPE(cdst->mc_pg[cdst->mc_top]) == pagetype);
    return MDBX_SUCCESS;
  }

  const int new_snum = save_snum - save_depth + cdst->mc_db->md_depth;
  if (unlikely(new_snum < 1 || new_snum > cdst->mc_db->md_depth)) {
    /* LY: out of range, unable restore cursor's stack */
    goto bailout;
  }

  if (top_page == cdst->mc_pg[new_snum - 1]) {
    mdbx_cassert(cdst, cdst->mc_ki[new_snum - 1] == top_indx);
    /* LY: restore cursor stack */
    cdst->mc_snum = (uint16_t)new_snum;
    cdst->mc_top = (uint16_t)new_snum - 1;
    mdbx_cassert(cdst, cdst->mc_snum < cdst->mc_db->md_depth ||
                           IS_LEAF(cdst->mc_pg[cdst->mc_db->md_depth - 1]));
    mdbx_cassert(cdst, IS_LEAF(cdst->mc_pg[cdst->mc_top]) ||
                           PAGETYPE(cdst->mc_pg[cdst->mc_top]) == pagetype);
    return MDBX_SUCCESS;
  }

  MDBX_page *const stub_page = (MDBX_page *)(~(uintptr_t)top_page);
  const indx_t stub_indx = top_indx;
  if (save_depth > cdst->mc_db->md_depth &&
      ((cdst->mc_pg[save_snum - 1] == top_page &&
        cdst->mc_ki[save_snum - 1] == top_indx) ||
       (cdst->mc_pg[save_snum - 1] == stub_page &&
        cdst->mc_ki[save_snum - 1] == stub_indx))) {
    /* LY: restore cursor stack */
    cdst->mc_pg[new_snum - 1] = top_page;
    cdst->mc_ki[new_snum - 1] = top_indx;
    cdst->mc_pg[new_snum] = (MDBX_page *)(~(uintptr_t)cdst->mc_pg[new_snum]);
    cdst->mc_ki[new_snum] = ~cdst->mc_ki[new_snum];
    cdst->mc_snum = (uint16_t)new_snum;
    cdst->mc_top = (uint16_t)new_snum - 1;
    mdbx_cassert(cdst, cdst->mc_snum < cdst->mc_db->md_depth ||
                           IS_LEAF(cdst->mc_pg[cdst->mc_db->md_depth - 1]));
    mdbx_cassert(cdst, IS_LEAF(cdst->mc_pg[cdst->mc_top]) ||
                           PAGETYPE(cdst->mc_pg[cdst->mc_top]) == pagetype);
    return MDBX_SUCCESS;
  }

bailout:
  /* LY: unable restore cursor's stack */
  cdst->mc_flags &= ~C_INITIALIZED;
  return MDBX_CURSOR_FULL;
}

/* Copy the contents of a cursor.
 * [in] csrc The cursor to copy from.
 * [out] cdst The cursor to copy to. */
static void mdbx_cursor_copy(const MDBX_cursor *csrc, MDBX_cursor *cdst) {
  unsigned i;

  mdbx_cassert(csrc,
               csrc->mc_txn->mt_txnid >= *csrc->mc_txn->mt_env->me_oldest);
  cdst->mc_txn = csrc->mc_txn;
  cdst->mc_dbi = csrc->mc_dbi;
  cdst->mc_db = csrc->mc_db;
  cdst->mc_dbx = csrc->mc_dbx;
  cdst->mc_snum = csrc->mc_snum;
  cdst->mc_top = csrc->mc_top;
  cdst->mc_flags = csrc->mc_flags;

  for (i = 0; i < csrc->mc_snum; i++) {
    cdst->mc_pg[i] = csrc->mc_pg[i];
    cdst->mc_ki[i] = csrc->mc_ki[i];
  }
}

/* Rebalance the tree after a delete operation.
 * [in] mc Cursor pointing to the page where rebalancing should begin.
 * Returns 0 on success, non-zero on failure. */
static int mdbx_rebalance(MDBX_cursor *mc) {
  MDBX_node *node;
  int rc;
  unsigned minkeys, thresh;

  mdbx_cassert(mc, mc->mc_snum > 0);
  mdbx_cassert(mc, mc->mc_snum < mc->mc_db->md_depth ||
                       IS_LEAF(mc->mc_pg[mc->mc_db->md_depth - 1]));
  const int pagetype = PAGETYPE(mc->mc_pg[mc->mc_top]);
  if (pagetype == P_BRANCH) {
    minkeys = 2;
    thresh = 1;
  } else {
    minkeys = 1;
    thresh = FILL_THRESHOLD;
  }
  mdbx_debug("rebalancing %s page %" PRIaPGNO " (has %u keys, %.1f%% full)",
             (pagetype & P_LEAF) ? "leaf" : "branch",
             mc->mc_pg[mc->mc_top]->mp_pgno, NUMKEYS(mc->mc_pg[mc->mc_top]),
             PAGEFILL(mc->mc_txn->mt_env, mc->mc_pg[mc->mc_top]) / 10.24);

  if (PAGEFILL(mc->mc_txn->mt_env, mc->mc_pg[mc->mc_top]) >= thresh &&
      NUMKEYS(mc->mc_pg[mc->mc_top]) >= minkeys) {
    mdbx_debug("no need to rebalance page %" PRIaPGNO ", above fill threshold",
               mc->mc_pg[mc->mc_top]->mp_pgno);
    mdbx_cassert(mc, mc->mc_db->md_entries > 0);
    return MDBX_SUCCESS;
  }

  if (mc->mc_snum < 2) {
    MDBX_page *const mp = mc->mc_pg[0];
    const unsigned nkeys = NUMKEYS(mp);
    mdbx_cassert(mc, (mc->mc_db->md_entries == 0) == (nkeys == 0));
    if (IS_SUBP(mp)) {
      mdbx_debug("Can't rebalance a subpage, ignoring");
      mdbx_cassert(mc, pagetype & P_LEAF);
      return MDBX_SUCCESS;
    }
    if (nkeys == 0) {
      mdbx_cassert(mc, IS_LEAF(mp));
      mdbx_debug("tree is completely empty");
      mc->mc_db->md_root = P_INVALID;
      mc->mc_db->md_depth = 0;
      mdbx_cassert(mc, mc->mc_db->md_branch_pages == 0 &&
                           mc->mc_db->md_overflow_pages == 0 &&
                           mc->mc_db->md_leaf_pages == 1);
      mc->mc_db->md_leaf_pages = 0;
      if (mc->mc_flags & C_SUB)
        mdbx_outer_db(mc)->md_leaf_pages -= 1;
      rc = mdbx_pnl_append(&mc->mc_txn->mt_befree_pages, mp->mp_pgno);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      /* Adjust cursors pointing to mp */
      const MDBX_dbi dbi = mc->mc_dbi;
      for (MDBX_cursor *m2 = mc->mc_txn->mt_cursors[dbi]; m2;
           m2 = m2->mc_next) {
        MDBX_cursor *m3 =
            (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
        if (m3 == mc || !(m3->mc_flags & C_INITIALIZED))
          continue;
        if (m3->mc_pg[0] == mp) {
          m3->mc_snum = 0;
          m3->mc_top = 0;
          m3->mc_flags &= ~C_INITIALIZED;
        }
      }
      mc->mc_snum = 0;
      mc->mc_top = 0;
      mc->mc_flags &= ~C_INITIALIZED;
    } else if (IS_BRANCH(mp) && nkeys == 1) {
      mdbx_debug("collapsing root page!");
      rc = mdbx_pnl_append(&mc->mc_txn->mt_befree_pages, mp->mp_pgno);
      if (unlikely(rc))
        return rc;
      mc->mc_db->md_root = NODEPGNO(NODEPTR(mp, 0));
      rc = mdbx_page_get(mc, mc->mc_db->md_root, &mc->mc_pg[0], NULL);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      mc->mc_db->md_depth--;
      mc->mc_db->md_branch_pages--;
      if (mc->mc_flags & C_SUB)
        mdbx_outer_db(mc)->md_branch_pages -= 1;
      mc->mc_ki[0] = mc->mc_ki[1];
      for (int i = 1; i < mc->mc_db->md_depth; i++) {
        mc->mc_pg[i] = mc->mc_pg[i + 1];
        mc->mc_ki[i] = mc->mc_ki[i + 1];
      }

      /* Adjust other cursors pointing to mp */
      MDBX_cursor *m2, *m3;
      MDBX_dbi dbi = mc->mc_dbi;

      for (m2 = mc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
        m3 = (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
        if (m3 == mc || !(m3->mc_flags & C_INITIALIZED))
          continue;
        if (m3->mc_pg[0] == mp) {
          for (int i = 0; i < mc->mc_db->md_depth; i++) {
            m3->mc_pg[i] = m3->mc_pg[i + 1];
            m3->mc_ki[i] = m3->mc_ki[i + 1];
          }
          m3->mc_snum--;
          m3->mc_top--;
        }
      }

      mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]) ||
                           PAGETYPE(mc->mc_pg[mc->mc_top]) == pagetype);
      mdbx_cassert(mc, mc->mc_snum < mc->mc_db->md_depth ||
                           IS_LEAF(mc->mc_pg[mc->mc_db->md_depth - 1]));
    } else {
      mdbx_debug("root page %" PRIaPGNO
                 " doesn't need rebalancing (flags 0x%x)",
                 mp->mp_pgno, mp->mp_flags);
    }
    return MDBX_SUCCESS;
  }

  /* The parent (branch page) must have at least 2 pointers,
   * otherwise the tree is invalid. */
  const unsigned pre_top = mc->mc_top - 1;
  mdbx_cassert(mc, IS_BRANCH(mc->mc_pg[pre_top]));
  mdbx_cassert(mc, !IS_SUBP(mc->mc_pg[0]));
  mdbx_cassert(mc, NUMKEYS(mc->mc_pg[pre_top]) > 1);

  /* Leaf page fill factor is below the threshold.
   * Try to move keys from left or right neighbor, or
   * merge with a neighbor page. */

  /* Find neighbors. */
  MDBX_cursor mn;
  mdbx_cursor_copy(mc, &mn);
  mn.mc_xcursor = NULL;

  indx_t oldki = mc->mc_ki[mc->mc_top];
  bool fromleft;
  if (mc->mc_ki[pre_top] == 0) {
    /* We're the leftmost leaf in our parent. */
    mdbx_debug("reading right neighbor");
    mn.mc_ki[pre_top]++;
    node = NODEPTR(mc->mc_pg[pre_top], mn.mc_ki[pre_top]);
    rc = mdbx_page_get(mc, NODEPGNO(node), &mn.mc_pg[mn.mc_top], NULL);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    mdbx_cassert(mc, PAGETYPE(mn.mc_pg[mn.mc_top]) ==
                         PAGETYPE(mc->mc_pg[mc->mc_top]));
    mn.mc_ki[mn.mc_top] = 0;
    mc->mc_ki[mc->mc_top] = NUMKEYS(mc->mc_pg[mc->mc_top]);
    fromleft = false;
  } else {
    /* There is at least one neighbor to the left. */
    mdbx_debug("reading left neighbor");
    mn.mc_ki[pre_top]--;
    node = NODEPTR(mc->mc_pg[pre_top], mn.mc_ki[pre_top]);
    rc = mdbx_page_get(mc, NODEPGNO(node), &mn.mc_pg[mn.mc_top], NULL);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    mdbx_cassert(mc, PAGETYPE(mn.mc_pg[mn.mc_top]) ==
                         PAGETYPE(mc->mc_pg[mc->mc_top]));
    mn.mc_ki[mn.mc_top] = NUMKEYS(mn.mc_pg[mn.mc_top]) - 1;
    mc->mc_ki[mc->mc_top] = 0;
    fromleft = true;
  }

  mdbx_debug("found neighbor page %" PRIaPGNO " (%u keys, %.1f%% full)",
             mn.mc_pg[mn.mc_top]->mp_pgno, NUMKEYS(mn.mc_pg[mn.mc_top]),
             PAGEFILL(mc->mc_txn->mt_env, mn.mc_pg[mn.mc_top]) / 10.24);

  /* If the neighbor page is above threshold and has enough keys,
   * move one key from it. Otherwise we should try to merge them.
   * (A branch page must never have less than 2 keys.) */
  if (PAGEFILL(mc->mc_txn->mt_env, mn.mc_pg[mn.mc_top]) >= thresh &&
      NUMKEYS(mn.mc_pg[mn.mc_top]) > minkeys) {
    rc = mdbx_node_move(&mn, mc, fromleft);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    oldki += fromleft /* if we inserted on left, bump position up */;
    mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]) ||
                         PAGETYPE(mc->mc_pg[mc->mc_top]) == pagetype);
    mdbx_cassert(mc, mc->mc_snum < mc->mc_db->md_depth ||
                         IS_LEAF(mc->mc_pg[mc->mc_db->md_depth - 1]));
  } else {
    if (!fromleft) {
      rc = mdbx_page_merge(&mn, mc);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]) ||
                           PAGETYPE(mc->mc_pg[mc->mc_top]) == pagetype);
      mdbx_cassert(mc, mc->mc_snum < mc->mc_db->md_depth ||
                           IS_LEAF(mc->mc_pg[mc->mc_db->md_depth - 1]));
    } else {
      oldki += NUMKEYS(mn.mc_pg[mn.mc_top]);
      mn.mc_ki[mn.mc_top] += mc->mc_ki[mn.mc_top] + 1;
      /* We want mdbx_rebalance to find mn when doing fixups */
      WITH_CURSOR_TRACKING(mn, rc = mdbx_page_merge(mc, &mn));
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;
      mdbx_cursor_copy(&mn, mc);
      mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]) ||
                           PAGETYPE(mc->mc_pg[mc->mc_top]) == pagetype);
      mdbx_cassert(mc, mc->mc_snum < mc->mc_db->md_depth ||
                           IS_LEAF(mc->mc_pg[mc->mc_db->md_depth - 1]));
    }
    mc->mc_flags &= ~C_EOF;
  }
  mc->mc_ki[mc->mc_top] = oldki;
  return MDBX_SUCCESS;
}

static __cold int mdbx_cursor_check(MDBX_cursor *mc, bool pending) {
  mdbx_cassert(mc, mc->mc_top == mc->mc_snum - 1);
  if (unlikely(mc->mc_top != mc->mc_snum - 1))
    return MDBX_CURSOR_FULL;
  mdbx_cassert(mc, pending ? mc->mc_snum <= mc->mc_db->md_depth
                           : mc->mc_snum == mc->mc_db->md_depth);
  if (unlikely(pending ? mc->mc_snum > mc->mc_db->md_depth
                       : mc->mc_snum != mc->mc_db->md_depth))
    return MDBX_CURSOR_FULL;

  for (int n = 0; n < mc->mc_snum; ++n) {
    MDBX_page *mp = mc->mc_pg[n];
    const unsigned numkeys = NUMKEYS(mp);
    const bool expect_branch = (n < mc->mc_db->md_depth - 1) ? true : false;
    const bool expect_nested_leaf =
        (n + 1 == mc->mc_db->md_depth - 1) ? true : false;
    const bool branch = IS_BRANCH(mp) ? true : false;
    mdbx_cassert(mc, branch == expect_branch);
    if (unlikely(branch != expect_branch))
      return MDBX_CURSOR_FULL;
    if (!pending) {
      mdbx_cassert(mc, numkeys > mc->mc_ki[n] ||
                           (!branch && numkeys == mc->mc_ki[n] &&
                            (mc->mc_flags & C_EOF) != 0));
      if (unlikely(numkeys <= mc->mc_ki[n] &&
                   !(!branch && numkeys == mc->mc_ki[n] &&
                     (mc->mc_flags & C_EOF) != 0)))
        return MDBX_CURSOR_FULL;
    } else {
      mdbx_cassert(mc, numkeys + 1 >= mc->mc_ki[n]);
      if (unlikely(numkeys + 1 < mc->mc_ki[n]))
        return MDBX_CURSOR_FULL;
    }

    for (unsigned i = 0; i < numkeys; ++i) {
      MDBX_node *node = NODEPTR(mp, i);
      if (branch) {
        mdbx_cassert(mc, node->mn_flags == 0);
        if (unlikely(node->mn_flags != 0))
          return MDBX_CURSOR_FULL;
        pgno_t pgno = NODEPGNO(node);
        MDBX_page *np;
        int rc = mdbx_page_get(mc, pgno, &np, NULL);
        mdbx_cassert(mc, rc == MDBX_SUCCESS);
        if (unlikely(rc != MDBX_SUCCESS))
          return rc;
        const bool nested_leaf = IS_LEAF(np) ? true : false;
        mdbx_cassert(mc, nested_leaf == expect_nested_leaf);
        if (unlikely(nested_leaf != expect_nested_leaf))
          return MDBX_CURSOR_FULL;
      }
    }
  }
  return MDBX_SUCCESS;
}

/* Complete a delete operation started by mdbx_cursor_del(). */
static int mdbx_cursor_del0(MDBX_cursor *mc) {
  int rc;
  MDBX_page *mp;
  indx_t ki;
  unsigned nkeys;
  MDBX_cursor *m2, *m3;
  MDBX_dbi dbi = mc->mc_dbi;

  mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]));
  ki = mc->mc_ki[mc->mc_top];
  mp = mc->mc_pg[mc->mc_top];
  mdbx_node_del(mc, mc->mc_db->md_xsize);
  mc->mc_db->md_entries--;
  {
    /* Adjust other cursors pointing to mp */
    for (m2 = mc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
      m3 = (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
      if (m3 == mc || !(m2->mc_flags & m3->mc_flags & C_INITIALIZED))
        continue;
      if (m3->mc_snum < mc->mc_snum)
        continue;
      if (m3->mc_pg[mc->mc_top] == mp) {
        if (m3->mc_ki[mc->mc_top] == ki) {
          m3->mc_flags |= C_DEL;
          if (mc->mc_db->md_flags & MDBX_DUPSORT) {
            /* Sub-cursor referred into dataset which is gone */
            m3->mc_xcursor->mx_cursor.mc_flags &= ~(C_INITIALIZED | C_EOF);
          }
          continue;
        } else if (m3->mc_ki[mc->mc_top] > ki) {
          m3->mc_ki[mc->mc_top]--;
        }
        if (XCURSOR_INITED(m3))
          XCURSOR_REFRESH(m3, m3->mc_pg[mc->mc_top], m3->mc_ki[mc->mc_top]);
      }
    }
  }
  rc = mdbx_rebalance(mc);

  if (likely(rc == MDBX_SUCCESS)) {
    /* DB is totally empty now, just bail out.
     * Other cursors adjustments were already done
     * by mdbx_rebalance and aren't needed here. */
    if (!mc->mc_snum) {
      mdbx_cassert(mc, mc->mc_db->md_entries == 0 && mc->mc_db->md_depth == 0 &&
                           mc->mc_db->md_root == P_INVALID);
      mc->mc_flags |= C_DEL | C_EOF;
      return rc;
    }

    ki = mc->mc_ki[mc->mc_top];
    mp = mc->mc_pg[mc->mc_top];
    mdbx_cassert(mc, IS_LEAF(mc->mc_pg[mc->mc_top]));
    nkeys = NUMKEYS(mp);
    mdbx_cassert(mc, (mc->mc_db->md_entries > 0 && nkeys > 0) ||
                         ((mc->mc_flags & C_SUB) &&
                          mc->mc_db->md_entries == 0 && nkeys == 0));

    /* Adjust THIS and other cursors pointing to mp */
    for (m2 = mc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
      m3 = (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
      if (m3 == mc || !(m2->mc_flags & m3->mc_flags & C_INITIALIZED))
        continue;
      if (m3->mc_snum < mc->mc_snum)
        continue;
      if (m3->mc_pg[mc->mc_top] == mp) {
        /* if m3 points past last node in page, find next sibling */
        if (m3->mc_ki[mc->mc_top] >= nkeys) {
          rc = mdbx_cursor_sibling(m3, true);
          if (rc == MDBX_NOTFOUND) {
            m3->mc_flags |= C_EOF;
            rc = MDBX_SUCCESS;
            continue;
          } else if (unlikely(rc != MDBX_SUCCESS))
            break;
        }
        if (m3->mc_ki[mc->mc_top] >= ki || m3->mc_pg[mc->mc_top] != mp) {
          if ((mc->mc_db->md_flags & MDBX_DUPSORT) != 0 &&
              (m3->mc_flags & C_EOF) == 0) {
            MDBX_node *node =
                NODEPTR(m3->mc_pg[m3->mc_top], m3->mc_ki[m3->mc_top]);
            /* If this node has dupdata, it may need to be reinited
             * because its data has moved.
             * If the xcursor was not initd it must be reinited.
             * Else if node points to a subDB, nothing is needed. */
            if (node->mn_flags & F_DUPDATA) {
              if (m3->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED) {
                if (!(node->mn_flags & F_SUBDATA))
                  m3->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(node);
              } else {
                rc = mdbx_xcursor_init1(m3, node);
                if (unlikely(rc != MDBX_SUCCESS))
                  break;
                m3->mc_xcursor->mx_cursor.mc_flags |= C_DEL;
              }
            }
          }
        }
      }
    }

    if (mc->mc_ki[mc->mc_top] >= nkeys) {
      rc = mdbx_cursor_sibling(mc, true);
      if (rc == MDBX_NOTFOUND) {
        mc->mc_flags |= C_EOF;
        rc = MDBX_SUCCESS;
      }
    }
    if ((mc->mc_db->md_flags & MDBX_DUPSORT) != 0 &&
        (mc->mc_flags & C_EOF) == 0) {
      MDBX_node *node = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
      /* If this node has dupdata, it may need to be reinited
       * because its data has moved.
       * If the xcursor was not initd it must be reinited.
       * Else if node points to a subDB, nothing is needed. */
      if (node->mn_flags & F_DUPDATA) {
        if (mc->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED) {
          if (!(node->mn_flags & F_SUBDATA))
            mc->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(node);
        } else {
          rc = mdbx_xcursor_init1(mc, node);
          if (likely(rc != MDBX_SUCCESS))
            mc->mc_xcursor->mx_cursor.mc_flags |= C_DEL;
        }
      }
    }
    mc->mc_flags |= C_DEL;
  }

  if (unlikely(rc))
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
  else if (mdbx_audit_enabled())
    rc = mdbx_cursor_check(mc, false);

  return rc;
}

int mdbx_del(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data) {
  if (unlikely(!key || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS : MDBX_BAD_TXN;

  return mdbx_del0(txn, dbi, key, data, 0);
}

static int mdbx_del0(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
                     unsigned flags) {
  MDBX_cursor_couple cx;
  MDBX_cursor_op op;
  MDBX_val rdata;
  int rc, exact = 0;
  DKBUF;

  mdbx_debug("====> delete db %u key [%s], data [%s]", dbi, DKEY(key),
             DVAL(data));

  rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  if (data) {
    op = MDBX_GET_BOTH;
    rdata = *data;
    data = &rdata;
  } else {
    op = MDBX_SET;
    flags |= MDBX_NODUPDATA;
  }
  rc = mdbx_cursor_set(&cx.outer, key, data, op, &exact);
  if (likely(rc == MDBX_SUCCESS)) {
    /* let mdbx_page_split know about this cursor if needed:
     * delete will trigger a rebalance; if it needs to move
     * a node from one page to another, it will have to
     * update the parent's separator key(s). If the new sepkey
     * is larger than the current one, the parent page may
     * run out of space, triggering a split. We need this
     * cursor to be consistent until the end of the rebalance. */
    cx.outer.mc_next = txn->mt_cursors[dbi];
    txn->mt_cursors[dbi] = &cx.outer;
    rc = mdbx_cursor_del(&cx.outer, flags);
    txn->mt_cursors[dbi] = cx.outer.mc_next;
  }
  return rc;
}

/* Split a page and insert a new node.
 * Set MDBX_TXN_ERROR on failure.
 * [in,out] mc Cursor pointing to the page and desired insertion index.
 * The cursor will be updated to point to the actual page and index where
 * the node got inserted after the split.
 * [in] newkey The key for the newly inserted node.
 * [in] newdata The data for the newly inserted node.
 * [in] newpgno The page number, if the new node is a branch node.
 * [in] nflags The NODE_ADD_FLAGS for the new node.
 * Returns 0 on success, non-zero on failure. */
static int mdbx_page_split(MDBX_cursor *mc, const MDBX_val *newkey,
                           MDBX_val *newdata, pgno_t newpgno, unsigned nflags) {
  unsigned flags;
  int rc = MDBX_SUCCESS, foliage = 0, did_split = 0;
  pgno_t pgno = 0;
  unsigned i, ptop;
  MDBX_env *env = mc->mc_txn->mt_env;
  MDBX_node *node;
  MDBX_val sepkey, rkey, xdata;
  MDBX_page *copy = NULL;
  MDBX_page *rp, *pp;
  MDBX_cursor mn;
  DKBUF;

  MDBX_page *mp = mc->mc_pg[mc->mc_top];
  unsigned newindx = mc->mc_ki[mc->mc_top];
  unsigned nkeys = NUMKEYS(mp);
  if (mdbx_audit_enabled()) {
    int err = mdbx_cursor_check(mc, true);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
  }

  mdbx_debug("-----> splitting %s page %" PRIaPGNO
             " and adding [%s] at index %i/%i",
             IS_LEAF(mp) ? "leaf" : "branch", mp->mp_pgno, DKEY(newkey),
             mc->mc_ki[mc->mc_top], nkeys);

  /* Create a right sibling. */
  if ((rc = mdbx_page_new(mc, mp->mp_flags, 1, &rp)))
    return rc;
  rp->mp_leaf2_ksize = mp->mp_leaf2_ksize;
  mdbx_debug("new right sibling: page %" PRIaPGNO, rp->mp_pgno);

  /* Usually when splitting the root page, the cursor
   * height is 1. But when called from mdbx_update_key,
   * the cursor height may be greater because it walks
   * up the stack while finding the branch slot to update. */
  if (mc->mc_top < 1) {
    if ((rc = mdbx_page_new(mc, P_BRANCH, 1, &pp)))
      goto done;
    /* shift current top to make room for new parent */
    mdbx_cassert(mc, mc->mc_snum < 2 && mc->mc_db->md_depth > 0);
    mc->mc_pg[2] = mc->mc_pg[1];
    mc->mc_ki[2] = mc->mc_ki[1];
    mc->mc_pg[1] = mc->mc_pg[0];
    mc->mc_ki[1] = mc->mc_ki[0];
    mc->mc_pg[0] = pp;
    mc->mc_ki[0] = 0;
    mc->mc_db->md_root = pp->mp_pgno;
    mdbx_debug("root split! new root = %" PRIaPGNO, pp->mp_pgno);
    foliage = mc->mc_db->md_depth++;

    /* Add left (implicit) pointer. */
    if (unlikely((rc = mdbx_node_add_branch(mc, 0, NULL, mp->mp_pgno)) !=
                 MDBX_SUCCESS)) {
      /* undo the pre-push */
      mc->mc_pg[0] = mc->mc_pg[1];
      mc->mc_ki[0] = mc->mc_ki[1];
      mc->mc_db->md_root = mp->mp_pgno;
      mc->mc_db->md_depth--;
      goto done;
    }
    mc->mc_snum++;
    mc->mc_top++;
    ptop = 0;
  } else {
    ptop = mc->mc_top - 1;
    mdbx_debug("parent branch page is %" PRIaPGNO, mc->mc_pg[ptop]->mp_pgno);
  }

  mdbx_cursor_copy(mc, &mn);
  mn.mc_xcursor = NULL;
  mn.mc_pg[mn.mc_top] = rp;
  mn.mc_ki[mn.mc_top] = 0;
  mn.mc_ki[ptop] = mc->mc_ki[ptop] + 1;

  unsigned split_indx;
  if (nflags & MDBX_APPEND) {
    mn.mc_ki[mn.mc_top] = 0;
    sepkey = *newkey;
    split_indx = newindx;
    nkeys = 0;
  } else {
    split_indx = (nkeys + 1) / 2;
    if (IS_LEAF2(rp)) {
      char *split, *ins;
      int x;
      unsigned lsize, rsize, ksize;
      /* Move half of the keys to the right sibling */
      x = mc->mc_ki[mc->mc_top] - split_indx;
      ksize = mc->mc_db->md_xsize;
      split = LEAF2KEY(mp, split_indx, ksize);
      rsize = (nkeys - split_indx) * ksize;
      lsize = (nkeys - split_indx) * sizeof(indx_t);
      mdbx_cassert(mc, mp->mp_lower >= lsize);
      mp->mp_lower -= (indx_t)lsize;
      mdbx_cassert(mc, rp->mp_lower + lsize <= UINT16_MAX);
      rp->mp_lower += (indx_t)lsize;
      mdbx_cassert(mc, mp->mp_upper + rsize - lsize <= UINT16_MAX);
      mp->mp_upper += (indx_t)(rsize - lsize);
      mdbx_cassert(mc, rp->mp_upper >= rsize - lsize);
      rp->mp_upper -= (indx_t)(rsize - lsize);
      sepkey.iov_len = ksize;
      if (newindx == split_indx) {
        sepkey.iov_base = newkey->iov_base;
      } else {
        sepkey.iov_base = split;
      }
      if (x < 0) {
        mdbx_cassert(mc, ksize >= sizeof(indx_t));
        ins = LEAF2KEY(mp, mc->mc_ki[mc->mc_top], ksize);
        memcpy(rp->mp_ptrs, split, rsize);
        sepkey.iov_base = rp->mp_ptrs;
        memmove(ins + ksize, ins, (split_indx - mc->mc_ki[mc->mc_top]) * ksize);
        memcpy(ins, newkey->iov_base, ksize);
        mdbx_cassert(mc, UINT16_MAX - mp->mp_lower >= (int)sizeof(indx_t));
        mp->mp_lower += sizeof(indx_t);
        mdbx_cassert(mc, mp->mp_upper >= ksize - sizeof(indx_t));
        mp->mp_upper -= (indx_t)(ksize - sizeof(indx_t));
      } else {
        if (x)
          memcpy(rp->mp_ptrs, split, x * ksize);
        ins = LEAF2KEY(rp, x, ksize);
        memcpy(ins, newkey->iov_base, ksize);
        memcpy(ins + ksize, split + x * ksize, rsize - x * ksize);
        mdbx_cassert(mc, UINT16_MAX - rp->mp_lower >= (int)sizeof(indx_t));
        rp->mp_lower += sizeof(indx_t);
        mdbx_cassert(mc, rp->mp_upper >= ksize - sizeof(indx_t));
        rp->mp_upper -= (indx_t)(ksize - sizeof(indx_t));
        mdbx_cassert(mc, x <= UINT16_MAX);
        mc->mc_ki[mc->mc_top] = (indx_t)x;
      }
    } else {
      size_t psize, nsize, k;
      /* Maximum free space in an empty page */
      const unsigned pmax = env->me_psize - PAGEHDRSZ;
      nsize = IS_LEAF(mp) ? mdbx_leaf_size(env, newkey, newdata)
                          : mdbx_branch_size(env, newkey);

      /* grab a page to hold a temporary copy */
      copy = mdbx_page_malloc(mc->mc_txn, 1);
      if (unlikely(copy == NULL)) {
        rc = MDBX_ENOMEM;
        goto done;
      }
      copy->mp_pgno = mp->mp_pgno;
      copy->mp_flags = mp->mp_flags;
      copy->mp_lower = 0;
      mdbx_cassert(mc, env->me_psize - PAGEHDRSZ <= UINT16_MAX);
      copy->mp_upper = (indx_t)(env->me_psize - PAGEHDRSZ);

      /* prepare to insert */
      for (unsigned j = i = 0; i < nkeys; i++) {
        if (i == newindx)
          copy->mp_ptrs[j++] = 0;
        copy->mp_ptrs[j++] = mp->mp_ptrs[i];
      }

      /* When items are relatively large the split point needs
       * to be checked, because being off-by-one will make the
       * difference between success or failure in mdbx_node_add.
       *
       * It's also relevant if a page happens to be laid out
       * such that one half of its nodes are all "small" and
       * the other half of its nodes are "large." If the new
       * item is also "large" and falls on the half with
       * "large" nodes, it also may not fit.
       *
       * As a final tweak, if the new item goes on the last
       * spot on the page (and thus, onto the new page), bias
       * the split so the new page is emptier than the old page.
       * This yields better packing during sequential inserts.
       */
      int dir;
      if (nkeys < 32 || nsize > pmax / 16 || newindx >= nkeys) {
        /* Find split point */
        psize = 0;
        if (newindx <= split_indx || newindx >= nkeys) {
          i = 0;
          dir = 1;
          k = (newindx >= nkeys) ? nkeys : split_indx + 1 + IS_LEAF(mp);
        } else {
          i = nkeys;
          dir = -1;
          k = split_indx - 1;
        }
        for (; i != k; i += dir) {
          if (i == newindx) {
            psize += nsize;
            node = NULL;
          } else {
            node = (MDBX_node *)((char *)mp + copy->mp_ptrs[i] + PAGEHDRSZ);
            psize += NODESIZE + NODEKSZ(node) + sizeof(indx_t);
            if (IS_LEAF(mp)) {
              if (F_ISSET(node->mn_flags, F_BIGDATA))
                psize += sizeof(pgno_t);
              else
                psize += NODEDSZ(node);
            }
            psize = EVEN(psize);
          }
          if (psize > pmax || i == k - dir) {
            split_indx = i + (dir < 0);
            break;
          }
        }
      }
      if (split_indx == newindx) {
        sepkey.iov_len = newkey->iov_len;
        sepkey.iov_base = newkey->iov_base;
      } else {
        node =
            (MDBX_node *)((char *)mp + copy->mp_ptrs[split_indx] + PAGEHDRSZ);
        sepkey.iov_len = node->mn_ksize;
        sepkey.iov_base = NODEKEY(node);
      }
    }
  }

  mdbx_debug("separator is %d [%s]", split_indx, DKEY(&sepkey));
  if (mdbx_audit_enabled()) {
    int err = mdbx_cursor_check(mc, true);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
    err = mdbx_cursor_check(&mn, true);
    if (unlikely(err != MDBX_SUCCESS))
      return err;
  }

  /* Copy separator key to the parent. */
  if (SIZELEFT(mn.mc_pg[ptop]) < mdbx_branch_size(env, &sepkey)) {
    const int snum = mc->mc_snum;
    const int depth = mc->mc_db->md_depth;
    mn.mc_snum--;
    mn.mc_top--;
    did_split = 1;
    /* We want other splits to find mn when doing fixups */
    WITH_CURSOR_TRACKING(
        mn, rc = mdbx_page_split(&mn, &sepkey, NULL, rp->mp_pgno, 0));
    if (unlikely(rc != MDBX_SUCCESS))
      goto done;
    mdbx_cassert(mc, mc->mc_snum - snum == mc->mc_db->md_depth - depth);
    if (mdbx_audit_enabled()) {
      int err = mdbx_cursor_check(mc, true);
      if (unlikely(err != MDBX_SUCCESS))
        return err;
    }

    /* root split? */
    ptop += mc->mc_snum - snum;

    /* Right page might now have changed parent.
     * Check if left page also changed parent. */
    if (mn.mc_pg[ptop] != mc->mc_pg[ptop] &&
        mc->mc_ki[ptop] >= NUMKEYS(mc->mc_pg[ptop])) {
      for (i = 0; i < ptop; i++) {
        mc->mc_pg[i] = mn.mc_pg[i];
        mc->mc_ki[i] = mn.mc_ki[i];
      }
      mc->mc_pg[ptop] = mn.mc_pg[ptop];
      if (mn.mc_ki[ptop]) {
        mc->mc_ki[ptop] = mn.mc_ki[ptop] - 1;
      } else {
        /* find right page's left sibling */
        mc->mc_ki[ptop] = mn.mc_ki[ptop];
        rc = mdbx_cursor_sibling(mc, false);
      }
    }
  } else {
    mn.mc_top--;
    rc = mdbx_node_add_branch(&mn, mn.mc_ki[ptop], &sepkey, rp->mp_pgno);
    mn.mc_top++;
  }
  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc == MDBX_NOTFOUND) /* improper mdbx_cursor_sibling() result */ {
      mdbx_error("unexpected %s", "MDBX_NOTFOUND");
      rc = MDBX_PROBLEM;
    }
    goto done;
  }

  if (nflags & MDBX_APPEND) {
    mc->mc_pg[mc->mc_top] = rp;
    mc->mc_ki[mc->mc_top] = 0;
    switch (PAGETYPE(rp)) {
    case P_BRANCH: {
      mdbx_cassert(mc, (nflags & (F_BIGDATA | F_SUBDATA | F_DUPDATA)) == 0);
      mdbx_cassert(mc, newpgno != 0 && newpgno != P_INVALID);
      rc = mdbx_node_add_branch(mc, 0, newkey, newpgno);
    } break;
    case P_LEAF: {
      mdbx_cassert(mc, newpgno == 0 || newpgno == P_INVALID);
      rc = mdbx_node_add_leaf(mc, 0, newkey, newdata, nflags);
    } break;
    case P_LEAF | P_LEAF2: {
      mdbx_cassert(mc, (nflags & (F_BIGDATA | F_SUBDATA | F_DUPDATA)) == 0);
      mdbx_cassert(mc, newpgno == 0 || newpgno == P_INVALID);
      rc = mdbx_node_add_leaf2(mc, 0, newkey);
    } break;
    default:
      rc = MDBX_CORRUPTED;
    }
    if (rc)
      goto done;
    for (i = 0; i < mc->mc_top; i++)
      mc->mc_ki[i] = mn.mc_ki[i];
  } else if (!IS_LEAF2(mp)) {
    /* Move nodes */
    mc->mc_pg[mc->mc_top] = rp;
    i = split_indx;
    indx_t n = 0;
    do {
      MDBX_val *rdata = NULL;
      if (i == newindx) {
        rkey.iov_base = newkey->iov_base;
        rkey.iov_len = newkey->iov_len;
        if (IS_LEAF(mp)) {
          rdata = newdata;
        } else
          pgno = newpgno;
        flags = nflags;
        /* Update index for the new key. */
        mc->mc_ki[mc->mc_top] = n;
      } else {
        node = (MDBX_node *)((char *)mp + copy->mp_ptrs[i] + PAGEHDRSZ);
        rkey.iov_base = NODEKEY(node);
        rkey.iov_len = node->mn_ksize;
        if (IS_LEAF(mp)) {
          xdata.iov_base = NODEDATA(node);
          xdata.iov_len = NODEDSZ(node);
          rdata = &xdata;
        } else
          pgno = NODEPGNO(node);
        flags = node->mn_flags;
      }

      switch (PAGETYPE(rp)) {
      case P_BRANCH: {
        mdbx_cassert(mc, 0 == (uint16_t)flags);
        if (n == 0) {
          /* First branch index doesn't need key data. */
          rkey.iov_len = 0;
        }
        rc = mdbx_node_add_branch(mc, n, &rkey, pgno);
      } break;
      case P_LEAF: {
        mdbx_cassert(mc, pgno == 0);
        mdbx_cassert(mc, rdata != NULL);
        rc = mdbx_node_add_leaf(mc, n, &rkey, rdata, flags);
      } break;
      /* case P_LEAF | P_LEAF2: {
        mdbx_cassert(mc, (nflags & (F_BIGDATA | F_SUBDATA | F_DUPDATA)) == 0);
        mdbx_cassert(mc, gno == 0);
        rc = mdbx_node_add_leaf2(mc, n, &rkey);
      } break; */
      default:
        rc = MDBX_CORRUPTED;
      }
      if (rc)
        goto done;

      if (i == nkeys) {
        i = 0;
        n = 0;
        mc->mc_pg[mc->mc_top] = copy;
      } else {
        i++;
        n++;
      }
    } while (i != split_indx);

    nkeys = NUMKEYS(copy);
    for (i = 0; i < nkeys; i++)
      mp->mp_ptrs[i] = copy->mp_ptrs[i];
    mp->mp_lower = copy->mp_lower;
    mp->mp_upper = copy->mp_upper;
    memcpy(NODEPTR(mp, nkeys - 1), NODEPTR(copy, nkeys - 1),
           env->me_psize - copy->mp_upper - PAGEHDRSZ);

    /* reset back to original page */
    if (newindx < split_indx) {
      mc->mc_pg[mc->mc_top] = mp;
    } else {
      mc->mc_pg[mc->mc_top] = rp;
      mc->mc_ki[ptop]++;
      /* Make sure mc_ki is still valid. */
      if (mn.mc_pg[ptop] != mc->mc_pg[ptop] &&
          mc->mc_ki[ptop] >= NUMKEYS(mc->mc_pg[ptop])) {
        for (i = 0; i <= ptop; i++) {
          mc->mc_pg[i] = mn.mc_pg[i];
          mc->mc_ki[i] = mn.mc_ki[i];
        }
      }
    }
    if (nflags & MDBX_RESERVE) {
      node = NODEPTR(mc->mc_pg[mc->mc_top], mc->mc_ki[mc->mc_top]);
      if (!(node->mn_flags & F_BIGDATA))
        newdata->iov_base = NODEDATA(node);
    }
  } else {
    if (newindx >= split_indx) {
      mc->mc_pg[mc->mc_top] = rp;
      mc->mc_ki[ptop]++;
      /* Make sure mc_ki is still valid. */
      if (mn.mc_pg[ptop] != mc->mc_pg[ptop] &&
          mc->mc_ki[ptop] >= NUMKEYS(mc->mc_pg[ptop])) {
        for (i = 0; i <= ptop; i++) {
          mc->mc_pg[i] = mn.mc_pg[i];
          mc->mc_ki[i] = mn.mc_ki[i];
        }
      }
    }
  }

  {
    /* Adjust other cursors pointing to mp */
    MDBX_cursor *m2, *m3;
    MDBX_dbi dbi = mc->mc_dbi;
    nkeys = NUMKEYS(mp);

    for (m2 = mc->mc_txn->mt_cursors[dbi]; m2; m2 = m2->mc_next) {
      m3 = (mc->mc_flags & C_SUB) ? &m2->mc_xcursor->mx_cursor : m2;
      if (m3 == mc)
        continue;
      if (!(m2->mc_flags & m3->mc_flags & C_INITIALIZED))
        continue;
      if (foliage) {
        int k;
        /* sub cursors may be on different DB */
        if (m3->mc_pg[0] != mp)
          continue;
        /* root split */
        for (k = foliage; k >= 0; k--) {
          m3->mc_ki[k + 1] = m3->mc_ki[k];
          m3->mc_pg[k + 1] = m3->mc_pg[k];
        }
        m3->mc_ki[0] = (m3->mc_ki[0] >= nkeys) ? 1 : 0;
        m3->mc_pg[0] = mc->mc_pg[0];
        m3->mc_snum++;
        m3->mc_top++;
      }
      if (m3->mc_top >= mc->mc_top && m3->mc_pg[mc->mc_top] == mp) {
        if (m3->mc_ki[mc->mc_top] >= newindx && !(nflags & MDBX_SPLIT_REPLACE))
          m3->mc_ki[mc->mc_top]++;
        if (m3->mc_ki[mc->mc_top] >= nkeys) {
          m3->mc_pg[mc->mc_top] = rp;
          mdbx_cassert(mc, m3->mc_ki[mc->mc_top] >= nkeys);
          m3->mc_ki[mc->mc_top] -= (indx_t)nkeys;
          for (i = 0; i < mc->mc_top; i++) {
            m3->mc_ki[i] = mn.mc_ki[i];
            m3->mc_pg[i] = mn.mc_pg[i];
          }
        }
      } else if (!did_split && m3->mc_top >= ptop &&
                 m3->mc_pg[ptop] == mc->mc_pg[ptop] &&
                 m3->mc_ki[ptop] >= mc->mc_ki[ptop]) {
        m3->mc_ki[ptop]++;
      }
      if (XCURSOR_INITED(m3) && IS_LEAF(mp))
        XCURSOR_REFRESH(m3, m3->mc_pg[mc->mc_top], m3->mc_ki[mc->mc_top]);
    }
  }
  mdbx_debug("mp left: %d, rp left: %d", SIZELEFT(mp), SIZELEFT(rp));

done:
  if (copy) /* tmp page */
    mdbx_dpage_free(env, copy, 1);
  if (unlikely(rc))
    mc->mc_txn->mt_flags |= MDBX_TXN_ERROR;
  return rc;
}

int mdbx_put(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
             unsigned flags) {

  if (unlikely(!key || !data || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(flags & ~(MDBX_NOOVERWRITE | MDBX_NODUPDATA | MDBX_RESERVE |
                         MDBX_APPEND | MDBX_APPENDDUP | MDBX_CURRENT)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS : MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  cx.outer.mc_next = txn->mt_cursors[dbi];
  txn->mt_cursors[dbi] = &cx.outer;

  /* LY: support for update (explicit overwrite) */
  if (flags & MDBX_CURRENT) {
    rc = mdbx_cursor_get(&cx.outer, key, NULL, MDBX_SET);
    if (likely(rc == MDBX_SUCCESS) &&
        (txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT)) {
      /* LY: allows update (explicit overwrite) only for unique keys */
      MDBX_node *leaf = NODEPTR(cx.outer.mc_pg[cx.outer.mc_top],
                                cx.outer.mc_ki[cx.outer.mc_top]);
      if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
        mdbx_tassert(txn, XCURSOR_INITED(&cx.outer) &&
                              cx.outer.mc_xcursor->mx_db.md_entries > 1);
        rc = MDBX_EMULTIVAL;
      }
    }
  }

  if (likely(rc == MDBX_SUCCESS))
    rc = mdbx_cursor_put(&cx.outer, key, data, flags);
  txn->mt_cursors[dbi] = cx.outer.mc_next;

  return rc;
}

#ifndef MDBX_WBUF
#define MDBX_WBUF ((size_t)1024 * 1024)
#endif
#define MDBX_EOF 0x10 /* mdbx_env_copyfd1() is done reading */

/* State needed for a double-buffering compacting copy. */
typedef struct mdbx_copy {
  MDBX_env *mc_env;
  MDBX_txn *mc_txn;
  mdbx_condmutex_t mc_condmutex;
  uint8_t *mc_wbuf[2];
  uint8_t *mc_over[2];
  size_t mc_wlen[2];
  size_t mc_olen[2];
  mdbx_filehandle_t mc_fd;
  volatile int mc_error;
  pgno_t mc_next_pgno;
  short mc_toggle; /* Buffer number in provider */
  short mc_new;    /* (0-2 buffers to write) | (MDBX_EOF at end) */
  /* Error code.  Never cleared if set.  Both threads can set nonzero
   * to fail the copy.  Not mutex-protected, MDBX expects atomic int. */
} mdbx_copy;

/* Dedicated writer thread for compacting copy. */
static THREAD_RESULT __cold THREAD_CALL mdbx_env_copythr(void *arg) {
  mdbx_copy *my = arg;
  uint8_t *ptr;
  int toggle = 0;
  int rc;
  size_t offset = pgno2bytes(my->mc_env, NUM_METAS);

  mdbx_condmutex_lock(&my->mc_condmutex);
  while (!my->mc_error) {
    while (!my->mc_new)
      mdbx_condmutex_wait(&my->mc_condmutex);
    if (my->mc_new == 0 + MDBX_EOF) /* 0 buffers, just EOF */
      break;
    size_t wsize = my->mc_wlen[toggle];
    ptr = my->mc_wbuf[toggle];
  again:
    if (wsize > 0 && !my->mc_error) {
      rc = mdbx_pwrite(my->mc_fd, ptr, wsize, offset);
      if (rc != MDBX_SUCCESS) {
        my->mc_error = rc;
        break;
      }
      offset += wsize;
    }

    /* If there's an overflow page tail, write it too */
    if (my->mc_olen[toggle]) {
      wsize = my->mc_olen[toggle];
      ptr = my->mc_over[toggle];
      my->mc_olen[toggle] = 0;
      goto again;
    }
    my->mc_wlen[toggle] = 0;
    toggle ^= 1;
    /* Return the empty buffer to provider */
    my->mc_new--;
    mdbx_condmutex_signal(&my->mc_condmutex);
  }
  mdbx_condmutex_unlock(&my->mc_condmutex);
  return (THREAD_RESULT)0;
}

/* Give buffer and/or MDBX_EOF to writer thread, await unused buffer.
 *
 * [in] my control structure.
 * [in] adjust (1 to hand off 1 buffer) | (MDBX_EOF when ending). */
static int __cold mdbx_env_cthr_toggle(mdbx_copy *my, int adjust) {
  mdbx_condmutex_lock(&my->mc_condmutex);
  my->mc_new += (short)adjust;
  mdbx_condmutex_signal(&my->mc_condmutex);
  while (my->mc_new & 2) /* both buffers in use */
    mdbx_condmutex_wait(&my->mc_condmutex);
  mdbx_condmutex_unlock(&my->mc_condmutex);

  my->mc_toggle ^= (adjust & 1);
  /* Both threads reset mc_wlen, to be safe from threading errors */
  my->mc_wlen[my->mc_toggle] = 0;
  return my->mc_error;
}

/* Depth-first tree traversal for compacting copy.
 * [in] my control structure.
 * [in,out] pg database root.
 * [in] flags includes F_DUPDATA if it is a sorted-duplicate sub-DB. */
static int __cold mdbx_env_cwalk(mdbx_copy *my, pgno_t *pg, int flags) {
  MDBX_cursor mc;
  MDBX_node *ni;
  MDBX_page *mo, *mp, *leaf;
  char *buf, *ptr;
  int rc, toggle;
  unsigned i;

  /* Empty DB, nothing to do */
  if (*pg == P_INVALID)
    return MDBX_SUCCESS;

  memset(&mc, 0, sizeof(mc));
  mc.mc_snum = 1;
  mc.mc_txn = my->mc_txn;

  rc = mdbx_page_get(&mc, *pg, &mc.mc_pg[0], NULL);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  rc = mdbx_page_search_root(&mc, NULL, MDBX_PS_FIRST);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  /* Make cursor pages writable */
  buf = ptr = mdbx_malloc(pgno2bytes(my->mc_env, mc.mc_snum));
  if (buf == NULL)
    return MDBX_ENOMEM;

  for (i = 0; i < mc.mc_top; i++) {
    mdbx_page_copy((MDBX_page *)ptr, mc.mc_pg[i], my->mc_env->me_psize);
    mc.mc_pg[i] = (MDBX_page *)ptr;
    ptr += my->mc_env->me_psize;
  }

  /* This is writable space for a leaf page. Usually not needed. */
  leaf = (MDBX_page *)ptr;

  toggle = my->mc_toggle;
  while (mc.mc_snum > 0) {
    unsigned n;
    mp = mc.mc_pg[mc.mc_top];
    n = NUMKEYS(mp);

    if (IS_LEAF(mp)) {
      if (!IS_LEAF2(mp) && !(flags & F_DUPDATA)) {
        for (i = 0; i < n; i++) {
          ni = NODEPTR(mp, i);
          if (ni->mn_flags & F_BIGDATA) {
            MDBX_page *omp;

            /* Need writable leaf */
            if (mp != leaf) {
              mc.mc_pg[mc.mc_top] = leaf;
              mdbx_page_copy(leaf, mp, my->mc_env->me_psize);
              mp = leaf;
              ni = NODEPTR(mp, i);
            }

            pgno_t pgno;
            memcpy(&pgno, NODEDATA(ni), sizeof(pgno));
            memcpy(NODEDATA(ni), &my->mc_next_pgno, sizeof(pgno_t));
            rc = mdbx_page_get(&mc, pgno, &omp, NULL);
            if (unlikely(rc != MDBX_SUCCESS))
              goto done;
            if (my->mc_wlen[toggle] >= MDBX_WBUF) {
              rc = mdbx_env_cthr_toggle(my, 1);
              if (unlikely(rc != MDBX_SUCCESS))
                goto done;
              toggle = my->mc_toggle;
            }
            mo = (MDBX_page *)(my->mc_wbuf[toggle] + my->mc_wlen[toggle]);
            memcpy(mo, omp, my->mc_env->me_psize);
            mo->mp_pgno = my->mc_next_pgno;
            my->mc_next_pgno += omp->mp_pages;
            my->mc_wlen[toggle] += my->mc_env->me_psize;
            if (omp->mp_pages > 1) {
              my->mc_olen[toggle] = pgno2bytes(my->mc_env, omp->mp_pages - 1);
              my->mc_over[toggle] = (uint8_t *)omp + my->mc_env->me_psize;
              rc = mdbx_env_cthr_toggle(my, 1);
              if (unlikely(rc != MDBX_SUCCESS))
                goto done;
              toggle = my->mc_toggle;
            }
          } else if (ni->mn_flags & F_SUBDATA) {
            MDBX_db db;

            /* Need writable leaf */
            if (mp != leaf) {
              mc.mc_pg[mc.mc_top] = leaf;
              mdbx_page_copy(leaf, mp, my->mc_env->me_psize);
              mp = leaf;
              ni = NODEPTR(mp, i);
            }

            memcpy(&db, NODEDATA(ni), sizeof(db));
            my->mc_toggle = (short)toggle;
            rc = mdbx_env_cwalk(my, &db.md_root, ni->mn_flags & F_DUPDATA);
            if (rc)
              goto done;
            toggle = my->mc_toggle;
            memcpy(NODEDATA(ni), &db, sizeof(db));
          }
        }
      }
    } else {
      mc.mc_ki[mc.mc_top]++;
      if (mc.mc_ki[mc.mc_top] < n) {
        pgno_t pgno;
      again:
        ni = NODEPTR(mp, mc.mc_ki[mc.mc_top]);
        pgno = NODEPGNO(ni);
        rc = mdbx_page_get(&mc, pgno, &mp, NULL);
        if (unlikely(rc != MDBX_SUCCESS))
          goto done;
        mc.mc_top++;
        mc.mc_snum++;
        mc.mc_ki[mc.mc_top] = 0;
        if (IS_BRANCH(mp)) {
          /* Whenever we advance to a sibling branch page,
           * we must proceed all the way down to its first leaf. */
          mdbx_page_copy(mc.mc_pg[mc.mc_top], mp, my->mc_env->me_psize);
          goto again;
        } else
          mc.mc_pg[mc.mc_top] = mp;
        continue;
      }
    }
    if (my->mc_wlen[toggle] >= MDBX_WBUF) {
      rc = mdbx_env_cthr_toggle(my, 1);
      if (unlikely(rc != MDBX_SUCCESS))
        goto done;
      toggle = my->mc_toggle;
    }
    mo = (MDBX_page *)(my->mc_wbuf[toggle] + my->mc_wlen[toggle]);
    mdbx_page_copy(mo, mp, my->mc_env->me_psize);
    mo->mp_pgno = my->mc_next_pgno++;
    my->mc_wlen[toggle] += my->mc_env->me_psize;
    if (mc.mc_top) {
      /* Update parent if there is one */
      ni = NODEPTR(mc.mc_pg[mc.mc_top - 1], mc.mc_ki[mc.mc_top - 1]);
      SETPGNO(ni, mo->mp_pgno);
      mdbx_cursor_pop(&mc);
    } else {
      /* Otherwise we're done */
      *pg = mo->mp_pgno;
      break;
    }
  }
done:
  mdbx_free(buf);
  return rc;
}

/* Copy environment with compaction. */
static int __cold mdbx_env_compact(MDBX_env *env, MDBX_txn *read_txn,
                                   mdbx_filehandle_t fd, uint8_t *buffer) {
  MDBX_page *const meta = mdbx_init_metas(env, buffer);
  /* copy canary sequenses if present */
  if (read_txn->mt_canary.v) {
    meta->mp_meta.mm_canary = read_txn->mt_canary;
    meta->mp_meta.mm_canary.v = mdbx_meta_txnid_stable(env, &meta->mp_meta);
  }

  /* Set metapage 1 with current main DB */
  pgno_t new_root, root = read_txn->mt_dbs[MAIN_DBI].md_root;
  if ((new_root = root) == P_INVALID) {
    /* When the DB is empty, handle it specially to
     * fix any breakage like page leaks from ITS#8174. */
    meta->mp_meta.mm_dbs[MAIN_DBI].md_flags =
        read_txn->mt_dbs[MAIN_DBI].md_flags;
  } else {
    /* Count free pages + freeDB pages.  Subtract from last_pg
     * to find the new last_pg, which also becomes the new root. */
    pgno_t freecount = 0;
    MDBX_cursor mc;
    MDBX_val key, data;

    int rc = mdbx_cursor_init(&mc, read_txn, FREE_DBI);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    while ((rc = mdbx_cursor_get(&mc, &key, &data, MDBX_NEXT)) == 0)
      freecount += *(pgno_t *)data.iov_base;
    if (unlikely(rc != MDBX_NOTFOUND))
      return rc;

    freecount += read_txn->mt_dbs[FREE_DBI].md_branch_pages +
                 read_txn->mt_dbs[FREE_DBI].md_leaf_pages +
                 read_txn->mt_dbs[FREE_DBI].md_overflow_pages;

    new_root = read_txn->mt_next_pgno - 1 - freecount;
    meta->mp_meta.mm_geo.next = new_root + 1;
    meta->mp_meta.mm_dbs[MAIN_DBI] = read_txn->mt_dbs[MAIN_DBI];
    meta->mp_meta.mm_dbs[MAIN_DBI].md_root = new_root;

    mdbx_copy ctx;
    memset(&ctx, 0, sizeof(ctx));
    rc = mdbx_condmutex_init(&ctx.mc_condmutex);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;

    ctx.mc_wbuf[0] = buffer + pgno2bytes(env, NUM_METAS);
    memset(ctx.mc_wbuf[0], 0, MDBX_WBUF * 2);
    ctx.mc_wbuf[1] = ctx.mc_wbuf[0] + MDBX_WBUF;
    ctx.mc_next_pgno = NUM_METAS;
    ctx.mc_env = env;
    ctx.mc_fd = fd;
    ctx.mc_txn = read_txn;

    mdbx_thread_t thread;
    int thread_err = mdbx_thread_create(&thread, mdbx_env_copythr, &ctx);
    if (likely(thread_err == MDBX_SUCCESS)) {
      rc = mdbx_env_cwalk(&ctx, &root, 0);
      mdbx_env_cthr_toggle(&ctx, 1 | MDBX_EOF);
      thread_err = mdbx_thread_join(thread);
      mdbx_condmutex_destroy(&ctx.mc_condmutex);
    }
    if (unlikely(thread_err != MDBX_SUCCESS))
      return thread_err;
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    if (unlikely(ctx.mc_error != MDBX_SUCCESS))
      return ctx.mc_error;

    if (root > new_root) {
      mdbx_error("post-compactification root %" PRIaPGNO
                 " GT expected %" PRIaPGNO " (source DB corrupted)",
                 root, new_root);
      return MDBX_CORRUPTED; /* page leak or corrupt DB */
    }
    if (root < new_root) {
      mdbx_notice("post-compactification root %" PRIaPGNO
                  " LT expected %" PRIaPGNO " (page leak(s) in source DB)",
                  root, new_root);
      /* fixup meta */
      meta->mp_meta.mm_dbs[MAIN_DBI].md_root = root;
      meta->mp_meta.mm_geo.next = root + 1;
    }
  }

  /* Calculate filesize taking in account shrink/growing thresholds */
  if (meta->mp_meta.mm_geo.next > meta->mp_meta.mm_geo.now) {
    const pgno_t aligned =
        pgno_align2os_pgno(env, pgno_add(meta->mp_meta.mm_geo.next,
                                         meta->mp_meta.mm_geo.grow -
                                             meta->mp_meta.mm_geo.next %
                                                 meta->mp_meta.mm_geo.grow));
    meta->mp_meta.mm_geo.now = aligned;
  } else if (meta->mp_meta.mm_geo.next < meta->mp_meta.mm_geo.now) {
    meta->mp_meta.mm_geo.now = meta->mp_meta.mm_geo.next;
    const pgno_t aligner = meta->mp_meta.mm_geo.grow
                               ? meta->mp_meta.mm_geo.grow
                               : meta->mp_meta.mm_geo.shrink;
    const pgno_t aligned =
        pgno_align2os_pgno(env, meta->mp_meta.mm_geo.next + aligner -
                                    meta->mp_meta.mm_geo.next % aligner);
    meta->mp_meta.mm_geo.now = aligned;
  }

  if (meta->mp_meta.mm_geo.now < meta->mp_meta.mm_geo.lower)
    meta->mp_meta.mm_geo.now = meta->mp_meta.mm_geo.lower;
  if (meta->mp_meta.mm_geo.now > meta->mp_meta.mm_geo.upper)
    meta->mp_meta.mm_geo.now = meta->mp_meta.mm_geo.upper;

  /* Update signature */
  assert(meta->mp_meta.mm_geo.now >= meta->mp_meta.mm_geo.next);
  meta->mp_meta.mm_datasync_sign = mdbx_meta_sign(&meta->mp_meta);

  /* Extend file if required */
  return (meta->mp_meta.mm_geo.now != meta->mp_meta.mm_geo.next)
             ? mdbx_ftruncate(fd, pgno2bytes(env, meta->mp_meta.mm_geo.now))
             : MDBX_SUCCESS;
}

/* Copy environment as-is. */
static int __cold mdbx_env_copy_asis(MDBX_env *env, MDBX_txn *read_txn,
                                     mdbx_filehandle_t fd, uint8_t *buffer) {
  /* We must start the actual read txn after blocking writers */
  int rc = mdbx_txn_end(read_txn, MDBX_END_RESET_TMP);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  /* Temporarily block writers until we snapshot the meta pages */
  rc = mdbx_txn_lock(env, false);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  rc = mdbx_txn_renew0(read_txn, MDBX_RDONLY);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_txn_unlock(env);
    return rc;
  }

  mdbx_jitter4testing(false);
  const size_t meta_bytes = pgno2bytes(env, NUM_METAS);
  /* Make a snapshot of meta-pages,
   * but writing ones after the data was flushed */
  memcpy(buffer, env->me_map, meta_bytes);
  MDBX_meta *const headcopy = /* LY: get pointer to the spanshot copy */
      (MDBX_meta *)(buffer + ((uint8_t *)mdbx_meta_head(env) - env->me_map));
  /* Update signature to steady */
  headcopy->mm_datasync_sign = mdbx_meta_sign(headcopy);
  mdbx_txn_unlock(env);

  /* Copy the data */
  const uint64_t whole_size =
      mdbx_roundup2(pgno2bytes(env, read_txn->mt_end_pgno), env->me_os_psize);
  const size_t used_size = pgno2bytes(env, read_txn->mt_next_pgno);
  mdbx_jitter4testing(false);
#if __GLIBC_PREREQ(2, 27) && defined(_GNU_SOURCE)
  for (off_t in_offset = meta_bytes; in_offset < (off_t)used_size;) {
    off_t out_offset = in_offset;
    ssize_t bytes_copied = copy_file_range(
        env->me_fd, &in_offset, fd, &out_offset, used_size - in_offset, 0);
    if (unlikely(bytes_copied <= 0)) {
      rc = bytes_copied ? errno : MDBX_ENODATA;
      break;
    }
  }
#else
  uint8_t *data_buffer = buffer + meta_bytes;
  for (size_t offset = meta_bytes; offset < used_size;) {
    const size_t chunk =
        (MDBX_WBUF < used_size - offset) ? MDBX_WBUF : used_size - offset;
    memcpy(data_buffer, env->me_map + offset, chunk);
    rc = mdbx_pwrite(fd, data_buffer, chunk, offset);
    if (unlikely(rc != MDBX_SUCCESS))
      break;
    offset += chunk;
  }
#endif

  if (likely(rc == MDBX_SUCCESS) && whole_size != used_size)
    rc = mdbx_ftruncate(fd, whole_size);

  return rc;
}

int __cold mdbx_env_copy2fd(MDBX_env *env, mdbx_filehandle_t fd,
                            unsigned flags) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  int rc = mdbx_fseek(fd, 0);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  const size_t buffer_size =
      pgno2bytes(env, NUM_METAS) +
      ((flags & MDBX_CP_COMPACT) ? MDBX_WBUF * 2 : MDBX_WBUF);

  uint8_t *buffer = NULL;
  rc = mdbx_memalign_alloc(env->me_os_psize, buffer_size, (void **)&buffer);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  MDBX_txn *read_txn = NULL;
  /* Do the lock/unlock of the reader mutex before starting the
   * write txn.  Otherwise other read txns could block writers. */
  rc = mdbx_txn_begin(env, NULL, MDBX_RDONLY, &read_txn);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_memalign_free(buffer);
    return rc;
  }

  /* Firstly write a stub to meta-pages.
   * Now we sure to incomplete copy will not be used. */
  memset(buffer, -1, pgno2bytes(env, NUM_METAS));
  rc = mdbx_pwrite(fd, buffer, pgno2bytes(env, NUM_METAS), 0);
  if (likely(rc == MDBX_SUCCESS)) {
    memset(buffer, 0, pgno2bytes(env, NUM_METAS));
    rc = (flags & MDBX_CP_COMPACT)
             ? mdbx_env_compact(env, read_txn, fd, buffer)
             : mdbx_env_copy_asis(env, read_txn, fd, buffer);
  }
  mdbx_txn_abort(read_txn);

  if (likely(rc == MDBX_SUCCESS))
    rc = mdbx_filesync(fd, MDBX_SYNC_DATA | MDBX_SYNC_SIZE);

  /* Write actual meta */
  if (likely(rc == MDBX_SUCCESS))
    rc = mdbx_pwrite(fd, buffer, pgno2bytes(env, NUM_METAS), 0);

  if (likely(rc == MDBX_SUCCESS))
    rc = mdbx_filesync(fd, MDBX_SYNC_DATA | MDBX_SYNC_IODQ);

  mdbx_memalign_free(buffer);
  return rc;
}

int __cold mdbx_env_copy(MDBX_env *env, const char *dest_path, unsigned flags) {
  if (unlikely(!env || !dest_path))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  char *dxb_pathname;
  mdbx_filehandle_t newfd = INVALID_HANDLE_VALUE;

  if (env->me_flags & MDBX_NOSUBDIR) {
    dxb_pathname = (char *)dest_path;
  } else {
    size_t len = strlen(dest_path);
    len += sizeof(MDBX_DATANAME);
    dxb_pathname = mdbx_malloc(len);
    if (!dxb_pathname)
      return MDBX_ENOMEM;
    sprintf(dxb_pathname, "%s" MDBX_DATANAME, dest_path);
  }

  /* The destination path must exist, but the destination file must not.
   * We don't want the OS to cache the writes, since the source data is
   * already in the OS cache. */
  int rc = mdbx_openfile(dxb_pathname, O_WRONLY | O_CREAT | O_EXCL, 0640,
                         &newfd, true);
  if (rc == MDBX_SUCCESS) {
    if (env->me_psize >= env->me_os_psize) {
#ifdef F_NOCACHE /* __APPLE__ */
      (void)fcntl(newfd, F_NOCACHE, 1);
#elif defined(O_DIRECT) && defined(F_GETFL)
      /* Set O_DIRECT if the file system supports it */
      if ((rc = fcntl(newfd, F_GETFL)) != -1)
        (void)fcntl(newfd, F_SETFL, rc | O_DIRECT);
#endif
    }
    rc = mdbx_env_copy2fd(env, newfd, flags);
  }

  if (newfd != INVALID_HANDLE_VALUE) {
    int err = mdbx_closefile(newfd);
    if (rc == MDBX_SUCCESS && err != rc)
      rc = err;
    if (rc != MDBX_SUCCESS)
      (void)mdbx_removefile(dxb_pathname);
  }

  if (dxb_pathname != dest_path)
    mdbx_free(dxb_pathname);

  return rc;
}

int __cold mdbx_env_set_flags(MDBX_env *env, unsigned flags, int onoff) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(flags & ~CHANGEABLE))
    return MDBX_EINVAL;

  int rc = mdbx_txn_lock(env, false);
  if (unlikely(rc))
    return rc;

  if (onoff)
    env->me_flags |= flags;
  else
    env->me_flags &= ~flags;

  mdbx_txn_unlock(env);
  return MDBX_SUCCESS;
}

int __cold mdbx_env_get_flags(MDBX_env *env, unsigned *arg) {
  if (unlikely(!env || !arg))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  *arg = env->me_flags & (CHANGEABLE | CHANGELESS);
  return MDBX_SUCCESS;
}

int __cold mdbx_env_set_userctx(MDBX_env *env, void *ctx) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  env->me_userctx = ctx;
  return MDBX_SUCCESS;
}

void *__cold mdbx_env_get_userctx(MDBX_env *env) {
  return env ? env->me_userctx : NULL;
}

int __cold mdbx_env_set_assert(MDBX_env *env, MDBX_assert_func *func) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

#if MDBX_DEBUG
  env->me_assert_func = func;
  return MDBX_SUCCESS;
#else
  (void)func;
  return MDBX_ENOSYS;
#endif
}

int __cold mdbx_env_get_path(MDBX_env *env, const char **arg) {
  if (unlikely(!env || !arg))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  *arg = env->me_path;
  return MDBX_SUCCESS;
}

int __cold mdbx_env_get_fd(MDBX_env *env, mdbx_filehandle_t *arg) {
  if (unlikely(!env || !arg))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  *arg = env->me_fd;
  return MDBX_SUCCESS;
}

/* Common code for mdbx_dbi_stat() and mdbx_env_stat().
 * [in] env the environment to operate in.
 * [in] db the MDBX_db record containing the stats to return.
 * [out] arg the address of an MDBX_stat structure to receive the stats.
 * Returns 0, this function always succeeds. */
static int __cold mdbx_stat0(const MDBX_env *env, const MDBX_db *db,
                             MDBX_stat *arg) {
  arg->ms_psize = env->me_psize;
  arg->ms_depth = db->md_depth;
  arg->ms_branch_pages = db->md_branch_pages;
  arg->ms_leaf_pages = db->md_leaf_pages;
  arg->ms_overflow_pages = db->md_overflow_pages;
  arg->ms_entries = db->md_entries;
  return MDBX_SUCCESS;
}

int __cold mdbx_env_stat(MDBX_env *env, MDBX_stat *arg, size_t bytes) {
  return mdbx_env_stat2(env, NULL, arg, bytes);
}

int __cold mdbx_env_stat2(const MDBX_env *env, const MDBX_txn *txn,
                          MDBX_stat *arg, size_t bytes) {
  if (unlikely((env == NULL && txn == NULL) || arg == NULL))
    return MDBX_EINVAL;

  if (txn) {
    if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
      return MDBX_EBADSIGN;
    if (unlikely(txn->mt_owner != mdbx_thread_self()))
      return MDBX_THREAD_MISMATCH;
  }
  if (env) {
    if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
      return MDBX_EBADSIGN;
    if (txn && unlikely(txn->mt_env != env))
      return MDBX_EINVAL;
  }

  if (unlikely(bytes != sizeof(MDBX_stat)))
    return MDBX_EINVAL;

  const MDBX_db *db =
      txn ? &txn->mt_dbs[MAIN_DBI] : &mdbx_meta_head(env)->mm_dbs[MAIN_DBI];
  return mdbx_stat0(txn ? txn->mt_env : env, db, arg);
}

int __cold mdbx_env_info(MDBX_env *env, MDBX_envinfo *arg, size_t bytes) {
  return mdbx_env_info2(env, NULL, arg, bytes);
}

int __cold mdbx_env_info2(const MDBX_env *env, const MDBX_txn *txn,
                          MDBX_envinfo *arg, size_t bytes) {
  if (unlikely((env == NULL && txn == NULL) || arg == NULL))
    return MDBX_EINVAL;

  if (txn) {
    if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
      return MDBX_EBADSIGN;
    if (unlikely(txn->mt_owner != mdbx_thread_self()))
      return MDBX_THREAD_MISMATCH;
  }
  if (env) {
    if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
      return MDBX_EBADSIGN;
    if (txn && unlikely(txn->mt_env != env))
      return MDBX_EINVAL;
  } else {
    env = txn->mt_env;
  }

  if (unlikely(bytes != sizeof(MDBX_envinfo)))
    return MDBX_EINVAL;

  const MDBX_meta *const meta0 = METAPAGE(env, 0);
  const MDBX_meta *const meta1 = METAPAGE(env, 1);
  const MDBX_meta *const meta2 = METAPAGE(env, 2);
  while (1) {
    if (unlikely(env->me_flags & MDBX_FATAL_ERROR))
      return MDBX_PANIC;

    const MDBX_meta *const recent_meta = mdbx_meta_head(env);
    arg->mi_recent_txnid = mdbx_meta_txnid_fluid(env, recent_meta);
    arg->mi_meta0_txnid = mdbx_meta_txnid_fluid(env, meta0);
    arg->mi_meta0_sign = meta0->mm_datasync_sign;
    arg->mi_meta1_txnid = mdbx_meta_txnid_fluid(env, meta1);
    arg->mi_meta1_sign = meta1->mm_datasync_sign;
    arg->mi_meta2_txnid = mdbx_meta_txnid_fluid(env, meta2);
    arg->mi_meta2_sign = meta2->mm_datasync_sign;

    const MDBX_meta *txn_meta = recent_meta;
    arg->mi_last_pgno = txn_meta->mm_geo.next - 1;
    arg->mi_geo.current = pgno2bytes(env, txn_meta->mm_geo.now);
    if (txn) {
      arg->mi_last_pgno = txn->mt_next_pgno - 1;
      arg->mi_geo.current = pgno2bytes(env, txn->mt_end_pgno);

      const txnid_t wanna_meta_txnid =
          (txn->mt_flags & MDBX_RDONLY) ? txn->mt_txnid : txn->mt_txnid - 1;
      txn_meta = (arg->mi_meta0_txnid == wanna_meta_txnid) ? meta0 : txn_meta;
      txn_meta = (arg->mi_meta1_txnid == wanna_meta_txnid) ? meta1 : txn_meta;
      txn_meta = (arg->mi_meta2_txnid == wanna_meta_txnid) ? meta2 : txn_meta;
    }
    arg->mi_geo.lower = pgno2bytes(env, txn_meta->mm_geo.lower);
    arg->mi_geo.upper = pgno2bytes(env, txn_meta->mm_geo.upper);
    arg->mi_geo.shrink = pgno2bytes(env, txn_meta->mm_geo.shrink);
    arg->mi_geo.grow = pgno2bytes(env, txn_meta->mm_geo.grow);

    arg->mi_mapsize = env->me_mapsize;
    mdbx_compiler_barrier();
    if (likely(arg->mi_meta0_txnid == mdbx_meta_txnid_fluid(env, meta0) &&
               arg->mi_meta0_sign == meta0->mm_datasync_sign &&
               arg->mi_meta1_txnid == mdbx_meta_txnid_fluid(env, meta1) &&
               arg->mi_meta1_sign == meta1->mm_datasync_sign &&
               arg->mi_meta2_txnid == mdbx_meta_txnid_fluid(env, meta2) &&
               arg->mi_meta2_sign == meta2->mm_datasync_sign &&
               recent_meta == mdbx_meta_head(env) &&
               arg->mi_recent_txnid == mdbx_meta_txnid_fluid(env, recent_meta)))
      break;
  }

  arg->mi_maxreaders = env->me_maxreaders;
  arg->mi_numreaders = env->me_lck ? env->me_lck->mti_numreaders : INT32_MAX;
  arg->mi_dxb_pagesize = env->me_psize;
  arg->mi_sys_pagesize = env->me_os_psize;

  arg->mi_self_latter_reader_txnid = arg->mi_latter_reader_txnid = 0;
  if (env->me_lck) {
    MDBX_reader *r = env->me_lck->mti_readers;
    arg->mi_self_latter_reader_txnid = arg->mi_latter_reader_txnid =
        arg->mi_recent_txnid;
    for (unsigned i = 0; i < arg->mi_numreaders; ++i) {
      const mdbx_pid_t pid = r[i].mr_pid;
      if (pid) {
        const txnid_t txnid = r[i].mr_txnid;
        if (arg->mi_latter_reader_txnid > txnid)
          arg->mi_latter_reader_txnid = txnid;
        if (pid == env->me_pid && arg->mi_self_latter_reader_txnid > txnid)
          arg->mi_self_latter_reader_txnid = txnid;
      }
    }
  }

  return MDBX_SUCCESS;
}

static MDBX_cmp_func *mdbx_default_keycmp(unsigned flags) {
  return (flags & MDBX_REVERSEKEY)
             ? mdbx_cmp_memnr
             : (flags & MDBX_INTEGERKEY) ? mdbx_cmp_int_a2 : mdbx_cmp_memn;
}

static MDBX_cmp_func *mdbx_default_datacmp(unsigned flags) {
  return !(flags & MDBX_DUPSORT)
             ? mdbx_cmp_memn
             : ((flags & MDBX_INTEGERDUP)
                    ? mdbx_cmp_int_ua
                    : ((flags & MDBX_REVERSEDUP) ? mdbx_cmp_memnr
                                                 : mdbx_cmp_memn));
}

static int mdbx_dbi_bind(MDBX_txn *txn, const MDBX_dbi dbi, unsigned user_flags,
                         MDBX_cmp_func *keycmp, MDBX_cmp_func *datacmp) {
  /* LY: so, accepting only three cases for the table's flags:
   * 1) user_flags and both comparators are zero
   *    = assume that a by-default mode/flags is requested for reading;
   * 2) user_flags exactly the same
   *    = assume that the target mode/flags are requested properly;
   * 3) user_flags differs, but table is empty and MDBX_CREATE is provided
   *    = assume that a properly create request with custom flags;
   */
  if ((user_flags ^ txn->mt_dbs[dbi].md_flags) & PERSISTENT_FLAGS) {
    /* flags ara differs, check other conditions */
    if (!user_flags && (!keycmp || keycmp == txn->mt_dbxs[dbi].md_cmp) &&
        (!datacmp || datacmp == txn->mt_dbxs[dbi].md_dcmp)) {
      /* no comparators were provided and flags are zero,
       * seems that is case #1 above */
      user_flags = txn->mt_dbs[dbi].md_flags;
    } else if ((user_flags & MDBX_CREATE) && txn->mt_dbs[dbi].md_entries == 0) {
      if (txn->mt_flags & MDBX_TXN_RDONLY)
        return /* FIXME: return extended info */ MDBX_EACCESS;
      /* make sure flags changes get committed */
      txn->mt_dbs[dbi].md_flags = user_flags & PERSISTENT_FLAGS;
      txn->mt_flags |= MDBX_TXN_DIRTY;
    } else {
      return /* FIXME: return extended info */ MDBX_INCOMPATIBLE;
    }
  }

  if (!txn->mt_dbxs[dbi].md_cmp || MDBX_DEBUG) {
    if (!keycmp)
      keycmp = mdbx_default_keycmp(user_flags);
    mdbx_tassert(txn, !txn->mt_dbxs[dbi].md_cmp ||
                          txn->mt_dbxs[dbi].md_cmp == keycmp);
    txn->mt_dbxs[dbi].md_cmp = keycmp;
  }

  if (!txn->mt_dbxs[dbi].md_dcmp || MDBX_DEBUG) {
    if (!datacmp)
      datacmp = mdbx_default_datacmp(user_flags);
    mdbx_tassert(txn, !txn->mt_dbxs[dbi].md_dcmp ||
                          txn->mt_dbxs[dbi].md_dcmp == datacmp);
    txn->mt_dbxs[dbi].md_dcmp = datacmp;
  }

  return MDBX_SUCCESS;
}

int mdbx_dbi_open_ex(MDBX_txn *txn, const char *table_name, unsigned user_flags,
                     MDBX_dbi *dbi, MDBX_cmp_func *keycmp,
                     MDBX_cmp_func *datacmp) {
  if (unlikely(!txn || !dbi || (user_flags & ~VALID_FLAGS) != 0))
    return MDBX_EINVAL;

  switch (user_flags &
          (MDBX_INTEGERDUP | MDBX_DUPFIXED | MDBX_DUPSORT | MDBX_REVERSEDUP)) {
  default:
    return MDBX_EINVAL;
  case MDBX_DUPSORT:
  case MDBX_DUPSORT | MDBX_REVERSEDUP:
  case MDBX_DUPSORT | MDBX_DUPFIXED:
  case MDBX_DUPSORT | MDBX_DUPFIXED | MDBX_REVERSEDUP:
  case MDBX_DUPSORT | MDBX_DUPFIXED | MDBX_INTEGERDUP:
  case MDBX_DUPSORT | MDBX_DUPFIXED | MDBX_INTEGERDUP | MDBX_REVERSEDUP:
  case 0:
    break;
  }

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  /* main table? */
  if (!table_name) {
    *dbi = MAIN_DBI;
    return mdbx_dbi_bind(txn, MAIN_DBI, user_flags, keycmp, datacmp);
  }

  if (txn->mt_dbxs[MAIN_DBI].md_cmp == NULL) {
    txn->mt_dbxs[MAIN_DBI].md_cmp =
        mdbx_default_keycmp(txn->mt_dbs[MAIN_DBI].md_flags);
    txn->mt_dbxs[MAIN_DBI].md_dcmp =
        mdbx_default_datacmp(txn->mt_dbs[MAIN_DBI].md_flags);
  }

  /* Is the DB already open? */
  size_t len = strlen(table_name);
  MDBX_dbi scan, slot;
  for (slot = scan = txn->mt_numdbs; --scan >= CORE_DBS;) {
    if (!txn->mt_dbxs[scan].md_name.iov_len) {
      /* Remember this free slot */
      slot = scan;
      continue;
    }
    if (len == txn->mt_dbxs[scan].md_name.iov_len &&
        !strncmp(table_name, txn->mt_dbxs[scan].md_name.iov_base, len)) {
      *dbi = scan;
      return mdbx_dbi_bind(txn, scan, user_flags, keycmp, datacmp);
    }
  }

  /* Fail, if no free slot and max hit */
  MDBX_env *env = txn->mt_env;
  if (unlikely(slot >= env->me_maxdbs))
    return MDBX_DBS_FULL;

  /* Cannot mix named table with some main-table flags */
  if (unlikely(txn->mt_dbs[MAIN_DBI].md_flags &
               (MDBX_DUPSORT | MDBX_INTEGERKEY)))
    return (user_flags & MDBX_CREATE) ? MDBX_INCOMPATIBLE : MDBX_NOTFOUND;

  /* Find the DB info */
  int exact = 0;
  MDBX_val key, data;
  key.iov_len = len;
  key.iov_base = (void *)table_name;
  MDBX_cursor mc;
  int rc = mdbx_cursor_init(&mc, txn, MAIN_DBI);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  rc = mdbx_cursor_set(&mc, &key, &data, MDBX_SET, &exact);
  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc != MDBX_NOTFOUND || !(user_flags & MDBX_CREATE))
      return rc;
  } else {
    /* make sure this is actually a table */
    MDBX_node *node = NODEPTR(mc.mc_pg[mc.mc_top], mc.mc_ki[mc.mc_top]);
    if (unlikely((node->mn_flags & (F_DUPDATA | F_SUBDATA)) != F_SUBDATA))
      return MDBX_INCOMPATIBLE;
  }

  if (rc != MDBX_SUCCESS && unlikely(txn->mt_flags & MDBX_TXN_RDONLY))
    return MDBX_EACCESS;

  /* Done here so we cannot fail after creating a new DB */
  char *namedup = mdbx_strdup(table_name);
  if (unlikely(!namedup))
    return MDBX_ENOMEM;

  int err = mdbx_fastmutex_acquire(&env->me_dbi_lock);
  if (unlikely(err != MDBX_SUCCESS)) {
    mdbx_free(namedup);
    return err;
  }

  if (txn->mt_numdbs < env->me_numdbs) {
    for (unsigned i = txn->mt_numdbs; i < env->me_numdbs; ++i) {
      txn->mt_dbflags[i] = 0;
      if (env->me_dbflags[i] & MDBX_VALID) {
        txn->mt_dbs[i].md_flags = env->me_dbflags[i] & PERSISTENT_FLAGS;
        txn->mt_dbflags[i] = DB_VALID | DB_USRVALID | DB_STALE;
        mdbx_tassert(txn, txn->mt_dbxs[i].md_cmp != NULL);
      }
    }
    txn->mt_numdbs = env->me_numdbs;
  }

  for (slot = scan = txn->mt_numdbs; --scan >= CORE_DBS;) {
    if (!txn->mt_dbxs[scan].md_name.iov_len) {
      /* Remember this free slot */
      slot = scan;
      continue;
    }
    if (len == txn->mt_dbxs[scan].md_name.iov_len &&
        !strncmp(table_name, txn->mt_dbxs[scan].md_name.iov_base, len)) {
      *dbi = scan;
      rc = mdbx_dbi_bind(txn, scan, user_flags, keycmp, datacmp);
      goto bailout;
    }
  }

  if (unlikely(slot >= env->me_maxdbs)) {
    rc = MDBX_DBS_FULL;
    goto bailout;
  }

  unsigned dbflag = DB_FRESH | DB_VALID | DB_USRVALID;
  MDBX_db db_dummy;
  if (unlikely(rc)) {
    /* MDBX_NOTFOUND and MDBX_CREATE: Create new DB */
    mdbx_tassert(txn, rc == MDBX_NOTFOUND);
    memset(&db_dummy, 0, sizeof(db_dummy));
    db_dummy.md_root = P_INVALID;
    db_dummy.md_flags = user_flags & PERSISTENT_FLAGS;
    data.iov_len = sizeof(db_dummy);
    data.iov_base = &db_dummy;
    WITH_CURSOR_TRACKING(
        mc,
        rc = mdbx_cursor_put(&mc, &key, &data, F_SUBDATA | MDBX_NOOVERWRITE));

    if (unlikely(rc != MDBX_SUCCESS))
      goto bailout;

    dbflag |= DB_DIRTY | DB_CREAT;
  }

  /* Got info, register DBI in this txn */
  txn->mt_dbxs[slot].md_cmp = nullptr;
  txn->mt_dbxs[slot].md_dcmp = nullptr;
  txn->mt_dbs[slot] = *(MDBX_db *)data.iov_base;
  env->me_dbflags[slot] = 0;
  rc = mdbx_dbi_bind(txn, slot, user_flags, keycmp, datacmp);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_tassert(txn, (dbflag & DB_CREAT) == 0);
  bailout:
    mdbx_free(namedup);
  } else {
    txn->mt_dbflags[slot] = (uint8_t)dbflag;
    txn->mt_dbxs[slot].md_name.iov_base = namedup;
    txn->mt_dbxs[slot].md_name.iov_len = len;
    txn->mt_numdbs += (slot == txn->mt_numdbs);
    if ((dbflag & DB_CREAT) == 0) {
      env->me_dbflags[slot] = txn->mt_dbs[slot].md_flags | MDBX_VALID;
      mdbx_compiler_barrier();
      if (env->me_numdbs <= slot)
        env->me_numdbs = slot + 1;
    } else {
      env->me_dbiseqs[slot] += 1;
    }
    txn->mt_dbiseqs[slot] = env->me_dbiseqs[slot];
    *dbi = slot;
  }

  mdbx_ensure(env, mdbx_fastmutex_release(&env->me_dbi_lock) == MDBX_SUCCESS);
  return rc;
}

int mdbx_dbi_open(MDBX_txn *txn, const char *table_name, unsigned table_flags,
                  MDBX_dbi *dbi) {
  return mdbx_dbi_open_ex(txn, table_name, table_flags, dbi, nullptr, nullptr);
}

int __cold mdbx_dbi_stat(MDBX_txn *txn, MDBX_dbi dbi, MDBX_stat *arg,
                         size_t bytes) {
  if (unlikely(!arg || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_VALID)))
    return MDBX_EINVAL;

  if (unlikely(bytes != sizeof(MDBX_stat)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  if (unlikely(txn->mt_dbflags[dbi] & DB_STALE)) {
    MDBX_cursor_couple cx;
    /* Stale, must read the DB's root. cursor_init does it for us. */
    int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }
  return mdbx_stat0(txn->mt_env, &txn->mt_dbs[dbi], arg);
}

static int mdbx_dbi_close_locked(MDBX_env *env, MDBX_dbi dbi) {
  if (unlikely(dbi < CORE_DBS || dbi >= env->me_maxdbs))
    return MDBX_EINVAL;

  char *ptr = env->me_dbxs[dbi].md_name.iov_base;
  /* If there was no name, this was already closed */
  if (unlikely(!ptr))
    return MDBX_BAD_DBI;

  env->me_dbflags[dbi] = 0;
  env->me_dbxs[dbi].md_name.iov_len = 0;
  mdbx_compiler_barrier();
  env->me_dbiseqs[dbi]++;
  env->me_dbxs[dbi].md_name.iov_base = NULL;
  mdbx_free(ptr);
  return MDBX_SUCCESS;
}

int mdbx_dbi_close(MDBX_env *env, MDBX_dbi dbi) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(dbi < CORE_DBS || dbi >= env->me_maxdbs))
    return MDBX_EINVAL;

  int rc = mdbx_fastmutex_acquire(&env->me_dbi_lock);
  if (likely(rc == MDBX_SUCCESS)) {
    rc = mdbx_dbi_close_locked(env, dbi);
    mdbx_ensure(env, mdbx_fastmutex_release(&env->me_dbi_lock) == MDBX_SUCCESS);
  }
  return rc;
}

int mdbx_dbi_flags_ex(MDBX_txn *txn, MDBX_dbi dbi, unsigned *flags,
                      unsigned *state) {
  if (unlikely(!txn || !flags || !state))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_VALID)))
    return MDBX_EINVAL;

  *flags = txn->mt_dbs[dbi].md_flags & PERSISTENT_FLAGS;
  *state = txn->mt_dbflags[dbi] & (DB_FRESH | DB_CREAT | DB_DIRTY | DB_STALE);

  return MDBX_SUCCESS;
}

int mdbx_dbi_flags(MDBX_txn *txn, MDBX_dbi dbi, unsigned *flags) {
  unsigned state;
  return mdbx_dbi_flags_ex(txn, dbi, flags, &state);
}

/* Add all the DB's pages to the free list.
 * [in] mc Cursor on the DB to free.
 * [in] subs non-Zero to check for sub-DBs in this DB.
 * Returns 0 on success, non-zero on failure. */
static int mdbx_drop0(MDBX_cursor *mc, int subs) {
  int rc = mdbx_page_search(mc, NULL, MDBX_PS_FIRST);
  if (likely(rc == MDBX_SUCCESS)) {
    MDBX_txn *txn = mc->mc_txn;
    MDBX_node *ni;
    MDBX_cursor mx;
    unsigned i;

    /* DUPSORT sub-DBs have no ovpages/DBs. Omit scanning leaves.
     * This also avoids any P_LEAF2 pages, which have no nodes.
     * Also if the DB doesn't have sub-DBs and has no overflow
     * pages, omit scanning leaves. */

    if (mc->mc_flags & C_SUB) {
      MDBX_db *outer = mdbx_outer_db(mc);
      outer->md_branch_pages -= mc->mc_db->md_branch_pages;
      outer->md_leaf_pages -= mc->mc_db->md_leaf_pages;
      outer->md_overflow_pages -= mc->mc_db->md_overflow_pages;
      mdbx_cursor_pop(mc);
    } else if (!subs && !mc->mc_db->md_overflow_pages)
      mdbx_cursor_pop(mc);

    mdbx_cursor_copy(mc, &mx);
    while (mc->mc_snum > 0) {
      MDBX_page *mp = mc->mc_pg[mc->mc_top];
      unsigned n = NUMKEYS(mp);
      if (IS_LEAF(mp)) {
        for (i = 0; i < n; i++) {
          ni = NODEPTR(mp, i);
          if (ni->mn_flags & F_BIGDATA) {
            MDBX_page *omp;
            pgno_t pg;
            memcpy(&pg, NODEDATA(ni), sizeof(pg));
            rc = mdbx_page_get(mc, pg, &omp, NULL);
            if (unlikely(rc))
              goto done;
            mdbx_cassert(mc, IS_OVERFLOW(omp));
            rc = mdbx_page_befree(mc, omp);
            if (unlikely(rc))
              goto done;
            if (!mc->mc_db->md_overflow_pages && !subs)
              break;
          } else if (subs && (ni->mn_flags & F_SUBDATA)) {
            rc = mdbx_xcursor_init1(mc, ni);
            if (unlikely(rc != MDBX_SUCCESS))
              goto done;
            rc = mdbx_drop0(&mc->mc_xcursor->mx_cursor, 0);
            if (unlikely(rc))
              goto done;
          }
        }
        if (!subs && !mc->mc_db->md_overflow_pages)
          goto pop;
      } else {
        if (unlikely((rc = mdbx_pnl_need(&txn->mt_befree_pages, n)) != 0))
          goto done;
        for (i = 0; i < n; i++) {
          pgno_t pg;
          ni = NODEPTR(mp, i);
          pg = NODEPGNO(ni);
          /* free it */
          mdbx_pnl_xappend(txn->mt_befree_pages, pg);
        }
      }
      if (!mc->mc_top)
        break;
      mdbx_cassert(mc, i <= UINT16_MAX);
      mc->mc_ki[mc->mc_top] = (indx_t)i;
      rc = mdbx_cursor_sibling(mc, 1);
      if (rc) {
        if (unlikely(rc != MDBX_NOTFOUND))
          goto done;
      /* no more siblings, go back to beginning
       * of previous level. */
      pop:
        mdbx_cursor_pop(mc);
        mc->mc_ki[0] = 0;
        for (i = 1; i < mc->mc_snum; i++) {
          mc->mc_ki[i] = 0;
          mc->mc_pg[i] = mx.mc_pg[i];
        }
      }
    }
    /* free it */
    rc = mdbx_pnl_append(&txn->mt_befree_pages, mc->mc_db->md_root);
  done:
    if (unlikely(rc))
      txn->mt_flags |= MDBX_TXN_ERROR;
  } else if (rc == MDBX_NOTFOUND) {
    rc = MDBX_SUCCESS;
  }
  mc->mc_flags &= ~C_INITIALIZED;
  return rc;
}

int mdbx_drop(MDBX_txn *txn, MDBX_dbi dbi, int del) {
  if (unlikely(1 < (unsigned)del || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(TXN_DBI_CHANGED(txn, dbi)))
    return MDBX_BAD_DBI;

  if (unlikely(F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)))
    return MDBX_EACCESS;

  MDBX_cursor *mc;
  int rc = mdbx_cursor_open(txn, dbi, &mc);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  MDBX_env *env = txn->mt_env;
  rc = mdbx_fastmutex_acquire(&env->me_dbi_lock);
  if (unlikely(rc != MDBX_SUCCESS)) {
    mdbx_cursor_close(mc);
    return rc;
  }

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID))) {
    rc = MDBX_EINVAL;
    goto bailout;
  }

  if (unlikely(TXN_DBI_CHANGED(txn, dbi))) {
    rc = MDBX_BAD_DBI;
    goto bailout;
  }

  rc = mdbx_drop0(mc, mc->mc_db->md_flags & MDBX_DUPSORT);
  /* Invalidate the dropped DB's cursors */
  for (MDBX_cursor *m2 = txn->mt_cursors[dbi]; m2; m2 = m2->mc_next)
    m2->mc_flags &= ~(C_INITIALIZED | C_EOF);
  if (unlikely(rc))
    goto bailout;

  /* Can't delete the main DB */
  if (del && dbi >= CORE_DBS) {
    rc = mdbx_del0(txn, MAIN_DBI, &mc->mc_dbx->md_name, NULL, F_SUBDATA);
    if (likely(!rc)) {
      txn->mt_dbflags[dbi] = DB_STALE;
      mdbx_dbi_close_locked(env, dbi);
    } else {
      txn->mt_flags |= MDBX_TXN_ERROR;
    }
  } else {
    /* reset the DB record, mark it dirty */
    txn->mt_dbflags[dbi] |= DB_DIRTY;
    txn->mt_dbs[dbi].md_depth = 0;
    txn->mt_dbs[dbi].md_branch_pages = 0;
    txn->mt_dbs[dbi].md_leaf_pages = 0;
    txn->mt_dbs[dbi].md_overflow_pages = 0;
    txn->mt_dbs[dbi].md_entries = 0;
    txn->mt_dbs[dbi].md_root = P_INVALID;
    txn->mt_dbs[dbi].md_seq = 0;
    txn->mt_flags |= MDBX_TXN_DIRTY;
  }

bailout:
  mdbx_cursor_close(mc);
  mdbx_ensure(env, mdbx_fastmutex_release(&env->me_dbi_lock) == MDBX_SUCCESS);
  return rc;
}

int mdbx_set_compare(MDBX_txn *txn, MDBX_dbi dbi, MDBX_cmp_func *cmp) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  txn->mt_dbxs[dbi].md_cmp = cmp;
  return MDBX_SUCCESS;
}

int mdbx_set_dupsort(MDBX_txn *txn, MDBX_dbi dbi, MDBX_cmp_func *cmp) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  txn->mt_dbxs[dbi].md_dcmp = cmp;
  return MDBX_SUCCESS;
}

int __cold mdbx_reader_list(MDBX_env *env, MDBX_msg_func *func, void *ctx) {
  char buf[64];
  int rc = 0, first = 1;

  if (unlikely(!env || !func))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  const MDBX_lockinfo *const lck = env->me_lck;
  if (likely(lck)) {
    const unsigned snap_nreaders = lck->mti_numreaders;
    for (unsigned i = 0; i < snap_nreaders; i++) {
      if (lck->mti_readers[i].mr_pid) {
        const txnid_t txnid = lck->mti_readers[i].mr_txnid;
        if (txnid == ~(txnid_t)0)
          snprintf(buf, sizeof(buf), "%10" PRIuPTR " %" PRIxPTR " -\n",
                   (uintptr_t)lck->mti_readers[i].mr_pid,
                   (uintptr_t)lck->mti_readers[i].mr_tid);
        else
          snprintf(buf, sizeof(buf),
                   "%10" PRIuPTR " %" PRIxPTR " %" PRIaTXN "\n",
                   (uintptr_t)lck->mti_readers[i].mr_pid,
                   (uintptr_t)lck->mti_readers[i].mr_tid, txnid);

        if (first) {
          first = 0;
          rc = func("    pid     thread     txnid\n", ctx);
          if (rc < 0)
            break;
        }
        rc = func(buf, ctx);
        if (rc < 0)
          break;
      }
    }
  }
  if (first)
    rc = func("(no active readers)\n", ctx);

  return rc;
}

/* Insert pid into list if not already present.
 * return -1 if already present. */
static int __cold mdbx_pid_insert(mdbx_pid_t *ids, mdbx_pid_t pid) {
  /* binary search of pid in list */
  unsigned base = 0;
  unsigned cursor = 1;
  int val = 0;
  unsigned n = ids[0];

  while (n > 0) {
    unsigned pivot = n >> 1;
    cursor = base + pivot + 1;
    val = pid - ids[cursor];

    if (val < 0) {
      n = pivot;
    } else if (val > 0) {
      base = cursor;
      n -= pivot + 1;
    } else {
      /* found, so it's a duplicate */
      return -1;
    }
  }

  if (val > 0)
    ++cursor;

  ids[0]++;
  for (n = ids[0]; n > cursor; n--)
    ids[n] = ids[n - 1];
  ids[n] = pid;
  return 0;
}

int __cold mdbx_reader_check(MDBX_env *env, int *dead) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (dead)
    *dead = 0;
  return mdbx_reader_check0(env, false, dead);
}

/* Return:
 *  MDBX_RESULT_TRUE - done and mutex recovered
 *  MDBX_SUCCESS     - done
 *  Otherwise errcode. */
int __cold mdbx_reader_check0(MDBX_env *env, int rdt_locked, int *dead) {
  mdbx_assert(env, rdt_locked >= 0);

#if MDBX_TXN_CHECKPID
  if (unlikely(env->me_pid != mdbx_getpid())) {
    env->me_flags |= MDBX_FATAL_ERROR;
    return MDBX_PANIC;
  }
#endif /* MDBX_TXN_CHECKPID */

  MDBX_lockinfo *const lck = env->me_lck;
  if (unlikely(lck == NULL)) {
    /* exclusive mode */
    if (dead)
      *dead = 0;
    return MDBX_SUCCESS;
  }

  const unsigned snap_nreaders = lck->mti_numreaders;
  mdbx_pid_t pidsbuf_onstask[142];
  mdbx_pid_t *const pids =
      (snap_nreaders < ARRAY_LENGTH(pidsbuf_onstask))
          ? pidsbuf_onstask
          : mdbx_malloc((snap_nreaders + 1) * sizeof(mdbx_pid_t));
  if (unlikely(!pids))
    return MDBX_ENOMEM;

  pids[0] = 0;

  int rc = MDBX_SUCCESS, count = 0;
  for (unsigned i = 0; i < snap_nreaders; i++) {
    const mdbx_pid_t pid = lck->mti_readers[i].mr_pid;
    if (pid == 0)
      continue /* skip empty */;
    if (pid == env->me_pid)
      continue /* skip self */;
    if (mdbx_pid_insert(pids, pid) != 0)
      continue /* such pid already processed */;

    int err = mdbx_rpid_check(env, pid);
    if (err == MDBX_RESULT_TRUE)
      continue /* reader is live */;

    if (err != MDBX_SUCCESS) {
      rc = err;
      break /* mdbx_rpid_check() failed */;
    }

    /* stale reader found */
    if (!rdt_locked) {
      err = mdbx_rdt_lock(env);
      if (MDBX_IS_ERROR(err)) {
        rc = err;
        break;
      }

      rdt_locked = -1;
      if (err == MDBX_RESULT_TRUE) {
        /* mutex recovered, the mdbx_mutex_failed() checked all readers */
        rc = MDBX_RESULT_TRUE;
        break;
      }

      /* a other process may have clean and reused slot, recheck */
      if (lck->mti_readers[i].mr_pid != pid)
        continue;

      err = mdbx_rpid_check(env, pid);
      if (MDBX_IS_ERROR(err)) {
        rc = err;
        break;
      }

      if (err != MDBX_SUCCESS)
        continue /* the race with other process, slot reused */;
    }

    /* clean it */
    for (unsigned j = i; j < snap_nreaders; j++) {
      if (lck->mti_readers[j].mr_pid == pid) {
        mdbx_debug("clear stale reader pid %" PRIuPTR " txn %" PRIaTXN,
                   (size_t)pid, lck->mti_readers[j].mr_txnid);
        lck->mti_readers[j].mr_pid = 0;
        lck->mti_readers_refresh_flag = true;
        count++;
      }
    }
  }

  if (rdt_locked < 0)
    mdbx_rdt_unlock(env);

  if (pids != pidsbuf_onstask)
    mdbx_free(pids);

  if (dead)
    *dead = count;
  return rc;
}

int __cold mdbx_setup_debug(int flags, MDBX_debug_func *logger) {
  const int rc = mdbx_runtime_flags;
  if (flags != -1) {
#if MDBX_DEBUG
    flags &= MDBX_DBG_DUMP | MDBX_DBG_LEGACY_MULTIOPEN;
#else
    flags &= MDBX_DBG_ASSERT | MDBX_DBG_PRINT | MDBX_DBG_TRACE |
             MDBX_DBG_EXTRA | MDBX_DBG_AUDIT | MDBX_DBG_JITTER | MDBX_DBG_DUMP |
             MDBX_DBG_LEGACY_MULTIOPEN;
#endif
#if defined(__linux__) || defined(__gnu_linux__)
    if ((mdbx_runtime_flags ^ flags) & MDBX_DBG_DUMP) {
      /* http://man7.org/linux/man-pages/man5/core.5.html */
      const unsigned long dump_bits =
          1 << 3   /* Dump file-backed shared mappings */
          | 1 << 6 /* Dump shared huge pages */
          | 1 << 8 /* Dump shared DAX pages */;
      const int core_filter_fd =
          open("/proc/self/coredump_filter", O_TRUNC | O_RDWR);
      if (core_filter_fd != -1) {
        char buf[32];
        intptr_t bytes = pread(core_filter_fd, buf, sizeof(buf), 0);
        if (bytes > 0 && (size_t)bytes < sizeof(buf)) {
          buf[bytes] = 0;
          const unsigned long present_mask = strtoul(buf, NULL, 16);
          const unsigned long wanna_mask = (flags & MDBX_DBG_DUMP)
                                               ? present_mask | dump_bits
                                               : present_mask & ~dump_bits;
          if (wanna_mask != present_mask) {
            bytes = snprintf(buf, sizeof(buf), "0x%lx\n", wanna_mask);
            if (bytes > 0 && (size_t)bytes < sizeof(buf)) {
              bytes = pwrite(core_filter_fd, buf, bytes, 0);
              (void)bytes;
            }
          }
        }
        close(core_filter_fd);
      }
    }
#endif /* Linux */
    mdbx_runtime_flags = flags;
  }

  if (-1 != (intptr_t)logger)
    mdbx_debug_logger = logger;
  return rc;
}

static txnid_t __cold mdbx_oomkick(MDBX_env *env, const txnid_t laggard) {
  mdbx_debug("DB size maxed out");

  int retry;
  for (retry = 0; retry < INT_MAX; ++retry) {
    txnid_t oldest = mdbx_reclaiming_detent(env);
    mdbx_assert(env, oldest < env->me_txn0->mt_txnid);
    mdbx_assert(env, oldest >= laggard);
    mdbx_assert(env, oldest >= *env->me_oldest);
    if (oldest == laggard || unlikely(env->me_lck == NULL /* exclusive mode */))
      return oldest;

    if (MDBX_IS_ERROR(mdbx_reader_check0(env, false, NULL)))
      break;

    MDBX_reader *const rtbl = env->me_lck->mti_readers;
    MDBX_reader *asleep = nullptr;
    for (int i = env->me_lck->mti_numreaders; --i >= 0;) {
      if (rtbl[i].mr_pid) {
        mdbx_jitter4testing(true);
        const txnid_t snap = rtbl[i].mr_txnid;
        if (oldest > snap && laggard <= /* ignore pending updates */ snap) {
          oldest = snap;
          asleep = &rtbl[i];
        }
      }
    }

    if (laggard < oldest || !asleep) {
      if (retry && env->me_oom_func) {
        /* LY: notify end of oom-loop */
        const txnid_t gap = oldest - laggard;
        env->me_oom_func(env, 0, 0, laggard,
                         (gap < UINT_MAX) ? (unsigned)gap : UINT_MAX, -retry);
      }
      mdbx_notice("oom-kick: update oldest %" PRIaTXN " -> %" PRIaTXN,
                  *env->me_oldest, oldest);
      mdbx_assert(env, *env->me_oldest <= oldest);
      return *env->me_oldest = oldest;
    }

    mdbx_tid_t tid;
    mdbx_pid_t pid;
    int rc;

    if (!env->me_oom_func)
      break;

    pid = asleep->mr_pid;
    tid = asleep->mr_tid;
    if (asleep->mr_txnid != laggard || pid <= 0)
      continue;

    const txnid_t gap =
        mdbx_meta_txnid_stable(env, mdbx_meta_head(env)) - laggard;
    rc = env->me_oom_func(env, pid, tid, laggard,
                          (gap < UINT_MAX) ? (unsigned)gap : UINT_MAX, retry);
    if (rc < 0)
      break;

    if (rc) {
      asleep->mr_txnid = ~(txnid_t)0;
      env->me_lck->mti_readers_refresh_flag = true;
      if (rc > 1) {
        asleep->mr_tid = 0;
        asleep->mr_pid = 0;
        mdbx_flush_noncoherent_cpu_writeback();
      }
    }
  }

  if (retry && env->me_oom_func) {
    /* LY: notify end of oom-loop */
    env->me_oom_func(env, 0, 0, laggard, 0, -retry);
  }
  return mdbx_find_oldest(env->me_txn);
}

int __cold mdbx_env_set_syncbytes(MDBX_env *env, size_t bytes) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(env->me_flags & (MDBX_RDONLY | MDBX_FATAL_ERROR)))
    return MDBX_EACCESS;

  if (unlikely(!env->me_map))
    return MDBX_EPERM;

  *env->me_autosync_threshold = bytes2pgno(env, bytes + env->me_psize - 1);
  return bytes ? mdbx_env_sync(env, false) : MDBX_SUCCESS;
}

int __cold mdbx_env_set_syncperiod(MDBX_env *env, unsigned seconds_16dot16) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(env->me_flags & (MDBX_RDONLY | MDBX_FATAL_ERROR)))
    return MDBX_EACCESS;

  if (unlikely(!env->me_map))
    return MDBX_EPERM;

  *env->me_autosync_period = mdbx_osal_16dot16_to_monotime(seconds_16dot16);
  return seconds_16dot16 ? mdbx_env_sync(env, false) : MDBX_SUCCESS;
}

int __cold mdbx_env_set_oomfunc(MDBX_env *env, MDBX_oom_func *oomfunc) {
  if (unlikely(!env))
    return MDBX_EINVAL;

  if (unlikely(env->me_signature != MDBX_ME_SIGNATURE))
    return MDBX_EBADSIGN;

  env->me_oom_func = oomfunc;
  return MDBX_SUCCESS;
}

MDBX_oom_func *__cold mdbx_env_get_oomfunc(MDBX_env *env) {
  return likely(env && env->me_signature == MDBX_ME_SIGNATURE)
             ? env->me_oom_func
             : NULL;
}

#ifdef __SANITIZE_THREAD__
/* LY: avoid tsan-trap by me_txn, mm_last_pg and mt_next_pgno */
__attribute__((__no_sanitize_thread__, __noinline__))
#endif
int mdbx_txn_straggler(MDBX_txn *txn, int *percent)
{
  if (unlikely(!txn))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  MDBX_env *env = txn->mt_env;
  if (unlikely((txn->mt_flags & MDBX_RDONLY) == 0)) {
    if (percent)
      *percent =
          (int)((txn->mt_next_pgno * UINT64_C(100) + txn->mt_end_pgno / 2) /
                txn->mt_end_pgno);
    return 0;
  }

  txnid_t recent;
  MDBX_meta *meta;
  do {
    meta = mdbx_meta_head(env);
    recent = mdbx_meta_txnid_fluid(env, meta);
    if (percent) {
      const pgno_t maxpg = meta->mm_geo.now;
      *percent = (int)((meta->mm_geo.next * UINT64_C(100) + maxpg / 2) / maxpg);
    }
  } while (unlikely(recent != mdbx_meta_txnid_fluid(env, meta)));

  txnid_t lag = recent - txn->mt_ro_reader->mr_txnid;
  return (lag > INT_MAX) ? INT_MAX : (int)lag;
}

typedef struct mdbx_walk_ctx {
  void *mw_user;
  MDBX_pgvisitor_func *mw_visitor;
  MDBX_cursor mw_cursor;
} mdbx_walk_ctx_t;

/* Depth-first tree traversal. */
static int __cold mdbx_env_walk(mdbx_walk_ctx_t *ctx, const char *dbi,
                                pgno_t pgno, int deep) {
  if (unlikely(pgno == P_INVALID))
    return MDBX_SUCCESS; /* empty db */

  MDBX_page *mp;
  int rc = mdbx_page_get(&ctx->mw_cursor, pgno, &mp, NULL);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  const int nkeys = NUMKEYS(mp);
  size_t header_size = IS_LEAF2(mp) ? PAGEHDRSZ : PAGEHDRSZ + mp->mp_lower;
  size_t unused_size = SIZELEFT(mp);
  size_t payload_size = 0;
  size_t align_bytes = 0;
  MDBX_page_type_t type;

  /* LY: Don't use mask here, e.g bitwise
   * (P_BRANCH|P_LEAF|P_LEAF2|P_META|P_OVERFLOW|P_SUBP).
   * Pages should not me marked dirty/loose or otherwise. */
  switch (mp->mp_flags) {
  case P_BRANCH:
    type = MDBX_page_branch;
    if (unlikely(nkeys < 2))
      return MDBX_CORRUPTED;
    break;
  case P_LEAF:
    type = MDBX_page_leaf;
    break;
  case P_LEAF | P_LEAF2:
    type = MDBX_page_dupfixed_leaf;
    break;
  default:
    return MDBX_CORRUPTED;
  }

  for (int i = 0; i < nkeys;
       align_bytes += ((payload_size + align_bytes) & 1), i++) {
    if (type == MDBX_page_dupfixed_leaf) {
      /* LEAF2 pages have no mp_ptrs[] or node headers */
      payload_size += mp->mp_leaf2_ksize;
      continue;
    }

    MDBX_node *node = NODEPTR(mp, i);
    payload_size += NODESIZE + NODEKSZ(node);

    if (type == MDBX_page_branch)
      continue;

    assert(type == MDBX_page_leaf);
    switch (node->mn_flags) {
    case 0 /* usual node */: {
      payload_size += NODEDSZ(node);
    } break;

    case F_BIGDATA /* long data on the large/overflow page */: {
      payload_size += sizeof(pgno_t);

      MDBX_page *op;
      pgno_t large_pgno;
      memcpy(&large_pgno, NODEDATA(node), sizeof(pgno_t));
      rc = mdbx_page_get(&ctx->mw_cursor, large_pgno, &op, NULL);
      if (unlikely(rc != MDBX_SUCCESS))
        return rc;

      /* LY: Don't use mask here, e.g bitwise
       * (P_BRANCH|P_LEAF|P_LEAF2|P_META|P_OVERFLOW|P_SUBP).
       * Pages should not me marked dirty/loose or otherwise. */
      if (unlikely(P_OVERFLOW != op->mp_flags))
        return MDBX_CORRUPTED;

      const size_t over_header = PAGEHDRSZ;
      const size_t over_payload = NODEDSZ(node);
      const size_t over_unused =
          pgno2bytes(ctx->mw_cursor.mc_txn->mt_env, op->mp_pages) -
          over_payload - over_header;

      rc = ctx->mw_visitor(
          large_pgno, op->mp_pages, ctx->mw_user, deep, dbi,
          pgno2bytes(ctx->mw_cursor.mc_txn->mt_env, op->mp_pages),
          MDBX_page_large, 1, over_payload, over_header, over_unused);
    } break;

    case F_SUBDATA /* sub-db */: {
      const size_t namelen = NODEKSZ(node);
      if (unlikely(namelen == 0 || NODEDSZ(node) != sizeof(MDBX_db)))
        return MDBX_CORRUPTED;
      payload_size += sizeof(MDBX_db);
    } break;

    case F_SUBDATA | F_DUPDATA /* dupsorted sub-tree */: {
      if (unlikely(NODEDSZ(node) != sizeof(MDBX_db)))
        return MDBX_CORRUPTED;
      payload_size += sizeof(MDBX_db);
    } break;

    case F_DUPDATA /* short sub-page */: {
      if (unlikely(NODEDSZ(node) < PAGEHDRSZ))
        return MDBX_CORRUPTED;

      MDBX_page *sp = NODEDATA(node);
      const int nsubkeys = NUMKEYS(sp);
      size_t subheader_size =
          IS_LEAF2(sp) ? PAGEHDRSZ : PAGEHDRSZ + sp->mp_lower;
      size_t subunused_size = SIZELEFT(sp);
      size_t subpayload_size = 0;
      size_t subalign_bytes = 0;
      MDBX_page_type_t subtype;

      switch (sp->mp_flags & ~P_DIRTY /* ignore for sub-pages */) {
      case P_LEAF | P_SUBP:
        subtype = MDBX_subpage_leaf;
        break;
      case P_LEAF | P_LEAF2 | P_SUBP:
        subtype = MDBX_subpage_dupfixed_leaf;
        break;
      default:
        return MDBX_CORRUPTED;
      }

      for (int j = 0; j < nsubkeys;
           subalign_bytes += ((subpayload_size + subalign_bytes) & 1), j++) {

        if (subtype == MDBX_subpage_dupfixed_leaf) {
          /* LEAF2 pages have no mp_ptrs[] or node headers */
          subpayload_size += sp->mp_leaf2_ksize;
        } else {
          assert(subtype == MDBX_subpage_leaf);
          MDBX_node *subnode = NODEPTR(sp, j);
          subpayload_size += NODESIZE + NODEKSZ(subnode) + NODEDSZ(subnode);
          if (unlikely(subnode->mn_flags != 0))
            return MDBX_CORRUPTED;
        }
      }

      rc = ctx->mw_visitor(pgno, 0, ctx->mw_user, deep + 1, dbi, NODEDSZ(node),
                           subtype, nsubkeys, subpayload_size, subheader_size,
                           subunused_size + subalign_bytes);
      header_size += subheader_size;
      unused_size += subunused_size;
      payload_size += subpayload_size;
      align_bytes += subalign_bytes;
    } break;

    default:
      return MDBX_CORRUPTED;
    }

    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  rc = ctx->mw_visitor(mp->mp_pgno, 1, ctx->mw_user, deep, dbi,
                       ctx->mw_cursor.mc_txn->mt_env->me_psize, type, nkeys,
                       payload_size, header_size, unused_size + align_bytes);

  if (unlikely(rc != MDBX_SUCCESS))
    return (rc == MDBX_RESULT_TRUE) ? MDBX_SUCCESS : rc;

  for (int i = 0; i < nkeys; i++) {
    if (type == MDBX_page_dupfixed_leaf)
      continue;

    MDBX_node *node = NODEPTR(mp, i);
    if (type == MDBX_page_branch) {
      rc = mdbx_env_walk(ctx, dbi, NODEPGNO(node), deep + 1);
      if (unlikely(rc != MDBX_SUCCESS)) {
        if (rc != MDBX_RESULT_TRUE)
          return rc;
        break;
      }
      continue;
    }

    assert(type == MDBX_page_leaf);
    MDBX_db db;
    switch (node->mn_flags) {
    default:
      continue;

    case F_SUBDATA /* sub-db */: {
      const size_t namelen = NODEKSZ(node);
      if (unlikely(namelen == 0 || NODEDSZ(node) != sizeof(MDBX_db)))
        return MDBX_CORRUPTED;

      char namebuf_onstask[142];
      char *const name = (namelen < sizeof(namebuf_onstask))
                             ? namebuf_onstask
                             : mdbx_malloc(namelen + 1);
      if (name) {
        memcpy(name, NODEKEY(node), namelen);
        name[namelen] = 0;
        memcpy(&db, NODEDATA(node), sizeof(db));
        rc = mdbx_env_walk(ctx, name, db.md_root, deep + 1);
        if (name != namebuf_onstask)
          mdbx_free(name);
      } else {
        rc = MDBX_ENOMEM;
      }
    } break;

    case F_SUBDATA | F_DUPDATA /* dupsorted sub-tree */:
      if (unlikely(NODEDSZ(node) != sizeof(MDBX_db)))
        return MDBX_CORRUPTED;

      memcpy(&db, NODEDATA(node), sizeof(db));
      rc = mdbx_env_walk(ctx, dbi, db.md_root, deep + 1);
      break;
    }

    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  return MDBX_SUCCESS;
}

int __cold mdbx_env_pgwalk(MDBX_txn *txn, MDBX_pgvisitor_func *visitor,
                           void *user) {
  if (unlikely(!txn))
    return MDBX_BAD_TXN;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  mdbx_walk_ctx_t ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.mw_cursor.mc_snum = 1;
  ctx.mw_cursor.mc_txn = txn;
  ctx.mw_user = user;
  ctx.mw_visitor = visitor;

  int rc = visitor(
      0, NUM_METAS, user, 0, MDBX_PGWALK_META,
      pgno2bytes(txn->mt_env, NUM_METAS), MDBX_page_meta, NUM_METAS,
      sizeof(MDBX_meta) * NUM_METAS, PAGEHDRSZ * NUM_METAS,
      (txn->mt_env->me_psize - sizeof(MDBX_meta) - PAGEHDRSZ) * NUM_METAS);
  if (!MDBX_IS_ERROR(rc))
    rc = mdbx_env_walk(&ctx, MDBX_PGWALK_GC, txn->mt_dbs[FREE_DBI].md_root, 0);
  if (!MDBX_IS_ERROR(rc))
    rc =
        mdbx_env_walk(&ctx, MDBX_PGWALK_MAIN, txn->mt_dbs[MAIN_DBI].md_root, 0);
  if (!MDBX_IS_ERROR(rc))
    rc = visitor(P_INVALID, 0, user, INT_MIN, NULL, 0, MDBX_page_void, 0, 0, 0,
                 0);
  return rc;
}

int mdbx_canary_put(MDBX_txn *txn, const mdbx_canary *canary) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  if (unlikely(F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)))
    return MDBX_EACCESS;

  if (likely(canary)) {
    if (txn->mt_canary.x == canary->x && txn->mt_canary.y == canary->y &&
        txn->mt_canary.z == canary->z)
      return MDBX_SUCCESS;
    txn->mt_canary.x = canary->x;
    txn->mt_canary.y = canary->y;
    txn->mt_canary.z = canary->z;
  }
  txn->mt_canary.v = txn->mt_txnid;

  if ((txn->mt_flags & MDBX_TXN_DIRTY) == 0) {
    txn->mt_flags |= MDBX_TXN_DIRTY;
    *txn->mt_env->me_unsynced_pages += 1;
  }
  return MDBX_SUCCESS;
}

int mdbx_canary_get(MDBX_txn *txn, mdbx_canary *canary) {
  if (unlikely(txn == NULL || canary == NULL))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  *canary = txn->mt_canary;
  return MDBX_SUCCESS;
}

int mdbx_cursor_on_first(MDBX_cursor *mc) {
  if (unlikely(mc == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (!(mc->mc_flags & C_INITIALIZED))
    return MDBX_RESULT_FALSE;

  for (unsigned i = 0; i < mc->mc_snum; ++i) {
    if (mc->mc_ki[i])
      return MDBX_RESULT_FALSE;
  }

  return MDBX_RESULT_TRUE;
}

int mdbx_cursor_on_last(MDBX_cursor *mc) {
  if (unlikely(mc == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (!(mc->mc_flags & C_INITIALIZED))
    return MDBX_RESULT_FALSE;

  for (unsigned i = 0; i < mc->mc_snum; ++i) {
    unsigned nkeys = NUMKEYS(mc->mc_pg[i]);
    if (mc->mc_ki[i] < nkeys - 1)
      return MDBX_RESULT_FALSE;
  }

  return MDBX_RESULT_TRUE;
}

int mdbx_cursor_eof(MDBX_cursor *mc) {
  if (unlikely(mc == NULL))
    return MDBX_EINVAL;

  if (unlikely(mc->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if ((mc->mc_flags & C_INITIALIZED) == 0)
    return MDBX_RESULT_TRUE;

  if (mc->mc_snum == 0)
    return MDBX_RESULT_TRUE;

  if ((mc->mc_flags & C_EOF) &&
      mc->mc_ki[mc->mc_top] >= NUMKEYS(mc->mc_pg[mc->mc_top]))
    return MDBX_RESULT_TRUE;

  return MDBX_RESULT_FALSE;
}

//------------------------------------------------------------------------------

struct diff_result {
  ptrdiff_t diff;
  int level;
  int root_nkeys;
};

/* calculates: r = x - y */
__hot static int cursor_diff(const MDBX_cursor *const __restrict x,
                             const MDBX_cursor *const __restrict y,
                             struct diff_result *const __restrict r) {
  r->diff = 0;
  r->level = 0;
  r->root_nkeys = 0;

  if (unlikely(y->mc_signature != MDBX_MC_SIGNATURE ||
               x->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(y->mc_dbi != x->mc_dbi))
    return MDBX_EINVAL;

  if (unlikely(!(y->mc_flags & x->mc_flags & C_INITIALIZED)))
    return MDBX_ENODATA;

  while (likely(r->level < y->mc_snum && r->level < x->mc_snum)) {
    if (unlikely(y->mc_pg[r->level] != x->mc_pg[r->level]))
      return MDBX_PROBLEM;

    int nkeys = NUMKEYS(y->mc_pg[r->level]);
    assert(nkeys > 0);
    if (r->level == 0)
      r->root_nkeys = nkeys;

    const int limit_ki = nkeys - 1;
    const int x_ki = x->mc_ki[r->level];
    const int y_ki = y->mc_ki[r->level];
    r->diff = ((x_ki < limit_ki) ? x_ki : limit_ki) -
              ((y_ki < limit_ki) ? y_ki : limit_ki);
    if (r->diff == 0) {
      r->level += 1;
      continue;
    }

    while (unlikely(r->diff == 1) &&
           likely(r->level + 1 < y->mc_snum && r->level + 1 < x->mc_snum)) {
      r->level += 1;
      /*   DB'PAGEs: 0------------------>MAX
       *
       *    CURSORs:       y < x
       *  STACK[i ]:         |
       *  STACK[+1]:  ...y++N|0++x...
       */
      nkeys = NUMKEYS(y->mc_pg[r->level]);
      r->diff = (nkeys - y->mc_ki[r->level]) + x->mc_ki[r->level];
      assert(r->diff > 0);
    }

    while (unlikely(r->diff == -1) &&
           likely(r->level + 1 < y->mc_snum && r->level + 1 < x->mc_snum)) {
      r->level += 1;
      /*   DB'PAGEs: 0------------------>MAX
       *
       *    CURSORs:       x < y
       *  STACK[i ]:         |
       *  STACK[+1]:  ...x--N|0--y...
       */
      nkeys = NUMKEYS(x->mc_pg[r->level]);
      r->diff = -(nkeys - x->mc_ki[r->level]) - y->mc_ki[r->level];
      assert(r->diff < 0);
    }

    return MDBX_SUCCESS;
  }

  r->diff = mdbx_cmp2int(x->mc_flags & C_EOF, y->mc_flags & C_EOF);
  return MDBX_SUCCESS;
}

__hot static ptrdiff_t estimate(const MDBX_db *db,
                                struct diff_result *const __restrict dr) {
  /*        root: branch-page    => scale = leaf-factor * branch-factor^(N-1)
   *     level-1: branch-page(s) => scale = leaf-factor * branch-factor^2
   *     level-2: branch-page(s) => scale = leaf-factor * branch-factor
   *     level-N: branch-page(s) => scale = leaf-factor
   *  last-level: leaf-page(s)   => scale = 1
   */
  ptrdiff_t btree_power = db->md_depth - 2 - dr->level;
  if (btree_power < 0)
    return dr->diff;

  ptrdiff_t estimated =
      (ptrdiff_t)db->md_entries * dr->diff / (ptrdiff_t)db->md_leaf_pages;
  if (btree_power == 0)
    return estimated;

  if (db->md_depth < 4) {
    assert(dr->level == 0 && btree_power == 1);
    return (ptrdiff_t)db->md_entries * dr->diff / (ptrdiff_t)dr->root_nkeys;
  }

  /* average_branchpage_fillfactor = total(branch_entries) / branch_pages
     total(branch_entries) = leaf_pages + branch_pages - 1 (root page) */
  const size_t log2_fixedpoint = sizeof(size_t) - 1;
  const size_t half = UINT64_C(1) << (log2_fixedpoint - 1);
  const size_t factor =
      ((db->md_leaf_pages + db->md_branch_pages - 1) << log2_fixedpoint) /
      db->md_branch_pages;
  while (1) {
    switch ((size_t)btree_power) {
    default: {
      const size_t square = (factor * factor + half) >> log2_fixedpoint;
      const size_t quad = (square * square + half) >> log2_fixedpoint;
      do {
        estimated = estimated * quad + half;
        estimated >>= log2_fixedpoint;
        btree_power -= 4;
      } while (btree_power >= 4);
      continue;
    }
    case 3:
      estimated = estimated * factor + half;
      estimated >>= log2_fixedpoint;
      __fallthrough /* fall through */;
    case 2:
      estimated = estimated * factor + half;
      estimated >>= log2_fixedpoint;
      __fallthrough /* fall through */;
    case 1:
      estimated = estimated * factor + half;
      estimated >>= log2_fixedpoint;
      __fallthrough /* fall through */;
    case 0:
      if (unlikely(estimated > (ptrdiff_t)db->md_entries))
        return (ptrdiff_t)db->md_entries;
      if (unlikely(estimated < -(ptrdiff_t)db->md_entries))
        return -(ptrdiff_t)db->md_entries;
      return estimated;
    }
  }
}

__hot int mdbx_estimate_distance(const MDBX_cursor *first,
                                 const MDBX_cursor *last,
                                 ptrdiff_t *distance_items) {
  if (unlikely(first == NULL || last == NULL || distance_items == NULL))
    return MDBX_EINVAL;

  *distance_items = 0;
  struct diff_result dr;
  int rc = cursor_diff(last, first, &dr);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  if (unlikely(dr.diff == 0) &&
      F_ISSET(first->mc_db->md_flags & first->mc_db->md_flags,
              MDBX_DUPSORT | C_INITIALIZED)) {
    first = &first->mc_xcursor->mx_cursor;
    last = &last->mc_xcursor->mx_cursor;
    rc = cursor_diff(first, last, &dr);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  if (likely(dr.diff != 0))
    *distance_items = estimate(first->mc_db, &dr);

  return MDBX_SUCCESS;
}

int mdbx_estimate_move(const MDBX_cursor *cursor, MDBX_val *key, MDBX_val *data,
                       MDBX_cursor_op move_op, ptrdiff_t *distance_items) {
  if (unlikely(cursor == NULL || distance_items == NULL ||
               move_op == MDBX_GET_CURRENT || move_op == MDBX_GET_MULTIPLE))
    return MDBX_EINVAL;

  if (unlikely(cursor->mc_signature != MDBX_MC_SIGNATURE))
    return MDBX_EBADSIGN;

  if (!(cursor->mc_flags & C_INITIALIZED))
    return MDBX_ENODATA;

  MDBX_cursor_couple next;
  mdbx_cursor_copy(cursor, &next.outer);
  next.outer.mc_xcursor = NULL;
  if (cursor->mc_db->md_flags & MDBX_DUPSORT) {
    next.outer.mc_xcursor = &next.inner;
    int rc = mdbx_xcursor_init0(&next.outer);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
    MDBX_xcursor *mx = &container_of(cursor, MDBX_cursor_couple, outer)->inner;
    mdbx_cursor_copy(&mx->mx_cursor, &next.inner.mx_cursor);
  }

  MDBX_val stub = {0, 0};
  if (data == NULL) {
    const unsigned mask =
        1 << MDBX_GET_BOTH | 1 << MDBX_GET_BOTH_RANGE | 1 << MDBX_SET_KEY;
    if (unlikely(mask & (1 << move_op)))
      return MDBX_EINVAL;
    data = &stub;
  }

  if (key == NULL) {
    const unsigned mask = 1 << MDBX_GET_BOTH | 1 << MDBX_GET_BOTH_RANGE |
                          1 << MDBX_SET_KEY | 1 << MDBX_SET |
                          1 << MDBX_SET_RANGE;
    if (unlikely(mask & (1 << move_op)))
      return MDBX_EINVAL;
    key = &stub;
  }

  int rc = mdbx_cursor_get(&next.outer, key, data, move_op);
  if (unlikely(rc != MDBX_SUCCESS &&
               (rc != MDBX_NOTFOUND || !(next.outer.mc_flags & C_INITIALIZED))))
    return rc;

  return mdbx_estimate_distance(cursor, &next.outer, distance_items);
}

static int mdbx_is_samedata(const MDBX_val *a, const MDBX_val *b) {
  return a->iov_len == b->iov_len &&
         memcmp(a->iov_base, b->iov_base, a->iov_len) == 0;
}

int mdbx_estimate_range(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *begin_key,
                        MDBX_val *begin_data, MDBX_val *end_key,
                        MDBX_val *end_data, ptrdiff_t *size_items) {

  if (unlikely(!txn || !size_items))
    return MDBX_EINVAL;

  if (unlikely(begin_data && (begin_key == NULL || begin_key == MDBX_EPSILON)))
    return MDBX_EINVAL;

  if (unlikely(end_data && (end_key == NULL || end_key == MDBX_EPSILON)))
    return MDBX_EINVAL;

  if (unlikely(begin_key == MDBX_EPSILON && end_key == MDBX_EPSILON))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  MDBX_cursor_couple begin;
  /* LY: first, initialize cursor to refresh a DB in case it have DB_STALE */
  int rc = mdbx_cursor_init(&begin.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  if (unlikely(begin.outer.mc_db->md_entries == 0)) {
    *size_items = 0;
    return MDBX_SUCCESS;
  }

  if (!begin_key) {
    if (unlikely(!end_key)) {
      /* LY: FIRST..LAST case */
      *size_items = (ptrdiff_t)begin.outer.mc_db->md_entries;
      return MDBX_SUCCESS;
    }
    MDBX_val stub = {0, 0};
    rc = mdbx_cursor_first(&begin.outer, &stub, &stub);
    if (unlikely(end_key == MDBX_EPSILON)) {
      /* LY: FIRST..+epsilon case */
      return (rc == MDBX_SUCCESS)
                 ? mdbx_cursor_count(&begin.outer, (size_t *)size_items)
                 : rc;
    }
  } else {
    if (unlikely(begin_key == MDBX_EPSILON)) {
      if (end_key == NULL) {
        /* LY: -epsilon..LAST case */
        MDBX_val stub = {0, 0};
        rc = mdbx_cursor_last(&begin.outer, &stub, &stub);
        return (rc == MDBX_SUCCESS)
                   ? mdbx_cursor_count(&begin.outer, (size_t *)size_items)
                   : rc;
      }
      /* LY: -epsilon..value case */
      assert(end_key != MDBX_EPSILON);
      begin_key = end_key;
    } else if (unlikely(end_key == MDBX_EPSILON)) {
      /* LY: value..+epsilon case */
      assert(begin_key != MDBX_EPSILON);
      end_key = begin_key;
    }
    if (end_key && !begin_data && !end_data &&
        (begin_key == end_key || mdbx_is_samedata(begin_key, end_key))) {
      /* LY: single key case */
      int exact = 0;
      rc = mdbx_cursor_set(&begin.outer, begin_key, NULL, MDBX_SET, &exact);
      if (unlikely(rc != MDBX_SUCCESS)) {
        *size_items = 0;
        return (rc == MDBX_NOTFOUND) ? MDBX_SUCCESS : rc;
      }
      *size_items = 1;
      if (begin.outer.mc_xcursor != NULL) {
        MDBX_node *leaf = NODEPTR(begin.outer.mc_pg[begin.outer.mc_top],
                                  begin.outer.mc_ki[begin.outer.mc_top]);
        if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
          /* LY: return the number of duplicates for given key */
          mdbx_tassert(txn,
                       begin.outer.mc_xcursor == &begin.inner &&
                           (begin.inner.mx_cursor.mc_flags & C_INITIALIZED));
          *size_items =
              (sizeof(*size_items) >= sizeof(begin.inner.mx_db.md_entries) ||
               begin.inner.mx_db.md_entries <= PTRDIFF_MAX)
                  ? (size_t)begin.inner.mx_db.md_entries
                  : PTRDIFF_MAX;
        }
      }
      return MDBX_SUCCESS;
    } else {
      rc = mdbx_cursor_set(&begin.outer, begin_key, begin_data,
                           begin_data ? MDBX_GET_BOTH_RANGE : MDBX_SET_RANGE,
                           NULL);
    }
  }

  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc != MDBX_NOTFOUND || !(begin.outer.mc_flags & C_INITIALIZED))
      return rc;
  }

  MDBX_cursor_couple end;
  rc = mdbx_cursor_init(&end.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  if (!end_key) {
    MDBX_val stub = {0, 0};
    rc = mdbx_cursor_last(&end.outer, &stub, &stub);
  } else {
    rc = mdbx_cursor_set(&end.outer, end_key, end_data,
                         end_data ? MDBX_GET_BOTH_RANGE : MDBX_SET_RANGE, NULL);
  }
  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc != MDBX_NOTFOUND || !(end.outer.mc_flags & C_INITIALIZED))
      return rc;
  }

  rc = mdbx_estimate_distance(&begin.outer, &end.outer, size_items);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  assert(*size_items >= -(ptrdiff_t)begin.outer.mc_db->md_entries &&
         *size_items <= (ptrdiff_t)begin.outer.mc_db->md_entries);

#if 0 /* LY: Was decided to returns as-is (i.e. negative) the estimation       \
       * results for an inverted ranges. */

  /* Commit 8ddfd1f34ad7cf7a3c4aa75d2e248ca7e639ed63
     Change-Id: If59eccf7311123ab6384c4b93f9b1fed5a0a10d1 */

  if (*size_items < 0) {
    /* LY: inverted range case */
    *size_items += (ptrdiff_t)begin.outer.mc_db->md_entries;
  } else if (*size_items == 0 && begin_key && end_key) {
    int cmp = begin.outer.mc_dbx->md_cmp(&origin_begin_key, &origin_end_key);
    if (cmp == 0 && (begin.inner.mx_cursor.mc_flags & C_INITIALIZED) &&
        begin_data && end_data)
      cmp = begin.outer.mc_dbx->md_dcmp(&origin_begin_data, &origin_end_data);
    if (cmp > 0) {
      /* LY: inverted range case with empty scope */
      *size_items = (ptrdiff_t)begin.outer.mc_db->md_entries;
    }
  }
  assert(*size_items >= 0 &&
         *size_items <= (ptrdiff_t)begin.outer.mc_db->md_entries);
#endif

  return MDBX_SUCCESS;
}

//------------------------------------------------------------------------------

/* Позволяет обновить или удалить существующую запись с получением
 * в old_data предыдущего значения данных. При этом если new_data равен
 * нулю, то выполняется удаление, иначе обновление/вставка.
 *
 * Текущее значение может находиться в уже измененной (грязной) странице.
 * В этом случае страница будет перезаписана при обновлении, а само старое
 * значение утрачено. Поэтому исходно в old_data должен быть передан
 * дополнительный буфер для копирования старого значения.
 * Если переданный буфер слишком мал, то функция вернет -1, установив
 * old_data->iov_len в соответствующее значение.
 *
 * Для не-уникальных ключей также возможен второй сценарий использования,
 * когда посредством old_data из записей с одинаковым ключом для
 * удаления/обновления выбирается конкретная. Для выбора этого сценария
 * во flags следует одновременно указать MDBX_CURRENT и MDBX_NOOVERWRITE.
 * Именно эта комбинация выбрана, так как она лишена смысла, и этим позволяет
 * идентифицировать запрос такого сценария.
 *
 * Функция может быть замещена соответствующими операциями с курсорами
 * после двух доработок (TODO):
 *  - внешняя аллокация курсоров, в том числе на стеке (без malloc).
 *  - получения статуса страницы по адресу (знать о P_DIRTY).
 */
int mdbx_replace(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *new_data,
                 MDBX_val *old_data, unsigned flags) {
  if (unlikely(!key || !old_data || !txn || old_data == new_data))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(old_data->iov_base == NULL && old_data->iov_len))
    return MDBX_EINVAL;

  if (unlikely(new_data == NULL && !(flags & MDBX_CURRENT)))
    return MDBX_EINVAL;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(flags & ~(MDBX_NOOVERWRITE | MDBX_NODUPDATA | MDBX_RESERVE |
                         MDBX_APPEND | MDBX_APPENDDUP | MDBX_CURRENT)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS : MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  cx.outer.mc_next = txn->mt_cursors[dbi];
  txn->mt_cursors[dbi] = &cx.outer;

  MDBX_val present_key = *key;
  if (F_ISSET(flags, MDBX_CURRENT | MDBX_NOOVERWRITE)) {
    /* в old_data значение для выбора конкретного дубликата */
    if (unlikely(!(txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT))) {
      rc = MDBX_EINVAL;
      goto bailout;
    }

    /* убираем лишний бит, он был признаком запрошенного режима */
    flags -= MDBX_NOOVERWRITE;

    rc = mdbx_cursor_get(&cx.outer, &present_key, old_data, MDBX_GET_BOTH);
    if (rc != MDBX_SUCCESS)
      goto bailout;

    if (new_data) {
      /* обновление конкретного дубликата */
      if (mdbx_is_samedata(old_data, new_data))
        /* если данные совпадают, то ничего делать не надо */
        goto bailout;
    }
  } else {
    /* в old_data буфер для сохранения предыдущего значения */
    if (unlikely(new_data && old_data->iov_base == new_data->iov_base))
      return MDBX_EINVAL;
    MDBX_val present_data;
    rc = mdbx_cursor_get(&cx.outer, &present_key, &present_data, MDBX_SET_KEY);
    if (unlikely(rc != MDBX_SUCCESS)) {
      old_data->iov_base = NULL;
      old_data->iov_len = 0;
      if (rc != MDBX_NOTFOUND || (flags & MDBX_CURRENT))
        goto bailout;
    } else if (flags & MDBX_NOOVERWRITE) {
      rc = MDBX_KEYEXIST;
      *old_data = present_data;
      goto bailout;
    } else {
      MDBX_page *page = cx.outer.mc_pg[cx.outer.mc_top];
      if (txn->mt_dbs[dbi].md_flags & MDBX_DUPSORT) {
        if (flags & MDBX_CURRENT) {
          /* для не-уникальных ключей позволяем update/delete только если ключ
           * один */
          MDBX_node *leaf = NODEPTR(page, cx.outer.mc_ki[cx.outer.mc_top]);
          if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
            mdbx_tassert(txn, XCURSOR_INITED(&cx.outer) &&
                                  cx.outer.mc_xcursor->mx_db.md_entries > 1);
            if (cx.outer.mc_xcursor->mx_db.md_entries > 1) {
              rc = MDBX_EMULTIVAL;
              goto bailout;
            }
          }
          /* если данные совпадают, то ничего делать не надо */
          if (new_data && mdbx_is_samedata(&present_data, new_data)) {
            *old_data = *new_data;
            goto bailout;
          }
          /* В оригинальной LMDB фладок MDBX_CURRENT здесь приведет
           * к замене данных без учета MDBX_DUPSORT сортировки,
           * но здесь это в любом случае допустимо, так как мы
           * проверили что для ключа есть только одно значение. */
        } else if ((flags & MDBX_NODUPDATA) &&
                   mdbx_is_samedata(&present_data, new_data)) {
          /* если данные совпадают и установлен MDBX_NODUPDATA */
          rc = MDBX_KEYEXIST;
          goto bailout;
        }
      } else {
        /* если данные совпадают, то ничего делать не надо */
        if (new_data && mdbx_is_samedata(&present_data, new_data)) {
          *old_data = *new_data;
          goto bailout;
        }
        flags |= MDBX_CURRENT;
      }

      if (IS_DIRTY(page)) {
        if (unlikely(old_data->iov_len < present_data.iov_len)) {
          old_data->iov_base = NULL;
          old_data->iov_len = present_data.iov_len;
          rc = MDBX_RESULT_TRUE;
          goto bailout;
        }
        memcpy(old_data->iov_base, present_data.iov_base, present_data.iov_len);
        old_data->iov_len = present_data.iov_len;
      } else {
        *old_data = present_data;
      }
    }
  }

  if (likely(new_data))
    rc = mdbx_cursor_put(&cx.outer, key, new_data, flags);
  else
    rc = mdbx_cursor_del(&cx.outer, 0);

bailout:
  txn->mt_cursors[dbi] = cx.outer.mc_next;
  return rc;
}

int mdbx_get_ex(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
                size_t *values_count) {
  DKBUF;
  mdbx_debug("===> get db %u key [%s]", dbi, DKEY(key));

  if (unlikely(!key || !data || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
    return MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  int exact = 0;
  rc = mdbx_cursor_set(&cx.outer, key, data, MDBX_SET_KEY, &exact);
  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc == MDBX_NOTFOUND && values_count)
      *values_count = 0;
    return rc;
  }

  if (values_count) {
    *values_count = 1;
    if (cx.outer.mc_xcursor != NULL) {
      MDBX_node *leaf = NODEPTR(cx.outer.mc_pg[cx.outer.mc_top],
                                cx.outer.mc_ki[cx.outer.mc_top]);
      if (F_ISSET(leaf->mn_flags, F_DUPDATA)) {
        mdbx_tassert(txn, cx.outer.mc_xcursor == &cx.inner &&
                              (cx.inner.mx_cursor.mc_flags & C_INITIALIZED));
        *values_count =
            (sizeof(*values_count) >= sizeof(cx.inner.mx_db.md_entries) ||
             cx.inner.mx_db.md_entries <= PTRDIFF_MAX)
                ? (size_t)cx.inner.mx_db.md_entries
                : PTRDIFF_MAX;
      }
    }
  }
  return MDBX_SUCCESS;
}

/* Функция сообщает находится ли указанный адрес в "грязной" странице у
 * заданной пишущей транзакции. В конечном счете это позволяет избавиться от
 * лишнего копирования данных из НЕ-грязных страниц.
 *
 * "Грязные" страницы - это те, которые уже были изменены в ходе пишущей
 * транзакции. Соответственно, какие-либо дальнейшие изменения могут привести
 * к перезаписи таких страниц. Поэтому все функции, выполняющие изменения, в
 * качестве аргументов НЕ должны получать указатели на данные в таких
 * страницах. В свою очередь "НЕ грязные" страницы перед модификацией будут
 * скопированы.
 *
 * Другими словами, данные из "грязных" страниц должны быть либо скопированы
 * перед передачей в качестве аргументов для дальнейших модификаций, либо
 * отвергнуты на стадии проверки корректности аргументов.
 *
 * Таким образом, функция позволяет как избавится от лишнего копирования,
 * так и выполнить более полную проверку аргументов.
 *
 * ВАЖНО: Передаваемый указатель должен указывать на начало данных. Только
 * так гарантируется что актуальный заголовок страницы будет физически
 * расположен в той-же странице памяти, в том числе для многостраничных
 * P_OVERFLOW страниц с длинными данными. */
int mdbx_is_dirty(const MDBX_txn *txn, const void *ptr) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(txn->mt_flags & MDBX_TXN_RDONLY))
    return MDBX_RESULT_FALSE;

  const MDBX_env *env = txn->mt_env;
  const uintptr_t mask = ~(uintptr_t)(env->me_psize - 1);
  const MDBX_page *page = (const MDBX_page *)((uintptr_t)ptr & mask);

  /* LY: Тут не всё хорошо с абсолютной достоверностью результата,
   * так как флажок P_DIRTY в LMDB может означать не совсем то,
   * что было исходно задумано, детали см в логике кода mdbx_page_touch().
   *
   * Более того, в режиме БЕЗ WRITEMAP грязные страницы выделяются через
   * malloc(), т.е. находятся вне mmap-диапазона и тогда чтобы отличить
   * действительно грязную страницу от указателя на данные пользователя
   * следует сканировать dirtylist, что накладно.
   *
   * Тем не менее, однозначно страница "не грязная" (не будет переписана
   * во время транзакции) если адрес находится внутри mmap-диапазона
   * и в заголовке страницы нет флажка P_DIRTY. */
  if (env->me_map < (uint8_t *)page) {
    const size_t usedbytes = pgno2bytes(env, txn->mt_next_pgno);
    if ((uint8_t *)page < env->me_map + usedbytes) {
      /* страница внутри диапазона, смотрим на флажки */
      return (page->mp_flags & (P_DIRTY | P_LOOSE | P_KEEP))
                 ? MDBX_RESULT_TRUE
                 : MDBX_RESULT_FALSE;
    }
    /* Гипотетически здесь возможна ситуация, когда указатель адресует что-то
     * в пределах mmap, но за границей распределенных страниц. Это тяжелая
     * ошибка, к которой не возможно прийти без каких-то больших нарушений.
     * Поэтому не проверяем этот случай кроме как assert-ом, на то что
     * страница вне mmap-диаппазона. */
    mdbx_tassert(txn, (uint8_t *)page >= env->me_map + env->me_mapsize);
  }

  /* Страница вне используемого mmap-диапазона, т.е. либо в функцию был
   * передан некорректный адрес, либо адрес в теневой странице, которая была
   * выделена посредством malloc().
   *
   * Для WRITE_MAP режима такая страница однозначно "не грязная",
   * а для режимов без WRITE_MAP следует просматривать списки dirty
   * и spilled страниц у каких-либо транзакций (в том числе дочерних).
   *
   * Поэтому для WRITE_MAP возвращаем false, а для остальных режимов
   * всегда true. Такая логика имеет ряд преимуществ:
   *  - не тратим время на просмотр списков;
   *  - результат всегда безопасен (может быть ложно-положительным,
   *    но не ложно-отрицательным);
   *  - результат не зависит от вложенности транзакций и от относительного
   *    положения переданной транзакции в этой рекурсии. */
  return (env->me_flags & MDBX_WRITEMAP) ? MDBX_RESULT_FALSE : MDBX_RESULT_TRUE;
}

int mdbx_dbi_sequence(MDBX_txn *txn, MDBX_dbi dbi, uint64_t *result,
                      uint64_t increment) {
  if (unlikely(!txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_EBADSIGN;

  if (unlikely(txn->mt_owner != mdbx_thread_self()))
    return MDBX_THREAD_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(TXN_DBI_CHANGED(txn, dbi)))
    return MDBX_BAD_DBI;

  if (unlikely(txn->mt_dbflags[dbi] & DB_STALE)) {
    MDBX_cursor_couple cx;
    /* Stale, must read the DB's root. cursor_init does it for us. */
    int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
    if (unlikely(rc != MDBX_SUCCESS))
      return rc;
  }

  MDBX_db *dbs = &txn->mt_dbs[dbi];
  if (likely(result))
    *result = dbs->md_seq;

  if (likely(increment > 0)) {
    if (unlikely(txn->mt_flags & MDBX_TXN_BLOCKED))
      return MDBX_BAD_TXN;

    if (unlikely(F_ISSET(txn->mt_flags, MDBX_TXN_RDONLY)))
      return MDBX_EACCESS;

    uint64_t new = dbs->md_seq + increment;
    if (unlikely(new < increment))
      return MDBX_RESULT_TRUE;

    mdbx_tassert(txn, new > dbs->md_seq);
    dbs->md_seq = new;
    txn->mt_flags |= MDBX_TXN_DIRTY;
    txn->mt_dbflags[dbi] |= DB_DIRTY;
  }

  return MDBX_SUCCESS;
}

/*----------------------------------------------------------------------------*/

__cold intptr_t mdbx_limits_keysize_max(intptr_t pagesize) {
  if (pagesize < 1)
    pagesize = (intptr_t)mdbx_syspagesize();
  else if (unlikely(pagesize < (intptr_t)MIN_PAGESIZE ||
                    pagesize > (intptr_t)MAX_PAGESIZE ||
                    !mdbx_is_power2((size_t)pagesize)))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  return mdbx_maxkey(mdbx_nodemax(pagesize));
}

__cold int mdbx_limits_pgsize_min(void) { return MIN_PAGESIZE; }

__cold int mdbx_limits_pgsize_max(void) { return MAX_PAGESIZE; }

__cold intptr_t mdbx_limits_dbsize_min(intptr_t pagesize) {
  if (pagesize < 1)
    pagesize = (intptr_t)mdbx_syspagesize();
  else if (unlikely(pagesize < (intptr_t)MIN_PAGESIZE ||
                    pagesize > (intptr_t)MAX_PAGESIZE ||
                    !mdbx_is_power2((size_t)pagesize)))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  return MIN_PAGENO * pagesize;
}

__cold intptr_t mdbx_limits_dbsize_max(intptr_t pagesize) {
  if (pagesize < 1)
    pagesize = (intptr_t)mdbx_syspagesize();
  else if (unlikely(pagesize < (intptr_t)MIN_PAGESIZE ||
                    pagesize > (intptr_t)MAX_PAGESIZE ||
                    !mdbx_is_power2((size_t)pagesize)))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  const uint64_t limit = MAX_PAGENO * (uint64_t)pagesize;
  return (limit < (intptr_t)MAX_MAPSIZE) ? (intptr_t)limit
                                         : (intptr_t)MAX_MAPSIZE;
}

__cold intptr_t mdbx_limits_txnsize_max(intptr_t pagesize) {
  if (pagesize < 1)
    pagesize = (intptr_t)mdbx_syspagesize();
  else if (unlikely(pagesize < (intptr_t)MIN_PAGESIZE ||
                    pagesize > (intptr_t)MAX_PAGESIZE ||
                    !mdbx_is_power2((size_t)pagesize)))
    return (MDBX_EINVAL > 0) ? -MDBX_EINVAL : MDBX_EINVAL;

  return pagesize * (MDBX_DPL_TXNFULL - 1);
}

/*----------------------------------------------------------------------------*/
/* attribute support functions for Nexenta */

static __inline int mdbx_attr_peek(MDBX_val *data, mdbx_attr_t *attrptr) {
  if (unlikely(data->iov_len < sizeof(mdbx_attr_t)))
    return MDBX_INCOMPATIBLE;

  if (likely(attrptr != NULL))
    *attrptr = *(mdbx_attr_t *)data->iov_base;
  data->iov_len -= sizeof(mdbx_attr_t);
  data->iov_base =
      likely(data->iov_len > 0) ? ((mdbx_attr_t *)data->iov_base) + 1 : NULL;

  return MDBX_SUCCESS;
}

static __inline int mdbx_attr_poke(MDBX_val *reserved, MDBX_val *data,
                                   mdbx_attr_t attr, unsigned flags) {
  mdbx_attr_t *space = reserved->iov_base;
  if (flags & MDBX_RESERVE) {
    if (likely(data != NULL)) {
      data->iov_base = data->iov_len ? space + 1 : NULL;
    }
  } else {
    *space = attr;
    if (likely(data != NULL)) {
      memcpy(space + 1, data->iov_base, data->iov_len);
    }
  }

  return MDBX_SUCCESS;
}

int mdbx_cursor_get_attr(MDBX_cursor *mc, MDBX_val *key, MDBX_val *data,
                         mdbx_attr_t *attrptr, MDBX_cursor_op op) {
  int rc = mdbx_cursor_get(mc, key, data, op);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  return mdbx_attr_peek(data, attrptr);
}

int mdbx_get_attr(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
                  uint64_t *attrptr) {
  int rc = mdbx_get(txn, dbi, key, data);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  return mdbx_attr_peek(data, attrptr);
}

int mdbx_put_attr(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
                  mdbx_attr_t attr, unsigned flags) {
  MDBX_val reserve;
  reserve.iov_base = NULL;
  reserve.iov_len = (data ? data->iov_len : 0) + sizeof(mdbx_attr_t);

  int rc = mdbx_put(txn, dbi, key, &reserve, flags | MDBX_RESERVE);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  return mdbx_attr_poke(&reserve, data, attr, flags);
}

int mdbx_cursor_put_attr(MDBX_cursor *cursor, MDBX_val *key, MDBX_val *data,
                         mdbx_attr_t attr, unsigned flags) {
  MDBX_val reserve;
  reserve.iov_base = NULL;
  reserve.iov_len = (data ? data->iov_len : 0) + sizeof(mdbx_attr_t);

  int rc = mdbx_cursor_put(cursor, key, &reserve, flags | MDBX_RESERVE);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  return mdbx_attr_poke(&reserve, data, attr, flags);
}

int mdbx_set_attr(MDBX_txn *txn, MDBX_dbi dbi, MDBX_val *key, MDBX_val *data,
                  mdbx_attr_t attr) {
  if (unlikely(!key || !txn))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_signature != MDBX_MT_SIGNATURE))
    return MDBX_VERSION_MISMATCH;

  if (unlikely(!TXN_DBI_EXIST(txn, dbi, DB_USRVALID)))
    return MDBX_EINVAL;

  if (unlikely(txn->mt_flags & (MDBX_TXN_RDONLY | MDBX_TXN_BLOCKED)))
    return (txn->mt_flags & MDBX_TXN_RDONLY) ? MDBX_EACCESS : MDBX_BAD_TXN;

  MDBX_cursor_couple cx;
  MDBX_val old_data;
  int rc = mdbx_cursor_init(&cx.outer, txn, dbi);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;
  rc = mdbx_cursor_set(&cx.outer, key, &old_data, MDBX_SET, NULL);
  if (unlikely(rc != MDBX_SUCCESS)) {
    if (rc == MDBX_NOTFOUND && data) {
      cx.outer.mc_next = txn->mt_cursors[dbi];
      txn->mt_cursors[dbi] = &cx.outer;
      rc = mdbx_cursor_put_attr(&cx.outer, key, data, attr, 0);
      txn->mt_cursors[dbi] = cx.outer.mc_next;
    }
    return rc;
  }

  mdbx_attr_t old_attr = 0;
  rc = mdbx_attr_peek(&old_data, &old_attr);
  if (unlikely(rc != MDBX_SUCCESS))
    return rc;

  if (old_attr == attr && (!data || (data->iov_len == old_data.iov_len &&
                                     memcmp(data->iov_base, old_data.iov_base,
                                            old_data.iov_len) == 0)))
    return MDBX_SUCCESS;

  cx.outer.mc_next = txn->mt_cursors[dbi];
  txn->mt_cursors[dbi] = &cx.outer;
  rc = mdbx_cursor_put_attr(&cx.outer, key, data ? data : &old_data, attr,
                            MDBX_CURRENT);
  txn->mt_cursors[dbi] = cx.outer.mc_next;
  return rc;
}

//----------------------------------------------------------------------------

#ifdef __SANITIZE_ADDRESS__
LIBMDBX_API __attribute__((__weak__)) const char *__asan_default_options() {
  return "symbolize=1:allow_addr2line=1:"
#ifdef _DEBUG
         "debug=1:"
#endif /* _DEBUG */
         "report_globals=1:"
         "replace_str=1:replace_intrin=1:"
         "malloc_context_size=9:"
         "detect_leaks=1:"
         "check_printf=1:"
         "detect_deadlocks=1:"
#ifndef LTO_ENABLED
         "check_initialization_order=1:"
#endif
         "detect_stack_use_after_return=1:"
         "intercept_tls_get_addr=1:"
         "decorate_proc_maps=1:"
         "abort_on_error=1";
}
#endif /* __SANITIZE_ADDRESS__ */
