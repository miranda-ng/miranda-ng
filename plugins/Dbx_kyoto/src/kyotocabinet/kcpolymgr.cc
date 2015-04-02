/*************************************************************************************************
 * The command line utility of the polymorphic database
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


#include <kcpolydb.h>
#include "cmdcommon.h"


// global variables
const char* g_progname;                  // program name


// function prototypes
int main(int argc, char** argv);
static void usage();
static void dberrprint(kc::BasicDB* db, const char* info);
static int32_t runcreate(int argc, char** argv);
static int32_t runinform(int argc, char** argv);
static int32_t runset(int argc, char** argv);
static int32_t runremove(int argc, char** argv);
static int32_t runget(int argc, char** argv);
static int32_t runlist(int argc, char** argv);
static int32_t runclear(int argc, char** argv);
static int32_t runimport(int argc, char** argv);
static int32_t runcopy(int argc, char** argv);
static int32_t rundump(int argc, char** argv);
static int32_t runload(int argc, char** argv);
static int32_t runmerge(int argc, char** argv);
static int32_t runsetbulk(int argc, char** argv);
static int32_t runremovebulk(int argc, char** argv);
static int32_t rungetbulk(int argc, char** argv);
static int32_t runcheck(int argc, char** argv);
static int32_t proccreate(const char* path, int32_t oflags);
static int32_t procinform(const char* path, int32_t oflags, bool st);
static int32_t procset(const char* path, const char* kbuf, size_t ksiz,
                       const char* vbuf, size_t vsiz, int32_t oflags, int32_t mode);
static int32_t procremove(const char* path, const char* kbuf, size_t ksiz, int32_t oflags);
static int32_t procget(const char* path, const char* kbuf, size_t ksiz,
                       int32_t oflags, bool rm, bool px, bool pz);
static int32_t proclist(const char* path, const char*kbuf, size_t ksiz, int32_t oflags,
                        int32_t mode, bool des, int64_t max, bool rm, bool pv, bool px);
static int32_t procclear(const char* path, int32_t oflags);
static int32_t procimport(const char* path, const char* file, int32_t oflags, bool sx);
static int32_t proccopy(const char* path, const char* file, int32_t oflags);
static int32_t procdump(const char* path, const char* file, int32_t oflags);
static int32_t procload(const char* path, const char* file, int32_t oflags);
static int32_t procmerge(const char* path, int32_t oflags, kc::PolyDB::MergeMode mode,
                         const std::vector<std::string>& srcpaths);
static int32_t procsetbulk(const char* path, int32_t oflags,
                           const std::map<std::string, std::string>& recs);
static int32_t procremovebulk(const char* path, int32_t oflags,
                              const std::vector<std::string>& keys);
static int32_t procgetbulk(const char* path, int32_t oflags,
                           const std::vector<std::string>& keys, bool px);
static int32_t proccheck(const char* path, int32_t oflags);


// main routine
int main(int argc, char** argv) {
  g_progname = argv[0];
  kc::setstdiobin();
  if (argc < 2) usage();
  int32_t rv = 0;
  if (!std::strcmp(argv[1], "create")) {
    rv = runcreate(argc, argv);
  } else if (!std::strcmp(argv[1], "inform")) {
    rv = runinform(argc, argv);
  } else if (!std::strcmp(argv[1], "set")) {
    rv = runset(argc, argv);
  } else if (!std::strcmp(argv[1], "remove")) {
    rv = runremove(argc, argv);
  } else if (!std::strcmp(argv[1], "get")) {
    rv = runget(argc, argv);
  } else if (!std::strcmp(argv[1], "list")) {
    rv = runlist(argc, argv);
  } else if (!std::strcmp(argv[1], "clear")) {
    rv = runclear(argc, argv);
  } else if (!std::strcmp(argv[1], "import")) {
    rv = runimport(argc, argv);
  } else if (!std::strcmp(argv[1], "copy")) {
    rv = runcopy(argc, argv);
  } else if (!std::strcmp(argv[1], "dump")) {
    rv = rundump(argc, argv);
  } else if (!std::strcmp(argv[1], "load")) {
    rv = runload(argc, argv);
  } else if (!std::strcmp(argv[1], "merge")) {
    rv = runmerge(argc, argv);
  } else if (!std::strcmp(argv[1], "setbulk")) {
    rv = runsetbulk(argc, argv);
  } else if (!std::strcmp(argv[1], "removebulk")) {
    rv = runremovebulk(argc, argv);
  } else if (!std::strcmp(argv[1], "getbulk")) {
    rv = rungetbulk(argc, argv);
  } else if (!std::strcmp(argv[1], "check")) {
    rv = runcheck(argc, argv);
  } else if (!std::strcmp(argv[1], "version") || !std::strcmp(argv[1], "--version")) {
    printversion();
  } else {
    usage();
  }
  return rv;
}


// print the usage and exit
static void usage() {
  eprintf("%s: the command line utility of the polymorphic database of Kyoto Cabinet\n",
          g_progname);
  eprintf("\n");
  eprintf("usage:\n");
  eprintf("  %s create [-otr] [-onl|-otl|-onr] path\n", g_progname);
  eprintf("  %s inform [-onl|-otl|-onr] [-st] path\n", g_progname);
  eprintf("  %s set [-onl|-otl|-onr] [-add|-rep|-app|-inci|-incd] [-sx] path key value\n",
          g_progname);
  eprintf("  %s remove [-onl|-otl|-onr] [-sx] path key\n", g_progname);
  eprintf("  %s get [-onl|-otl|-onr] [-rm] [-sx] [-px] [-pz] path key\n", g_progname);
  eprintf("  %s list [-onl|-otl|-onr] [-mp|-mr|-ms] [-des] [-max num] [-rm] [-sx] [-pv] [-px]"
          " path [key]\n", g_progname);
  eprintf("  %s clear [-onl|-otl|-onr] path\n", g_progname);
  eprintf("  %s import [-onl|-otl|-onr] [-sx] path [file]\n", g_progname);
  eprintf("  %s copy [-onl|-otl|-onr] path file\n", g_progname);
  eprintf("  %s dump [-onl|-otl|-onr] path [file]\n", g_progname);
  eprintf("  %s load [-otr] [-onl|-otl|-onr] path [file]\n", g_progname);
  eprintf("  %s merge [-onl|-otl|-onr] [-add|-rep|-app] path src...\n", g_progname);
  eprintf("  %s setbulk [-onl|-otl|-onr] [-sx] path key value ...\n", g_progname);
  eprintf("  %s removebulk [-onl|-otl|-onr] [-sx] path key ...\n", g_progname);
  eprintf("  %s getbulk [-onl|-otl|-onr] [-sx] [-px] path key ...\n", g_progname);
  eprintf("  %s check [-onl|-otl|-onr] path\n", g_progname);
  eprintf("\n");
  std::exit(1);
}


// print error message of database
static void dberrprint(kc::BasicDB* db, const char* info) {
  const kc::BasicDB::Error& err = db->error();
  eprintf("%s: %s: %s: %d: %s: %s\n",
          g_progname, info, db->path().c_str(), err.code(), err.name(), err.message());
}


// parse arguments of create command
static int32_t runcreate(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-otr")) {
        oflags |= kc::PolyDB::OTRUNCATE;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = proccreate(path, oflags);
  return rv;
}


// parse arguments of inform command
static int32_t runinform(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  bool st = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-st")) {
        st = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procinform(path, oflags, st);
  return rv;
}


// parse arguments of set command
static int32_t runset(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  const char* vstr = NULL;
  int32_t oflags = 0;
  int32_t mode = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-add")) {
        mode = 'a';
      } else if (!std::strcmp(argv[i], "-rep")) {
        mode = 'r';
      } else if (!std::strcmp(argv[i], "-app")) {
        mode = 'c';
      } else if (!std::strcmp(argv[i], "-inci")) {
        mode = 'i';
      } else if (!std::strcmp(argv[i], "-incd")) {
        mode = 'd';
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else if (!vstr) {
      vstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr || !vstr) usage();
  char* kbuf;
  size_t ksiz;
  char* vbuf;
  size_t vsiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
    vbuf = kc::hexdecode(vstr, &vsiz);
    vstr = vbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
    vsiz = std::strlen(vstr);
    vbuf = NULL;
  }
  int32_t rv = procset(path, kstr, ksiz, vstr, vsiz, oflags, mode);
  delete[] kbuf;
  delete[] vbuf;
  return rv;
}


// parse arguments of remove command
static int32_t runremove(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr) usage();
  char* kbuf;
  size_t ksiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
  }
  int32_t rv = procremove(path, kstr, ksiz, oflags);
  delete[] kbuf;
  return rv;
}


// parse arguments of get command
static int32_t runget(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  bool rm = false;
  bool sx = false;
  bool px = false;
  bool pz = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-rm")) {
        rm = true;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else if (!std::strcmp(argv[i], "-px")) {
        px = true;
      } else if (!std::strcmp(argv[i], "-pz")) {
        pz = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !kstr) usage();
  char* kbuf;
  size_t ksiz;
  if (sx) {
    kbuf = kc::hexdecode(kstr, &ksiz);
    kstr = kbuf;
  } else {
    ksiz = std::strlen(kstr);
    kbuf = NULL;
  }
  int32_t rv = procget(path, kstr, ksiz, oflags, rm, px, pz);
  delete[] kbuf;
  return rv;
}


// parse arguments of list command
static int32_t runlist(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* kstr = NULL;
  int32_t oflags = 0;
  int32_t mode = 0;
  bool des = false;
  int64_t max = -1;
  bool rm = false;
  bool sx = false;
  bool pv = false;
  bool px = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-mp")) {
        mode = 'p';
      } else if (!std::strcmp(argv[i], "-mr")) {
        mode = 'r';
      } else if (!std::strcmp(argv[i], "-ms")) {
        mode = 's';
      } else if (!std::strcmp(argv[i], "-des")) {
        des = true;
      } else if (!std::strcmp(argv[i], "-max")) {
        if (++i >= argc) usage();
        max = kc::atoix(argv[i]);
      } else if (!std::strcmp(argv[i], "-rm")) {
        rm = true;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else if (!std::strcmp(argv[i], "-pv")) {
        pv = true;
      } else if (!std::strcmp(argv[i], "-px")) {
        px = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!kstr) {
      kstr = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  char* kbuf = NULL;
  size_t ksiz = 0;
  if (kstr) {
    if (sx) {
      kbuf = kc::hexdecode(kstr, &ksiz);
      kstr = kbuf;
    } else {
      ksiz = std::strlen(kstr);
      kbuf = new char[ksiz+1];
      std::memcpy(kbuf, kstr, ksiz);
      kbuf[ksiz] = '\0';
    }
  }
  int32_t rv = proclist(path, kbuf, ksiz, oflags, mode, des, max, rm, pv, px);
  delete[] kbuf;
  return rv;
}


// parse arguments of clear command
static int32_t runclear(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procclear(path, oflags);
  return rv;
}


// parse arguments of import command
static int32_t runimport(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procimport(path, file, oflags, sx);
  return rv;
}


// parse arguments of copy command
static int32_t runcopy(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path || !file) usage();
  int32_t rv = proccopy(path, file, oflags);
  return rv;
}


// parse arguments of dump command
static int32_t rundump(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procdump(path, file, oflags);
  return rv;
}


// parse arguments of load command
static int32_t runload(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  const char* file = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-otr")) {
        oflags |= kc::PolyDB::OTRUNCATE;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else if (!file) {
      file = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = procload(path, file, oflags);
  return rv;
}


// parse arguments of merge command
static int32_t runmerge(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  kc::PolyDB::MergeMode mode = kc::PolyDB::MSET;
  std::vector<std::string> srcpaths;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-add")) {
        mode = kc::PolyDB::MADD;
      } else if (!std::strcmp(argv[i], "-rep")) {
        mode = kc::PolyDB::MREPLACE;
      } else if (!std::strcmp(argv[i], "-app")) {
        mode = kc::PolyDB::MAPPEND;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      srcpaths.push_back(argv[i]);
    }
  }
  if (!path && srcpaths.size() < 1) usage();
  int32_t rv = procmerge(path, oflags, mode, srcpaths);
  return rv;
}


// parse arguments of setbulk command
static int32_t runsetbulk(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  std::map<std::string, std::string> recs;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      const char* kstr = argv[i];
      if (++i >= argc) usage();
      const char* vstr = argv[i];
      char* kbuf;
      size_t ksiz;
      char* vbuf;
      size_t vsiz;
      if (sx) {
        kbuf = kc::hexdecode(kstr, &ksiz);
        kstr = kbuf;
        vbuf = kc::hexdecode(vstr, &vsiz);
        vstr = vbuf;
      } else {
        ksiz = std::strlen(kstr);
        kbuf = NULL;
        vsiz = std::strlen(vstr);
        vbuf = NULL;
      }
      std::string key(kstr, ksiz);
      std::string value(vstr, vsiz);
      recs[key] = value;
      delete[] kbuf;
      delete[] vbuf;
    }
  }
  if (!path) usage();
  int32_t rv = procsetbulk(path, oflags, recs);
  return rv;
}


// parse arguments of removebulk command
static int32_t runremovebulk(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  std::vector<std::string> keys;
  int32_t oflags = 0;
  bool sx = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      const char* kstr = argv[i];
      char* kbuf;
      size_t ksiz;
      if (sx) {
        kbuf = kc::hexdecode(kstr, &ksiz);
        kstr = kbuf;
      } else {
        ksiz = std::strlen(kstr);
        kbuf = NULL;
      }
      std::string key(kstr, ksiz);
      keys.push_back(key);
      delete[] kbuf;
    }
  }
  if (!path) usage();
  int32_t rv = procremovebulk(path, oflags, keys);
  return rv;
}


// parse arguments of getbulk command
static int32_t rungetbulk(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  std::vector<std::string> keys;
  int32_t oflags = 0;
  bool sx = false;
  bool px = false;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else if (!std::strcmp(argv[i], "-sx")) {
        sx = true;
      } else if (!std::strcmp(argv[i], "-px")) {
        px = true;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      const char* kstr = argv[i];
      char* kbuf;
      size_t ksiz;
      if (sx) {
        kbuf = kc::hexdecode(kstr, &ksiz);
        kstr = kbuf;
      } else {
        ksiz = std::strlen(kstr);
        kbuf = NULL;
      }
      std::string key(kstr, ksiz);
      keys.push_back(key);
      delete[] kbuf;
    }
  }
  if (!path) usage();
  int32_t rv = procgetbulk(path, oflags, keys, px);
  return rv;
}


// parse arguments of check command
static int32_t runcheck(int argc, char** argv) {
  bool argbrk = false;
  const char* path = NULL;
  int32_t oflags = 0;
  for (int32_t i = 2; i < argc; i++) {
    if (!argbrk && argv[i][0] == '-') {
      if (!std::strcmp(argv[i], "--")) {
        argbrk = true;
      } else if (!std::strcmp(argv[i], "-onl")) {
        oflags |= kc::PolyDB::ONOLOCK;
      } else if (!std::strcmp(argv[i], "-otl")) {
        oflags |= kc::PolyDB::OTRYLOCK;
      } else if (!std::strcmp(argv[i], "-onr")) {
        oflags |= kc::PolyDB::ONOREPAIR;
      } else {
        usage();
      }
    } else if (!path) {
      argbrk = true;
      path = argv[i];
    } else {
      usage();
    }
  }
  if (!path) usage();
  int32_t rv = proccheck(path, oflags);
  return rv;
}


// perform create command
static int32_t proccreate(const char* path, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform inform command
static int32_t procinform(const char* path, int32_t oflags, bool st) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (st) {
    std::map<std::string, std::string> status;
    if (db.status(&status)) {
      std::map<std::string, std::string>::iterator it = status.begin();
      std::map<std::string, std::string>::iterator itend = status.end();
      while (it != itend) {
        oprintf("%s: %s\n", it->first.c_str(), it->second.c_str());
        ++it;
      }
    } else {
      dberrprint(&db, "DB::status failed");
      err = true;
    }
  } else {
    oprintf("count: %lld\n", (long long)db.count());
    oprintf("size: %lld\n", (long long)db.size());
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform set command
static int32_t procset(const char* path, const char* kbuf, size_t ksiz,
                       const char* vbuf, size_t vsiz, int32_t oflags, int32_t mode) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  switch (mode) {
    default: {
      if (!db.set(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::set failed");
        err = true;
      }
      break;
    }
    case 'a': {
      if (!db.add(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::add failed");
        err = true;
      }
      break;
    }
    case 'r': {
      if (!db.replace(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::replace failed");
        err = true;
      }
      break;
    }
    case 'c': {
      if (!db.append(kbuf, ksiz, vbuf, vsiz)) {
        dberrprint(&db, "DB::append failed");
        err = true;
      }
      break;
    }
    case 'i': {
      int64_t onum = db.increment(kbuf, ksiz, kc::atoi(vbuf));
      if (onum == kc::INT64MIN) {
        dberrprint(&db, "DB::increment failed");
        err = true;
      } else {
        oprintf("%lld\n", (long long)onum);
      }
      break;
    }
    case 'd': {
      double onum = db.increment_double(kbuf, ksiz, kc::atof(vbuf));
      if (kc::chknan(onum)) {
        dberrprint(&db, "DB::increment_double failed");
        err = true;
      } else {
        oprintf("%f\n", onum);
      }
      break;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform remove command
static int32_t procremove(const char* path, const char* kbuf, size_t ksiz, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.remove(kbuf, ksiz)) {
    dberrprint(&db, "DB::remove failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform get command
static int32_t procget(const char* path, const char* kbuf, size_t ksiz,
                       int32_t oflags, bool rm, bool px, bool pz) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  uint32_t omode = rm ? kc::PolyDB::OWRITER : kc::PolyDB::OREADER;
  if (!db.open(path, omode | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  char* vbuf;
  size_t vsiz;
  if (rm) {
    vbuf = db.seize(kbuf, ksiz, &vsiz);
  } else {
    vbuf = db.get(kbuf, ksiz, &vsiz);
  }
  if (vbuf) {
    printdata(vbuf, vsiz, px);
    if (!pz) oprintf("\n");
    delete[] vbuf;
  } else {
    dberrprint(&db, "DB::get failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform list command
static int32_t proclist(const char* path, const char*kbuf, size_t ksiz, int32_t oflags,
                        int32_t mode, bool des, int64_t max, bool rm, bool pv, bool px) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  uint32_t omode = rm ? kc::PolyDB::OWRITER : kc::PolyDB::OREADER;
  if (!db.open(path, omode | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  class VisitorImpl : public kc::DB::Visitor {
   public:
    explicit VisitorImpl(bool rm, bool pv, bool px) : rm_(rm), pv_(pv), px_(px) {}
   private:
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      printdata(kbuf, ksiz, px_);
      if (pv_) {
        oprintf("\t");
        printdata(vbuf, vsiz, px_);
      }
      oprintf("\n");
      return rm_ ? REMOVE : NOP;
    }
    bool rm_;
    bool pv_;
    bool px_;
  } visitor(rm, pv, px);
  class Printer {
   public:
    static bool print(kc::BasicDB* db, const std::vector<std::string>& keys,
                      bool des, bool rm, bool pv, bool px) {
      bool err = false;
      if (des) {
        for (int64_t i = (int64_t)keys.size() - 1; i >= 0; i--) {
          if (!proc_one(db, keys[i], rm, pv, px)) err = true;
        }
      } else {
        std::vector<std::string>::const_iterator it = keys.begin();
        std::vector<std::string>::const_iterator itend = keys.end();
        while (it != itend) {
          if (!proc_one(db, *it, rm, pv, px)) err = true;
          ++it;
        }
      }
      return !err;
    }
   private:
    static bool proc_one(kc::BasicDB* db, const std::string& key, bool rm, bool pv, bool px) {
      bool err = false;
      printdata(key.data(), key.size(), px);
      if (pv) {
        size_t vsiz;
        char* vbuf = db->get(key.data(), key.size(), &vsiz);
        if (vbuf) {
          oprintf("\t");
          printdata(vbuf, vsiz, px);
          delete[] vbuf;
        } else {
          dberrprint(db, "DB::get failed");
          err = true;
        }
      }
      oprintf("\n");
      if (rm && !db->remove(key.data(), key.size())) {
        dberrprint(db, "DB::remove failed");
        err = true;
      }
      return !err;
    }
  };
  if (mode == 'p') {
    std::vector<std::string> keys;
    if (db.match_prefix(std::string(kbuf, ksiz), &keys, max) >= 0) {
      if (!Printer::print(&db, keys, des, rm, pv, px)) err = true;
    } else {
      dberrprint(&db, "DB::match_prefix failed");
      err = true;
    }
  } else if (mode == 'r') {
    std::vector<std::string> keys;
    if (db.match_regex(std::string(kbuf, ksiz), &keys, max) >= 0) {
      if (!Printer::print(&db, keys, des, rm, pv, px)) err = true;
    } else {
      dberrprint(&db, "DB::match_regex failed");
      err = true;
    }
  } else if (mode == 's') {
    size_t range = ksiz / 3 + 1;
    std::vector<std::string> keys;
    if (db.match_similar(std::string(kbuf, ksiz), range, false, &keys, max) >= 0) {
      if (!Printer::print(&db, keys, des, rm, pv, px)) err = true;
    } else {
      dberrprint(&db, "DB::match_similar failed");
      err = true;
    }
  } else if (kbuf || des || max >= 0) {
    if (max < 0) max = kc::INT64MAX;
    kc::PolyDB::Cursor cur(&db);
    if (des) {
      if (kbuf) {
        if (!cur.jump_back(kbuf, ksiz) && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "Cursor::jump failed");
          err = true;
        }
      } else {
        if (!cur.jump_back() && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "Cursor::jump failed");
          err = true;
        }
      }
      while (!err && max > 0) {
        if (!cur.accept(&visitor, rm, true)) {
          if (db.error() != kc::BasicDB::Error::NOREC) {
            dberrprint(&db, "Cursor::accept failed");
            err = true;
          }
          break;
        }
        max--;
      }
    } else {
      if (kbuf) {
        if (!cur.jump(kbuf, ksiz) && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "Cursor::jump failed");
          err = true;
        }
      } else {
        if (!cur.jump() && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "Cursor::jump failed");
          err = true;
        }
      }
      while (!err && max > 0) {
        if (!cur.accept(&visitor, rm, true)) {
          if (db.error() != kc::BasicDB::Error::NOREC) {
            dberrprint(&db, "Cursor::accept failed");
            err = true;
          }
          break;
        }
        max--;
      }
    }
  } else {
    if (!db.iterate(&visitor, rm)) {
      dberrprint(&db, "DB::iterate failed");
      err = true;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform clear command
static int32_t procclear(const char* path, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (!db.clear()) {
    dberrprint(&db, "DB::clear failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform import command
static int32_t procimport(const char* path, const char* file, int32_t oflags, bool sx) {
  std::istream *is = &std::cin;
  std::ifstream ifs;
  if (file) {
    ifs.open(file, std::ios_base::in | std::ios_base::binary);
    if (!ifs) {
      eprintf("%s: %s: open error\n", g_progname, file);
      return 1;
    }
    is = &ifs;
  }
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  int64_t cnt = 0;
  std::string line;
  std::vector<std::string> fields;
  while (!err && mygetline(is, &line)) {
    cnt++;
    kc::strsplit(line, '\t', &fields);
    if (sx) {
      std::vector<std::string>::iterator it = fields.begin();
      std::vector<std::string>::iterator itend = fields.end();
      while (it != itend) {
        size_t esiz;
        char* ebuf = kc::hexdecode(it->c_str(), &esiz);
        it->clear();
        it->append(ebuf, esiz);
        delete[] ebuf;
        ++it;
      }
    }
    switch (fields.size()) {
      case 2: {
        if (!db.set(fields[0], fields[1])) {
          dberrprint(&db, "DB::set failed");
          err = true;
        }
        break;
      }
      case 1: {
        if (!db.remove(fields[0]) && db.error() != kc::BasicDB::Error::NOREC) {
          dberrprint(&db, "DB::remove failed");
          err = true;
        }
        break;
      }
    }
    oputchar('.');
    if (cnt % 50 == 0) oprintf(" (%lld)\n", (long long)cnt);
  }
  if (cnt % 50 > 0) oprintf(" (%lld)\n", (long long)cnt);
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform copy command
static int32_t proccopy(const char* path, const char* file, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  DotChecker checker(&std::cout, -100);
  if (!db.copy(file, &checker)) {
    dberrprint(&db, "DB::copy failed");
    err = true;
  }
  oprintf(" (end)\n");
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  if (!err) oprintf("%lld blocks were copied successfully\n", (long long)checker.count());
  return err ? 1 : 0;
}


// perform dump command
static int32_t procdump(const char* path, const char* file, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (file) {
    DotChecker checker(&std::cout, 1000);
    if (!db.dump_snapshot(file, &checker)) {
      dberrprint(&db, "DB::dump_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were dumped successfully\n", (long long)checker.count());
  } else {
    if (!db.dump_snapshot(&std::cout)) {
      dberrprint(&db, "DB::dump_snapshot");
      err = true;
    }
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform load command
static int32_t procload(const char* path, const char* file, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (file) {
    DotChecker checker(&std::cout, -1000);
    if (!db.load_snapshot(file, &checker)) {
      dberrprint(&db, "DB::load_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were loaded successfully\n", (long long)checker.count());
  } else {
    DotChecker checker(&std::cout, -1000);
    if (!db.load_snapshot(&std::cin)) {
      dberrprint(&db, "DB::load_snapshot");
      err = true;
    }
    oprintf(" (end)\n");
    if (!err) oprintf("%lld records were loaded successfully\n", (long long)checker.count());
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform merge command
static int32_t procmerge(const char* path, int32_t oflags, kc::PolyDB::MergeMode mode,
                         const std::vector<std::string>& srcpaths) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | kc::PolyDB::OCREATE | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  kc::BasicDB** srcary = new kc::BasicDB*[srcpaths.size()];
  size_t srcnum = 0;
  std::vector<std::string>::const_iterator it = srcpaths.begin();
  std::vector<std::string>::const_iterator itend = srcpaths.end();
  while (it != itend) {
    const std::string srcpath = *it;
    kc::PolyDB* srcdb = new kc::PolyDB;
    if (srcdb->open(srcpath, kc::PolyDB::OREADER | oflags)) {
      srcary[srcnum++] = srcdb;
    } else {
      dberrprint(srcdb, "DB::open failed");
      err = true;
      delete srcdb;
    }
    ++it;
  }
  DotChecker checker(&std::cout, 1000);
  if (!db.merge(srcary, srcnum, mode, &checker)) {
    dberrprint(&db, "DB::merge failed");
    err = true;
  }
  oprintf(" (end)\n");
  for (size_t i = 0; i < srcnum; i++) {
    kc::BasicDB* srcdb = srcary[i];
    if (!srcdb->close()) {
      dberrprint(srcdb, "DB::close failed");
      err = true;
    }
    delete srcdb;
  }
  delete[] srcary;
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  if (!err) oprintf("%lld records were merged successfully\n", (long long)checker.count());
  return err ? 1 : 0;
}


// perform setbulk command
static int32_t procsetbulk(const char* path, int32_t oflags,
                           const std::map<std::string, std::string>& recs) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (db.set_bulk(recs) != (int64_t)recs.size()) {
    dberrprint(&db, "DB::set_bulk failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform removebulk command
static int32_t procremovebulk(const char* path, int32_t oflags,
                              const std::vector<std::string>& keys) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OWRITER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  if (db.remove_bulk(keys) < 0) {
    dberrprint(&db, "DB::remove_bulk failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform getbulk command
static int32_t procgetbulk(const char* path, int32_t oflags,
                           const std::vector<std::string>& keys, bool px) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  std::map<std::string, std::string> recs;
  if (db.get_bulk(keys, &recs) >= 0) {
    std::map<std::string, std::string>::iterator it = recs.begin();
    std::map<std::string, std::string>::iterator itend = recs.end();
    while (it != itend) {
      printdata(it->first.data(), it->first.size(), px);
      oprintf("\t");
      printdata(it->second.data(), it->second.size(), px);
      oprintf("\n");
      ++it;
    }
  } else {
    dberrprint(&db, "DB::get_bulk failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  return err ? 1 : 0;
}


// perform check command
static int32_t proccheck(const char* path, int32_t oflags) {
  kc::PolyDB db;
  db.tune_logger(stdlogger(g_progname, &std::cerr));
  if (!db.open(path, kc::PolyDB::OREADER | oflags)) {
    dberrprint(&db, "DB::open failed");
    return 1;
  }
  bool err = false;
  kc::PolyDB::Cursor cur(&db);
  if (!cur.jump() && db.error() != kc::BasicDB::Error::NOREC) {
    dberrprint(&db, "DB::jump failed");
    err = true;
  }
  int64_t cnt = 0;
  while (!err) {
    size_t ksiz;
    const char* vbuf;
    size_t vsiz;
    char* kbuf = cur.get(&ksiz, &vbuf, &vsiz);
    if (kbuf) {
      cnt++;
      size_t rsiz;
      char* rbuf = db.get(kbuf, ksiz, &rsiz);
      if (rbuf) {
        if (rsiz != vsiz || std::memcmp(rbuf, vbuf, rsiz)) {
          dberrprint(&db, "DB::get failed");
          err = true;
        }
        delete[] rbuf;
      } else {
        dberrprint(&db, "DB::get failed");
        err = true;
      }
      delete[] kbuf;
      if (cnt % 1000 == 0) {
        oputchar('.');
        if (cnt % 50000 == 0) oprintf(" (%lld)\n", (long long)cnt);
      }
    } else {
      if (db.error() != kc::BasicDB::Error::NOREC) {
        dberrprint(&db, "Cursor::get failed");
        err = true;
      }
      break;
    }
    if (!cur.step() && db.error() != kc::BasicDB::Error::NOREC) {
      dberrprint(&db, "Cursor::step failed");
      err = true;
    }
  }
  oprintf(" (end)\n");
  if (db.count() != cnt) {
    dberrprint(&db, "DB::count failed");
    err = true;
  }
  const std::string& rpath = db.path();
  kc::File::Status sbuf;
  if (kc::File::status(rpath, &sbuf)) {
    if (!sbuf.isdir && db.size() != sbuf.size) {
      dberrprint(&db, "DB::size failed");
      err = true;
    }
  } else {
    dberrprint(&db, "File::status failed");
    err = true;
  }
  if (!db.close()) {
    dberrprint(&db, "DB::close failed");
    err = true;
  }
  if (!err) oprintf("%lld records were checked successfully\n", (long long)cnt);
  return err ? 1 : 0;
}



// END OF FILE
