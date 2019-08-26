﻿/*
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
 * <http://www.OpenLDAP.org/license.html>. */

#pragma once
/* *INDENT-OFF* */
/* clang-format off */

/* In case the MDBX_DEBUG is undefined set it corresponding to NDEBUG */
#ifndef MDBX_DEBUG
#ifdef NDEBUG
#   define MDBX_DEBUG 0
#else
#   define MDBX_DEBUG 1
#endif
#endif

/* Undefine the NDEBUG if debugging is enforced by MDBX_DEBUG */
#if MDBX_DEBUG
#   undef NDEBUG
#endif

#define MDBX_OSX_WANNA_DURABILITY 0 /* using fcntl(F_FULLFSYNC) with 5-10 times slowdown */
#define MDBX_OSX_WANNA_SPEED 1      /* using fsync() with chance of data lost on power failure */
#ifndef MDBX_OSX_SPEED_INSTEADOF_DURABILITY
#define MDBX_OSX_SPEED_INSTEADOF_DURABILITY MDBX_OSX_WANNA_DURABILITY
#endif

/*----------------------------------------------------------------------------*/

/* Should be defined before any includes */
#ifndef _FILE_OFFSET_BITS
#   define _FILE_OFFSET_BITS 64
#endif

#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#ifdef _MSC_VER
#   if _MSC_VER < 1400
#       error "Microsoft Visual C++ 8.0 (Visual Studio 2005) or later version is required"
#   endif
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS
#   endif
#if _MSC_VER > 1800
#   pragma warning(disable : 4464) /* relative include path contains '..' */
#endif
#if _MSC_VER > 1913
#   pragma warning(disable : 5045) /* Compiler will insert Spectre mitigation... */
#endif
#pragma warning(disable : 4710) /* 'xyz': function not inlined */
#pragma warning(disable : 4711) /* function 'xyz' selected for automatic inline expansion */
#pragma warning(disable : 4201) /* nonstandard extension used : nameless struct / union */
#pragma warning(disable : 4702) /* unreachable code */
#pragma warning(disable : 4706) /* assignment within conditional expression */
#pragma warning(disable : 4127) /* conditional expression is constant */
#pragma warning(disable : 4324) /* 'xyz': structure was padded due to alignment specifier */
#pragma warning(disable : 4310) /* cast truncates constant value */
#pragma warning(disable : 4820) /* bytes padding added after data member for aligment */
#pragma warning(disable : 4548) /* expression before comma has no effect; expected expression with side - effect */
#pragma warning(disable : 4366) /* the result of the unary '&' operator may be unaligned */
#endif                          /* _MSC_VER (warnings) */

#include "../mdbx.h"
#include "./defs.h"

#if defined(__GNUC__) && !__GNUC_PREREQ(4,2)
    /* Actualy libmdbx was not tested with compilers older than GCC from RHEL6.
     * But you could remove this #error and try to continue at your own risk.
     * In such case please don't rise up an issues related ONLY to old compilers.
     */
#   warning "libmdbx required GCC >= 4.2"
#endif

#if defined(__clang__) && !__CLANG_PREREQ(3,8)
    /* Actualy libmdbx was not tested with CLANG older than 3.8.
     * But you could remove this #error and try to continue at your own risk.
     * In such case please don't rise up an issues related ONLY to old compilers.
     */
#   warning "libmdbx required CLANG >= 3.8"
#endif

#if defined(__GLIBC__) && !__GLIBC_PREREQ(2,12)
    /* Actualy libmdbx was not tested with something older than glibc 2.12 (from RHEL6).
     * But you could remove this #error and try to continue at your own risk.
     * In such case please don't rise up an issues related ONLY to old systems.
     */
#   warning "libmdbx required at least GLIBC 2.12."
#endif

#ifdef __SANITIZE_THREAD__
#   warning "libmdbx don't compatible with ThreadSanitizer, you will get a lot of false-positive issues."
#endif /* __SANITIZE_THREAD__ */

#if __has_warning("-Wconstant-logical-operand")
#   if defined(__clang__)
#       pragma clang diagnostic ignored "-Wconstant-logical-operand"
#   elif defined(__GNUC__)
#       pragma GCC diagnostic ignored "-Wconstant-logical-operand"
#   else
#      pragma warning disable "constant-logical-operand"
#   endif
#endif /* -Wconstant-logical-operand */

#if defined(__LCC__) && (__LCC__ <= 121)
    /* bug #2798 */
#   pragma diag_suppress alignment_reduction_ignored
#elif defined(__ICC)
#   pragma warning(disable: 3453 1366)
#elif __has_warning("-Walignment-reduction-ignored")
#   if defined(__clang__)
#       pragma clang diagnostic ignored "-Walignment-reduction-ignored"
#   elif defined(__GNUC__)
#       pragma GCC diagnostic ignored "-Walignment-reduction-ignored"
#   else
#       pragma warning disable "alignment-reduction-ignored"
#   endif
#endif /* -Walignment-reduction-ignored */

#include "./osal.h"

/* *INDENT-ON* */
/* clang-format on */

#if UINTPTR_MAX > 0xffffFFFFul || ULONG_MAX > 0xffffFFFFul
#define MDBX_WORDBITS 64
#else
#define MDBX_WORDBITS 32
#endif /* MDBX_WORDBITS */

/*----------------------------------------------------------------------------*/
/* Basic constants and types */

/* The minimum number of keys required in a database page.
 * Setting this to a larger value will place a smaller bound on the
 * maximum size of a data item. Data items larger than this size will
 * be pushed into overflow pages instead of being stored directly in
 * the B-tree node. This value used to default to 4. With a page size
 * of 4096 bytes that meant that any item larger than 1024 bytes would
 * go into an overflow page. That also meant that on average 2-3KB of
 * each overflow page was wasted space. The value cannot be lower than
 * 2 because then there would no longer be a tree structure. With this
 * value, items larger than 2KB will go into overflow pages, and on
 * average only 1KB will be wasted. */
#define MDBX_MINKEYS 2

/* A stamp that identifies a file as an MDBX file.
 * There's nothing special about this value other than that it is easily
 * recognizable, and it will reflect any byte order mismatches. */
#define MDBX_MAGIC UINT64_C(/* 56-bit prime */ 0x59659DBDEF4C11)

/* The version number for a database's datafile format. */
#define MDBX_DATA_VERSION 2
/* The version number for a database's lockfile format. */
#define MDBX_LOCK_VERSION 3

/* handle for the DB used to track free pages. */
#define FREE_DBI 0
/* handle for the default DB. */
#define MAIN_DBI 1
/* Number of DBs in metapage (free and main) - also hardcoded elsewhere */
#define CORE_DBS 2
#define MAX_DBI (INT16_MAX - CORE_DBS)

/* Number of meta pages - also hardcoded elsewhere */
#define NUM_METAS 3

/* A page number in the database.
 *
 * MDBX uses 32 bit for page numbers. This limits database
 * size up to 2^44 bytes, in case of 4K pages. */
