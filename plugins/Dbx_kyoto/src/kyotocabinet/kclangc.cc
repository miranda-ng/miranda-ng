/*************************************************************************************************
 * C language binding
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


#include "kcpolydb.h"
#include "kcdbext.h"
#include "kclangc.h"
#include "myconf.h"

using namespace kyotocabinet;

extern "C" {


/** The package version. */
const char* const KCVERSION = VERSION;


/** Special pointer for no operation by the visiting function. */
const char* const KCVISNOP = DB::Visitor::NOP;


/** Special pointer to remove the record by the visiting function. */
const char* const KCVISREMOVE = DB::Visitor::REMOVE;


/**
 * Allocate a region on memory.
 */
void* kcmalloc(size_t size) {
  _assert_(size > 0 && size <= MEMMAXSIZ);
  return new char[size];
}


/**
 * Release a region allocated in the library.
 */
void kcfree(void* ptr) {
  _assert_(true);
  delete[] (char*)ptr;
}


/**
 * Get the time of day in seconds.
 */
double kctime(void) {
  _assert_(true);
  return kyotocabinet::time();
}


/**
 * Convert a string to an integer.
 */
int64_t kcatoi(const char* str) {
  _assert_(str);
  return kyotocabinet::atoi(str);
}


/**
 * Convert a string with a metric prefix to an integer.
 */
int64_t kcatoix(const char* str) {
  _assert_(str);
  return kyotocabinet::atoix(str);
}


/**
 * Convert a string to a real number.
 */
double kcatof(const char* str) {
  _assert_(str);
  return kyotocabinet::atof(str);
}


/**
 * Get the hash value by MurMur hashing.
 */
uint64_t kchashmurmur(const void* buf, size_t size) {
  _assert_(buf && size <= MEMMAXSIZ);
  return kyotocabinet::hashmurmur(buf, size);
}


/**
 * Get the hash value by FNV hashing.
 */
uint64_t kchashfnv(const void* buf, size_t size) {
  _assert_(buf && size <= MEMMAXSIZ);
  return kyotocabinet::hashfnv(buf, size);
}


/**
 * Calculate the levenshtein distance of two regions.
 */
size_t kclevdist(const void* abuf, size_t asiz, const void* bbuf, size_t bsiz, int32_t utf) {
  _assert_(abuf && asiz <= MEMMAXSIZ && bbuf && bsiz <= MEMMAXSIZ);
  size_t dist;
  if (utf) {
    uint32_t astack[128];
    uint32_t* aary = asiz > sizeof(astack) / sizeof(*astack) ? new uint32_t[asiz] : astack;
    size_t anum;
    strutftoucs((const char*)abuf, asiz, aary, &anum);
    uint32_t bstack[128];
    uint32_t* bary = bsiz > sizeof(bstack) / sizeof(*bstack) ? new uint32_t[bsiz] : bstack;
    size_t bnum;
    strutftoucs((const char*)bbuf, bsiz, bary, &bnum);
    dist = strucsdist(aary, anum, bary, bnum);
    if (bary != bstack) delete[] bary;
    if (aary != astack) delete[] aary;
  } else {
    dist = memdist(abuf, asiz, bbuf, bsiz);
  }
  return dist;
}


/**
 * Get the quiet Not-a-Number value.
 */
double kcnan() {
  _assert_(true);
  return kyotocabinet::nan();
}


/**
 * Get the positive infinity value.
 */
double kcinf() {
  _assert_(true);
  return kyotocabinet::inf();
}


/**
 * Check a number is a Not-a-Number value.
 */
int32_t kcchknan(double num) {
  _assert_(true);
  return kyotocabinet::chknan(num);
}


/**
 * Check a number is an infinity value.
 */
int32_t kcchkinf(double num) {
  _assert_(true);
  return kyotocabinet::chkinf(num);
}


/**
 * Get the readable string of an error code.
 */
const char* kcecodename(int32_t code) {
  _assert_(true);
  return BasicDB::Error::codename((BasicDB::Error::Code)code);
}


/**
 * Create a database object.
 */
KCDB* kcdbnew(void) {
  _assert_(true);
  return (KCDB*)new PolyDB;
}


/**
 * Destroy a database object.
 */
void kcdbdel(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  delete pdb;
}


/**
 * Open a database file.
 */
int32_t kcdbopen(KCDB* db, const char* path, uint32_t mode) {
  _assert_(db && path);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->open(path, mode);
}


