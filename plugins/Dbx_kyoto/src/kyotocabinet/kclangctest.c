/*************************************************************************************************
 * The test cases of the C language binding
 *                                                               Copyright (C) 2009-2012 FAL Labs
 * This file is part of Kyoto Cabinet.
 * This program is free software: you can redistribute it and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation, either version
 * 3 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 *************************************************************************************************/


#include <kclangc.h>

#define RECBUFSIZ    64                  /* buffer size for a record */
#define RECBUFSIZL   1024                /* buffer size for a long record */
#if !defined(TRUE)
#define TRUE         1                   /* boolean true */
#endif
#if !defined(FALSE)
#define FALSE        0                   /* boolean false */
#endif

typedef struct {                         /* arguments of visitor */
  int64_t rnum;
  int32_t rnd;
  int64_t cnt;
  char rbuf[RECBUFSIZ];
} VISARG;


/* global variables */
const char* g_progname;                  /* program name */


/* function prototypes */
int main(int argc, char** argv);
static void usage(void);
static int64_t myrand(int64_t range);
static void oprintf(const char* format, ...);
static void oputchar(char c);
static void eprintf(const char* format, ...);
static void dberrprint(KCDB* db, int32_t line, const char* func);
static void idxerrprint(KCIDX* idx, int32_t line, const char* func);
static void dbmetaprint(KCDB* db, int32_t verbose);
static void idxmetaprint(KCIDX* idx, int32_t verbose);
const char* visitfull(const char* kbuf, size_t ksiz,
                      const char* vbuf, size_t vsiz, size_t* sp, void* opq);
static int32_t runorder(int argc, char** argv);
static int32_t runindex(int argc, char** argv);
static int32_t runmap(int argc, char** argv);
static int32_t runlist(int argc, char** argv);
static int32_t procorder(const char* path, int64_t rnum, int32_t rnd, int32_t etc,
                         int32_t tran, int32_t oflags);
static int32_t procindex(const char* path, int64_t rnum, int32_t rnd, int32_t etc,
                         int32_t oflags);
static int32_t procmap(int64_t rnum, int32_t rnd, int32_t etc, int64_t bnum);
static int32_t proclist(int64_t rnum, int32_t rnd, int32_t etc);


/* main routine */
int main(int argc, char **argv) {
  int32_t i, rv;
  g_progname = argv[0];
  srand(time(NULL));
  if (argc < 2) usage();
  rv = 0;
  if (!strcmp(argv[1], "order")) {
    rv = runorder(argc, argv);
  } else if (!strcmp(argv[1], "index")) {
    rv = runindex(argc, argv);
  } else if (!strcmp(argv[1], "map")) {
    rv = runmap(argc, argv);
  } else if (!strcmp(argv[1], "list")) {
    rv = runlist(argc, argv);
  } else {
    usage();
  }
  if (rv != 0) {
    oprintf("FAILED:");
    for (i = 0; i < argc; i++) {
      oprintf(" %s", argv[i]);
    }
    oprintf("\n\n");
  }
  return rv;
}


/* print the usage and exit */
static void usage() {
  eprintf("%s: test cases of the C binding of Kyoto Cabinet\n", g_progname);
  eprintf("\n");
  eprintf("usage:\n");
  eprintf("  %s order [-rnd] [-etc] [-tran] [-oat|-oas|-onl|-otl|-onr] path rnum\n",
          g_progname);
  eprintf("  %s index [-rnd] [-etc] [-oat|-oas|-onl|-otl|-onr] path rnum\n", g_progname);
  eprintf("  %s map [-rnd] [-etc] [-bnum num] rnum\n", g_progname);
  eprintf("  %s list [-rnd] [-etc] rnum\n", g_progname);
  eprintf("\n");
  exit(1);
}


/* get a random number */
static int64_t myrand(int64_t range) {
  uint64_t base, mask;
  if (range < 2) return 0;
  base = range * (rand() / (RAND_MAX + 1.0));
  mask = (uint64_t)rand() << 30;
  mask += (uint64_t)rand() >> 2;
  return (base ^ mask) % range;
}