typedef uint32_t pgno_t;
#define PRIaPGNO PRIu32
#define MAX_PAGENO UINT32_C(0x7FFFffff)
#define MIN_PAGENO NUM_METAS

/* A transaction ID. */
typedef uint64_t txnid_t;
#define PRIaTXN PRIi64
#if MDBX_DEBUG
#define MIN_TXNID UINT64_C(0x100000000)
#else
#define MIN_TXNID UINT64_C(1)
#endif /* MIN_TXNID */

/* Used for offsets within a single page.
 * Since memory pages are typically 4 or 8KB in size, 12-13 bits,
 * this is plenty. */
typedef uint16_t indx_t;

#define MEGABYTE ((size_t)1 << 20)

/*----------------------------------------------------------------------------*/
/* Core structures for database and shared memory (i.e. format definition) */
#pragma pack(push, 1)

/* Information about a single database in the environment. */
typedef struct MDBX_db {
  uint16_t md_flags;        /* see mdbx_dbi_open */
  uint16_t md_depth;        /* depth of this tree */
  uint32_t md_xsize;        /* also ksize for LEAF2 pages */
  pgno_t md_root;           /* the root page of this tree */
  pgno_t md_branch_pages;   /* number of internal pages */
  pgno_t md_leaf_pages;     /* number of leaf pages */
  pgno_t md_overflow_pages; /* number of overflow pages */
  uint64_t md_seq;          /* table sequence counter */
  uint64_t md_entries;      /* number of data items */
  uint64_t md_merkle;       /* Merkle tree checksum */
} MDBX_db;

/* Meta page content.
 * A meta page is the start point for accessing a database snapshot.
 * Pages 0-1 are meta pages. Transaction N writes meta page (N % 2). */
typedef struct MDBX_meta {
  /* Stamp identifying this as an MDBX file.
   * It must be set to MDBX_MAGIC with MDBX_DATA_VERSION. */
  uint64_t mm_magic_and_version;

  /* txnid that committed this page, the first of a two-phase-update pair */
  volatile txnid_t mm_txnid_a;

  uint16_t mm_extra_flags;  /* extra DB flags, zero (nothing) for now */
  uint8_t mm_validator_id;  /* ID of checksum and page validation method,
                             * zero (nothing) for now */
  uint8_t mm_extra_pagehdr; /* extra bytes in the page header,
                             * zero (nothing) for now */

  struct {
    uint16_t grow;   /* datafile growth step in pages */
    uint16_t shrink; /* datafile shrink threshold in pages */
    pgno_t lower;    /* minimal size of datafile in pages */
    pgno_t upper;    /* maximal size of datafile in pages */
    pgno_t now;      /* current size of datafile in pages */
    pgno_t next;     /* first unused page in the datafile,
                      * but actually the file may be shorter. */
  } mm_geo;

  MDBX_db mm_dbs[CORE_DBS]; /* first is free space, 2nd is main db */
                            /* The size of pages used in this DB */
#define mm_psize mm_dbs[FREE_DBI].md_xsize
/* Any persistent environment flags, see mdbx_env */
#define mm_flags mm_dbs[FREE_DBI].md_flags
  mdbx_canary mm_canary;

#define MDBX_DATASIGN_NONE 0u
#define MDBX_DATASIGN_WEAK 1u
#define SIGN_IS_WEAK(sign) ((sign) == MDBX_DATASIGN_WEAK)
#define SIGN_IS_STEADY(sign) ((sign) > MDBX_DATASIGN_WEAK)
#define META_IS_WEAK(meta) SIGN_IS_WEAK((meta)->mm_datasync_sign)
#define META_IS_STEADY(meta) SIGN_IS_STEADY((meta)->mm_datasync_sign)
  volatile uint64_t mm_datasync_sign;

  /* txnid that committed this page, the second of a two-phase-update pair */
  volatile txnid_t mm_txnid_b;

  /* Number of non-meta pages which were put in GC after COW. May be 0 in case
   * DB was previously handled by libmdbx without corresponding feature.
   * This value in couple with mr_snapshot_pages_retired allows fast estimation
   * of "how much reader is restraining GC recycling". */
  uint64_t mm_pages_retired;
} MDBX_meta;

/* Common header for all page types. The page type depends on mp_flags.
 *
 * P_BRANCH and P_LEAF pages have unsorted 'MDBX_node's at the end, with
 * sorted mp_ptrs[] entries referring to them. Exception: P_LEAF2 pages
 * omit mp_ptrs and pack sorted MDBX_DUPFIXED values after the page header.
 *
 * P_OVERFLOW records occupy one or more contiguous pages where only the
 * first has a page header. They hold the real data of F_BIGDATA nodes.
 *
 * P_SUBP sub-pages are small leaf "pages" with duplicate data.
 * A node with flag F_DUPDATA but not F_SUBDATA contains a sub-page.
 * (Duplicate data can also go in sub-databases, which use normal pages.)
 *
 * P_META pages contain MDBX_meta, the start point of an MDBX snapshot.
 *
 * Each non-metapage up to MDBX_meta.mm_last_pg is reachable exactly once
 * in the snapshot: Either used by a database or listed in a freeDB record. */
typedef struct MDBX_page {
  union {
    struct MDBX_page *mp_next; /* for in-memory list of freed pages,
                                * must be first field, see NEXT_LOOSE_PAGE */
    uint64_t mp_validator;     /* checksum of page content or a txnid during
                                * which the page has been updated */
  };
  uint16_t mp_leaf2_ksize; /* key size if this is a LEAF2 page */
#define P_BRANCH 0x01      /* branch page */
#define P_LEAF 0x02        /* leaf page */
#define P_OVERFLOW 0x04    /* overflow page */
#define P_META 0x08        /* meta page */
#define P_DIRTY 0x10       /* dirty page, also set for P_SUBP pages */
#define P_LEAF2 0x20       /* for MDBX_DUPFIXED records */
#define P_SUBP 0x40        /* for MDBX_DUPSORT sub-pages */
#define P_LOOSE 0x4000     /* page was dirtied then freed, can be reused */
#define P_KEEP 0x8000      /* leave this page alone during spill */
  uint16_t mp_flags;
  union {
    struct {
      indx_t mp_lower; /* lower bound of free space */
      indx_t mp_upper; /* upper bound of free space */
    };
    uint32_t mp_pages; /* number of overflow pages */
  };
  pgno_t mp_pgno; /* page number */

  /* dynamic size */
  union {
    indx_t mp_ptrs[1];
    MDBX_meta mp_meta;
    uint8_t mp_data[1];
  };
} MDBX_page;

/* Size of the page header, excluding dynamic data at the end */
#define PAGEHDRSZ ((unsigned)offsetof(MDBX_page, mp_data))

/* The maximum size of a database page.
 *
 * It is 64K, but value-PAGEHDRSZ must fit in MDBX_page.mp_upper.
 *
 * MDBX will use database pages < OS pages if needed.
 * That causes more I/O in write transactions: The OS must
 * know (read) the whole page before writing a partial page.
 *
 * Note that we don't currently support Huge pages. On Linux,
 * regular data files cannot use Huge pages, and in general
 * Huge pages aren't actually pageable. We rely on the OS
 * demand-pager to read our data and page it out when memory
 * pressure from other processes is high. So until OSs have
 * actual paging support for Huge pages, they're not viable. */