/**
 * Close the database file.
 */
int32_t kcdbclose(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->close();
}


/**
 * Get the code of the last happened error.
 */
int32_t kcdbecode(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->error().code();
}


/**
 * Get the supplement message of the last happened error.
 */
const char* kcdbemsg(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->error().message();
}


/**
 * Accept a visitor to a record.
 */
int32_t kcdbaccept(KCDB* db, const char* kbuf, size_t ksiz,
                   KCVISITFULL fullproc, KCVISITEMPTY emptyproc, void* opq, int32_t writable) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  class VisitorImpl : public DB::Visitor {
   public:
    explicit VisitorImpl(KCVISITFULL fullproc, KCVISITEMPTY emptyproc, void* opq) :
        fullproc_(fullproc), emptyproc_(emptyproc), opq_(opq) {}
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!fullproc_) return NOP;
      return fullproc_(kbuf, ksiz, vbuf, vsiz, sp, opq_);
    }
    const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
      if (!emptyproc_) return NOP;
      return emptyproc_(kbuf, ksiz, sp, opq_);
    }
   private:
    KCVISITFULL fullproc_;
    KCVISITEMPTY emptyproc_;
    void* opq_;
  };
  VisitorImpl visitor(fullproc, emptyproc, opq);
  return pdb->accept(kbuf, ksiz, &visitor, writable);
}


/**
 * Accept a visitor to multiple records at once.
 */
int32_t kcdbacceptbulk(KCDB* db, const KCSTR* keys, size_t knum,
                       KCVISITFULL fullproc, KCVISITEMPTY emptyproc,
                       void* opq, int32_t writable) {
  _assert_(db && keys && knum <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> xkeys;
  xkeys.reserve(knum);
  for (size_t i = 0; i < knum; i++) {
    xkeys.push_back(std::string(keys[i].buf, keys[i].size));
  }
  class VisitorImpl : public DB::Visitor {
   public:
    explicit VisitorImpl(KCVISITFULL fullproc, KCVISITEMPTY emptyproc, void* opq) :
        fullproc_(fullproc), emptyproc_(emptyproc), opq_(opq) {}
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!fullproc_) return NOP;
      return fullproc_(kbuf, ksiz, vbuf, vsiz, sp, opq_);
    }
    const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
      if (!emptyproc_) return NOP;
      return emptyproc_(kbuf, ksiz, sp, opq_);
    }
   private:
    KCVISITFULL fullproc_;
    KCVISITEMPTY emptyproc_;
    void* opq_;
  };
  VisitorImpl visitor(fullproc, emptyproc, opq);
  return pdb->accept_bulk(xkeys, &visitor, writable);
}


/**
 * Iterate to accept a visitor for each record.
 */
int32_t kcdbiterate(KCDB* db, KCVISITFULL fullproc, void* opq, int32_t writable) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  class VisitorImpl : public DB::Visitor {
   public:
    explicit VisitorImpl(KCVISITFULL fullproc, void* opq) : fullproc_(fullproc), opq_(opq) {}
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!fullproc_) return NOP;
      return fullproc_(kbuf, ksiz, vbuf, vsiz, sp, opq_);
    }
   private:
    KCVISITFULL fullproc_;
    void* opq_;
  };
  VisitorImpl visitor(fullproc, opq);
  return pdb->iterate(&visitor, writable);
}


/**
 * Scan each record in parallel.
 */
int32_t kcdbscanpara(KCDB* db, KCVISITFULL fullproc, void* opq, size_t thnum) {
  _assert_(db && thnum <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  class VisitorImpl : public DB::Visitor {
   public:
    explicit VisitorImpl(KCVISITFULL fullproc, void* opq) : fullproc_(fullproc), opq_(opq) {}
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!fullproc_) return NOP;
      return fullproc_(kbuf, ksiz, vbuf, vsiz, sp, opq_);
    }
   private:
    KCVISITFULL fullproc_;
    void* opq_;
  };
  VisitorImpl visitor(fullproc, opq);
  return pdb->scan_parallel(&visitor, thnum);
}


/**
 * Set the value of a record.
 */