/* print formatted error string and flush the buffer */
static void oprintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
  fflush(stdout);
}


/* print a character and flush the buffer */
static void oputchar(char c) {
  putchar(c);
  fflush(stdout);
}


/* print formatted error string and flush the buffer */
static void eprintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  fflush(stderr);
}


/* print error message of database */
static void dberrprint(KCDB* db, int32_t line, const char* func) {
  char* path;
  const char* emsg;
  int32_t ecode;
  path = kcdbpath(db);
  ecode = kcdbecode(db);
  emsg = kcdbemsg(db);
  oprintf("%s: %d: %s: %s: %d: %s: %s\n",
          g_progname, line, func, path ? path : "-", ecode, kcecodename(ecode), emsg);
  kcfree(path);
}


/* print error message of database */
static void idxerrprint(KCIDX* idx, int32_t line, const char* func) {
  dberrprint(kcidxrevealinnerdb(idx), line, func);
}


/* print members of database */
static void dbmetaprint(KCDB* db, int32_t verbose) {
  char* status, *rp;
  if (verbose) {
    status = kcdbstatus(db);
    if (status) {
      rp = status;
      while (*rp != '\0') {
        if (*rp == '\t') {
          printf(": ");
        } else {
          putchar(*rp);
        }
        rp++;
      }
      kcfree(status);
    }
  } else {
    oprintf("count: %ld\n", (long)kcdbcount(db));
    oprintf("size: %ld\n", (long)kcdbsize(db));
  }
}


/* print members of database */
static void idxmetaprint(KCIDX* idx, int32_t verbose) {
  dbmetaprint(kcidxrevealinnerdb(idx), verbose);
}


/* visit a full record */
const char* visitfull(const char* kbuf, size_t ksiz,
                      const char* vbuf, size_t vsiz, size_t* sp, void* opq) {
  VISARG* arg;
  const char* rv;
  arg = opq;
  arg->cnt++;
  rv = KCVISNOP;
  switch (arg->rnd ? myrand(7) : arg->cnt % 7) {
    case 0: {
      rv = arg->rbuf;
      *sp = arg->rnd ? (size_t)myrand(sizeof(arg->rbuf)) :
        sizeof(arg->rbuf) / (arg->cnt % 5 + 1);
      break;
    }
    case 1: {
      rv = KCVISREMOVE;
      break;
    }
  }
  if (arg->rnum > 250 && arg->cnt % (arg->rnum / 250) == 0) {
    oputchar('.');
    if (arg->cnt == arg->rnum || arg->cnt % (arg->rnum / 10) == 0)
      oprintf(" (%08ld)\n", (long)arg->cnt);
  }
  return rv;
}


/* parse arguments of order command */
static int32_t runorder(int argc, char** argv) {
  int32_t argbrk = FALSE;
  const char* path, *rstr;
  int32_t rnd, etc, tran, mode, oflags, i;
  int64_t rnum;
  path = NULL;
  rstr = NULL;
  rnd = FALSE;
  etc = FALSE;
  mode = 0;
  tran = FALSE;
  oflags = 0;
  for (i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!strcmp(argv[i], "--")) {
        argbrk = TRUE;
      } else if (!strcmp(argv[i], "-rnd")) {
        rnd = TRUE;
      } else if (!strcmp(argv[i], "-etc")) {
        etc = TRUE;
      } else if (!strcmp(argv[i], "-tran")) {
        tran = TRUE;
      } else if (!strcmp(argv[i], "-oat")) {
        oflags |= KCOAUTOTRAN;
      } else if (!strcmp(argv[i], "-oas")) {
        oflags |= KCOAUTOSYNC;
      } else if (!strcmp(argv[i], "-onl")) {
        oflags |= KCONOLOCK;
      } else if (!strcmp(argv[i], "-otl")) {
        oflags |= KCOTRYLOCK;
      } else if (!strcmp(argv[i], "-onr")) {
        oflags |= KCONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = TRUE;
      path = argv[i];
    } else if (!rstr) {
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !rstr) usage();
  rnum = kcatoix(rstr);
  if (rnum < 1) usage();
  return procorder(path, rnum, rnd, etc, tran, oflags);
}