#define MAX_PAGESIZE 0x10000u
#define MIN_PAGESIZE 512u

#define MIN_MAPSIZE (MIN_PAGESIZE * MIN_PAGENO)
#if defined(_WIN32) || defined(_WIN64)
#define MAX_MAPSIZE32 UINT32_C(0x38000000)
#else
#define MAX_MAPSIZE32 UINT32_C(0x7ff80000)
#endif
#define MAX_MAPSIZE64 (MAX_PAGENO * (uint64_t)MAX_PAGESIZE)

#if MDBX_WORDBITS >= 64
#define MAX_MAPSIZE MAX_MAPSIZE64
#else
#define MAX_MAPSIZE MAX_MAPSIZE32
#endif /* MDBX_WORDBITS */

#pragma pack(pop)

/* Reader Lock Table
 *
 * Readers don't acquire any locks for their data access. Instead, they
 * simply record their transaction ID in the reader table. The reader
 * mutex is needed just to find an empty slot in the reader table. The
 * slot's address is saved in thread-specific data so that subsequent
 * read transactions started by the same thread need no further locking to
 * proceed.
 *
 * If MDBX_NOTLS is set, the slot address is not saved in thread-specific data.
 * No reader table is used if the database is on a read-only filesystem.
 *
 * Since the database uses multi-version concurrency control, readers don't
 * actually need any locking. This table is used to keep track of which
 * readers are using data from which old transactions, so that we'll know
 * when a particular old transaction is no longer in use. Old transactions
 * that have discarded any data pages can then have those pages reclaimed
 * for use by a later write transaction.
 *
 * The lock table is constructed such that reader slots are aligned with the
 * processor's cache line size. Any slot is only ever used by one thread.
 * This alignment guarantees that there will be no contention or cache
 * thrashing as threads update their own slot info, and also eliminates
 * any need for locking when accessing a slot.
 *
 * A writer thread will scan every slot in the table to determine the oldest
 * outstanding reader transaction. Any freed pages older than this will be
 * reclaimed by the writer. The writer doesn't use any locks when scanning
 * this table. This means that there's no guarantee that the writer will
 * see the most up-to-date reader info, but that's not required for correct
 * operation - all we need is to know the upper bound on the oldest reader,
 * we don't care at all about the newest reader. So the only consequence of
 * reading stale information here is that old pages might hang around a
 * while longer before being reclaimed. That's actually good anyway, because
 * the longer we delay reclaiming old pages, the more likely it is that a
 * string of contiguous pages can be found after coalescing old pages from
 * many old transactions together. */

/* The actual reader record, with cacheline padding. */
typedef struct MDBX_reader {
  /* Current Transaction ID when this transaction began, or (txnid_t)-1.
   * Multiple readers that start at the same time will probably have the
   * same ID here. Again, it's not important to exclude them from
   * anything; all we need to know is which version of the DB they
   * started from so we can avoid overwriting any data used in that
   * particular version. */
  volatile txnid_t mr_txnid;

  /* The information we store in a single slot of the reader table.
   * In addition to a transaction ID, we also record the process and
   * thread ID that owns a slot, so that we can detect stale information,
   * e.g. threads or processes that went away without cleaning up.
   *
   * NOTE: We currently don't check for stale records.
   * We simply re-init the table when we know that we're the only process
   * opening the lock file. */

  /* The thread ID of the thread owning this txn. */
  union {
    volatile mdbx_tid_t mr_tid;
    volatile uint64_t mr_tid_u64;
  };
  /* The process ID of the process owning this reader txn. */
  union {
    volatile mdbx_pid_t mr_pid;
    volatile uint32_t mr_pid_u32;
  };
  /* The number of pages used in the reader's MVCC snapshot,
   * i.e. the value of meta->mm_geo.next and txn->mt_next_pgno */
  volatile pgno_t mr_snapshot_pages_used;
  /* Number of retired pages at the time this reader starts transaction. So,
   * at any time the difference mm_pages_retired - mr_snapshot_pages_retired
   * will give the number of pages which this reader restraining from reuse. */
  volatile uint64_t mr_snapshot_pages_retired;
} MDBX_reader;

/* The header for the reader table (a memory-mapped lock file). */
typedef struct MDBX_lockinfo {
  /* Stamp identifying this as an MDBX file.
   * It must be set to MDBX_MAGIC with with MDBX_LOCK_VERSION. */
  uint64_t mti_magic_and_version;

  /* Format of this lock file. Must be set to MDBX_LOCK_FORMAT. */
  uint32_t mti_os_and_format;

  /* Flags which environment was opened. */
  volatile uint32_t mti_envmode;

  /* Threshold of un-synced-with-disk pages for auto-sync feature,
   * zero means no-threshold, i.e. auto-sync is disabled. */
  volatile pgno_t mti_autosync_threshold;
  /* Period for timed auto-sync feature, i.e. at the every steady checkpoint
   * the mti_unsynced_timeout sets to the current_time + mti_autosync_period.
   * The time value is represented in a suitable system-dependent form, for
   * example clock_gettime(CLOCK_BOOTTIME) or clock_gettime(CLOCK_MONOTONIC).
   * Zero means timed auto-sync is disabled. */
  volatile uint64_t mti_autosync_period;

  alignas(MDBX_CACHELINE_SIZE) /* cacheline ---------------------------------*/
#ifdef MDBX_OSAL_LOCK
      /* Mutex protecting write-txn. */
      MDBX_OSAL_LOCK mti_wmutex;
#endif

  volatile txnid_t mti_oldest_reader;

  /* Timestamp for auto-sync feature, i.e. the steady checkpoint should be
   * created at the first commit that will be not early this timestamp.
   * The time value is represented in a suitable system-dependent form, for
   * example clock_gettime(CLOCK_BOOTTIME) or clock_gettime(CLOCK_MONOTONIC).
   * Zero means timed auto-sync is not pending. */
  volatile uint64_t mti_unsynced_timeout;

  /* Number un-synced-with-disk pages for auto-sync feature. */
  volatile pgno_t mti_unsynced_pages;

  alignas(MDBX_CACHELINE_SIZE) /* cacheline ---------------------------------*/

#ifdef MDBX_OSAL_LOCK
      /* Mutex protecting readers registration access to this table. */
      MDBX_OSAL_LOCK mti_rmutex;
#endif

  /* The number of slots that have been used in the reader table.
   * This always records the maximum count, it is not decremented
   * when readers release their slots. */
  volatile unsigned mti_numreaders;
  volatile unsigned mti_readers_refresh_flag;

  alignas(MDBX_CACHELINE_SIZE) /* cacheline ---------------------------------*/
      MDBX_reader mti_readers[1];
} MDBX_lockinfo;