int32_t kcdbset(KCDB* db, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->set(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Add a record.
 */
int32_t kcdbadd(KCDB* db, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->add(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Replace the value of a record.
 */
int32_t kcdbreplace(KCDB* db, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->replace(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Append the value of a record.
 */
int32_t kcdbappend(KCDB* db, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->append(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Add a number to the numeric value of a record.
 */
int64_t kcdbincrint(KCDB* db, const char* kbuf, size_t ksiz, int64_t num, int64_t orig) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->increment(kbuf, ksiz, num, orig);
}


/**
 * Add a number to the numeric value of a record.
 */
double kcdbincrdouble(KCDB* db, const char* kbuf, size_t ksiz, double num, double orig) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->increment_double(kbuf, ksiz, num, orig);
}


/**
 * Perform compare-and-swap.
 */
int32_t kcdbcas(KCDB* db, const char* kbuf, size_t ksiz,
                const char* ovbuf, size_t ovsiz, const char* nvbuf, size_t nvsiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->cas(kbuf, ksiz, ovbuf, ovsiz, nvbuf, nvsiz);
}


/**
 * Remove a record.
 */
int32_t kcdbremove(KCDB* db, const char* kbuf, size_t ksiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->remove(kbuf, ksiz);
}


/**
 * Retrieve the value of a record.
 */
char* kcdbget(KCDB* db, const char* kbuf, size_t ksiz, size_t* sp) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && sp);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->get(kbuf, ksiz, sp);
}


/**
 * Check the existence of a record.
 */
int32_t kcdbcheck(KCDB* db, const char* kbuf, size_t ksiz) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->check(kbuf, ksiz);
}


/**
 * Retrieve the value of a record.
 */
int32_t kcdbgetbuf(KCDB* db, const char* kbuf, size_t ksiz, char* vbuf, size_t max) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && vbuf);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->get(kbuf, ksiz, vbuf, max);
}


/**
 * Retrieve the value of a record and remove it atomically.
 */
char* kcdbseize(KCDB* db, const char* kbuf, size_t ksiz, size_t* sp) {
  _assert_(db && kbuf && ksiz <= MEMMAXSIZ && sp);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->seize(kbuf, ksiz, sp);
}


/**
 * Store records at once.
 */
int64_t kcdbsetbulk(KCDB* db, const KCREC* recs, size_t rnum, int32_t atomic) {
  _assert_(db && recs && rnum <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::map<std::string, std::string> xrecs;
  for (size_t i = 0; i < rnum; i++) {
    const KCREC* rec = recs + i;
    xrecs[std::string(rec->key.buf, rec->key.size)] =
        std::string(rec->value.buf, rec->value.size);
  }
  return pdb->set_bulk(xrecs, atomic);
}


/**
 * Remove records at once.
 */
int64_t kcdbremovebulk(KCDB* db, const KCSTR* keys, size_t knum, int32_t atomic) {
  _assert_(db && keys && knum <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> xkeys;
  xkeys.reserve(knum);
  for (size_t i = 0; i < knum; i++) {
    const KCSTR* key = keys + i;
    xkeys.push_back(std::string(key->buf, key->size));
  }
  return pdb->remove_bulk(xkeys, atomic);
}


/**
 * Retrieve records at once.
 */
int64_t kcdbgetbulk(KCDB* db, const KCSTR* keys, size_t knum, KCREC* recs, int32_t atomic) {
  _assert_(db && keys && knum <= MEMMAXSIZ && recs);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> xkeys;
  xkeys.reserve(knum);
  for (size_t i = 0; i < knum; i++) {
    const KCSTR* key = keys + i;
    xkeys.push_back(std::string(key->buf, key->size));
  }
  std::map<std::string, std::string> xrecs;
  if (pdb->get_bulk(xkeys, &xrecs, atomic) < 0) return -1;
  std::map<std::string, std::string>::iterator it = xrecs.begin();
  std::map<std::string, std::string>::iterator itend = xrecs.end();
  size_t ridx = 0;
  while (ridx < knum && it != itend) {
    size_t ksiz = it->first.size();
    char* kbuf = new char[ksiz+1];
    std::memcpy(kbuf, it->first.data(), ksiz);
    kbuf[ksiz] = '\0';
    size_t vsiz = it->second.size();
    char* vbuf = new char[vsiz+1];
    std::memcpy(vbuf, it->second.data(), vsiz);
    vbuf[vsiz] = '\0';
    KCREC* rec = recs + (ridx++);
    rec->key.buf = kbuf;
    rec->key.size = ksiz;
    rec->value.buf = vbuf;
    rec->value.size = vsiz;
    ++it;
  }
  return ridx;
}


/**
 * Synchronize updated contents with the file and the device.
 */
int32_t kcdbsync(KCDB* db, int32_t hard, KCFILEPROC proc, void* opq) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  class FileProcessorImpl : public BasicDB::FileProcessor {
   public:
    explicit FileProcessorImpl(KCFILEPROC proc, void* opq) : proc_(proc), opq_(opq) {}
    bool process(const std::string& path, int64_t count, int64_t size) {
      if (!proc_) return true;
      return proc_(path.c_str(), count, size, opq_);
    }
   private:
    KCFILEPROC proc_;
    void* opq_;
  };
  FileProcessorImpl myproc(proc, opq);
  return pdb->synchronize(hard, &myproc);
}


/**
 * Occupy database by locking and do something meanwhile.
 */
int32_t kcdboccupy(KCDB* db, int32_t writable, KCFILEPROC proc, void* opq) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  class FileProcessorImpl : public BasicDB::FileProcessor {
   public:
    explicit FileProcessorImpl(KCFILEPROC proc, void* opq) : proc_(proc), opq_(opq) {}
    bool process(const std::string& path, int64_t count, int64_t size) {
      if (!proc_) return true;
      return proc_(path.c_str(), count, size, opq_);
    }
   private:
    KCFILEPROC proc_;
    void* opq_;
  };
  FileProcessorImpl myproc(proc, opq);
  return pdb->occupy(writable, &myproc);
}


/**
 * Create a copy of the database file.
 */
int32_t kcdbcopy(KCDB* db, const char* dest) {
  _assert_(db && dest);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->copy(dest);
}


/**
 * Begin transaction.
 */
int32_t kcdbbegintran(KCDB* db, int32_t hard) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->begin_transaction(hard);
}