/* parse arguments of index command */
static int32_t runindex(int argc, char** argv) {
  int32_t argbrk = FALSE;
  const char* path, *rstr;
  int32_t rnd, etc, mode, oflags, i;
  int64_t rnum;
  path = NULL;
  rstr = NULL;
  rnd = FALSE;
  etc = FALSE;
  mode = 0;
  oflags = 0;
  for (i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!strcmp(argv[i], "--")) {
        argbrk = TRUE;
      } else if (!strcmp(argv[i], "-rnd")) {
        rnd = TRUE;
      } else if (!strcmp(argv[i], "-etc")) {
        etc = TRUE;
      } else if (!strcmp(argv[i], "-oat")) {
        oflags |= KCOAUTOTRAN;
      } else if (!strcmp(argv[i], "-oas")) {
        oflags |= KCOAUTOSYNC;
      } else if (!strcmp(argv[i], "-onl")) {
        oflags |= KCONOLOCK;
      } else if (!strcmp(argv[i], "-otl")) {
        oflags |= KCOTRYLOCK;
      } else if (!strcmp(argv[i], "-onr")) {
        oflags |= KCONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = TRUE;
      path = argv[i];
    } else if (!rstr) {
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !rstr) usage();
  rnum = kcatoix(rstr);
  if (rnum < 1) usage();
  return procindex(path, rnum, rnd, etc, oflags);
}