/* Lockfile format signature: version, features and field layout */
#define MDBX_LOCK_FORMAT                                                       \
  (MDBX_OSAL_LOCK_SIGN * 27733 + (unsigned)sizeof(MDBX_reader) * 13 +          \
   (unsigned)offsetof(MDBX_reader, mr_snapshot_pages_used) * 251 +             \
   (unsigned)offsetof(MDBX_lockinfo, mti_oldest_reader) * 83 +                 \
   (unsigned)offsetof(MDBX_lockinfo, mti_numreaders) * 29)

#define MDBX_DATA_MAGIC ((MDBX_MAGIC << 8) + MDBX_DATA_VERSION)
#define MDBX_DATA_MAGIC_DEVEL ((MDBX_MAGIC << 8) + 255)

#define MDBX_LOCK_MAGIC ((MDBX_MAGIC << 8) + MDBX_LOCK_VERSION)

#ifndef MDBX_ASSUME_MALLOC_OVERHEAD
#define MDBX_ASSUME_MALLOC_OVERHEAD (sizeof(void *) * 2u)
#endif /* MDBX_ASSUME_MALLOC_OVERHEAD */

/*----------------------------------------------------------------------------*/
/* Two kind lists of pages (aka PNL) */

/* An PNL is an Page Number List, a sorted array of IDs. The first element of
 * the array is a counter for how many actual page-numbers are in the list.
 * PNLs are sorted in descending order, this allow cut off a page with lowest
 * pgno (at the tail) just truncating the list */
#define MDBX_PNL_ASCENDING 0
typedef pgno_t *MDBX_PNL;

#if MDBX_PNL_ASCENDING
#define MDBX_PNL_ORDERED(first, last) ((first) < (last))
#define MDBX_PNL_DISORDERED(first, last) ((first) >= (last))
#else
#define MDBX_PNL_ORDERED(first, last) ((first) > (last))
#define MDBX_PNL_DISORDERED(first, last) ((first) <= (last))
#endif

/* List of txnid, only for MDBX_env.mt_lifo_reclaimed */
typedef txnid_t *MDBX_TXL;

/* An Dirty-Page list item is an pgno/pointer pair. */
typedef union MDBX_DP {
  struct {
    pgno_t pgno;
    void *ptr;
  };
  struct {
    pgno_t unused;
    unsigned length;
  };
} MDBX_DP;

/* An DPL (dirty-page list) is a sorted array of MDBX_DPs.
 * The first element's length member is a count of how many actual
 * elements are in the array. */
typedef MDBX_DP *MDBX_DPL;

/* PNL sizes - likely should be even bigger */
#define MDBX_PNL_GRANULATE 1024
#define MDBX_PNL_INITIAL                                                       \
  (MDBX_PNL_GRANULATE - 2 - MDBX_ASSUME_MALLOC_OVERHEAD / sizeof(pgno_t))
#define MDBX_PNL_MAX                                                           \
  ((1u << 24) - 2 - MDBX_ASSUME_MALLOC_OVERHEAD / sizeof(pgno_t))
#define MDBX_DPL_TXNFULL (MDBX_PNL_MAX / 4)

#define MDBX_TXL_GRANULATE 32
#define MDBX_TXL_INITIAL                                                       \
  (MDBX_TXL_GRANULATE - 2 - MDBX_ASSUME_MALLOC_OVERHEAD / sizeof(txnid_t))
#define MDBX_TXL_MAX                                                           \
  ((1u << 17) - 2 - MDBX_ASSUME_MALLOC_OVERHEAD / sizeof(txnid_t))

#define MDBX_PNL_ALLOCLEN(pl) ((pl)[-1])
#define MDBX_PNL_SIZE(pl) ((pl)[0])
#define MDBX_PNL_FIRST(pl) ((pl)[1])
#define MDBX_PNL_LAST(pl) ((pl)[MDBX_PNL_SIZE(pl)])
#define MDBX_PNL_BEGIN(pl) (&(pl)[1])
#define MDBX_PNL_END(pl) (&(pl)[MDBX_PNL_SIZE(pl) + 1])

#define MDBX_PNL_SIZEOF(pl) ((MDBX_PNL_SIZE(pl) + 1) * sizeof(pgno_t))
#define MDBX_PNL_IS_EMPTY(pl) (MDBX_PNL_SIZE(pl) == 0)

/*----------------------------------------------------------------------------*/
/* Internal structures */

/* Auxiliary DB info.
 * The information here is mostly static/read-only. There is
 * only a single copy of this record in the environment. */
typedef struct MDBX_dbx {
  MDBX_val md_name;       /* name of the database */
  MDBX_cmp_func *md_cmp;  /* function for comparing keys */
  MDBX_cmp_func *md_dcmp; /* function for comparing data items */
} MDBX_dbx;

/* A database transaction.
 * Every operation requires a transaction handle. */