/**
 * Try to begin transaction.
 */
int32_t kcdbbegintrantry(KCDB* db, int32_t hard) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->begin_transaction_try(hard);
}


/**
 * End transaction.
 */
int32_t kcdbendtran(KCDB* db, int32_t commit) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->end_transaction(commit);
}


/**
 * Remove all records.
 */
int32_t kcdbclear(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->clear();
}


/**
 * Dump records into a file.
 */
int32_t kcdbdumpsnap(KCDB* db, const char* dest) {
  _assert_(db && dest);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->dump_snapshot(dest);
}


/**
 * Load records from a file.
 */
int32_t kcdbloadsnap(KCDB* db, const char* src) {
  _assert_(db && src);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->load_snapshot(src);
}


/**
 * Get the number of records.
 */
int64_t kcdbcount(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->count();
}


/**
 * Get the size of the database file.
 */
int64_t kcdbsize(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->size();
}


/**
 * Get the path of the database file.
 */
char* kcdbpath(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  std::string path = pdb->path();
  size_t psiz = path.size();
  char* pbuf = new char[psiz+1];
  std::memcpy(pbuf, path.c_str(), psiz + 1);
  return pbuf;
}


/**
 * Get the miscellaneous status information.
 */
char* kcdbstatus(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  std::map<std::string, std::string> status;
  if (!pdb->status(&status)) return NULL;
  std::ostringstream obuf;
  std::map<std::string, std::string>::iterator it = status.begin();
  std::map<std::string, std::string>::iterator itend = status.end();
  while (it != itend) {
    obuf << it->first << "\t" << it->second << "\n";
    ++it;
  }
  std::string sstr = obuf.str();
  size_t ssiz = sstr.size();
  char* sbuf = new char[ssiz+1];
  std::memcpy(sbuf, sstr.c_str(), ssiz + 1);
  return sbuf;
}


/**
 * Get keys matching a prefix string.
 */