/* parse arguments of map command */
static int32_t runmap(int argc, char** argv) {
  int32_t argbrk = FALSE;
  const char* rstr;
  int32_t rnd, etc, i;
  int64_t rnum, bnum;
  rstr = NULL;
  rnd = FALSE;
  etc = FALSE;
  bnum = -1;
  for (i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!strcmp(argv[i], "--")) {
        argbrk = TRUE;
      } else if (!strcmp(argv[i], "-rnd")) {
        rnd = TRUE;
      } else if (!strcmp(argv[i], "-etc")) {
        etc = TRUE;
      } else if (!strcmp(argv[i], "-bnum")) {
        if (++i >= argc) usage();
        bnum = kcatoix(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = TRUE;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  rnum = kcatoix(rstr);
  if (rnum < 1) usage();
  return procmap(rnum, rnd, etc, bnum);
}


/* parse arguments of list command */
static int32_t runlist(int argc, char** argv) {
  int32_t argbrk = FALSE;
  const char* rstr;
  int32_t rnd, etc, i;
  int64_t rnum;
  rstr = NULL;
  rnd = FALSE;
  etc = FALSE;
  for (i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!strcmp(argv[i], "--")) {
        argbrk = TRUE;
      } else if (!strcmp(argv[i], "-rnd")) {
        rnd = TRUE;
      } else if (!strcmp(argv[i], "-etc")) {
        etc = TRUE;
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = TRUE;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  rnum = kcatoix(rstr);
  if (rnum < 1) usage();
  return proclist(rnum, rnd, etc);
}


/* perform order command */
static int32_t procorder(const char* path, int64_t rnum, int32_t rnd, int32_t etc,
                         int32_t tran, int32_t oflags) {
  KCDB* db;
  KCCUR* cur, *paracur;
  int32_t err;
  char kbuf[RECBUFSIZ], *vbuf, wbuf[RECBUFSIZ], *corepath, *copypath, *snappath;
  size_t ksiz, vsiz, psiz;
  int32_t wsiz;
  int64_t i, cnt;
  double stime, etime;
  VISARG visarg;
  oprintf("<In-order Test>\n  path=%s  rnum=%ld  rnd=%d  etc=%d  tran=%d  oflags=%d\n\n",
          path, (long)rnum, rnd, etc, tran, oflags);
  err = FALSE;
  db = kcdbnew();
  oprintf("opening the database:\n");
  stime = kctime();
  if (!kcdbopen(db, path, KCOWRITER | KCOCREATE | KCOTRUNCATE | oflags)) {
    dberrprint(db, __LINE__, "kcdbopen");
    err = TRUE;
  }
  etime = kctime();
  dbmetaprint(db, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("setting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    if (tran && !kcdbbegintran(db, FALSE)) {
      dberrprint(db, __LINE__, "kcdbbegintran");
      err = TRUE;
    }
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    if (!kcdbset(db, kbuf, ksiz, kbuf, ksiz)) {
      dberrprint(db, __LINE__, "kcdbset");
      err = TRUE;
    }
    if (tran && !kcdbendtran(db, TRUE)) {
      dberrprint(db, __LINE__, "kcdbendtran");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  dbmetaprint(db, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("adding records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      if (tran && !kcdbbegintran(db, FALSE)) {
        dberrprint(db, __LINE__, "kcdbbegintran");
        err = TRUE;
      }
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      if (!kcdbadd(db, kbuf, ksiz, kbuf, ksiz) && kcdbecode(db) != KCEDUPREC) {
        dberrprint(db, __LINE__, "kcdbadd");
        err = TRUE;
      }
      if (tran && !kcdbendtran(db, TRUE)) {
        dberrprint(db, __LINE__, "kcdbendtran");
        err = TRUE;
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("appending records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      if (tran && !kcdbbegintran(db, FALSE)) {
        dberrprint(db, __LINE__, "kcdbbegintran");
        err = TRUE;
      }
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      if (!kcdbappend(db, kbuf, ksiz, kbuf, ksiz)) {
        dberrprint(db, __LINE__, "kcdbadd");
        err = TRUE;
      }
      if (tran && !kcdbendtran(db, TRUE)) {
        dberrprint(db, __LINE__, "kcdbendtran");
        err = TRUE;
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  oprintf("getting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    if (tran && !kcdbbegintran(db, FALSE)) {
      dberrprint(db, __LINE__, "kcdbbegintran");
      err = TRUE;
    }
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    vbuf = kcdbget(db, kbuf, ksiz, &vsiz);
    if (vbuf) {
      if (vsiz < ksiz || memcmp(vbuf, kbuf, ksiz)) {
        dberrprint(db, __LINE__, "kcdbget");
        err = TRUE;
      }
      kcfree(vbuf);
    } else if (!rnd || kcdbecode(db) != KCENOREC) {
      dberrprint(db, __LINE__, "kcdbget");
      err = TRUE;
    }
    if (tran && !kcdbendtran(db, TRUE)) {
      dberrprint(db, __LINE__, "kcdbendtran");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  dbmetaprint(db, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("getting records with a buffer:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      if (tran && !kcdbbegintran(db, FALSE)) {
        dberrprint(db, __LINE__, "kcdbbegintran");
        err = TRUE;
      }
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      wsiz = kcdbgetbuf(db, kbuf, ksiz, wbuf, sizeof(wbuf));
      if (wsiz >= 0) {
        if (wsiz < (int32_t)ksiz || memcmp(wbuf, kbuf, ksiz)) {
          dberrprint(db, __LINE__, "kcdbgetbuf");
          err = TRUE;
        }
      } else if (!rnd || kcdbecode(db) != KCENOREC) {
        dberrprint(db, __LINE__, "kcdbgetbuf");
        err = TRUE;
      }
      if (tran && !kcdbendtran(db, TRUE)) {
        dberrprint(db, __LINE__, "kcdbendtran");
        err = TRUE;
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("traversing the database by the inner iterator:\n");
    stime = kctime();
    cnt = kcdbcount(db);
    visarg.rnum = rnum;
    visarg.rnd = rnd;
    visarg.cnt = 0;
    memset(visarg.rbuf, '+', sizeof(visarg.rbuf));
    if (tran && !kcdbbegintran(db, FALSE)) {
      dberrprint(db, __LINE__, "kcdbbegintran");
      err = TRUE;
    }
    if (!kcdbiterate(db, visitfull, &visarg, TRUE)) {
      dberrprint(db, __LINE__, "kcdbiterate");
      err = TRUE;
    }
    if (rnd) oprintf(" (end)\n");
    if (tran && !kcdbendtran(db, TRUE)) {
      dberrprint(db, __LINE__, "kcdbendtran");
      err = TRUE;
    }
    if (visarg.cnt != cnt) {
      dberrprint(db, __LINE__, "kcdbiterate");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("traversing the database by the outer cursor:\n");
    stime = kctime();
    cnt = kcdbcount(db);
    visarg.rnum = rnum;
    visarg.rnd = rnd;
    visarg.cnt = 0;
    if (tran && !kcdbbegintran(db, FALSE)) {
      dberrprint(db, __LINE__, "kcdbbegintran");
      err = TRUE;
    }
    cur = kcdbcursor(db);
    if (!kccurjump(cur) && kccurecode(cur) != KCENOREC) {
      dberrprint(db, __LINE__, "kccurjump");
      err = TRUE;
    }
    paracur = kcdbcursor(db);
    while (!err && kccuraccept(cur, &visitfull, &visarg, TRUE, !rnd)) {
      if (rnd) {
        ksiz = sprintf(kbuf, "%08ld", (long)myrand(rnum));
        switch (myrand(3)) {
          case 0: {
            if (!kcdbremove(db, kbuf, ksiz) && kcdbecode(db) != KCENOREC) {
              dberrprint(db, __LINE__, "kcdbremove");
              err = TRUE;
            }
            break;
          }
          case 1: {
            if (!kccurjumpkey(paracur, kbuf, ksiz) && kccurecode(paracur) != KCENOREC) {
              dberrprint(db, __LINE__, "kccurjump");
              err = TRUE;
            }
            break;
          }
          default: {
            if (!kccurstep(cur) && kccurecode(cur) != KCENOREC) {
              dberrprint(db, __LINE__, "kccurstep");
              err = TRUE;
            }
            break;
          }
        }
      }
    }
    oprintf(" (end)\n");
    kccurdel(paracur);
    kccurdel(cur);
    if (tran && !kcdbendtran(db, TRUE)) {
      dberrprint(db, __LINE__, "kcdbendtran");
      err = TRUE;
    }
    if (!rnd && visarg.cnt != cnt) {
      dberrprint(db, __LINE__, "kccuraccept");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("synchronizing the database:\n");
    stime = kctime();
    if (!kcdbsync(db, FALSE, NULL, NULL)) {
      dberrprint(db, __LINE__, "kcdbsync");
      err = TRUE;
    }
    if (!kcdboccupy(db, FALSE, NULL, NULL)) {
      dberrprint(db, __LINE__, "kcdboccupy");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    corepath = kcdbpath(db);
    psiz = strlen(corepath);
    if (strstr(corepath, ".kch") || strstr(corepath, ".kct")) {
      copypath = kcmalloc(psiz + 256);
      sprintf(copypath, "%s.tmp", corepath);
      snappath = kcmalloc(psiz + 256);
      sprintf(snappath, "%s.kcss", corepath);
    } else {
      copypath = kcmalloc(256);
      sprintf(copypath, "kclangctest.tmp");
      snappath = kcmalloc(256);
      sprintf(snappath, "kclangctest.kcss");
    }
    oprintf("copying the database file:\n");
    stime = kctime();
    if (!kcdbcopy(db, copypath)) {
      dberrprint(db, __LINE__, "kcdbcopy");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
    remove(copypath);
    oprintf("dumping records into snapshot:\n");
    stime = kctime();
    if (!kcdbdumpsnap(db, snappath)) {
      dberrprint(db, __LINE__, "kcdbdumpsnap");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
    oprintf("loading records into snapshot:\n");
    stime = kctime();
    cnt = kcdbcount(db);
    if (rnd && myrand(2) == 0 && !kcdbclear(db)) {
      dberrprint(db, __LINE__, "kcdbclear");
      err = TRUE;
    }
    if (!kcdbloadsnap(db, snappath) || kcdbcount(db) != cnt) {
      dberrprint(db, __LINE__, "kcdbloadsnap");
      err = TRUE;
    }
    etime = kctime();
    dbmetaprint(db, FALSE);
    oprintf("time: %.3f\n", etime - stime);
    remove(snappath);
    kcfree(copypath);
    kcfree(snappath);
    kcfree(corepath);
  }
  oprintf("removing records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    if (tran && !kcdbbegintran(db, FALSE)) {
      dberrprint(db, __LINE__, "kcdbbegintran");
      err = TRUE;
    }
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    if (!kcdbremove(db, kbuf, ksiz) &&
        ((!rnd && !etc) || kcdbecode(db) != KCENOREC)) {
      dberrprint(db, __LINE__, "kcdbremove");
      err = TRUE;
    }
    if (tran && !kcdbendtran(db, TRUE)) {
      dberrprint(db, __LINE__, "kcdbendtran");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  dbmetaprint(db, TRUE);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("closing the database:\n");
  stime = kctime();
  if (!kcdbclose(db)) {
    dberrprint(db, __LINE__, "kcdbclose");
    err = TRUE;
  }
  etime = kctime();
  oprintf("time: %.3f\n", etime - stime);
  kcdbdel(db);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


/* perform index command */
static int32_t procindex(const char* path, int64_t rnum, int32_t rnd, int32_t etc,
                         int32_t oflags) {
  KCIDX* idx;
  int32_t err;
  char kbuf[RECBUFSIZ], *vbuf;
  size_t ksiz, vsiz;
  int64_t i;
  double stime, etime;
  oprintf("<Index Database Test>\n  path=%s  rnum=%ld  rnd=%d  etc=%d  oflags=%d\n\n",
          path, (long)rnum, rnd, etc, oflags);
  err = FALSE;
  idx = kcidxnew();
  oprintf("opening the database:\n");
  stime = kctime();
  if (!kcidxopen(idx, path, KCOWRITER | KCOCREATE | KCOTRUNCATE | oflags)) {
    idxerrprint(idx, __LINE__, "kcidxopen");
    err = TRUE;
  }
  etime = kctime();
  idxmetaprint(idx, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("setting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    if (!kcidxset(idx, kbuf, ksiz, kbuf, ksiz)) {
      idxerrprint(idx, __LINE__, "kcidxset");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  idxmetaprint(idx, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("adding records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      if (!kcidxadd(idx, kbuf, ksiz, kbuf, ksiz) && kcidxecode(idx) != KCEDUPREC) {
        idxerrprint(idx, __LINE__, "kcidxadd");
        err = TRUE;
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    idxmetaprint(idx, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("appending records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      if (!kcidxappend(idx, kbuf, ksiz, kbuf, ksiz)) {
        idxerrprint(idx, __LINE__, "kcidxadd");
        err = TRUE;
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    idxmetaprint(idx, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  oprintf("getting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    vbuf = kcidxget(idx, kbuf, ksiz, &vsiz);
    if (vbuf) {
      if (vsiz < ksiz || memcmp(vbuf, kbuf, ksiz)) {
        idxerrprint(idx, __LINE__, "kcidxget");
        err = TRUE;
      }
      kcfree(vbuf);
    } else if (!rnd || kcidxecode(idx) != KCENOREC) {
      idxerrprint(idx, __LINE__, "kcidxget");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  idxmetaprint(idx, FALSE);
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("synchronizing the database:\n");
    stime = kctime();
    if (!kcidxsync(idx, FALSE, NULL, NULL)) {
      idxerrprint(idx, __LINE__, "kcidxsync");
      err = TRUE;
    }
    etime = kctime();
    idxmetaprint(idx, FALSE);
    oprintf("time: %.3f\n", etime - stime);
  }
  oprintf("removing records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    if (!kcidxremove(idx, kbuf, ksiz) &&
        ((!rnd && !etc) || kcidxecode(idx) != KCENOREC)) {
      idxerrprint(idx, __LINE__, "kcidxremove");
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  idxmetaprint(idx, TRUE);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("closing the database:\n");
  stime = kctime();
  if (!kcidxclose(idx)) {
    idxerrprint(idx, __LINE__, "kcidxclose");
    err = TRUE;
  }
  etime = kctime();
  oprintf("time: %.3f\n", etime - stime);
  kcidxdel(idx);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


/* perform map command */
static int32_t procmap(int64_t rnum, int32_t rnd, int32_t etc, int64_t bnum) {
  KCMAP* map;
  KCMAPITER* iter;
  KCMAPSORT* sort;
  int32_t err;
  char kbuf[RECBUFSIZ];
  const char* vbuf, *ikbuf;
  size_t ksiz, vsiz;
  int64_t i, cnt;
  double stime, etime;
  oprintf("<Memory-saving Hash Map Test>\n  rnum=%ld  rnd=%d  etc=%d  bnum=%ld\n\n",
          (long)rnum, rnd, etc, (long)bnum);
  err = FALSE;
  if (bnum < 0) bnum = 0;
  map = kcmapnew(bnum);
  oprintf("setting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    kcmapset(map, kbuf, ksiz, kbuf, ksiz);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kcmapcount(map));
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("adding records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      kcmapadd(map, kbuf, ksiz, kbuf, ksiz);
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    oprintf("count: %ld\n", (long)kcmapcount(map));
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("appending records:\n");
    stime = kctime();
    for (i = 1; !err && i <= rnum; i++) {
      ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
      kcmapappend(map, kbuf, ksiz, kbuf, ksiz);
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    oprintf("count: %ld\n", (long)kcmapcount(map));
    oprintf("time: %.3f\n", etime - stime);
  }
  oprintf("getting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    vbuf = kcmapget(map, kbuf, ksiz, &vsiz);
    if (vbuf) {
      if (vsiz < ksiz || memcmp(vbuf, kbuf, ksiz)) {
        eprintf("%s: kcmapget failed\n", g_progname);
        err = TRUE;
      }
    } else if (!rnd) {
      eprintf("%s: kcmapget failed\n", g_progname);
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kcmapcount(map));
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("traversing records:\n");
    stime = kctime();
    cnt = 0;
    iter = kcmapiterator(map);
    while (!err && (ikbuf = kcmapiterget(iter, &ksiz, &vbuf, &vsiz)) != NULL) {
      if (rnd) {
        ksiz = sprintf(kbuf, "%08ld", (long)myrand(rnum));
        switch (myrand(3)) {
          case 0: {
            kcmapremove(map, kbuf, ksiz);
            break;
          }
          case 1: {
            kcmapappend(map, kbuf, ksiz, kbuf, ksiz);
            break;
          }
        }
      }
      if (!kcmapitergetkey(iter, &ksiz)) {
        eprintf("%s: kcmapitergetkey failed\n", g_progname);
        err = TRUE;
      }
      if (!kcmapitergetvalue(iter, &vsiz)) {
        eprintf("%s: kcmapitergetvalue failed\n", g_progname);
        err = TRUE;
      }
      cnt++;
      if (rnum > 250 && cnt % (rnum / 250) == 0) {
        oputchar('.');
        if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)cnt);
      }
      kcmapiterstep(iter);
    }
    if (rnd) oprintf(" (end)\n");
    kcmapiterdel(iter);
    if (!rnd && cnt != (int64_t)kcmapcount(map)) {
      eprintf("%s: kcmapcount failed\n", g_progname);
      err = TRUE;
    }
    etime = kctime();
    oprintf("count: %ld\n", (long)kcmapcount(map));
    oprintf("time: %.3f\n", etime - stime);
  }
  if (etc) {
    oprintf("sorting records:\n");
    stime = kctime();
    cnt = 0;
    sort = kcmapsorter(map);
    while (!err && (ikbuf = kcmapsortget(sort, &ksiz, &vbuf, &vsiz)) != NULL) {
      if (!kcmapsortgetkey(sort, &ksiz)) {
        eprintf("%s: kcmapsortgetkey failed\n", g_progname);
        err = TRUE;
      }
      if (!kcmapsortgetvalue(sort, &vsiz)) {
        eprintf("%s: kcmapsortgetvalue failed\n", g_progname);
        err = TRUE;
      }
      cnt++;
      if (rnum > 250 && cnt % (rnum / 250) == 0) {
        oputchar('.');
        if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)cnt);
      }
      kcmapsortstep(sort);
    }
    if (rnd) oprintf(" (end)\n");
    kcmapsortdel(sort);
    if (!rnd && cnt != (int64_t)kcmapcount(map)) {
      eprintf("%s: kcmapcount failed\n", g_progname);
      err = TRUE;
    }
    etime = kctime();
    oprintf("count: %ld\n", (long)kcmapcount(map));
    oprintf("time: %.3f\n", etime - stime);
  }
  oprintf("removing records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    ksiz = sprintf(kbuf, "%08ld", (long)(rnd ? myrand(rnum) + 1 : i));
    if (!kcmapremove(map, kbuf, ksiz) && !rnd) {
      eprintf("%s: kcmapremove failed\n", g_progname);
      err = TRUE;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kcmapcount(map));
  oprintf("time: %.3f\n", etime - stime);
  kcmapdel(map);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


/* perform list command */
static int32_t proclist(int64_t rnum, int32_t rnd, int32_t etc) {
  KCLIST* list;
  int32_t err;
  char buf[RECBUFSIZ];
  size_t size;
  int64_t i, cnt;
  double stime, etime;
  oprintf("<Memory-saving Array List Test>\n  rnum=%ld  rnd=%d  etc=%d\n\n",
          (long)rnum, rnd, etc);
  err = FALSE;
  list = kclistnew();
  oprintf("setting records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    size = sprintf(buf, "%08ld", (long)i);
    if (rnd && myrand(2) == 0) {
      kclistunshift(list, buf, size);
    } else {
      kclistpush(list, buf, size);
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kclistcount(list));
  oprintf("time: %.3f\n", etime - stime);
  oprintf("getting records:\n");
  stime = kctime();
  cnt = kclistcount(list);
  for (i = 1; !err && i <= rnum; i++) {
    kclistget(list, rnd ? myrand(cnt) : i - 1, &size);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kclistcount(list));
  oprintf("time: %.3f\n", etime - stime);
  oprintf("removing records:\n");
  stime = kctime();
  for (i = 1; !err && i <= rnum; i++) {
    if (rnd && myrand(2) == 0) {
      kclistshift(list);
    } else {
      kclistpop(list);
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
    }
  }
  etime = kctime();
  oprintf("count: %ld\n", (long)kclistcount(list));
  oprintf("time: %.3f\n", etime - stime);
  if (etc) {
    oprintf("wicked testing:\n");
    stime = kctime();
    memset(buf, '*', sizeof(buf));
    for (i = 1; !err && i <= rnum; i++) {
      size = rnd ? (size_t)myrand(sizeof(buf)) : sizeof(buf);
      cnt = kclistcount(list);
      switch (rnd ? myrand(10) : i % 10) {
        case 0: {
          kclistpop(list);
          break;
        }
        case 1: {
          kclistunshift(list, buf, size);
          break;
        }
        case 2: {
          kclistshift(list);
          break;
        }
        case 3: {
          kclistinsert(list, buf, size, rnd && cnt > 0 ? myrand(cnt) : cnt / 2);
          break;
        }
        case 4: {
          if (cnt > 0) kclistremove(list, rnd ? myrand(cnt) : cnt / 2);
          break;
        }
        case 5: {
          if (cnt > 0) kclistget(list, rnd ? myrand(cnt) : cnt / 2, &size);
          break;
        }
        case 6: {
          if (rnd ? myrand(100) == 0 : i % 127 == 0) kclistclear(list);
          break;
        }
        default: {
          kclistpush(list, buf, size);
          break;
        }
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08ld)\n", (long)i);
      }
    }
    etime = kctime();
    oprintf("time: %.3f\n", etime - stime);
    oprintf("count: %ld\n", (long)kclistcount(list));
  }
  kclistdel(list);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}



/* END OF FILE */