struct MDBX_txn {
#define MDBX_MT_SIGNATURE UINT32_C(0x93D53A31)
  size_t mt_signature;
  MDBX_txn *mt_parent; /* parent of a nested txn */
  /* Nested txn under this txn, set together with flag MDBX_TXN_HAS_CHILD */
  MDBX_txn *mt_child;
  pgno_t mt_next_pgno; /* next unallocated page */
  pgno_t mt_end_pgno;  /* corresponding to the current size of datafile */
  /* The ID of this transaction. IDs are integers incrementing from 1.
   * Only committed write transactions increment the ID. If a transaction
   * aborts, the ID may be re-used by the next writer. */
  txnid_t mt_txnid;
  MDBX_env *mt_env; /* the DB environment */
                    /* The list of reclaimed txns from freeDB */
  MDBX_TXL mt_lifo_reclaimed;
  /* The list of pages that became unused during this transaction. */
  MDBX_PNL mt_befree_pages;
  /* The list of loose pages that became unused and may be reused
   * in this transaction, linked through NEXT_LOOSE_PAGE(page). */
  MDBX_page *mt_loose_pages;
  /* Number of loose pages (mt_loose_pages) */
  unsigned mt_loose_count;
  /* The sorted list of dirty pages we temporarily wrote to disk
   * because the dirty list was full. page numbers in here are
   * shifted left by 1, deleted slots have the LSB set. */
  MDBX_PNL mt_spill_pages;
  union {
    /* For write txns: Modified pages. Sorted when not MDBX_WRITEMAP. */
    MDBX_DPL mt_rw_dirtylist;
    /* For read txns: This thread/txn's reader table slot, or NULL. */
    MDBX_reader *mt_ro_reader;
  };
  /* Array of records for each DB known in the environment. */
  MDBX_dbx *mt_dbxs;
  /* Array of MDBX_db records for each known DB */
  MDBX_db *mt_dbs;
  /* Array of sequence numbers for each DB handle */
  unsigned *mt_dbiseqs;

/* Transaction DB Flags */
#define DB_DIRTY MDBX_TBL_DIRTY /* DB was written in this txn */
#define DB_STALE MDBX_TBL_STALE /* Named-DB record is older than txnID */
#define DB_FRESH MDBX_TBL_FRESH /* Named-DB handle opened in this txn */
#define DB_CREAT MDBX_TBL_CREAT /* Named-DB handle created in this txn */
#define DB_VALID 0x10           /* DB handle is valid, see also MDBX_VALID */
#define DB_USRVALID 0x20        /* As DB_VALID, but not set for FREE_DBI */
#define DB_DUPDATA 0x40         /* DB is MDBX_DUPSORT data */
  /* In write txns, array of cursors for each DB */
  MDBX_cursor **mt_cursors;
  /* Array of flags for each DB */
  uint8_t *mt_dbflags;
  /* Number of DB records in use, or 0 when the txn is finished.
   * This number only ever increments until the txn finishes; we
   * don't decrement it when individual DB handles are closed. */
  MDBX_dbi mt_numdbs;

/* Transaction Flags */
/* mdbx_txn_begin() flags */
#define MDBX_TXN_BEGIN_FLAGS                                                   \
  (MDBX_NOMETASYNC | MDBX_NOSYNC | MDBX_RDONLY | MDBX_TRYTXN)
#define MDBX_TXN_NOMETASYNC                                                    \
  MDBX_NOMETASYNC                   /* don't sync meta for this txn on commit */
#define MDBX_TXN_NOSYNC MDBX_NOSYNC /* don't sync this txn on commit */
#define MDBX_TXN_RDONLY MDBX_RDONLY /* read-only transaction */
                                    /* internal txn flags */
#define MDBX_TXN_WRITEMAP MDBX_WRITEMAP /* copy of MDBX_env flag in writers */
#define MDBX_TXN_FINISHED 0x01          /* txn is finished or never began */
#define MDBX_TXN_ERROR 0x02             /* txn is unusable after an error */
#define MDBX_TXN_DIRTY 0x04     /* must write, even if dirty list is empty */
#define MDBX_TXN_SPILLS 0x08    /* txn or a parent has spilled pages */
#define MDBX_TXN_HAS_CHILD 0x10 /* txn has an MDBX_txn.mt_child */
/* most operations on the txn are currently illegal */
#define MDBX_TXN_BLOCKED                                                       \
  (MDBX_TXN_FINISHED | MDBX_TXN_ERROR | MDBX_TXN_HAS_CHILD)
  unsigned mt_flags;
  /* dirtylist room: Array size - dirty pages visible to this txn.
   * Includes ancestor txns' dirty pages not hidden by other txns'
   * dirty/spilled pages. Thus commit(nested txn) has room to merge
   * dirtylist into mt_parent after freeing hidden mt_parent pages. */
  unsigned mt_dirtyroom;
  mdbx_tid_t mt_owner; /* thread ID that owns this transaction */
  mdbx_canary mt_canary;
};

/* Enough space for 2^32 nodes with minimum of 2 keys per node. I.e., plenty.
 * At 4 keys per node, enough for 2^64 nodes, so there's probably no need to
 * raise this on a 64 bit machine. */
#define CURSOR_STACK 32

struct MDBX_xcursor;

/* Cursors are used for all DB operations.
 * A cursor holds a path of (page pointer, key index) from the DB
 * root to a position in the DB, plus other state. MDBX_DUPSORT
 * cursors include an xcursor to the current data item. Write txns
 * track their cursors and keep them up to date when data moves.
 * Exception: An xcursor's pointer to a P_SUBP page can be stale.
 * (A node with F_DUPDATA but no F_SUBDATA contains a subpage). */
struct MDBX_cursor {
#define MDBX_MC_SIGNATURE UINT32_C(0xFE05D5B1)
#define MDBX_MC_READY4CLOSE UINT32_C(0x2817A047)
#define MDBX_MC_WAIT4EOT UINT32_C(0x90E297A7)
  uint32_t mc_signature;
  /* The database handle this cursor operates on */
  MDBX_dbi mc_dbi;
  /* Next cursor on this DB in this txn */
  MDBX_cursor *mc_next;
  /* Backup of the original cursor if this cursor is a shadow */
  MDBX_cursor *mc_backup;
  /* Context used for databases with MDBX_DUPSORT, otherwise NULL */
  struct MDBX_xcursor *mc_xcursor;
  /* The transaction that owns this cursor */
  MDBX_txn *mc_txn;
  /* The database record for this cursor */
  MDBX_db *mc_db;
  /* The database auxiliary record for this cursor */
  MDBX_dbx *mc_dbx;
  /* The mt_dbflag for this database */
  uint8_t *mc_dbflag;
  uint16_t mc_snum;               /* number of pushed pages */
  uint16_t mc_top;                /* index of top page, normally mc_snum-1 */
                                  /* Cursor state flags. */
#define C_INITIALIZED 0x01        /* cursor has been initialized and is valid */
#define C_EOF 0x02                /* No more data */
#define C_SUB 0x04                /* Cursor is a sub-cursor */
#define C_DEL 0x08                /* last op was a cursor_del */
#define C_UNTRACK 0x10            /* Un-track cursor when closing */
#define C_RECLAIMING 0x20         /* FreeDB lookup is prohibited */
#define C_GCFREEZE 0x40           /* me_reclaimed_pglist must not be updated */
  unsigned mc_flags;              /* see mdbx_cursor */
  MDBX_page *mc_pg[CURSOR_STACK]; /* stack of pushed pages */
  indx_t mc_ki[CURSOR_STACK];     /* stack of page indices */
};

/* Context for sorted-dup records.
 * We could have gone to a fully recursive design, with arbitrarily
 * deep nesting of sub-databases. But for now we only handle these
 * levels - main DB, optional sub-DB, sorted-duplicate DB. */
typedef struct MDBX_xcursor {
  /* A sub-cursor for traversing the Dup DB */
  MDBX_cursor mx_cursor;
  /* The database record for this Dup DB */
  MDBX_db mx_db;
  /* The auxiliary DB record for this Dup DB */
  MDBX_dbx mx_dbx;
  /* The mt_dbflag for this Dup DB */
  uint8_t mx_dbflag;
} MDBX_xcursor;

typedef struct MDBX_cursor_couple {
  MDBX_cursor outer;
  MDBX_xcursor inner;
} MDBX_cursor_couple;

/* Check if there is an inited xcursor, so XCURSOR_REFRESH() is proper */
#define XCURSOR_INITED(mc)                                                     \
  ((mc)->mc_xcursor && ((mc)->mc_xcursor->mx_cursor.mc_flags & C_INITIALIZED))

/* Update sub-page pointer, if any, in mc->mc_xcursor.
 * Needed when the node which contains the sub-page may have moved.
 * Called with mp = mc->mc_pg[mc->mc_top], ki = mc->mc_ki[mc->mc_top]. */
#define XCURSOR_REFRESH(mc, mp, ki)                                            \
  do {                                                                         \
    MDBX_page *xr_pg = (mp);                                                   \
    MDBX_node *xr_node = NODEPTR(xr_pg, ki);                                   \
    if ((xr_node->mn_flags & (F_DUPDATA | F_SUBDATA)) == F_DUPDATA)            \
      (mc)->mc_xcursor->mx_cursor.mc_pg[0] = NODEDATA(xr_node);                \
  } while (0)