int64_t kcdbmatchprefix(KCDB* db, const char* prefix, char** strary, size_t max) {
  _assert_(db && prefix && strary && max <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> strvec;
  if (pdb->match_prefix(prefix, &strvec, max) == -1) return -1;
  int64_t cnt = 0;
  std::vector<std::string>::iterator it = strvec.begin();
  std::vector<std::string>::iterator itend = strvec.end();
  while (it != itend) {
    size_t ksiz = it->size();
    char* kbuf = new char[ksiz+1];
    std::memcpy(kbuf, it->data(), ksiz);
    kbuf[ksiz] = '\0';
    strary[cnt++] = kbuf;
    ++it;
  }
  return cnt;
}


/**
 * Get keys matching a regular expression string.
 */
int64_t kcdbmatchregex(KCDB* db, const char* regex, char** strary, size_t max) {
  _assert_(db && regex && strary && max <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> strvec;
  if (pdb->match_regex(regex, &strvec, max) == -1) return -1;
  int64_t cnt = 0;
  std::vector<std::string>::iterator it = strvec.begin();
  std::vector<std::string>::iterator itend = strvec.end();
  while (it != itend) {
    size_t ksiz = it->size();
    char* kbuf = new char[ksiz+1];
    std::memcpy(kbuf, it->data(), ksiz);
    kbuf[ksiz] = '\0';
    strary[cnt++] = kbuf;
    ++it;
  }
  return cnt;
}


/**
 * Get keys similar to a string in terms of the levenshtein distance.
 */
int64_t kcdbmatchsimilar(KCDB* db, const char* origin, uint32_t range, int32_t utf,
                         char** strary, size_t max) {
  _assert_(db && origin && strary && max <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  std::vector<std::string> strvec;
  if (pdb->match_similar(origin, range, utf, &strvec, max) == -1) return -1;
  int64_t cnt = 0;
  std::vector<std::string>::iterator it = strvec.begin();
  std::vector<std::string>::iterator itend = strvec.end();
  while (it != itend) {
    size_t ksiz = it->size();
    char* kbuf = new char[ksiz+1];
    std::memcpy(kbuf, it->data(), ksiz);
    kbuf[ksiz] = '\0';
    strary[cnt++] = kbuf;
    ++it;
  }
  return cnt;
}


/**
 * Merge records from other databases.
 */
int32_t kcdbmerge(KCDB* db, KCDB** srcary, size_t srcnum, uint32_t mode) {
  _assert_(db && srcary && srcnum <= MEMMAXSIZ);
  PolyDB* pdb = (PolyDB*)db;
  return pdb->merge((BasicDB**)srcary, srcnum, (PolyDB::MergeMode)mode);
}


/**
 * Create a cursor object.
 */
KCCUR* kcdbcursor(KCDB* db) {
  _assert_(db);
  PolyDB* pdb = (PolyDB*)db;
  return (KCCUR*)pdb->cursor();
}


/**
 * Destroy a cursor object.
 */
void kccurdel(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  delete pcur;
}


/**
 * Accept a visitor to the current record.
 */
int32_t kccuraccept(KCCUR* cur, KCVISITFULL fullproc, void* opq,
                    int32_t writable, int32_t step) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  class VisitorImpl : public DB::Visitor {
   public:
    explicit VisitorImpl(KCVISITFULL fullproc, void* opq) : fullproc_(fullproc), opq_(opq) {}
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!fullproc_) return NOP;
      return fullproc_(kbuf, ksiz, vbuf, vsiz, sp, opq_);
    }
   private:
    KCVISITFULL fullproc_;
    void* opq_;
  };
  VisitorImpl visitor(fullproc, opq);
  return pcur->accept(&visitor, writable, step);
}


/**
 * Set the value of the current record.
 */
int32_t kccursetvalue(KCCUR* cur, const char* vbuf, size_t vsiz, int32_t step) {
  _assert_(cur && vbuf && vsiz <= MEMMAXSIZ);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->set_value(vbuf, vsiz, step);
}


/**
 * Remove the current record.
 */
int32_t kccurremove(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->remove();
}


/**
 * Get the key of the current record.
 */
char* kccurgetkey(KCCUR* cur, size_t* sp, int32_t step) {
  _assert_(cur && sp);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->get_key(sp, step);
}


/**
 * Get the value of the current record.
 */
char* kccurgetvalue(KCCUR* cur, size_t* sp, int32_t step) {
  _assert_(cur && sp);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->get_value(sp, step);
}


/**
 * Get a pair of the key and the value of the current record.
 */
char* kccurget(KCCUR* cur, size_t* ksp, const char** vbp, size_t* vsp, int32_t step) {
  _assert_(cur && ksp && vbp && vsp);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->get(ksp, vbp, vsp, step);
}


/**
 * Get a pair of the key and the value of the current record and remove it atomically.
 */
char* kccurseize(KCCUR* cur, size_t* ksp, const char** vbp, size_t* vsp) {
  _assert_(cur && ksp && vbp && vsp);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->seize(ksp, vbp, vsp);
}


/**
 * Jump the cursor to the first record.
 */
int32_t kccurjump(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->jump();
}


/**
 * Jump the cursor to a record.
 */
