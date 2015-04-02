/*************************************************************************************************
 * The test cases of the utility functions
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


#include "cmdcommon.h"


// constants
const size_t LOCKSLOTNUM = 128;          // number of lock slots
const size_t FILEIOUNIT = 50;            // file I/O unit size


// global variables
const char* g_progname;                  // program name
uint32_t g_randseed;                     // random seed
int64_t g_memusage;                      // memory usage


// function prototypes
int main(int argc, char** argv);
static void usage();
static void errprint(int32_t line, const char* format, ...);
static void fileerrprint(kc::File* file, int32_t line, const char* func);
static void filemetaprint(kc::File* file);
static int32_t runmutex(int argc, char** argv);
static int32_t runcond(int argc, char** argv);
static int32_t runpara(int argc, char** argv);
static int32_t runfile(int argc, char** argv);
static int32_t runlhmap(int argc, char** argv);
static int32_t runthmap(int argc, char** argv);
static int32_t runtalist(int argc, char** argv);
static int32_t runmisc(int argc, char** argv);
static int32_t procmutex(int64_t rnum, int32_t thnum, double iv);
static int32_t proccond(int64_t rnum, int32_t thnum, double iv);
static int32_t procpara(int64_t rnum, int32_t thnum, double iv);
static int32_t procfile(const char* path, int64_t rnum, int32_t thnum, bool rnd, int64_t msiz);
static int32_t proclhmap(int64_t rnum, bool rnd, int64_t bnum);
static int32_t procthmap(int64_t rnum, bool rnd, int64_t bnum);
static int32_t proctalist(int64_t rnum, bool rnd);
static int32_t procmisc(int64_t rnum);


// main routine
int main(int argc, char** argv) {
  g_progname = argv[0];
  const char* ebuf = kc::getenv("KCRNDSEED");
  g_randseed = ebuf ? (uint32_t)kc::atoi(ebuf) : (uint32_t)(kc::time() * 1000);
  mysrand(g_randseed);
  g_memusage = memusage();
  kc::setstdiobin();
  if (argc < 2) usage();
  int32_t rv = 0;
  if (!std::strcmp(argv[1], "mutex")) {
    rv = runmutex(argc, argv);
  } else if (!std::strcmp(argv[1], "cond")) {
    rv = runcond(argc, argv);
  } else if (!std::strcmp(argv[1], "para")) {
    rv = runpara(argc, argv);
  } else if (!std::strcmp(argv[1], "file")) {
    rv = runfile(argc, argv);
  } else if (!std::strcmp(argv[1], "lhmap")) {
    rv = runlhmap(argc, argv);
  } else if (!std::strcmp(argv[1], "thmap")) {
    rv = runthmap(argc, argv);
  } else if (!std::strcmp(argv[1], "talist")) {
    rv = runtalist(argc, argv);
  } else if (!std::strcmp(argv[1], "misc")) {
    rv = runmisc(argc, argv);
  } else {
    usage();
  }
  if (rv != 0) {
    oprintf("FAILED: KCRNDSEED=%u PID=%ld", g_randseed, (long)kc::getpid());
    for (int32_t i = 0; i < argc; i++) {
      oprintf(" %s", argv[i]);
    }
    oprintf("\n\n");
  }
  return rv;
}


// print the usage and exit
static void usage() {
  eprintf("%s: test cases of the utility functions of Kyoto Cabinet\n", g_progname);
  eprintf("\n");
  eprintf("usage:\n");
  eprintf("  %s mutex [-th num] [-iv num] rnum\n", g_progname);
  eprintf("  %s para [-th num] [-iv num] rnum\n", g_progname);
  eprintf("  %s cond [-th num] [-iv num] rnum\n", g_progname);
  eprintf("  %s file [-th num] [-rnd] [-msiz num] path rnum\n", g_progname);
  eprintf("  %s lhmap [-rnd] [-bnum num] rnum\n", g_progname);
  eprintf("  %s thmap [-rnd] [-bnum num] rnum\n", g_progname);
  eprintf("  %s talist [-rnd] rnum\n", g_progname);
  eprintf("  %s misc rnum\n", g_progname);
  eprintf("\n");
  std::exit(1);
}


// print formatted error information string and flush the buffer
static void errprint(int32_t line, const char* format, ...) {
  std::string msg;
  kc::strprintf(&msg, "%s: %d: ", g_progname, line);
  va_list ap;
  va_start(ap, format);
  kc::vstrprintf(&msg, format, ap);
  va_end(ap);
  kc::strprintf(&msg, "\n");
  std::cout << msg;
  std::cout.flush();
}


// print error message of file
static void fileerrprint(kc::File* file, int32_t line, const char* func) {
  oprintf("%s: %d: %s: %s: %s\n", g_progname, line, func, file->path().c_str(), file->error());
}


// print members of file
static void filemetaprint(kc::File* file) {
  oprintf("size: %lld\n", (long long)file->size());
}


// parse arguments of mutex command
static int32_t runmutex(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  int32_t thnum = 1;
  double iv = 0.0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-th")) {
        if (++i >= argc) usage();
        thnum = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-iv")) {
        if (++i >= argc) usage();
        iv = kc::atof(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1 || thnum < 1) usage();
  if (thnum > THREADMAX) thnum = THREADMAX;
  int32_t rv = procmutex(rnum, thnum, iv);
  return rv;
}


// parse arguments of cond command
static int32_t runcond(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  int32_t thnum = 1;
  double iv = 0.0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-th")) {
        if (++i >= argc) usage();
        thnum = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-iv")) {
        if (++i >= argc) usage();
        iv = kc::atof(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1 || thnum < 1) usage();
  if (thnum > THREADMAX) thnum = THREADMAX;
  int32_t rv = proccond(rnum, thnum, iv);
  return rv;
}


// parse arguments of para command
static int32_t runpara(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  int32_t thnum = 1;
  double iv = 0.0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-th")) {
        if (++i >= argc) usage();
        thnum = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-iv")) {
        if (++i >= argc) usage();
        iv = kc::atof(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1 || thnum < 1) usage();
  if (thnum > THREADMAX) thnum = THREADMAX;
  int32_t rv = procpara(rnum, thnum, iv);
  return rv;
}


// parse arguments of file command
static int32_t runfile(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* rstr = NULL;
  int32_t thnum = 1;
  bool rnd = false;
  int64_t msiz = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-th")) {
        if (++i >= argc) usage();
        thnum = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-rnd")) {
        rnd = true;
      } else if (!std::strcmp(argv[i], "-msiz")) {
        if (++i >= argc) usage();
        msiz = kc::atoix(argv[i]);
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!rstr) {
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1 || thnum < 1 || msiz < 0) usage();
  if (thnum > THREADMAX) thnum = THREADMAX;
  int32_t rv = procfile(path, rnum, thnum, rnd, msiz);
  return rv;
}


// parse arguments of lhmap command
static int32_t runlhmap(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  bool rnd = false;
  int64_t bnum = -1;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-rnd")) {
        rnd = true;
      } else if (!std::strcmp(argv[i], "-bnum")) {
        if (++i >= argc) usage();
        bnum = kc::atoix(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1) usage();
  int32_t rv = proclhmap(rnum, rnd, bnum);
  return rv;
}


// parse arguments of thmap command
static int32_t runthmap(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  bool rnd = false;
  int64_t bnum = -1;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-rnd")) {
        rnd = true;
      } else if (!std::strcmp(argv[i], "-bnum")) {
        if (++i >= argc) usage();
        bnum = kc::atoix(argv[i]);
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1) usage();
  int32_t rv = procthmap(rnum, rnd, bnum);
  return rv;
}


// parse arguments of talist command
static int32_t runtalist(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  bool rnd = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-rnd")) {
        rnd = true;
      } else {
        usage();
      }
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1) usage();
  int32_t rv = proctalist(rnum, rnd);
  return rv;
}


// parse arguments of misc command
static int32_t runmisc(int argc, char** argv) {
  bool argbrk = false;
  const char* rstr = NULL;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else      usage();
    } else if (!rstr) {
      argbrk = true;
      rstr = argv[i];
    } else {
      usage();
    }
  }
  if (!rstr) usage();
  int64_t rnum = kc::atoix(rstr);
  if (rnum < 1) usage();
  int32_t rv = procmisc(rnum);
  return rv;
}


// perform mutex command
static int32_t procmutex(int64_t rnum, int32_t thnum, double iv) {
  oprintf("<Mutex Test>\n  seed=%u  rnum=%lld  thnum=%d  iv=%.3f\n\n",
          g_randseed, (long long)rnum, thnum, iv);
  bool err = false;
  kc::Mutex mutex;
  oprintf("mutex:\n");
  double stime = kc::time();
  class ThreadMutex : public kc::Thread {
   public:
    void setparams(int32_t id, kc::Mutex* mutex, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      mutex_ = mutex;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        mutex_->lock();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        mutex_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::Mutex* mutex_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadMutex threadmutexs[THREADMAX];
  if (thnum < 2) {
    threadmutexs[0].setparams(0, &mutex, rnum, thnum, iv);
    threadmutexs[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadmutexs[i].setparams(i, &mutex, rnum, thnum, iv);
      threadmutexs[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadmutexs[i].join();
    }
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SlottedMutex smutex(LOCKSLOTNUM);
  oprintf("slotted mutex:\n");
  stime = kc::time();
  class ThreadSlottedMutex : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedMutex* smutex,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      smutex_ = smutex;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        smutex_->lock(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        smutex_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedMutex* smutex_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedMutex threadsmutexs[THREADMAX];
  if (thnum < 2) {
    threadsmutexs[0].setparams(0, &smutex, rnum, thnum, iv);
    threadsmutexs[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadsmutexs[i].setparams(i, &smutex, rnum, thnum, iv);
      threadsmutexs[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadsmutexs[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SpinLock spinlock;
  oprintf("spin lock:\n");
  stime = kc::time();
  class ThreadSpinLock : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SpinLock* spinlock, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      spinlock_ = spinlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        spinlock_->lock();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        spinlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SpinLock* spinlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSpinLock threadspins[THREADMAX];
  if (thnum < 2) {
    threadspins[0].setparams(0, &spinlock, rnum, thnum, iv);
    threadspins[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadspins[i].setparams(i, &spinlock, rnum, thnum, iv);
      threadspins[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadspins[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SlottedSpinLock sspinlock(LOCKSLOTNUM);
  oprintf("slotted spin lock:\n");
  stime = kc::time();
  class ThreadSlottedSpinLock : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedSpinLock* sspinlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      sspinlock_ = sspinlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        sspinlock_->lock(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        sspinlock_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedSpinLock* sspinlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedSpinLock threadsspinlocks[THREADMAX];
  if (thnum < 2) {
    threadsspinlocks[0].setparams(0, &sspinlock, rnum, thnum, iv);
    threadsspinlocks[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadsspinlocks[i].setparams(i, &sspinlock, rnum, thnum, iv);
      threadsspinlocks[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadsspinlocks[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::RWLock rwlock;
  oprintf("reader-writer lock writer:\n");
  stime = kc::time();
  class ThreadRWLockWriter : public kc::Thread {
   public:
    void setparams(int32_t id, kc::RWLock* rwlock, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      rwlock_ = rwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        rwlock_->lock_writer();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        rwlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::RWLock* rwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadRWLockWriter threadrwlockwriters[THREADMAX];
  if (thnum < 2) {
    threadrwlockwriters[0].setparams(0, &rwlock, rnum, thnum, iv);
    threadrwlockwriters[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadrwlockwriters[i].setparams(i, &rwlock, rnum, thnum, iv);
      threadrwlockwriters[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadrwlockwriters[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("reader-writer lock reader:\n");
  stime = kc::time();
  class ThreadRWLockReader : public kc::Thread {
   public:
    void setparams(int32_t id, kc::RWLock* rwlock, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      rwlock_ = rwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        rwlock_->lock_reader();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        rwlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::RWLock* rwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadRWLockReader threadrwlockreaders[THREADMAX];
  if (thnum < 2) {
    threadrwlockreaders[0].setparams(0, &rwlock, rnum, thnum, iv);
    threadrwlockreaders[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadrwlockreaders[i].setparams(i, &rwlock, rnum, thnum, iv);
      threadrwlockreaders[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadrwlockreaders[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SlottedRWLock srwlock(LOCKSLOTNUM);
  oprintf("slotted reader-writer lock writer:\n");
  stime = kc::time();
  class ThreadSlottedRWLockWriter : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedRWLock* srwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      srwlock_ = srwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        srwlock_->lock_writer(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        srwlock_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedRWLock* srwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedRWLockWriter threadsrwlockwriters[THREADMAX];
  if (thnum < 2) {
    threadsrwlockwriters[0].setparams(0, &srwlock, rnum, thnum, iv);
    threadsrwlockwriters[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadsrwlockwriters[i].setparams(i, &srwlock, rnum, thnum, iv);
      threadsrwlockwriters[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadsrwlockwriters[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("slotted reader-writer lock reader:\n");
  stime = kc::time();
  class ThreadSlottedRWLockReader : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedRWLock* srwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      srwlock_ = srwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        srwlock_->lock_reader(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        srwlock_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedRWLock* srwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedRWLockReader threadsrwlockreaders[THREADMAX];
  if (thnum < 2) {
    threadsrwlockreaders[0].setparams(0, &srwlock, rnum, thnum, iv);
    threadsrwlockreaders[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadsrwlockreaders[i].setparams(i, &srwlock, rnum, thnum, iv);
      threadsrwlockreaders[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadsrwlockreaders[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SpinRWLock spinrwlock;
  oprintf("spin reader-writer lock writer:\n");
  stime = kc::time();
  class ThreadSpinRWLockWriter : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SpinRWLock* spinrwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      spinrwlock_ = spinrwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        spinrwlock_->lock_writer();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        spinrwlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SpinRWLock* spinrwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSpinRWLockWriter threadspinrwlockwriters[THREADMAX];
  if (thnum < 2) {
    threadspinrwlockwriters[0].setparams(0, &spinrwlock, rnum, thnum, iv);
    threadspinrwlockwriters[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockwriters[i].setparams(i, &spinrwlock, rnum, thnum, iv);
      threadspinrwlockwriters[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockwriters[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("spin reader-writer lock reader:\n");
  stime = kc::time();
  class ThreadSpinRWLockReader : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SpinRWLock* spinrwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      spinrwlock_ = spinrwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        spinrwlock_->lock_reader();
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        spinrwlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SpinRWLock* spinrwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSpinRWLockReader threadspinrwlockreaders[THREADMAX];
  if (thnum < 2) {
    threadspinrwlockreaders[0].setparams(0, &spinrwlock, rnum, thnum, iv);
    threadspinrwlockreaders[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockreaders[i].setparams(i, &spinrwlock, rnum, thnum, iv);
      threadspinrwlockreaders[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockreaders[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("spin reader-writer lock wicked:\n");
  stime = kc::time();
  class ThreadSpinRWLockWicked : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SpinRWLock* spinrwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      spinrwlock_ = spinrwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        if (i % 4 == 0) {
          spinrwlock_->lock_writer();
          if (i % 16 == 0) {
            if (iv_ > 0) {
              sleep(iv_);
            } else if (iv_ < 0) {
              yield();
            }
            spinrwlock_->demote();
          }
        } else {
          spinrwlock_->lock_reader();
          if (i % 7 == 0) {
            if (iv_ > 0) {
              sleep(iv_);
            } else if (iv_ < 0) {
              yield();
            }
            spinrwlock_->promote();
          }
        }
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        spinrwlock_->unlock();
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SpinRWLock* spinrwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSpinRWLockWicked threadspinrwlockwickeds[THREADMAX];
  if (thnum < 2) {
    threadspinrwlockwickeds[0].setparams(0, &spinrwlock, rnum, thnum, iv);
    threadspinrwlockwickeds[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockwickeds[i].setparams(i, &spinrwlock, rnum, thnum, iv);
      threadspinrwlockwickeds[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadspinrwlockwickeds[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::SlottedSpinRWLock ssrwlock(LOCKSLOTNUM);
  oprintf("slotted spin reader-writer lock writer:\n");
  stime = kc::time();
  class ThreadSlottedSpinRWLockWriter : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedSpinRWLock* ssrwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      ssrwlock_ = ssrwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        ssrwlock_->lock_writer(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        ssrwlock_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedSpinRWLock* ssrwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedSpinRWLockWriter threadssrwlockwriters[THREADMAX];
  if (thnum < 2) {
    threadssrwlockwriters[0].setparams(0, &ssrwlock, rnum, thnum, iv);
    threadssrwlockwriters[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadssrwlockwriters[i].setparams(i, &ssrwlock, rnum, thnum, iv);
      threadssrwlockwriters[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadssrwlockwriters[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("slotted spin reader-writer lock reader:\n");
  stime = kc::time();
  class ThreadSlottedSpinRWLockReader : public kc::Thread {
   public:
    void setparams(int32_t id, kc::SlottedSpinRWLock* ssrwlock,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      ssrwlock_ = ssrwlock;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        size_t idx = i % LOCKSLOTNUM;
        ssrwlock_->lock_reader(idx);
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        ssrwlock_->unlock(idx);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::SlottedSpinRWLock* ssrwlock_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadSlottedSpinRWLockReader threadssrwlockreaders[THREADMAX];
  if (thnum < 2) {
    threadssrwlockreaders[0].setparams(0, &ssrwlock, rnum, thnum, iv);
    threadssrwlockreaders[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadssrwlockreaders[i].setparams(i, &ssrwlock, rnum, thnum, iv);
      threadssrwlockreaders[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadssrwlockreaders[i].join();
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("atomic increment:\n");
  stime = kc::time();
  kc::AtomicInt64 anum;
  anum = rnum * thnum;
  class ThreadAtomic : public kc::Thread {
   public:
    void setparams(int32_t id, kc::AtomicInt64* anum, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      anum_ = anum;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        anum_->add(1);
        *anum_ += 1;
        *anum_ -= 1;
        while (true) {
          int64_t num = anum_->get();
          if (iv_ > 0) {
            sleep(iv_);
          } else if (iv_ < 0) {
            yield();
          }
          if (anum_->cas(num, num + 1)) break;
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::AtomicInt64* anum_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
  };
  ThreadAtomic threadatomic[THREADMAX];
  if (thnum < 2) {
    threadatomic[0].setparams(0, &anum, rnum, thnum, iv);
    threadatomic[0].run();
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadatomic[i].setparams(i, &anum, rnum, thnum, iv);
      threadatomic[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadatomic[i].join();
    }
  }
  if (anum.get() != rnum * thnum * 3) {
    errprint(__LINE__, "AtomicInt64::get: %lld", (long long)anum.get());
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform cond command
static int32_t proccond(int64_t rnum, int32_t thnum, double iv) {
  oprintf("<Mutex Test>\n  seed=%u  rnum=%lld  thnum=%d  iv=%.3f\n\n",
          g_randseed, (long long)rnum, thnum, iv);
  bool err = false;
  kc::Mutex mutex;
  kc::CondVar cond;
  oprintf("conditon variable:\n");
  double stime = kc::time();
  class ThreadCondVar : public kc::Thread {
   public:
    void setparams(int32_t id, kc::Mutex* mutex, kc::CondVar* cond,
                   int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      mutex_ = mutex;
      cond_ = cond;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
      active_ = 1;
    }
    bool active() {
      return active_ > 0;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        mutex_->lock();
        if (i % 2 < 1) {
          if (iv_ > 0) {
            sleep(iv_);
          } else if (iv_ < 0) {
            yield();
          }
        }
        if (i % 7 == 0) {
          cond_->wait(mutex_, 0.001);
        } else {
          cond_->wait(mutex_);
        }
        mutex_->unlock();
        if (i % 2 > 0) {
          if (iv_ > 0) {
            sleep(iv_);
          } else if (iv_ < 0) {
            yield();
          }
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
      active_ = 0;
    }
   private:
    int32_t id_;
    kc::Mutex* mutex_;
    kc::CondVar* cond_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
    kc::AtomicInt64 active_;
  };
  ThreadCondVar threadcondvars[THREADMAX];
  for (int32_t i = 0; i < thnum; i++) {
    threadcondvars[i].setparams(i, &mutex, &cond, rnum, thnum, iv);
    threadcondvars[i].start();
  }
  int64_t cnt = 0;
  while (true) {
    if (iv > 0) {
      kc::Thread::sleep(iv);
    } else if (iv < 0) {
      kc::Thread::yield();
    }
    int32_t actnum = 0;
    for (int32_t i = 0; i < thnum; i++) {
      if (threadcondvars[i].active()) actnum++;
      bool lock = (cnt + i) % 5 == 0;
      if (lock) mutex.lock();
      if (cnt % (thnum + 1) < 1) {
        cond.broadcast();
      } else {
        cond.signal();
      }
      if (lock) mutex.unlock();
    }
    if (actnum < 1) break;
    cnt++;
  }
  for (int32_t i = 0; i < thnum; i++) {
    threadcondvars[i].join();
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  kc::CondMap cmap;
  oprintf("conditon map:\n");
  stime = kc::time();
  class ThreadCondMap : public kc::Thread {
   public:
    void setparams(int32_t id, kc::CondMap* cmap, int64_t rnum, int32_t thnum, double iv) {
      id_ = id;
      cmap_ = cmap;
      rnum_ = rnum;
      thnum_ = thnum;
      iv_ = iv;
      active_ = 1;
    }
    bool active() {
      return active_ > 0;
    }
    void run() {
      for (int64_t i = 1; i <= rnum_; i++) {
        if (iv_ > 0) {
          sleep(iv_);
        } else if (iv_ < 0) {
          yield();
        }
        char kbuf[RECBUFSIZ];
        size_t ksiz = std::sprintf(kbuf, "%08d", (int)(i % thnum_));
        cmap_->wait(kbuf, ksiz, 0.001);
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
      active_ = 0;
    }
   private:
    int32_t id_;
    kc::CondMap* cmap_;
    int64_t rnum_;
    int32_t thnum_;
    double iv_;
    kc::AtomicInt64 active_;
  };
  ThreadCondMap threadcondmaps[THREADMAX];
  for (int32_t i = 0; i < thnum; i++) {
    threadcondmaps[i].setparams(i, &cmap, rnum, thnum, iv);
    threadcondmaps[i].start();
  }
  cnt = 0;
  while (true) {
    if (iv > 0) {
      kc::Thread::sleep(iv);
    } else if (iv < 0) {
      kc::Thread::yield();
    }
    int32_t actnum = 0;
    for (int32_t i = 0; i < thnum; i++) {
      if (threadcondmaps[i].active()) actnum++;
      char kbuf[RECBUFSIZ];
      size_t ksiz = std::sprintf(kbuf, "%08d", (int)i);
      bool lock = (cnt + i) % 5 == 0;
      if (lock) mutex.lock();
      if (cnt % (thnum + 1) < 1) {
        cmap.broadcast(kbuf, ksiz);
      } else {
        cmap.signal(kbuf, ksiz);
      }
      if (lock) mutex.unlock();
    }
    if (cnt % 1024 < 1) cmap.broadcast_all();
    if (actnum < 1) break;
    cnt++;
  }
  for (int32_t i = 0; i < thnum; i++) {
    threadcondmaps[i].join();
  }
  if (cmap.count() != 0) {
    errprint(__LINE__, "CondMap::count");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform para command
static int32_t procpara(int64_t rnum, int32_t thnum, double iv) {
  oprintf("<Parallel Test>\n  seed=%u  rnum=%lld  thnum=%d  iv=%.3f\n\n",
          g_randseed, (long long)rnum, thnum, iv);
  bool err = false;
  double stime = kc::time();
  class TaskQueueImpl : public kc::TaskQueue {
   public:
    void setparams(int32_t thnum, double iv) {
      thnum_ = thnum;
      iv_ = iv;
      cnt_ = 0;
    }
    void do_task(kc::TaskQueue::Task* task) {
      cnt_ += 1;
      if (iv_ > 0) {
        kc::Thread::sleep(iv_ * thnum_);
      } else if (iv_ < 0) {
        kc::Thread::yield();
      }
      delete task;
    }
    int64_t done_count() {
      return cnt_;
    }
   private:
    int32_t thnum_;
    double iv_;
    kc::AtomicInt64 cnt_;
  };
  TaskQueueImpl queue;
  queue.setparams(thnum, iv);
  queue.start(thnum);
  for (int64_t i = 1; i <= rnum; i++) {
    kc::TaskQueue::Task* task = new kc::TaskQueue::Task;
    queue.add_task(task);
    if (iv > 0) {
      kc::Thread::sleep(iv);
    } else if (iv < 0) {
      kc::Thread::yield();
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  oprintf("count: %lld\n", queue.count());
  oprintf("done: %lld\n", queue.done_count());
  queue.finish();
  if (queue.count() != 0) {
    errprint(__LINE__, "TaskQueue::count");
    err = true;
  }
  if (queue.done_count() != rnum) {
    errprint(__LINE__, "TaskQueueImpl::done_count");
    err = true;
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  int64_t musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform file command
static int32_t procfile(const char* path, int64_t rnum, int32_t thnum, bool rnd, int64_t msiz) {
  oprintf("<File Test>\n  seed=%u  path=%s  rnum=%lld  thnum=%d  rnd=%d  msiz=%lld\n\n",
          g_randseed, path, (long long)rnum, thnum, rnd, (long long)msiz);
  bool err = false;
  kc::File file;
  oprintf("opening the file:\n");
  double stime = kc::time();
  if (!file.open(path, kc::File::OWRITER | kc::File::OCREATE | kc::File::OTRUNCATE, msiz)) {
    fileerrprint(&file, __LINE__, "File::open");
    err = true;
  }
  double etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("writing:\n");
  class ThreadWrite : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum, bool rnd) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        if (rnd_ && myrand(2) == 0) {
          char rbuf[RECBUFSIZ];
          size_t rsiz = myrand(FILEIOUNIT);
          if (rsiz > 0) std::memset(rbuf, '*', rsiz);
          if (!file_->append(rbuf, rsiz)) {
            fileerrprint(file_, __LINE__, "File::append");
            err_ = true;
          }
        } else {
          int64_t num = rnd_ ? myrand(range) : base + i - 1;
          int64_t off = num * FILEIOUNIT;
          char rbuf[RECBUFSIZ];
          size_t rsiz = std::sprintf(rbuf, "[%048lld]", (long long)num);
          if (!file_->write(off, rbuf, rsiz)) {
            fileerrprint(file_, __LINE__, "File::write");
            err_ = true;
          }
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    bool err_;
  };
  ThreadWrite threadwrites[THREADMAX];
  if (thnum < 2) {
    threadwrites[0].setparams(0, &file, rnum, thnum, rnd);
    threadwrites[0].run();
    if (threadwrites[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadwrites[i].setparams(i, &file, rnum, thnum, rnd);
      threadwrites[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadwrites[i].join();
      if (threadwrites[i].error()) err = true;
    }
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("reading:\n");
  stime = kc::time();
  class ThreadRead : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum, bool rnd) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t size = file_->size();
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        int64_t num = rnd_ ? myrand(range) : base + i - 1;
        int64_t off = num * FILEIOUNIT;
        char rbuf[RECBUFSIZ];
        if (!file_->read(off, rbuf, FILEIOUNIT) && off + (int64_t)FILEIOUNIT < size) {
          fileerrprint(file_, __LINE__, "File::read");
          err_ = true;
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    bool err_;
  };
  ThreadRead threadreads[THREADMAX];
  if (thnum < 2) {
    threadreads[0].setparams(0, &file, rnum, thnum, rnd);
    threadreads[0].run();
    if (threadreads[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadreads[i].setparams(i, &file, rnum, thnum, rnd);
      threadreads[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadreads[i].join();
      if (threadreads[i].error()) err = true;
    }
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  if (rnd) {
    int64_t off = rnum * thnum * FILEIOUNIT;
    char rbuf[RECBUFSIZ];
    std::memset(rbuf, '@', FILEIOUNIT);
    if (!file.write(off, rbuf, FILEIOUNIT)) {
      fileerrprint(&file, __LINE__, "File::write");
      err = true;
    }
  }
  oprintf("fast writing:\n");
  stime = kc::time();
  class ThreadWriteFast : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum, bool rnd) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        int64_t num = rnd_ ? myrand(range) : base + i - 1;
        int64_t off = num * FILEIOUNIT;
        char rbuf[RECBUFSIZ];
        size_t rsiz = std::sprintf(rbuf, "[%048lld]", (long long)num);
        if (!file_->write_fast(off, rbuf, rsiz)) {
          fileerrprint(file_, __LINE__, "File::write_fast");
          err_ = true;
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    bool err_;
  };
  ThreadWriteFast threadwritefasts[THREADMAX];
  if (thnum < 2) {
    threadwritefasts[0].setparams(0, &file, rnum, thnum, rnd);
    threadwritefasts[0].run();
    if (threadwritefasts[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadwritefasts[i].setparams(i, &file, rnum, thnum, rnd);
      threadwritefasts[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadwritefasts[i].join();
      if (threadwritefasts[i].error()) err = true;
    }
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("fast reading:\n");
  stime = kc::time();
  class ThreadReadFast : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum, bool rnd) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        int64_t num = rnd_ ? myrand(range) : base + i - 1;
        int64_t off = num * FILEIOUNIT;
        char rbuf[RECBUFSIZ];
        if (!file_->read_fast(off, rbuf, FILEIOUNIT)) {
          fileerrprint(file_, __LINE__, "File::read_fast");
          err_ = true;
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    bool err_;
  };
  ThreadReadFast threadreadfasts[THREADMAX];
  if (thnum < 2) {
    threadreadfasts[0].setparams(0, &file, rnum, thnum, rnd);
    threadreadfasts[0].run();
    if (threadreadfasts[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadreadfasts[i].setparams(i, &file, rnum, thnum, rnd);
      threadreadfasts[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadreadfasts[i].join();
      if (threadreadfasts[i].error()) err = true;
    }
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("committing transaction:\n");
  stime = kc::time();
  int64_t qsiz = file.size() / 4;
  if (!file.begin_transaction(rnd ? myrand(100) == 0 : false, qsiz)) {
    fileerrprint(&file, __LINE__, "File::begin_transaction");
    err = true;
  }
  if (!file.write_transaction(0, qsiz)) {
    fileerrprint(&file, __LINE__, "File::write_transaction");
    err = true;
  }
  int64_t fsiz = rnd ? myrand(rnum * thnum * FILEIOUNIT) : rnum * thnum / 2 * FILEIOUNIT + 5;
  if (!file.truncate(fsiz)) {
    fileerrprint(&file, __LINE__, "File::truncate");
    err = true;
  }
  if (file.size() != fsiz) {
    fileerrprint(&file, __LINE__, "File::truncate");
    err = true;
  }
  class ThreadCommit : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum,
                   bool rnd, int64_t fsiz) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      fsiz_ = fsiz;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        int64_t num = rnd_ ? myrand(range) : base + i - 1;
        int64_t off = num * FILEIOUNIT;
        char rbuf[RECBUFSIZ];
        size_t rsiz = std::sprintf(rbuf, "[%048lld]", (long long)num);
        if (i % 2 == 0 || off > fsiz_ - (int64_t)FILEIOUNIT) {
          if (!file_->write(off, rbuf, rsiz)) {
            fileerrprint(file_, __LINE__, "File::write");
            err_ = true;
          }
        } else {
          if (!file_->write_fast(off, rbuf, rsiz)) {
            fileerrprint(file_, __LINE__, "File::write_fast");
            err_ = true;
          }
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    int64_t fsiz_;
    bool err_;
  };
  ThreadCommit threadcommits[THREADMAX];
  if (thnum < 2) {
    threadcommits[0].setparams(0, &file, rnum, thnum, rnd, fsiz);
    threadcommits[0].run();
    if (threadcommits[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadcommits[i].setparams(i, &file, rnum, thnum, rnd, fsiz);
      threadcommits[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadcommits[i].join();
      if (threadcommits[i].error()) err = true;
    }
  }
  if (!file.end_transaction(true)) {
    fileerrprint(&file, __LINE__, "File::end_transaction");
    err = true;
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("aborting transaction:\n");
  stime = kc::time();
  qsiz = file.size() / 4;
  if (!file.begin_transaction(rnd ? myrand(100) == 0 : false, qsiz)) {
    fileerrprint(&file, __LINE__, "File::begin_transaction");
    err = true;
  }
  if (!file.write_transaction(0, qsiz)) {
    fileerrprint(&file, __LINE__, "File::write_transaction");
    err = true;
  }
  int64_t osiz = file.size();
  kc::StringTreeMap chkmap;
  int64_t chknum = rnum / 100 + 1;
  for (int64_t i = 0; i < chknum; i++) {
    char rbuf[RECBUFSIZ];
    int64_t roff = myrand(osiz);
    int32_t rsiz = myrand(RECBUFSIZ);
    if (file.read(roff, rbuf, rsiz)) {
      std::string key = kc::strprintf("%lld:%d", (long long)roff, rsiz);
      chkmap[key] = std::string(rbuf, rsiz);
    }
  }
  fsiz = rnd ? myrand(rnum * thnum * FILEIOUNIT) : rnum * thnum / 2 * FILEIOUNIT + 5;
  if (!file.truncate(fsiz)) {
    fileerrprint(&file, __LINE__, "File::truncate");
    err = true;
  }
  if (file.size() != fsiz) {
    fileerrprint(&file, __LINE__, "File::truncate");
    err = true;
  }
  class ThreadAbort : public kc::Thread {
   public:
    void setparams(int32_t id, kc::File* file, int64_t rnum, int32_t thnum,
                   bool rnd, int64_t fsiz) {
      id_ = id;
      file_ = file;
      rnum_ = rnum;
      thnum_ = thnum;
      rnd_ = rnd;
      fsiz_ = fsiz;
      err_ = false;
    }
    bool error() {
      return err_;
    }
    void run() {
      int64_t base = id_ * rnum_;
      int64_t range = rnum_ * thnum_;
      for (int64_t i = 1; !err_ && i <= rnum_; i++) {
        int64_t num = rnd_ ? myrand(range) : base + i - 1;
        int64_t off = num * FILEIOUNIT;
        char rbuf[RECBUFSIZ];
        std::memset(rbuf, num, FILEIOUNIT);
        if (i % 2 == 0 || off > fsiz_ - (int64_t)FILEIOUNIT) {
          if (!file_->write(off, rbuf, FILEIOUNIT)) {
            fileerrprint(file_, __LINE__, "File::write");
            err_ = true;
          }
        } else {
          if (!file_->write_fast(off, rbuf, FILEIOUNIT)) {
            fileerrprint(file_, __LINE__, "File::write_fast");
            err_ = true;
          }
        }
        if (id_ < 1 && rnum_ > 250 && i % (rnum_ / 250) == 0) {
          oputchar('.');
          if (i == rnum_ || i % (rnum_ / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
        }
      }
    }
   private:
    int32_t id_;
    kc::File* file_;
    int64_t rnum_;
    int32_t thnum_;
    bool rnd_;
    int64_t fsiz_;
    bool err_;
  };
  ThreadAbort threadaborts[THREADMAX];
  if (thnum < 2) {
    threadaborts[0].setparams(0, &file, rnum, thnum, rnd, fsiz);
    threadaborts[0].run();
    if (threadaborts[0].error()) err = true;
  } else {
    for (int32_t i = 0; i < thnum; i++) {
      threadaborts[i].setparams(i, &file, rnum, thnum, rnd, fsiz);
      threadaborts[i].start();
    }
    for (int32_t i = 0; i < thnum; i++) {
      threadaborts[i].join();
      if (threadaborts[i].error()) err = true;
    }
  }
  if (!file.end_transaction(false)) {
    fileerrprint(&file, __LINE__, "File::end_transaction");
    err = true;
  }
  if (file.size() != osiz) {
    fileerrprint(&file, __LINE__, "File::end_transaction");
    err = true;
  }
  for (kc::StringTreeMap::iterator it = chkmap.begin(); it != chkmap.end(); ++it) {
    const char* key = it->first.c_str();
    int64_t roff = kc::atoi(key);
    int32_t rsiz = kc::atoi(std::strchr(key, ':') + 1);
    char rbuf[RECBUFSIZ];
    if (file.read(roff, rbuf, rsiz)) {
      if (it->second != std::string(rbuf, rsiz)) {
        fileerrprint(&file, __LINE__, "File::end_transaction");
        err = true;
      }
    } else {
      fileerrprint(&file, __LINE__, "File::end_transaction");
      err = true;
    }
  }
  etime = kc::time();
  filemetaprint(&file);
  oprintf("time: %.3f\n", etime - stime);
  oprintf("closing the file:\n");
  stime = kc::time();
  if (!file.close()) {
    fileerrprint(&file, __LINE__, "File::close");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("testing file utility functions:\n");
  stime = kc::time();
  std::string ostr = "_";
  for (int32_t i = 0; i < 100; i++) {
    ostr.append(path);
  }
  ostr.append("_");
  if (!kc::File::write_file(path, ostr.c_str(), ostr.size())) {
    errprint(__LINE__, "File::write_file");
    err = true;
  }
  int64_t isiz;
  char* ibuf = kc::File::read_file(path, &isiz);
  if (ibuf) {
    if (ostr != ibuf) {
      errprint(__LINE__, "File::read_file");
      err = true;
    }
    delete[] ibuf;
  } else {
    errprint(__LINE__, "File::read_file");
    err = true;
  }
  kc::File::Status sbuf;
  if (!kc::File::status(path, &sbuf) || sbuf.isdir || sbuf.size < 1) {
    errprint(__LINE__, "File::status");
    err = true;
  }
  if (!kc::File::status(kc::File::CDIRSTR, &sbuf) || !sbuf.isdir) {
    errprint(__LINE__, "File::status");
    err = true;
  }
  const std::string& abspath = kc::File::absolute_path(path);
  if (abspath.empty()) {
    errprint(__LINE__, "File::absolute_path");
    err = true;
  }
  const std::string& tmppath = kc::strprintf("%s%ctmp", path, kc::File::EXTCHR);
  if (!kc::File::rename(path, tmppath) || !kc::File::rename(tmppath, path)) {
    errprint(__LINE__, "File::rename");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("testing directory utility functions:\n");
  stime = kc::time();
  std::vector<std::string> files;
  if (!kc::File::read_directory(kc::File::CDIRSTR, &files)) {
    errprint(__LINE__, "File::read_directory");
    err = true;
  }
  if (!kc::File::make_directory(tmppath)) {
    errprint(__LINE__, "File::make_directory");
    err = true;
  }
  if (!kc::File::remove_directory(tmppath)) {
    errprint(__LINE__, "File::remove_directory");
    err = true;
  }
  if (!kc::File::make_directory(tmppath)) {
    errprint(__LINE__, "File::make_directory");
    err = true;
  }
  const std::string chldpath = tmppath + kc::File::PATHCHR + "tmp";
  if (!kc::File::write_file(chldpath, tmppath.c_str(), tmppath.size())) {
    errprint(__LINE__, "File::write_file");
    err = true;
  }
  if (!kc::File::remove_recursively(tmppath)) {
    errprint(__LINE__, "File::make_recursively");
    err = true;
  }
  const std::string& cwdpath = kc::File::get_current_directory();
  if (cwdpath.empty()) {
    errprint(__LINE__, "File::get_current_directory");
    err = true;
  }
  if (!kc::File::set_current_directory(cwdpath)) {
    errprint(__LINE__, "File::set_current_directory");
    err = true;
  }
  kc::DirStream dir;
  if (!dir.open(cwdpath)) {
    errprint(__LINE__, "DirStream::open");
    err = true;
  }
  std::string cpath;
  while (dir.read(&cpath)) {
    if (!kc::File::status(cpath, &sbuf)) {
      errprint(__LINE__, "File::status");
      err = true;
    }
  }
  if (!dir.close()) {
    errprint(__LINE__, "DirStream::close");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform lhmap command
static int32_t proclhmap(int64_t rnum, bool rnd, int64_t bnum) {
  oprintf("<Doubly-linked Hash Map Test>\n  seed=%u  rnum=%lld  rnd=%d  bnum=%lld\n\n",
          g_randseed, (long long)rnum, rnd, (long long)bnum);
  bool err = false;
  if (bnum < 0) bnum = 0;
  typedef kc::LinkedHashMap<std::string, std::string> Map;
  Map map(bnum);
  oprintf("setting records:\n");
  double stime = kc::time();
  for (int64_t i = 1; i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    map.set(kbuf, kbuf, Map::MCURRENT);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  int64_t musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("getting records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    Map::MoveMode mode = Map::MCURRENT;
    if (rnd) {
      switch (myrand(4)) {
        case 0: mode = Map::MFIRST;
        case 1: mode = Map::MLAST;
      }
    }
    if (!map.get(kbuf, mode) && !rnd) {
      errprint(__LINE__, "LinkedHashMap::get: %s", kbuf);
      err = true;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("traversing records:\n");
  stime = kc::time();
  int64_t cnt = 0;
  for (Map::Iterator it = map.begin(); !err && it != map.end(); ++it) {
    cnt++;
    if (it.key() != it.value()) {
      errprint(__LINE__, "LinkedHashMap::Iterator::key");
      err = true;
    }
    if (rnum > 250 && cnt % (rnum / 250) == 0) {
      oputchar('.');
      if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)cnt);
    }
  }
  if (rnd) oprintf(" (end)\n");
  if (cnt != (int64_t)map.count()) {
    errprint(__LINE__, "LinkedHashMap::count");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  Map paramap(bnum + 31);
  oprintf("migrating records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    Map::MoveMode mode = Map::MCURRENT;
    if (rnd) {
      switch (myrand(4)) {
        case 0: mode = Map::MFIRST;
        case 1: mode = Map::MLAST;
      }
    }
    if (!map.migrate(kbuf, &paramap, mode) && !rnd) {
      errprint(__LINE__, "LinkedHashMap::migrate: %s", kbuf);
      err = true;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld,%lld\n", (long long)map.count(), (long long)paramap.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("removing records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    if (!paramap.remove(kbuf) && !rnd) {
      errprint(__LINE__, "LinkedHashMap::remove: %s", kbuf);
      err = true;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld,%lld\n", (long long)map.count(), (long long)paramap.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  if (rnd) {
    oprintf("wicked testing:\n");
    stime = kc::time();
    for (int64_t i = 1; !err && i <= rnum; i++) {
      char kbuf[RECBUFSIZ];
      std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
      Map::MoveMode mode = Map::MCURRENT;
      if (rnd) {
        switch (myrand(4)) {
          case 0: mode = Map::MFIRST;
          case 1: mode = Map::MLAST;
        }
      }
      Map *ptr = &map;
      Map *paraptr = &paramap;
      if (myrand(2) == 0) {
        ptr = &paramap;
        paraptr = &map;
      }
      switch (myrand(4)) {
        case 0: {
          ptr->set(kbuf, kbuf, mode);
          break;
        }
        case 1: {
          ptr->get(kbuf, mode);
          break;
        }
        case 2: {
          ptr->remove(kbuf);
          break;
        }
        case 3: {
          ptr->migrate(kbuf, paraptr, mode);
          break;
        }
      }
      if (myrand(rnum * 2 + 1) == 0) ptr->clear();
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
      }
    }
    cnt = 0;
    for (Map::Iterator it = map.begin(); !err && it != map.end(); ++it) {
      cnt++;
      if (it.key() != it.value()) {
        errprint(__LINE__, "LinkedHashMap::Iterator::key");
        err = true;
      }
      if (rnum > 250 && cnt % (rnum / 250) == 0) {
        oputchar('.');
        if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)cnt);
      }
    }
    if (rnd) oprintf(" (end)\n");
    if (cnt != (int64_t)map.count()) {
      errprint(__LINE__, "LinkedHashMap::count");
      err = true;
    }
    cnt = 0;
    Map::Iterator it = map.end();
    while (!err && it != map.begin()) {
      --it;
      cnt++;
      if (it.key() != it.value()) {
        errprint(__LINE__, "LinkedHashMap::Iterator::key");
        err = true;
      }
      if (rnum > 250 && cnt % (rnum / 250) == 0) {
        oputchar('.');
        if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)cnt);
      }
    }
    if (rnd) oprintf(" (end)\n");
    if (cnt != (int64_t)map.count()) {
      errprint(__LINE__, "LinkedHashMap::count");
      err = true;
    }
    etime = kc::time();
    oprintf("time: %.3f\n", etime - stime);
    oprintf("count: %lld\n", (long long)map.count());
    musage = memusage();
    if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  }
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform thmap command
static int32_t procthmap(int64_t rnum, bool rnd, int64_t bnum) {
  oprintf("<Memory-saving Hash Map Test>\n  seed=%u  rnum=%lld  rnd=%d  bnum=%lld\n\n",
          g_randseed, (long long)rnum, rnd, (long long)bnum);
  bool err = false;
  if (bnum < 0) bnum = 0;
  kc::TinyHashMap map(bnum);
  oprintf("setting records:\n");
  double stime = kc::time();
  for (int64_t i = 1; i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    size_t ksiz = std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    map.set(kbuf, ksiz, kbuf, ksiz);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  int64_t musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("getting records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    size_t ksiz = std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    size_t vsiz;
    const char* vbuf = map.get(kbuf, ksiz, &vsiz);
    if (!vbuf && !rnd) {
      errprint(__LINE__, "TinyHashMap::get: %s", kbuf);
      err = true;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("appending records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    size_t ksiz = std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    map.append(kbuf, ksiz, kbuf, ksiz);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("traversing records:\n");
  stime = kc::time();
  int64_t cnt = 0;
  kc::TinyHashMap::Iterator it(&map);
  const char* kbuf, *vbuf;
  size_t ksiz, vsiz;
  while ((kbuf = it.get(&ksiz, &vbuf, &vsiz)) != NULL) {
    cnt++;
    it.step();
    if (rnum > 250 && cnt % (rnum / 250) == 0) {
      oputchar('.');
      if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)cnt);
    }
  }
  if (rnd) oprintf(" (end)\n");
  if (cnt != (int64_t)map.count()) {
    errprint(__LINE__, "TinyHashMap::count");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("sorting records:\n");
  stime = kc::time();
  cnt = 0;
  kc::TinyHashMap::Sorter sorter(&map);
  while ((kbuf = sorter.get(&ksiz, &vbuf, &vsiz)) != NULL) {
    cnt++;
    sorter.step();
    if (rnum > 250 && cnt % (rnum / 250) == 0) {
      oputchar('.');
      if (cnt == rnum || cnt % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)cnt);
    }
  }
  if (rnd) oprintf(" (end)\n");
  if (cnt != (int64_t)map.count()) {
    errprint(__LINE__, "TinyHashMap::count");
    err = true;
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("removing records:\n");
  stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    char kbuf[RECBUFSIZ];
    size_t ksiz = std::sprintf(kbuf, "%08lld", (long long)(rnd ? myrand(rnum) + 1 : i));
    if (!map.remove(kbuf, ksiz) && !rnd) {
      errprint(__LINE__, "TinyHashMap::remove: %s", kbuf);
      err = true;
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)map.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  if (rnd) {
    oprintf("wicked testing:\n");
    stime = kc::time();
    char lbuf[RECBUFSIZL];
    std::memset(lbuf, '*', sizeof(lbuf));
    for (int64_t i = 1; !err && i <= rnum; i++) {
      char kbuf[RECBUFSIZ];
      size_t ksiz = std::sprintf(kbuf, "%lld", (long long)(myrand(rnum) + 1));
      size_t vsiz = myrand(sizeof(lbuf));
      switch (myrand(6)) {
        case 0: {
          map.set(kbuf, ksiz, lbuf, vsiz);
          break;
        }
        case 1: {
          map.add(kbuf, ksiz, lbuf, vsiz);
          break;
        }
        case 2: {
          map.replace(kbuf, ksiz, lbuf, vsiz);
          break;
        }
        case 3: {
          map.append(kbuf, ksiz, lbuf, vsiz);
          break;
        }
        case 6: {
          map.remove(kbuf, ksiz);
          break;
        }
        default: {
          map.get(kbuf, ksiz, &vsiz);
          break;
        }
      }
      if (myrand(rnum * 2 + 1) == 0) map.clear();
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
      }
    }
    etime = kc::time();
    oprintf("time: %.3f\n", etime - stime);
    oprintf("count: %lld\n", (long long)map.count());
    musage = memusage();
    if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  }
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform talist command
static int32_t proctalist(int64_t rnum, bool rnd) {
  oprintf("<Memory-saving Array List Test>\n  seed=%u  rnum=%lld  rnd=%d\n\n",
          g_randseed, (long long)rnum, rnd);
  bool err = false;
  kc::TinyArrayList list;
  oprintf("setting records:\n");
  double stime = kc::time();
  for (int64_t i = 1; i <= rnum; i++) {
    char buf[RECBUFSIZ];
    size_t size = std::sprintf(buf, "%08lld", (long long)i);
    if (rnd && myrand(2) == 0) {
      list.unshift(buf, size);
    } else {
      list.push(buf, size);
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  double etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)list.count());
  int64_t musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("getting records:\n");
  stime = kc::time();
  size_t cnt = list.count();
  for (int64_t i = 1; i <= rnum; i++) {
    size_t size;
    list.get(rnd ? myrand(cnt) : i - 1, &size);
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)list.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  oprintf("removing records:\n");
  stime = kc::time();
  for (int64_t i = 1; i <= rnum; i++) {
    if (rnd && myrand(2) == 0) {
      list.shift();
    } else {
      list.pop();
    }
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  etime = kc::time();
  oprintf("time: %.3f\n", etime - stime);
  oprintf("count: %lld\n", (long long)list.count());
  musage = memusage();
  if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  if (rnd) {
    oprintf("wicked testing:\n");
    stime = kc::time();
    char lbuf[RECBUFSIZL];
    std::memset(lbuf, '*', sizeof(lbuf));
    for (int64_t i = 1; !err && i <= rnum; i++) {
      size_t size = myrand(sizeof(lbuf));
      cnt = list.count();
      switch (myrand(10)) {
        case 0: {
          list.pop();
          break;
        }
        case 1: {
          list.unshift(lbuf, size);
          break;
        }
        case 2: {
          list.shift();
          break;
        }
        case 3: {
          list.insert(lbuf, size, cnt > 0 ? myrand(cnt) : 0);
          break;
        }
        case 4: {
          if (cnt > 0) list.remove(myrand(cnt));
          break;
        }
        case 5: {
          if (cnt > 0) list.get(myrand(cnt), &size);
          break;
        }
        case 6: {
          if (myrand(100) == 0) list.clear();
          break;
        }
        default: {
          list.push(lbuf, size);
          break;
        }
      }
      if (rnum > 250 && i % (rnum / 250) == 0) {
        oputchar('.');
        if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
      }
    }
    etime = kc::time();
    oprintf("time: %.3f\n", etime - stime);
    oprintf("count: %lld\n", (long long)list.count());
    musage = memusage();
    if (musage > 0) oprintf("memory: %lld\n", (long long)(musage - g_memusage));
  }
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}


// perform misc command
static int32_t procmisc(int64_t rnum) {
  oprintf("<Miscellaneous Test>\n  seed=%u  rnum=%lld\n\n", g_randseed, (long long)rnum);
  bool err = false;
  if (!kc::_dummytest()) {
    errprint(__LINE__, "_dummytest");
    err = true;
  }
  double stime = kc::time();
  for (int64_t i = 1; !err && i <= rnum; i++) {
    uint16_t num16 = (1ULL << myrand(sizeof(num16) * 8)) - 5 + myrand(10);
    uint32_t num32 = (1ULL << myrand(sizeof(num32) * 8)) - 5 + myrand(10);
    uint64_t num64 = (1ULL << myrand(sizeof(num64) * 8)) - 5 + myrand(10);
    if (kc::ntoh16(kc::hton16(num16)) != num16) {
      errprint(__LINE__, "ntoh16: %llu", (unsigned long long)num16);
      err = true;
    }
    if (kc::ntoh32(kc::hton32(num32)) != num32) {
      errprint(__LINE__, "ntoh32: %llu", (unsigned long long)num32);
      err = true;
    }
    if (kc::ntoh64(kc::hton64(num64)) != num64) {
      errprint(__LINE__, "ntoh64: %llu", (unsigned long long)num64);
      err = true;
    }
    char fbuf[sizeof(num64)];
    num64 = (uint64_t)myrand(kc::INT32MAX) * myrand(kc::INT16MAX);
    kc::writefixnum(fbuf, num64, 6);
    uint64_t onum = kc::readfixnum(fbuf, 6);
    if (onum != num64) {
      errprint(__LINE__, "readfixnum: %llu:%llu",
               (unsigned long long)num64, (unsigned long long)onum);
      err = true;
    }
    unsigned char ubuf[RECBUFSIZ];
    unsigned char* uwp = ubuf;
    if (kc::writevarnum(uwp, num32) != kc::sizevarnum(num32)) {
      errprint(__LINE__, "sizevarnum: %llu", (unsigned long long)num32);
      err = true;
    }
    uwp += kc::writevarnum(uwp, num16);
    uwp += kc::writevarnum(uwp, num32);
    uwp += kc::writevarnum(uwp, num64);
    const unsigned char* urp = ubuf;
    urp += kc::readvarnum(urp, uwp - urp, &onum);
    if (onum != num16) {
      errprint(__LINE__, "readvarnum: %llu:%llu",
               (unsigned long long)num16, (unsigned long long)onum);
      err = true;
    }
    urp += kc::readvarnum(urp, uwp - urp, &onum);
    if (onum != num32) {
      errprint(__LINE__, "readvarnum: %llu:%llu",
               (unsigned long long)num32, (unsigned long long)onum);
      err = true;
    }
    urp += kc::readvarnum(urp, uwp - urp, &onum);
    if (onum != num64) {
      errprint(__LINE__, "readvarnum: %llu:%llu",
               (unsigned long long)num64, (unsigned long long)onum);
      err = true;
    }
    if (urp != uwp) {
      errprint(__LINE__, "readvarnum: %d", (int)(uwp - urp));
      err = true;
    }
    size_t usiz = urp - ubuf;
    uint64_t hash = kc::hashmurmur(&num16, sizeof(num16)) + kc::hashmurmur(ubuf, usiz);
    hash += kc::hashfnv(&num16, sizeof(num16)) + kc::hashfnv(ubuf, usiz);
    char name[kc::NUMBUFSIZ];
    hash += kc::hashpath(ubuf, usiz, name);
    hash = kc::nearbyprime(myrand(kc::INT32MAX));
    if (myrand(256) == 0) {
      int32_t unum = myrand(64);
      std::vector<uint32_t> oucs;
      for (int32_t j = 0; j < unum; j++) {
        uint32_t c = std::pow(2, myrand(31000000) / 1000000.0);
        oucs.push_back(c);
      }
      std::string utf;
      kc::strucstoutf(oucs, &utf);
      std::vector<uint32_t> nucs;
      kc::strutftoucs(utf, &nucs);
      if (nucs.size() == oucs.size()) {
        for (int32_t j = 0; j < (int32_t)nucs.size(); j++) {
          if (nucs[j] != oucs[j]) {
            errprint(__LINE__, "strutftoucs: %d:%d", (int)nucs[j], (int)oucs[j]);
            err = true;
            break;
          }
        }
      } else {
        errprint(__LINE__, "strutftoucs: %d:%d", (int)nucs.size(), (int)oucs.size());
        err = true;
      }
      uint32_t* cucs = new uint32_t[utf.size()+1];
      size_t cucsnum;
      if (myrand(2) == 0) {
        kc::strutftoucs(utf.c_str(), cucs, &cucsnum);
      } else {
        kc::strutftoucs(utf.data(), utf.size(), cucs, &cucsnum);
      }
      if (cucsnum == oucs.size()) {
        char* cutf = new char[cucsnum*6+1];
        kc::strucstoutf(cucs, cucsnum, cutf);
        if (std::strcmp(cutf, utf.c_str())) {
          errprint(__LINE__, "strucstoutf");
          err = true;
        }
        delete[] cutf;
      } else {
        errprint(__LINE__, "strutftoucs");
        err = true;
      }
      delete[] cucs;
      int32_t tnum = myrand(64);
      std::vector<std::string> ovec;
      std::map<std::string, std::string> omap;
      for (int32_t j = 0; j < tnum; j++) {
        char kbuf[RECBUFSIZ];
        std::sprintf(kbuf, "%lld", (long long)myrand(rnum));
        char vbuf[RECBUFSIZ];
        std::sprintf(vbuf, "%lld", (long long)myrand(rnum));
        ovec.push_back(vbuf);
        omap[kbuf] = vbuf;
      }
      std::string vstr;
      kc::strvecdump(ovec, &vstr);
      std::vector<std::string> nvec;
      kc::strvecload(vstr, &nvec);
      if (nvec.size() != ovec.size()) {
        errprint(__LINE__, "strvecload: %d:%d", (int)nvec.size(), (int)ovec.size());
        err = true;
      }
      std::string mstr;
      kc::strmapdump(omap, &mstr);
      std::map<std::string, std::string> nmap;
      kc::strmapload(mstr, &nmap);
      if (nmap.size() != omap.size()) {
        errprint(__LINE__, "strmapload: %d:%d", (int)nvec.size(), (int)ovec.size());
        err = true;
      }
    }
    char* ebuf = kc::hexencode(ubuf, usiz);
    size_t osiz;
    char* obuf = kc::hexdecode(ebuf, &osiz);
    if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
      errprint(__LINE__, "hexencode: %d:%d", (int)osiz, (int)usiz);
      err = true;
    }
    delete[] obuf;
    delete[] ebuf;
    ebuf = kc::urlencode(ubuf, usiz);
    obuf = kc::urldecode(ebuf, &osiz);
    if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
      errprint(__LINE__, "urlencode: %d:%d", (int)osiz, (int)usiz);
      err = true;
    }
    delete[] obuf;
    delete[] ebuf;
    ebuf = kc::quoteencode(ubuf, usiz);
    obuf = kc::quotedecode(ebuf, &osiz);
    if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
      errprint(__LINE__, "quoteencode: %d:%d", (int)osiz, (int)usiz);
      err = true;
    }
    delete[] obuf;
    delete[] ebuf;
    ebuf = kc::baseencode(ubuf, usiz);
    obuf = kc::basedecode(ebuf, &osiz);
    if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
      errprint(__LINE__, "baseencode: %d:%d", (int)osiz, (int)usiz);
      err = true;
    }
    delete[] obuf;
    delete[] ebuf;
    size_t nsiz = std::strlen(name);
    nsiz -= i % nsiz;
    ebuf = new char[usiz];
    kc::arccipher(ubuf, usiz, name, nsiz, ebuf);
    obuf = new char[usiz];
    kc::arccipher(ebuf, usiz, name, nsiz, obuf);
    if (std::memcmp(obuf, ubuf, usiz)) {
      errprint(__LINE__, "arccipher: %s", name);
      err = true;
    }
    if (kc::memicmp(obuf, ubuf, usiz)) {
      errprint(__LINE__, "memicmp");
      err = true;
    }
    if (!kc::memmem(obuf, osiz, ubuf, usiz)) {
      errprint(__LINE__, "memmem");
      err = true;
    }
    if (!kc::memimem(obuf, osiz, ubuf, usiz)) {
      errprint(__LINE__, "memimem");
      err = true;
    }
    if (kc::memdist(obuf, osiz, ubuf, usiz)) {
      errprint(__LINE__, "memdist");
      err = true;
    }
    delete[] obuf;
    delete[] ebuf;
    ebuf = kc::memdup((char*)ubuf, usiz);
    ebuf[usiz] = '\0';
    obuf = kc::strdup(ebuf);
    switch (myrand(18)) {
      case 0: kc::atoi(obuf); break;
      case 1: kc::atoix(obuf); break;
      case 2: kc::atoih(obuf); break;
      case 3: kc::atoin((char*)ubuf, usiz); break;
      case 4: kc::atof(obuf); break;
      case 5: kc::atofn((char*)ubuf, usiz); break;
      case 6: kc::strtoupper(obuf); break;
      case 7: kc::strtolower(obuf); break;
      case 8: kc::strtrim(obuf); break;
      case 9: kc::strsqzspc(obuf); break;
      case 10: kc::strnrmspc(obuf); break;
      case 11: kc::stricmp(obuf, ebuf); break;
      case 12: kc::stristr(obuf, ebuf); break;
      case 13: kc::strfwm(obuf, ebuf); break;
      case 14: kc::strifwm(obuf, ebuf); break;
      case 15: kc::strbwm(obuf, ebuf); break;
      case 16: kc::stribwm(obuf, ebuf); break;
      case 17: kc::strutflen(obuf); break;
    }
    delete[] obuf;
    delete[] ebuf;
    kc::ZLIB::Mode zmode;
    switch (myrand(3)) {
      default: zmode = kc::ZLIB::RAW; break;
      case 0: zmode = kc::ZLIB::DEFLATE; break;
      case 1: zmode = kc::ZLIB::GZIP; break;
    }
    size_t zsiz;
    char* zbuf = kc::ZLIB::compress(ubuf, usiz, &zsiz, zmode);
    if (zbuf) {
      obuf = kc::ZLIB::decompress(zbuf, zsiz, &osiz, zmode);
      if (obuf) {
        if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
          errprint(__LINE__, "ZLIB::decompress");
          err = true;
        }
        delete[] obuf;
      } else {
        errprint(__LINE__, "ZLIB::decompress");
        err = true;
      }
      delete[] zbuf;
    } else {
      errprint(__LINE__, "ZLIB::compress");
      err = true;
    }
    zbuf = kc::LZO::compress(ubuf, usiz, &zsiz);
    if (zbuf) {
      obuf = kc::LZO::decompress(zbuf, zsiz, &osiz);
      if (obuf) {
        if (osiz != usiz || std::memcmp(obuf, ubuf, osiz)) {
          errprint(__LINE__, "LZO::decompress");
          err = true;
        }
        delete[] obuf;
      } else {
        errprint(__LINE__, "LZO::decompress");
        err = true;
      }
      delete[] zbuf;
    } else {
      errprint(__LINE__, "LZO::compress");
      err = true;
    }
    std::string ustr((char*)ubuf, usiz);
    kc::Regex::match(ustr, ".(\x80).");
    kc::Regex::replace(ustr, ".(\x80).", "[$0$1$2$&]");
    if (rnum > 250 && i % (rnum / 250) == 0) {
      oputchar('.');
      if (i == rnum || i % (rnum / 10) == 0) oprintf(" (%08lld)\n", (long long)i);
    }
  }
  oprintf("time: %.3f\n", kc::time() - stime);
  oprintf("%s\n\n", err ? "error" : "ok");
  return err ? 1 : 0;
}



// END OF FILE