/* State of FreeDB old pages, stored in the MDBX_env */
typedef struct MDBX_pgstate {
  pgno_t *mf_reclaimed_pglist; /* Reclaimed freeDB pages, or NULL before use */
  txnid_t mf_last_reclaimed;   /* ID of last used record, or 0 if
                                  !mf_reclaimed_pglist */
} MDBX_pgstate;

/* The database environment. */
struct MDBX_env {
#define MDBX_ME_SIGNATURE UINT32_C(0x9A899641)
  size_t me_signature;
  mdbx_mmap_t me_dxb_mmap; /*  The main data file */
#define me_map me_dxb_mmap.dxb
#define me_fd me_dxb_mmap.fd
#define me_mapsize me_dxb_mmap.length
  mdbx_mmap_t me_lck_mmap; /*  The lock file */
#define me_lfd me_lck_mmap.fd
#define me_lck me_lck_mmap.lck

/* Failed to update the meta page. Probably an I/O error. */
#define MDBX_FATAL_ERROR UINT32_C(0x80000000)
/* Additional flag for mdbx_sync_locked() */
#define MDBX_SHRINK_ALLOWED UINT32_C(0x40000000)
/* Some fields are initialized. */
#define MDBX_ENV_ACTIVE UINT32_C(0x20000000)
/* me_txkey is set */
#define MDBX_ENV_TXKEY UINT32_C(0x10000000)
  uint32_t me_flags;      /* see mdbx_env */
  unsigned me_psize;      /* DB page size, inited from me_os_psize */
  unsigned me_psize2log;  /* log2 of DB page size */
  unsigned me_os_psize;   /* OS page size, from mdbx_syspagesize() */
  unsigned me_maxreaders; /* size of the reader table */
  mdbx_fastmutex_t me_dbi_lock;
  MDBX_dbi me_numdbs;         /* number of DBs opened */
  MDBX_dbi me_maxdbs;         /* size of the DB table */
  mdbx_pid_t me_pid;          /* process ID of this env */
  mdbx_thread_key_t me_txkey; /* thread-key for readers */
  char *me_path;              /* path to the DB files */
  void *me_pbuf;              /* scratch area for DUPSORT put() */
  MDBX_txn *me_txn;           /* current write transaction */
  MDBX_txn *me_txn0;          /* prealloc'd write transaction */
#ifdef MDBX_OSAL_LOCK
  MDBX_OSAL_LOCK *me_wmutex; /* write-txn mutex */
#endif
  MDBX_dbx *me_dbxs;           /* array of static DB info */
  uint16_t *me_dbflags;        /* array of flags from MDBX_db.md_flags */
  unsigned *me_dbiseqs;        /* array of dbi sequence numbers */
  volatile txnid_t *me_oldest; /* ID of oldest reader last time we looked */
  MDBX_pgstate me_pgstate;     /* state of old pages from freeDB */
#define me_last_reclaimed me_pgstate.mf_last_reclaimed
#define me_reclaimed_pglist me_pgstate.mf_reclaimed_pglist
  MDBX_page *me_dpages; /* list of malloc'd blocks for re-use */
                        /* PNL of pages that became unused in a write txn */
  MDBX_PNL me_free_pgs;
  /* MDBX_DP of pages written during a write txn. Length MDBX_DPL_TXNFULL. */
  MDBX_DPL me_dirtylist;
  /* Number of freelist items that can fit in a single overflow page */
  unsigned me_maxgc_ov1page;
  /* Max size of a node on a page */
  unsigned me_nodemax;
  unsigned me_maxkey_limit;  /* max size of a key */
  mdbx_pid_t me_live_reader; /* have liveness lock in reader table */
  void *me_userctx;          /* User-settable context */
  volatile uint64_t *me_unsynced_timeout;
  volatile uint64_t *me_autosync_period;
  volatile pgno_t *me_unsynced_pages;
  volatile pgno_t *me_autosync_threshold;
  MDBX_oom_func *me_oom_func; /* Callback for kicking laggard readers */
  struct {
#ifdef MDBX_OSAL_LOCK
    MDBX_OSAL_LOCK wmutex;
#endif
    txnid_t oldest;
    uint64_t unsynced_timeout;
    uint64_t autosync_period;
    pgno_t autosync_pending;
    pgno_t autosync_threshold;
  } me_lckless_stub;
#if MDBX_DEBUG
  MDBX_assert_func *me_assert_func; /*  Callback for assertion failures */
#endif
#ifdef USE_VALGRIND
  int me_valgrind_handle;
#endif

  struct {
    size_t lower;  /* minimal size of datafile */
    size_t upper;  /* maximal size of datafile */
    size_t now;    /* current size of datafile */
    size_t grow;   /* step to grow datafile */
    size_t shrink; /* threshold to shrink datafile */
  } me_dbgeo;      /* */

#if defined(_WIN32) || defined(_WIN64)
  MDBX_srwlock me_remap_guard;
  /* Workaround for LockFileEx and WriteFile multithread bug */
  CRITICAL_SECTION me_windowsbug_lock;
#else
  mdbx_fastmutex_t me_remap_guard;
#endif
};

/* Nested transaction */
typedef struct MDBX_ntxn {
  MDBX_txn mnt_txn;         /* the transaction */
  MDBX_pgstate mnt_pgstate; /* parent transaction's saved freestate */
} MDBX_ntxn;

/*----------------------------------------------------------------------------*/
/* Debug and Logging stuff */

extern int mdbx_runtime_flags;
extern MDBX_debug_func *mdbx_debug_logger;
extern txnid_t mdbx_debug_edge;

void mdbx_debug_log(int type, const char *function, int line, const char *fmt,
                    ...) __printf_args(4, 5);

void mdbx_panic(const char *fmt, ...) __printf_args(1, 2);

#if MDBX_DEBUG

#define mdbx_assert_enabled() unlikely(mdbx_runtime_flags &MDBX_DBG_ASSERT)

#define mdbx_audit_enabled() unlikely(mdbx_runtime_flags &MDBX_DBG_AUDIT)

#define mdbx_debug_enabled(type)                                               \
  unlikely(mdbx_runtime_flags &(type & (MDBX_DBG_TRACE | MDBX_DBG_EXTRA)))

#else
#define mdbx_debug_enabled(type) (0)
#define mdbx_audit_enabled() (0)
#if !defined(NDEBUG) || defined(MDBX_FORCE_ASSERT)
#define mdbx_assert_enabled() (1)
#else
#define mdbx_assert_enabled() (0)
#endif /* NDEBUG */
#endif /* MDBX_DEBUG */

LIBMDBX_API void mdbx_assert_fail(const MDBX_env *env, const char *msg,
                                  const char *func, int line);