int32_t kccurjumpkey(KCCUR* cur, const char* kbuf, size_t ksiz) {
  _assert_(cur && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->jump(kbuf, ksiz);
}


/**
 * Jump the cursor to the last record for backward scan.
 */
int32_t kccurjumpback(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->jump_back();
}


/**
 * Jump the cursor to a record for backward scan.
 */
int32_t kccurjumpbackkey(KCCUR* cur, const char* kbuf, size_t ksiz) {
  _assert_(cur && kbuf && ksiz <= MEMMAXSIZ);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->jump_back(kbuf, ksiz);
}


/**
 * Step the cursor to the next record.
 */
int32_t kccurstep(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->step();
}


/**
 * Step the cursor to the previous record.
 */
int32_t kccurstepback(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->step_back();
}


/**
 * Get the database object.
 */
KCDB* kccurdb(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return (KCDB*)pcur->db();
}


/**
 * Get the code of the last happened error.
 */
int32_t kccurecode(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->error().code();
}


/**
 * Get the supplement message of the last happened error.
 */
const char* kccuremsg(KCCUR* cur) {
  _assert_(cur);
  PolyDB::Cursor* pcur = (PolyDB::Cursor*)cur;
  return pcur->error().message();
}


/**
 * Create an index database object.
 */
KCIDX* kcidxnew(void) {
  _assert_(true);
  return (KCIDX*)new IndexDB;
}


/**
 * Destroy a database object.
 */
void kcidxdel(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  delete idb;
}


/**
 * Open a database file.
 */
int32_t kcidxopen(KCIDX* idx, const char* path, uint32_t mode) {
  _assert_(idx && path);
  IndexDB* idb = (IndexDB*)idx;
  return idb->open(path, mode);
}


/**
 * Close the database file.
 */
int32_t kcidxclose(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->close();
}


/**
 * Get the code of the last happened error.
 */
int32_t kcidxecode(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->error().code();
}


/**
 * Get the supplement message of the last happened error.
 */
const char* kcidxemsg(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->error().message();
}


/**
 * Set the value of a record.
 */
int32_t kcidxset(KCIDX* idx, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  IndexDB* idb = (IndexDB*)idx;
  return idb->set(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Add a record.
 */
int32_t kcidxadd(KCIDX* idx, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  IndexDB* idb = (IndexDB*)idx;
  return idb->add(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Replace the value of a record.
 */
int32_t kcidxreplace(KCIDX* idx, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  IndexDB* idb = (IndexDB*)idx;
  return idb->replace(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Append the value of a record.
 */
int32_t kcidxappend(KCIDX* idx, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  IndexDB* idb = (IndexDB*)idx;
  return idb->append(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Remove a record.
 */
int32_t kcidxremove(KCIDX* idx, const char* kbuf, size_t ksiz) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ);
  IndexDB* idb = (IndexDB*)idx;
  return idb->remove(kbuf, ksiz);
}


/**
 * Retrieve the value of a record.
 */
char* kcidxget(KCIDX* idx, const char* kbuf, size_t ksiz, size_t* sp) {
  _assert_(idx && kbuf && ksiz <= MEMMAXSIZ && sp);
  IndexDB* idb = (IndexDB*)idx;
  return idb->get(kbuf, ksiz, sp);
}


/**
 * Synchronize updated contents with the file and the device.
 */
int32_t kcidxsync(KCIDX* idx, int32_t hard, KCFILEPROC proc, void* opq) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  class FileProcessorImpl : public BasicDB::FileProcessor {
   public:
    explicit FileProcessorImpl(KCFILEPROC proc, void* opq) : proc_(proc), opq_(opq) {}
    bool process(const std::string& path, int64_t count, int64_t size) {
      if (!proc_) return true;
      return proc_(path.c_str(), count, size, opq_);
    }
   private:
    KCFILEPROC proc_;
    void* opq_;
  };
  FileProcessorImpl myproc(proc, opq);
  return idb->synchronize(hard, &myproc);
}


/**
 * Remove all records.
 */
int32_t kcidxclear(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->clear();
}


/**
 * Get the number of records.
 */
int64_t kcidxcount(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->count();
}


/**
 * Get the size of the database file.
 */
int64_t kcidxsize(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return idb->size();
}


/**
 * Get the path of the database file.
 */
char* kcidxpath(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  std::string path = idb->path();
  size_t psiz = path.size();
  char* pbuf = new char[psiz+1];
  std::memcpy(pbuf, path.c_str(), psiz + 1);
  return pbuf;
}


/**
 * Get the miscellaneous status information.
 */
char* kcidxstatus(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  std::map<std::string, std::string> status;
  if (!idb->status(&status)) return NULL;
  std::ostringstream obuf;
  std::map<std::string, std::string>::iterator it = status.begin();
  std::map<std::string, std::string>::iterator itend = status.end();
  while (it != itend) {
    obuf << it->first << "\t" << it->second << "\n";
    ++it;
  }
  std::string sstr = obuf.str();
  size_t ssiz = sstr.size();
  char* sbuf = new char[ssiz+1];
  std::memcpy(sbuf, sstr.c_str(), ssiz + 1);
  return sbuf;
}


/**
 * Reveal the inner database object.
 */
KCDB* kcidxrevealinnerdb(KCIDX* idx) {
  _assert_(idx);
  IndexDB* idb = (IndexDB*)idx;
  return (KCDB*)idb->reveal_inner_db();
}


/**
 * Create a string hash map object.
 */
KCMAP* kcmapnew(size_t bnum) {
  _assert_(true);
  return (KCMAP*)new TinyHashMap(bnum);
}


/**
 * Destroy a map object.
 */
void kcmapdel(KCMAP* map) {
  _assert_(map);
  TinyHashMap* thm = (TinyHashMap*)map;
  delete thm;
}


/**
 * Set the value of a record.
 */
void kcmapset(KCMAP* map, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  TinyHashMap* thm = (TinyHashMap*)map;
  thm->set(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Add a record.
 */
int32_t kcmapadd(KCMAP* map, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  TinyHashMap* thm = (TinyHashMap*)map;
  return thm->add(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Replace the value of a record.
 */
int32_t kcmapreplace(KCMAP* map, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  TinyHashMap* thm = (TinyHashMap*)map;
  return thm->replace(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Append the value of a record.
 */
void kcmapappend(KCMAP* map, const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
  TinyHashMap* thm = (TinyHashMap*)map;
  thm->append(kbuf, ksiz, vbuf, vsiz);
}


/**
 * Remove a record.
 */
int32_t kcmapremove(KCMAP* map, const char* kbuf, size_t ksiz) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ);
  TinyHashMap* thm = (TinyHashMap*)map;
  return thm->remove(kbuf, ksiz);
}


/**
 * Retrieve the value of a record.
 */
const char* kcmapget(KCMAP* map, const char* kbuf, size_t ksiz, size_t* sp) {
  _assert_(map && kbuf && ksiz <= MEMMAXSIZ && sp);
  TinyHashMap* thm = (TinyHashMap*)map;
  return thm->get(kbuf, ksiz, sp);
}


/**
 * Remove all records.
 */
void kcmapclear(KCMAP* map) {
  _assert_(map);
  TinyHashMap* thm = (TinyHashMap*)map;
  thm->clear();
}


/**
 * Get the number of records.
 */
size_t kcmapcount(KCMAP* map) {
  _assert_(map);
  TinyHashMap* thm = (TinyHashMap*)map;
  return thm->count();
}


/**
 * Create a string hash map iterator object.
 */
KCMAPITER* kcmapiterator(KCMAP* map) {
  _assert_(map);
  TinyHashMap* thm = (TinyHashMap*)map;
  return (KCMAPITER*)new TinyHashMap::Iterator(thm);
}


/**
 * Destroy an iterator object.
 */
void kcmapiterdel(KCMAPITER* iter) {
  _assert_(iter);
  TinyHashMap::Iterator* thmi = (TinyHashMap::Iterator*)iter;
  delete thmi;
}


/**
 * Get the key of the current record.
 */
const char* kcmapitergetkey(KCMAPITER* iter, size_t* sp) {
  _assert_(iter && sp);
  TinyHashMap::Iterator* thmi = (TinyHashMap::Iterator*)iter;
  return thmi->get_key(sp);
}


/**
 * Get the value of the current record.
 */
const char* kcmapitergetvalue(KCMAPITER* iter, size_t* sp) {
  _assert_(iter && sp);
  TinyHashMap::Iterator* thmi = (TinyHashMap::Iterator*)iter;
  return thmi->get_value(sp);
}


/**
 * Get a pair of the key and the value of the current record.
 */
const char* kcmapiterget(KCMAPITER* iter, size_t* ksp, const char** vbp, size_t* vsp) {
  _assert_(iter && ksp && vbp && vsp);
  TinyHashMap::Iterator* thmi = (TinyHashMap::Iterator*)iter;
  return thmi->get(ksp, vbp, vsp);
}


/**
 * Step the cursor to the next record.
 */
void kcmapiterstep(KCMAPITER* iter) {
  _assert_(iter);
  TinyHashMap::Iterator* thmi = (TinyHashMap::Iterator*)iter;
  return thmi->step();
}


/**
 * Create a string hash map sorter object.
 */
KCMAPSORT* kcmapsorter(KCMAP* map) {
  _assert_(map);
  TinyHashMap* thm = (TinyHashMap*)map;
  return (KCMAPSORT*)new TinyHashMap::Sorter(thm);
}


/**
 * Destroy an sorter object.
 */
void kcmapsortdel(KCMAPSORT* sort) {
  _assert_(sort);
  TinyHashMap::Sorter* thms = (TinyHashMap::Sorter*)sort;
  delete thms;
}


/**
 * Get the key of the current record.
 */
const char* kcmapsortgetkey(KCMAPSORT* sort, size_t* sp) {
  _assert_(sort && sp);
  TinyHashMap::Sorter* thms = (TinyHashMap::Sorter*)sort;
  return thms->get_key(sp);
}


/**
 * Get the value of the current record.
 */
const char* kcmapsortgetvalue(KCMAPSORT* sort, size_t* sp) {
  _assert_(sort && sp);
  TinyHashMap::Sorter* thms = (TinyHashMap::Sorter*)sort;
  return thms->get_value(sp);
}


/**
 * Get a pair of the key and the value of the current record.
 */
const char* kcmapsortget(KCMAPSORT* sort, size_t* ksp, const char** vbp, size_t* vsp) {
  _assert_(sort && ksp && vbp && vsp);
  TinyHashMap::Sorter* thms = (TinyHashMap::Sorter*)sort;
  return thms->get(ksp, vbp, vsp);
}


/**
 * Step the cursor to the next record.
 */
void kcmapsortstep(KCMAPSORT* sort) {
  _assert_(sort);
  TinyHashMap::Sorter* thms = (TinyHashMap::Sorter*)sort;
  return thms->step();
}


/**
 * Create a string array list object.
 */
KCLIST* kclistnew() {
  _assert_(true);
  return (KCLIST*)new TinyArrayList();
}


/**
 * Destroy a list object.
 */
void kclistdel(KCLIST* list) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  delete tal;
}


/**
 * Insert a record at the bottom of the list.
 */
void kclistpush(KCLIST* list, const char* buf, size_t size) {
  _assert_(list && buf && size <= MEMMAXSIZ);
  TinyArrayList* tal = (TinyArrayList*)list;
  tal->push(buf, size);
}


/**
 * Remove a record at the bottom of the list.
 */
int32_t kclistpop(KCLIST* list) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  return tal->pop();
}


/**
 * Insert a record at the top of the list.
 */
void kclistunshift(KCLIST* list, const char* buf, size_t size) {
  _assert_(list && buf && size <= MEMMAXSIZ);
  TinyArrayList* tal = (TinyArrayList*)list;
  tal->unshift(buf, size);
}


/**
 * Remove a record at the top of the list.
 */
int32_t kclistshift(KCLIST* list) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  return tal->shift();
}


/**
 * Insert a record at the position of the given index of the list.
 */
void kclistinsert(KCLIST* list, const char* buf, size_t size, size_t idx) {
  _assert_(list && buf && size <= MEMMAXSIZ);
  TinyArrayList* tal = (TinyArrayList*)list;
  tal->insert(buf, size, idx);
}


/**
 * Remove a record at the position of the given index of the list.
 */
void kclistremove(KCLIST* list, size_t idx) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  return tal->remove(idx);
}


/**
 * Retrieve a record at the position of the given index of the list.
 */
const char* kclistget(KCLIST* list, size_t idx, size_t* sp) {
  _assert_(list && sp);
  TinyArrayList* tal = (TinyArrayList*)list;
  return tal->get(idx, sp);
}


/**
 * Remove all records.
 */
void kclistclear(KCLIST* list) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  tal->clear();
}


/**
 * Get the number of records.
 */
size_t kclistcount(KCLIST* list) {
  _assert_(list);
  TinyArrayList* tal = (TinyArrayList*)list;
  return tal->count();
}


}

// END OF FILE