#define mdbx_print(fmt, ...)                                                   \
  mdbx_debug_log(MDBX_DBG_PRINT, NULL, 0, fmt, ##__VA_ARGS__)

#define mdbx_trace(fmt, ...)                                                   \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE))                                    \
      mdbx_debug_log(MDBX_DBG_TRACE, __FUNCTION__, __LINE__, fmt "\n",         \
                     ##__VA_ARGS__);                                           \
  } while (0)

#define mdbx_verbose(fmt, ...)                                                 \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_info(fmt, ...)                                                    \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_notice(fmt, ...)                                                  \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_warning(fmt, ...)                                                 \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_error(fmt, ...)                                                   \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_fatal(fmt, ...)                                                   \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE /* FIXME */))                        \
      mdbx_debug_log(MDBX_DBG_TRACE /* FIXME */, __FUNCTION__, __LINE__,       \
                     fmt "\n", ##__VA_ARGS__);                                 \
  } while (0)

#define mdbx_debug(fmt, ...)                                                   \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE))                                    \
      mdbx_debug_log(MDBX_DBG_TRACE, __FUNCTION__, __LINE__, fmt "\n",         \
                     ##__VA_ARGS__);                                           \
  } while (0)

#define mdbx_debug_print(fmt, ...)                                             \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_TRACE))                                    \
      mdbx_debug_log(MDBX_DBG_TRACE, NULL, 0, fmt, ##__VA_ARGS__);             \
  } while (0)

#define mdbx_debug_extra(fmt, ...)                                             \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_EXTRA))                                    \
      mdbx_debug_log(MDBX_DBG_EXTRA, __FUNCTION__, __LINE__, fmt,              \
                     ##__VA_ARGS__);                                           \
  } while (0)

#define mdbx_debug_extra_print(fmt, ...)                                       \
  do {                                                                         \
    if (mdbx_debug_enabled(MDBX_DBG_EXTRA))                                    \
      mdbx_debug_log(MDBX_DBG_EXTRA, NULL, 0, fmt, ##__VA_ARGS__);             \
  } while (0)

#define mdbx_ensure_msg(env, expr, msg)                                        \
  do {                                                                         \
    if (unlikely(!(expr)))                                                     \
      mdbx_assert_fail(env, msg, __FUNCTION__, __LINE__);                      \
  } while (0)

#define mdbx_ensure(env, expr) mdbx_ensure_msg(env, expr, #expr)

/* assert(3) variant in environment context */
#define mdbx_assert(env, expr)                                                 \
  do {                                                                         \
    if (mdbx_assert_enabled())                                                 \
      mdbx_ensure(env, expr);                                                  \
  } while (0)

/* assert(3) variant in cursor context */
#define mdbx_cassert(mc, expr) mdbx_assert((mc)->mc_txn->mt_env, expr)

/* assert(3) variant in transaction context */
#define mdbx_tassert(txn, expr) mdbx_assert((txn)->mt_env, expr)

#undef assert
#define assert(expr) mdbx_assert(NULL, expr)

/*----------------------------------------------------------------------------*/
/* Internal prototypes */

int mdbx_reader_check0(MDBX_env *env, int rlocked, int *dead);
int mdbx_rthc_alloc(mdbx_thread_key_t *key, MDBX_reader *begin,
                    MDBX_reader *end);
void mdbx_rthc_remove(const mdbx_thread_key_t key);

void mdbx_rthc_global_init(void);
void mdbx_rthc_global_dtor(void);
void mdbx_rthc_thread_dtor(void *ptr);

#define MDBX_IS_ERROR(rc)                                                      \
  ((rc) != MDBX_RESULT_TRUE && (rc) != MDBX_RESULT_FALSE)

/* Internal error codes, not exposed outside libmdbx */
#define MDBX_NO_ROOT (MDBX_LAST_ERRCODE + 10)

/* Debuging output value of a cursor DBI: Negative in a sub-cursor. */
#define DDBI(mc)                                                               \
  (((mc)->mc_flags & C_SUB) ? -(int)(mc)->mc_dbi : (int)(mc)->mc_dbi)

/* Key size which fits in a DKBUF. */
#define DKBUF_MAXKEYSIZE 511 /* FIXME */

#if MDBX_DEBUG
#define DKBUF char _kbuf[DKBUF_MAXKEYSIZE * 4 + 2]
#define DKEY(x) mdbx_dkey(x, _kbuf, DKBUF_MAXKEYSIZE * 2 + 1)
#define DVAL(x)                                                                \
  mdbx_dkey(x, _kbuf + DKBUF_MAXKEYSIZE * 2 + 1, DKBUF_MAXKEYSIZE * 2 + 1)
#else
#define DKBUF ((void)(0))
#define DKEY(x) ("-")
#define DVAL(x) ("-")
#endif

/* An invalid page number.
 * Mainly used to denote an empty tree. */
#define P_INVALID (~(pgno_t)0)

/* Test if the flags f are set in a flag word w. */
#define F_ISSET(w, f) (((w) & (f)) == (f))

/* Round n up to an even number. */
#define EVEN(n) (((n) + 1U) & -2) /* sign-extending -2 to match n+1U */

/* Default size of memory map.
 * This is certainly too small for any actual applications. Apps should
 * always set  the size explicitly using mdbx_env_set_mapsize(). */
#define DEFAULT_MAPSIZE 1048576

/* Number of slots in the reader table.
 * This value was chosen somewhat arbitrarily. The 61 is a prime number,
 * and such readers plus a couple mutexes fit into single 4KB page.
 * Applications should set the table size using mdbx_env_set_maxreaders(). */
#define DEFAULT_READERS 61

/* Address of first usable data byte in a page, after the header */
#define PAGEDATA(p) ((void *)((char *)(p) + PAGEHDRSZ))

/* Number of nodes on a page */
#define NUMKEYS(p) ((unsigned)(p)->mp_lower >> 1)

/* The amount of space remaining in the page */
#define SIZELEFT(p) ((indx_t)((p)->mp_upper - (p)->mp_lower))

/* The percentage of space used in the page, in tenths of a percent. */
#define PAGEFILL(env, p)                                                       \
  (1024UL * ((env)->me_psize - PAGEHDRSZ - SIZELEFT(p)) /                      \
   ((env)->me_psize - PAGEHDRSZ))
/* The minimum page fill factor, in tenths of a percent.
 * Pages emptier than this are candidates for merging. */
#define FILL_THRESHOLD 256

/* Test if a page is a leaf page */
#define IS_LEAF(p) (((p)->mp_flags & P_LEAF) != 0)
/* Test if a page is a LEAF2 page */
#define IS_LEAF2(p) unlikely(((p)->mp_flags & P_LEAF2) != 0)
/* Test if a page is a branch page */
#define IS_BRANCH(p) (((p)->mp_flags & P_BRANCH) != 0)
/* Test if a page is an overflow page */
#define IS_OVERFLOW(p) unlikely(((p)->mp_flags & P_OVERFLOW) != 0)
/* Test if a page is a sub page */
#define IS_SUBP(p) (((p)->mp_flags & P_SUBP) != 0)
/* Test if a page is dirty */
#define IS_DIRTY(p) (((p)->mp_flags & P_DIRTY) != 0)

#define PAGETYPE(p) ((p)->mp_flags & (P_BRANCH | P_LEAF | P_LEAF2 | P_OVERFLOW))

/* The number of overflow pages needed to store the given size. */
#define OVPAGES(env, size) (bytes2pgno(env, PAGEHDRSZ - 1 + (size)) + 1)

/* Link in MDBX_txn.mt_loose_pages list.
 * Kept outside the page header, which is needed when reusing the page. */
#define NEXT_LOOSE_PAGE(p) (*(MDBX_page **)((p) + 2))

/* Header for a single key/data pair within a page.
 * Used in pages of type P_BRANCH and P_LEAF without P_LEAF2.
 * We guarantee 2-byte alignment for 'MDBX_node's.
 *
 * mn_lo and mn_hi are used for data size on leaf nodes, and for child
 * pgno on branch nodes.  On 64 bit platforms, mn_flags is also used
 * for pgno.  (Branch nodes have no flags).  Lo and hi are in host byte
 * order in case some accesses can be optimized to 32-bit word access.
 *
 * Leaf node flags describe node contents.  F_BIGDATA says the node's
 * data part is the page number of an overflow page with actual data.
 * F_DUPDATA and F_SUBDATA can be combined giving duplicate data in
 * a sub-page/sub-database, and named databases (just F_SUBDATA). */
typedef struct MDBX_node {
  union {
    struct {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
      union {
        struct {
          uint16_t mn_lo, mn_hi; /* part of data size or pgno */
        };
        uint32_t mn_dsize;
      };
      uint16_t mn_flags; /* see mdbx_node */
      uint16_t mn_ksize; /* key size */
#else
      uint16_t mn_ksize; /* key size */
      uint16_t mn_flags; /* see mdbx_node */
      union {
        struct {
          uint16_t mn_hi, mn_lo; /* part of data size or pgno */
        };
        uint32_t mn_dsize;
      };
#endif
    };
    pgno_t mn_ksize_and_pgno;
  };

/* mdbx_node Flags */
#define F_BIGDATA 0x01 /* data put on overflow page */
#define F_SUBDATA 0x02 /* data is a sub-database */
#define F_DUPDATA 0x04 /* data has duplicates */

/* valid flags for mdbx_node_add() */
#define NODE_ADD_FLAGS (F_DUPDATA | F_SUBDATA | MDBX_RESERVE | MDBX_APPEND)
  uint8_t mn_data[1]; /* key and data are appended here */
} MDBX_node;

/* Size of the node header, excluding dynamic data at the end */
#define NODESIZE offsetof(MDBX_node, mn_data)

/* Bit position of top word in page number, for shifting mn_flags */
#define PGNO_TOPWORD ((pgno_t)-1 > 0xffffffffu ? 32 : 0)

/* Size of a node in a branch page with a given key.
 * This is just the node header plus the key, there is no data. */
#define INDXSIZE(k) (NODESIZE + ((k) == NULL ? 0 : (k)->iov_len))

/* Size of a node in a leaf page with a given key and data.
 * This is node header plus key plus data size. */
#define LEAFSIZE(k, d) (NODESIZE + (k)->iov_len + (d)->iov_len)

/* Address of the key for the node */
#define NODEKEY(node) (void *)((node)->mn_data)

/* Address of the data for a node */
#define NODEDATA(node) (void *)((char *)(node)->mn_data + (node)->mn_ksize)

/* The size of a key in a node */
#define NODEKSZ(node) ((node)->mn_ksize)

/* The address of a key in a LEAF2 page.
 * LEAF2 pages are used for MDBX_DUPFIXED sorted-duplicate sub-DBs.
 * There are no node headers, keys are stored contiguously. */
#define LEAF2KEY(p, i, ks) ((char *)(p) + PAGEHDRSZ + ((i) * (ks)))

/* Set the node's key into keyptr, if requested. */
#define MDBX_GET_MAYNULL_KEYPTR(node, keyptr)                                  \
  do {                                                                         \
    if ((keyptr) != NULL) {                                                    \
      (keyptr)->iov_len = NODEKSZ(node);                                       \
      (keyptr)->iov_base = NODEKEY(node);                                      \
    }                                                                          \
  } while (0)

/* Set the node's key into key. */
#define MDBX_GET_KEYVALUE(node, key)                                           \
  do {                                                                         \
    key.iov_len = NODEKSZ(node);                                               \
    key.iov_base = NODEKEY(node);                                              \
  } while (0)

#define MDBX_VALID 0x8000 /* DB handle is valid, for me_dbflags */
#define PERSISTENT_FLAGS (0xffff & ~(MDBX_VALID))
/* mdbx_dbi_open() flags */
#define VALID_FLAGS                                                            \
  (MDBX_REVERSEKEY | MDBX_DUPSORT | MDBX_INTEGERKEY | MDBX_DUPFIXED |          \
   MDBX_INTEGERDUP | MDBX_REVERSEDUP | MDBX_CREATE)

/* max number of pages to commit in one writev() call */
#define MDBX_COMMIT_PAGES 64
#if defined(IOV_MAX) && IOV_MAX < MDBX_COMMIT_PAGES /* sysconf(_SC_IOV_MAX) */
#undef MDBX_COMMIT_PAGES
#define MDBX_COMMIT_PAGES IOV_MAX
#endif

/* LY: fast enough on most systems
 *
 *                /
 *                | -1, a < b
 * cmp2int(a,b) = <  0, a == b
 *                |  1, a > b
 *                \
 */
#if 1
#define mdbx_cmp2int(a, b) (((b) > (a)) ? -1 : (a) > (b))
#else
#define mdbx_cmp2int(a, b) (((a) > (b)) - ((b) > (a)))
#endif

/* Do not spill pages to disk if txn is getting full, may fail instead */
#define MDBX_NOSPILL 0x8000

static __inline pgno_t pgno_add(pgno_t base, pgno_t augend) {
  assert(base <= MAX_PAGENO);
  return (augend < MAX_PAGENO - base) ? base + augend : MAX_PAGENO;
}

static __inline pgno_t pgno_sub(pgno_t base, pgno_t subtrahend) {
  assert(base >= MIN_PAGENO);
  return (subtrahend < base - MIN_PAGENO) ? base - subtrahend : MIN_PAGENO;
}

static __inline void mdbx_jitter4testing(bool tiny) {
#if MDBX_DEBUG
  if (MDBX_DBG_JITTER & mdbx_runtime_flags)
    mdbx_osal_jitter(tiny);
#else
  (void)tiny;
#endif
}

/* Controls checking PID against reuse DB environment after the fork() */
#ifndef MDBX_TXN_CHECKPID
#if defined(MADV_DONTFORK) || defined(_WIN32) || defined(_WIN64)
/* PID check could be ommited:
 *  - on Linux when madvise(MADV_DONTFORK) is available. i.e. after the fork()
 *    mapped pages will not be available for child process.
 *  - in Windows where fork() not available. */
#define MDBX_TXN_CHECKPID 0
#else
#define MDBX_TXN_CHECKPID 1
#endif
#endif /* MDBX_TXN_CHECKPID */
