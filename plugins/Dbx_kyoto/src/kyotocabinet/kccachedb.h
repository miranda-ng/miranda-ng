/*************************************************************************************************
 * Cache hash database
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


#ifndef _KCCACHEDB_H                     // duplication check
#define _KCCACHEDB_H

#include <kccommon.h>
#include <kcutil.h>
#include <kcthread.h>
#include <kcfile.h>
#include <kccompress.h>
#include <kccompare.h>
#include <kcmap.h>
#include <kcregex.h>
#include <kcdb.h>
#include <kcplantdb.h>

namespace kyotocabinet {                 // common namespace


/**
 * On-memory hash database with LRU deletion.
 * @note This class is a concrete class to operate a hash database on memory.  This class can be
 * inherited but overwriting methods is forbidden.  Before every database operation, it is
 * necessary to call the CacheDB::open method in order to open a database file and connect the
 * database object to it.  To avoid data missing or corruption, it is important to close every
 * database file by the CacheDB::close method when the database is no longer in use.  It is
 * forbidden for multible database objects in a process to open the same database at the same
 * time.  It is forbidden to share a database object with child processes.
 */
class CacheDB : public BasicDB {
  friend class PlantDB<CacheDB, BasicDB::TYPEGRASS>;
 public:
  class Cursor;
 private:
  struct Record;
  struct TranLog;
  struct Slot;
  class Repeater;
  class Setter;
  class Remover;
  class ScopedVisitor;
  /** An alias of list of cursors. */
  typedef std::list<Cursor*> CursorList;
  /** An alias of list of transaction logs. */
  typedef std::list<TranLog> TranLogList;
  /** The number of slot tables. */
  static const int32_t SLOTNUM = 16;
  /** The default bucket number. */
  static const size_t DEFBNUM = 1048583LL;
  /** The mininum number of buckets to use mmap. */
  static const size_t ZMAPBNUM = 32768;
  /** The maximum size of each key. */
  static const uint32_t KSIZMAX = 0xfffff;
  /** The size of the record buffer. */
  static const size_t RECBUFSIZ = 48;
  /** The size of the opaque buffer. */
  static const size_t OPAQUESIZ = 16;
  /** The threshold of busy loop and sleep for locking. */
  static const uint32_t LOCKBUSYLOOP = 8192;
 public:
  /**
   * Cursor to indicate a record.
   */
  class Cursor : public BasicDB::Cursor {
    friend class CacheDB;
   public:
    /**
     * Constructor.
     * @param db the container database object.
     */
    explicit Cursor(CacheDB* db) : db_(db), sidx_(-1), rec_(NULL) {
      _assert_(db);
      ScopedRWLock lock(&db_->mlock_, true);
      db_->curs_.push_back(this);
    }
    /**
     * Destructor.
     */
    virtual ~Cursor() {
      _assert_(true);
      if (!db_) return;
      ScopedRWLock lock(&db_->mlock_, true);
      db_->curs_.remove(this);
    }
    /**
     * Accept a visitor to the current record.
     * @param visitor a visitor object.
     * @param writable true for writable operation, or false for read-only operation.
     * @param step true to move the cursor to the next record, or false for no move.
     * @return true on success, or false on failure.
     * @note The operation for each record is performed atomically and other threads accessing
     * the same record are blocked.  To avoid deadlock, any explicit database operation must not
     * be performed in this function.
     */
    bool accept(Visitor* visitor, bool writable = true, bool step = false) {
      _assert_(visitor);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (writable && !(db_->omode_ & OWRITER)) {
        db_->set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        return false;
      }
      if (sidx_ < 0 || !rec_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      uint32_t rksiz = rec_->ksiz & KSIZMAX;
      char* dbuf = (char*)rec_ + sizeof(*rec_);
      const char* rvbuf = dbuf + rksiz;
      size_t rvsiz = rec_->vsiz;
      char* zbuf = NULL;
      size_t zsiz = 0;
      if (db_->comp_) {
        zbuf = db_->comp_->decompress(rvbuf, rvsiz, &zsiz);
        if (zbuf) {
          rvbuf = zbuf;
          rvsiz = zsiz;
        }
      }
      size_t vsiz;
      const char* vbuf = visitor->visit_full(dbuf, rksiz, rvbuf, rvsiz, &vsiz);
      delete[] zbuf;
      if (vbuf == Visitor::REMOVE) {
        uint64_t hash = db_->hash_record(dbuf, rksiz) / SLOTNUM;
        Slot* slot = db_->slots_ + sidx_;
        Repeater repeater(Visitor::REMOVE, 0);
        db_->accept_impl(slot, hash, dbuf, rksiz, &repeater, db_->comp_, false);
      } else if (vbuf == Visitor::NOP) {
        if (step) step_impl();
      } else {
        uint64_t hash = db_->hash_record(dbuf, rksiz) / SLOTNUM;
        Slot* slot = db_->slots_ + sidx_;
        Repeater repeater(vbuf, vsiz);
        db_->accept_impl(slot, hash, dbuf, rksiz, &repeater, db_->comp_, false);
        if (step) step_impl();
      }
      return true;
    }
    /**
     * Jump the cursor to the first record for forward scan.
     * @return true on success, or false on failure.
     */
    bool jump() {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      for (int32_t i = 0; i < SLOTNUM; i++) {
        Slot* slot = db_->slots_ + i;
        if (slot->first) {
          sidx_ = i;
          rec_ = slot->first;
          return true;
        }
      }
      db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
      sidx_ = -1;
      rec_ = NULL;
      return false;
    }
    /**
     * Jump the cursor to a record for forward scan.
     * @param kbuf the pointer to the key region.
     * @param ksiz the size of the key region.
     * @return true on success, or false on failure.
     */
    bool jump(const char* kbuf, size_t ksiz) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (ksiz > KSIZMAX) ksiz = KSIZMAX;
      uint64_t hash = db_->hash_record(kbuf, ksiz);
      int32_t sidx = hash % SLOTNUM;
      hash /= SLOTNUM;
      Slot* slot = db_->slots_ + sidx;
      size_t bidx = hash % slot->bnum;
      Record* rec = slot->buckets[bidx];
      Record** entp = slot->buckets + bidx;
      uint32_t fhash = db_->fold_hash(hash) & ~KSIZMAX;
      while (rec) {
        uint32_t rhash = rec->ksiz & ~KSIZMAX;
        uint32_t rksiz = rec->ksiz & KSIZMAX;
        if (fhash > rhash) {
          entp = &rec->left;
          rec = rec->left;
        } else if (fhash < rhash) {
          entp = &rec->right;
          rec = rec->right;
        } else {
          char* dbuf = (char*)rec + sizeof(*rec);
          int32_t kcmp = db_->compare_keys(kbuf, ksiz, dbuf, rksiz);
          if (kcmp < 0) {
            entp = &rec->left;
            rec = rec->left;
          } else if (kcmp > 0) {
            entp = &rec->right;
            rec = rec->right;
          } else {
            sidx_ = sidx;
            rec_ = rec;
            return true;
          }
        }
      }
      db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
      sidx_ = -1;
      rec_ = NULL;
      return false;
    }
    /**
     * Jump the cursor to a record for forward scan.
     * @note Equal to the original Cursor::jump method except that the parameter is std::string.
     */
    bool jump(const std::string& key) {
      _assert_(true);
      return jump(key.c_str(), key.size());
    }
    /**
     * Jump the cursor to the last record for backward scan.
     * @note This is a dummy implementation for compatibility.
     */
    bool jump_back() {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      db_->set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
      return false;
    }
    /**
     * Jump the cursor to a record for backward scan.
     * @note This is a dummy implementation for compatibility.
     */
    bool jump_back(const char* kbuf, size_t ksiz) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      db_->set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
      return false;
    }
    /**
     * Jump the cursor to a record for backward scan.
     * @note This is a dummy implementation for compatibility.
     */
    bool jump_back(const std::string& key) {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      db_->set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
      return false;
    }
    /**
     * Step the cursor to the next record.
     * @return true on success, or false on failure.
     */
    bool step() {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (sidx_ < 0 || !rec_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      bool err = false;
      if (!step_impl()) err = true;
      return !err;
    }
    /**
     * Step the cursor to the previous record.
     * @note This is a dummy implementation for compatibility.
     */
    bool step_back() {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      db_->set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
      return false;
    }
    /**
     * Get the database object.
     * @return the database object.
     */
    CacheDB* db() {
      _assert_(true);
      return db_;
    }
   private:
    /**
     * Step the cursor to the next record.
     * @return true on success, or false on failure.
     */
    bool step_impl() {
      _assert_(true);
      rec_ = rec_->next;
      if (!rec_) {
        for (int32_t i = sidx_ + 1; i < SLOTNUM; i++) {
          Slot* slot = db_->slots_ + i;
          if (slot->first) {
            sidx_ = i;
            rec_ = slot->first;
            return true;
          }
        }
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        sidx_ = -1;
        rec_ = NULL;
        return false;
      }
      return true;
    }
    /** Dummy constructor to forbid the use. */
    Cursor(const Cursor&);
    /** Dummy Operator to forbid the use. */
    Cursor& operator =(const Cursor&);
    /** The inner database. */
    CacheDB* db_;
    /** The index of the current slot. */
    int32_t sidx_;
    /** The current record. */
    Record* rec_;
  };
  /**
   * Tuning options.
   */
  enum Option {
    TSMALL = 1 << 0,                     ///< dummy for compatibility
    TLINEAR = 1 << 1,                    ///< dummy for compatibility
    TCOMPRESS = 1 << 2                   ///< compress each record
  };
  /**
   * Status flags.
   */
  enum Flag {
    FOPEN = 1 << 0,                      ///< dummy for compatibility
    FFATAL = 1 << 1                      ///< dummy for compatibility
  };
  /**
   * Default constructor.
   */
  explicit CacheDB() :
      mlock_(), flock_(), error_(), logger_(NULL), logkinds_(0), mtrigger_(NULL),
      omode_(0), curs_(), path_(""), type_(TYPECACHE),
      opts_(0), bnum_(DEFBNUM), capcnt_(-1), capsiz_(-1),
      opaque_(), embcomp_(ZLIBRAWCOMP), comp_(NULL), slots_(), rttmode_(true), tran_(false) {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  virtual ~CacheDB() {
    _assert_(true);
    if (omode_ != 0) close();
    if (!curs_.empty()) {
      CursorList::const_iterator cit = curs_.begin();
      CursorList::const_iterator citend = curs_.end();
      while (cit != citend) {
        Cursor* cur = *cit;
        cur->db_ = NULL;
        ++cit;
      }
    }
  }
  /**
   * Accept a visitor to a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param visitor a visitor object.
   * @param writable true for writable operation, or false for read-only operation.
   * @return true on success, or false on failure.
   * @note The operation for each record is performed atomically and other threads accessing the
   * same record are blocked.  To avoid deadlock, any explicit database operation must not be
   * performed in this function.
   */
  bool accept(const char* kbuf, size_t ksiz, Visitor* visitor, bool writable = true) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    if (ksiz > KSIZMAX) ksiz = KSIZMAX;
    uint64_t hash = hash_record(kbuf, ksiz);
    int32_t sidx = hash % SLOTNUM;
    hash /= SLOTNUM;
    Slot* slot = slots_ + sidx;
    slot->lock.lock();
    accept_impl(slot, hash, kbuf, ksiz, visitor, comp_, rttmode_);
    slot->lock.unlock();
    return true;
  }
  /**
   * Accept a visitor to multiple records at once.
   * @param keys specifies a string vector of the keys.
   * @param visitor a visitor object.
   * @param writable true for writable operation, or false for read-only operation.
   * @return true on success, or false on failure.
   * @note The operations for specified records are performed atomically and other threads
   * accessing the same records are blocked.  To avoid deadlock, any explicit database operation
   * must not be performed in this function.
   */
  bool accept_bulk(const std::vector<std::string>& keys, Visitor* visitor,
                   bool writable = true) {
    _assert_(visitor);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    size_t knum = keys.size();
    if (knum < 1) return true;
    struct RecordKey {
      const char* kbuf;
      size_t ksiz;
      uint64_t hash;
      int32_t sidx;
    };
    RecordKey* rkeys = new RecordKey[knum];
    std::set<int32_t> sidxs;
    for (size_t i = 0; i < knum; i++) {
      const std::string& key = keys[i];
      RecordKey* rkey = rkeys + i;
      rkey->kbuf = key.data();
      rkey->ksiz = key.size();
      if (rkey->ksiz > KSIZMAX) rkey->ksiz = KSIZMAX;
      rkey->hash = hash_record(rkey->kbuf, rkey->ksiz);
      rkey->sidx = rkey->hash % SLOTNUM;
      sidxs.insert(rkey->sidx);
      rkey->hash /= SLOTNUM;
    }
    std::set<int32_t>::iterator sit = sidxs.begin();
    std::set<int32_t>::iterator sitend = sidxs.end();
    while (sit != sitend) {
      Slot* slot = slots_ + *sit;
      slot->lock.lock();
      ++sit;
    }
    for (size_t i = 0; i < knum; i++) {
      RecordKey* rkey = rkeys + i;
      Slot* slot = slots_ + rkey->sidx;
      accept_impl(slot, rkey->hash, rkey->kbuf, rkey->ksiz, visitor, comp_, rttmode_);
    }
    sit = sidxs.begin();
    sitend = sidxs.end();
    while (sit != sitend) {
      Slot* slot = slots_ + *sit;
      slot->lock.unlock();
      ++sit;
    }
    delete[] rkeys;
    return true;
  }
  /**
   * Iterate to accept a visitor for each record.
   * @param visitor a visitor object.
   * @param writable true for writable operation, or false for read-only operation.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   * @note The whole iteration is performed atomically and other threads are blocked.  To avoid
   * deadlock, any explicit database operation must not be performed in this function.
   */
  bool iterate(Visitor *visitor, bool writable = true, ProgressChecker* checker = NULL) {
    _assert_(visitor);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    int64_t allcnt = count_impl();
    if (checker && !checker->check("iterate", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    int64_t curcnt = 0;
    for (int32_t i = 0; i < SLOTNUM; i++) {
      Slot* slot = slots_ + i;
      Record* rec = slot->first;
      while (rec) {
        Record* next = rec->next;
        uint32_t rksiz = rec->ksiz & KSIZMAX;
        char* dbuf = (char*)rec + sizeof(*rec);
        const char* rvbuf = dbuf + rksiz;
        size_t rvsiz = rec->vsiz;
        char* zbuf = NULL;
        size_t zsiz = 0;
        if (comp_) {
          zbuf = comp_->decompress(rvbuf, rvsiz, &zsiz);
          if (zbuf) {
            rvbuf = zbuf;
            rvsiz = zsiz;
          }
        }
        size_t vsiz;
        const char* vbuf = visitor->visit_full(dbuf, rksiz, rvbuf, rvsiz, &vsiz);
        delete[] zbuf;
        if (vbuf == Visitor::REMOVE) {
          uint64_t hash = hash_record(dbuf, rksiz) / SLOTNUM;
          Repeater repeater(Visitor::REMOVE, 0);
          accept_impl(slot, hash, dbuf, rksiz, &repeater, comp_, false);
        } else if (vbuf != Visitor::NOP) {
          uint64_t hash = hash_record(dbuf, rksiz) / SLOTNUM;
          Repeater repeater(vbuf, vsiz);
          accept_impl(slot, hash, dbuf, rksiz, &repeater, comp_, false);
        }
        rec = next;
        curcnt++;
        if (checker && !checker->check("iterate", "processing", curcnt, allcnt)) {
          set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
          return false;
        }
      }
    }
    if (checker && !checker->check("iterate", "ending", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    trigger_meta(MetaTrigger::ITERATE, "iterate");
    return true;
  }
  /**
   * Scan each record in parallel.
   * @param visitor a visitor object.
   * @param thnum the number of worker threads.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   * @note This function is for reading records and not for updating ones.  The return value of
   * the visitor is just ignored.  To avoid deadlock, any explicit database operation must not
   * be performed in this function.
   */
  bool scan_parallel(Visitor *visitor, size_t thnum, ProgressChecker* checker = NULL) {
    _assert_(visitor && thnum <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (thnum < 1) thnum = 1;
    thnum = std::pow(2.0, (int32_t)(std::log(thnum * std::sqrt(2.0)) / std::log(2.0)));
    if (thnum > (size_t)SLOTNUM) thnum = SLOTNUM;
    ScopedVisitor svis(visitor);
    int64_t allcnt = count_impl();
    if (checker && !checker->check("scan_parallel", "beginning", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    class ThreadImpl : public Thread {
     public:
      explicit ThreadImpl() :
          db_(NULL), visitor_(NULL), checker_(NULL), allcnt_(0), slots_(), error_() {}
      void init(CacheDB* db, Visitor* visitor, ProgressChecker* checker, int64_t allcnt) {
        db_ = db;
        visitor_ = visitor;
        checker_ = checker;
        allcnt_ = allcnt;
      }
      void add_slot(Slot* slot) {
        slots_.push_back(slot);
      }
      const Error& error() {
        return error_;
      }
     private:
      void run() {
        CacheDB* db = db_;
        Visitor* visitor = visitor_;
        ProgressChecker* checker = checker_;
        int64_t allcnt = allcnt_;
        Compressor* comp = db->comp_;
        std::vector<Slot*>::iterator sit = slots_.begin();
        std::vector<Slot*>::iterator sitend = slots_.end();
        while (sit != sitend) {
          Slot* slot = *sit;
          Record* rec = slot->first;
          while (rec) {
            Record* next = rec->next;
            uint32_t rksiz = rec->ksiz & KSIZMAX;
            char* dbuf = (char*)rec + sizeof(*rec);
            const char* rvbuf = dbuf + rksiz;
            size_t rvsiz = rec->vsiz;
            char* zbuf = NULL;
            size_t zsiz = 0;
            if (comp) {
              zbuf = comp->decompress(rvbuf, rvsiz, &zsiz);
              if (zbuf) {
                rvbuf = zbuf;
                rvsiz = zsiz;
              }
            }
            size_t vsiz;
            visitor->visit_full(dbuf, rksiz, rvbuf, rvsiz, &vsiz);
            delete[] zbuf;
            rec = next;
            if (checker && !checker->check("scan_parallel", "processing", -1, allcnt)) {
              db->set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
              error_ = db->error();
              break;
            }
          }
          ++sit;
        }
      }
      CacheDB* db_;
      Visitor* visitor_;
      ProgressChecker* checker_;
      int64_t allcnt_;
      std::vector<Slot*> slots_;
      Error error_;
    };
    bool err = false;
    bool orttmode = rttmode_;
    rttmode_ = false;
    ThreadImpl* threads = new ThreadImpl[thnum];
    for (int32_t i = 0; i < SLOTNUM; i++) {
      ThreadImpl* thread = threads + (i % thnum);
      thread->add_slot(slots_ + i);
    }
    for (size_t i = 0; i < thnum; i++) {
      ThreadImpl* thread = threads + i;
      thread->init(this, visitor, checker, allcnt);
      thread->start();
    }
    for (size_t i = 0; i < thnum; i++) {
      ThreadImpl* thread = threads + i;
      thread->join();
      if (thread->error() != Error::SUCCESS) {
        *error_ = thread->error();
        err = true;
      }
    }
    delete[] threads;
    rttmode_ = orttmode;
    if (err) return false;
    if (checker && !checker->check("scan_parallel", "ending", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    trigger_meta(MetaTrigger::ITERATE, "scan_parallel");
    return true;
  }
  /**
   * Get the last happened error.
   * @return the last happened error.
   */
  Error error() const {
    _assert_(true);
    return error_;
  }
  /**
   * Set the error information.
   * @param file the file name of the program source code.
   * @param line the line number of the program source code.
   * @param func the function name of the program source code.
   * @param code an error code.
   * @param message a supplement message.
   */
  void set_error(const char* file, int32_t line, const char* func,
                 Error::Code code, const char* message) {
    _assert_(file && line > 0 && func && message);
    error_->set(code, message);
    if (logger_) {
      Logger::Kind kind = code == Error::BROKEN || code == Error::SYSTEM ?
          Logger::ERROR : Logger::INFO;
      if (kind & logkinds_)
        report(file, line, func, kind, "%d: %s: %s", code, Error::codename(code), message);
    }
  }
  /**
   * Open a database file.
   * @param path the path of a database file.
   * @param mode the connection mode.  CacheDB::OWRITER as a writer, CacheDB::OREADER as a
   * reader.  The following may be added to the writer mode by bitwise-or: CacheDB::OCREATE,
   * which means it creates a new database if the file does not exist, CacheDB::OTRUNCATE, which
   * means it creates a new database regardless if the file exists, CacheDB::OAUTOTRAN, which
   * means each updating operation is performed in implicit transaction, CacheDB::OAUTOSYNC,
   * which means each updating operation is followed by implicit synchronization with the file
   * system.  The following may be added to both of the reader mode and the writer mode by
   * bitwise-or: CacheDB::ONOLOCK, which means it opens the database file without file locking,
   * CacheDB::OTRYLOCK, which means locking is performed without blocking, CacheDB::ONOREPAIR,
   * which means the database file is not repaired implicitly even if file destruction is
   * detected.
   * @return true on success, or false on failure.
   * @note Every opened database must be closed by the CacheDB::close method when it is no
   * longer in use.  It is not allowed for two or more database objects in the same process to
   * keep their connections to the same database file at the same time.
   */
  bool open(const std::string& path, uint32_t mode = OWRITER | OCREATE) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    report(_KCCODELINE_, Logger::DEBUG, "opening the database (path=%s)", path.c_str());
    omode_ = mode;
    path_.append(path);
    size_t bnum = nearbyprime(bnum_ / SLOTNUM);
    size_t capcnt = capcnt_ > 0 ? capcnt_ / SLOTNUM + 1 : (1ULL << (sizeof(capcnt) * 8 - 1));
    size_t capsiz = capsiz_ > 0 ? capsiz_ / SLOTNUM + 1 : (1ULL << (sizeof(capsiz) * 8 - 1));
    if (capsiz > sizeof(*this) / SLOTNUM) capsiz -= sizeof(*this) / SLOTNUM;
    if (capsiz > bnum * sizeof(Record*)) capsiz -= bnum * sizeof(Record*);
    for (int32_t i = 0; i < SLOTNUM; i++) {
      initialize_slot(slots_ + i, bnum, capcnt, capsiz);
    }
    comp_ = (opts_ & TCOMPRESS) ? embcomp_ : NULL;
    std::memset(opaque_, 0, sizeof(opaque_));
    trigger_meta(MetaTrigger::OPEN, "open");
    return true;
  }
  /**
   * Close the database file.
   * @return true on success, or false on failure.
   */
  bool close() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    report(_KCCODELINE_, Logger::DEBUG, "closing the database (path=%s)", path_.c_str());
    tran_ = false;
    for (int32_t i = SLOTNUM - 1; i >= 0; i--) {
      destroy_slot(slots_ + i);
    }
    path_.clear();
    omode_ = 0;
    trigger_meta(MetaTrigger::CLOSE, "close");
    return true;
  }
  /**
   * Synchronize updated contents with the file and the device.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @param proc a postprocessor object.  If it is NULL, no postprocessing is performed.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   * @note The operation of the postprocessor is performed atomically and other threads accessing
   * the same record are blocked.  To avoid deadlock, any explicit database operation must not
   * be performed in this function.
   */
  bool synchronize(bool hard = false, FileProcessor* proc = NULL,
                   ProgressChecker* checker = NULL) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    bool err = false;
    if ((omode_ & OWRITER) && checker &&
        !checker->check("synchronize", "nothing to be synchronized", -1, -1)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    if (proc) {
      if (checker && !checker->check("synchronize", "running the post processor", -1, -1)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        return false;
      }
      if (!proc->process(path_, count_impl(), size_impl())) {
        set_error(_KCCODELINE_, Error::LOGIC, "postprocessing failed");
        err = true;
      }
    }
    trigger_meta(MetaTrigger::SYNCHRONIZE, "synchronize");
    return !err;
  }
  /**
   * Occupy database by locking and do something meanwhile.
   * @param writable true to use writer lock, or false to use reader lock.
   * @param proc a processor object.  If it is NULL, no processing is performed.
   * @return true on success, or false on failure.
   * @note The operation of the processor is performed atomically and other threads accessing
   * the same record are blocked.  To avoid deadlock, any explicit database operation must not
   * be performed in this function.
   */
  bool occupy(bool writable = true, FileProcessor* proc = NULL) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, writable);
    bool err = false;
    if (proc && !proc->process(path_, count_impl(), size_impl())) {
      set_error(_KCCODELINE_, Error::LOGIC, "processing failed");
      err = true;
    }
    trigger_meta(MetaTrigger::OCCUPY, "occupy");
    return !err;
  }
  /**
   * Begin transaction.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @return true on success, or false on failure.
   */
  bool begin_transaction(bool hard = false) {
    _assert_(true);
    uint32_t wcnt = 0;
    while (true) {
      mlock_.lock_writer();
      if (omode_ == 0) {
        set_error(_KCCODELINE_, Error::INVALID, "not opened");
        mlock_.unlock();
        return false;
      }
      if (!(omode_ & OWRITER)) {
        set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        mlock_.unlock();
        return false;
      }
      if (!tran_) break;
      mlock_.unlock();
      if (wcnt >= LOCKBUSYLOOP) {
        Thread::chill();
      } else {
        Thread::yield();
        wcnt++;
      }
    }
    tran_ = true;
    trigger_meta(MetaTrigger::BEGINTRAN, "begin_transaction");
    mlock_.unlock();
    return true;
  }
  /**
   * Try to begin transaction.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @return true on success, or false on failure.
   */
  bool begin_transaction_try(bool hard = false) {
    _assert_(true);
    mlock_.lock_writer();
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      mlock_.unlock();
      return false;
    }
    if (!(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      mlock_.unlock();
      return false;
    }
    if (tran_) {
      set_error(_KCCODELINE_, Error::LOGIC, "competition avoided");
      mlock_.unlock();
      return false;
    }
    tran_ = true;
    trigger_meta(MetaTrigger::BEGINTRAN, "begin_transaction_try");
    mlock_.unlock();
    return true;
  }
  /**
   * End transaction.
   * @param commit true to commit the transaction, or false to abort the transaction.
   * @return true on success, or false on failure.
   */
  bool end_transaction(bool commit = true) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (!tran_) {
      set_error(_KCCODELINE_, Error::INVALID, "not in transaction");
      return false;
    }
    if (!commit) disable_cursors();
    for (int32_t i = 0; i < SLOTNUM; i++) {
      if (!commit) apply_slot_trlogs(slots_ + i);
      slots_[i].trlogs.clear();
      adjust_slot_capacity(slots_ + i);
    }
    tran_ = false;
    trigger_meta(commit ? MetaTrigger::COMMITTRAN : MetaTrigger::ABORTTRAN, "end_transaction");
    return true;
  }
  /**
   * Remove all records.
   * @return true on success, or false on failure.
   */
  bool clear() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    disable_cursors();
    for (int32_t i = 0; i < SLOTNUM; i++) {
      Slot* slot = slots_ + i;
      clear_slot(slot);
    }
    std::memset(opaque_, 0, sizeof(opaque_));
    trigger_meta(MetaTrigger::CLEAR, "clear");
    return true;
  }
  /**
   * Get the number of records.
   * @return the number of records, or -1 on failure.
   */
  int64_t count() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return -1;
    }
    return count_impl();
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes, or -1 on failure.
   */
  int64_t size() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return -1;
    }
    return size_impl();
  }
  /**
   * Get the path of the database file.
   * @return the path of the database file, or an empty string on failure.
   */
  std::string path() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return "";
    }
    return path_;
  }
  /**
   * Get the miscellaneous status information.
   * @param strmap a string map to contain the result.
   * @return true on success, or false on failure.
   */
  bool status(std::map<std::string, std::string>* strmap) {
    _assert_(strmap);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    (*strmap)["type"] = strprintf("%u", (unsigned)TYPECACHE);
    (*strmap)["realtype"] = strprintf("%u", (unsigned)type_);
    (*strmap)["path"] = path_;
    (*strmap)["libver"] = strprintf("%u", LIBVER);
    (*strmap)["librev"] = strprintf("%u", LIBREV);
    (*strmap)["fmtver"] = strprintf("%u", FMTVER);
    (*strmap)["chksum"] = strprintf("%u", 0xff);
    (*strmap)["opts"] = strprintf("%u", opts_);
    (*strmap)["bnum"] = strprintf("%lld", (long long)bnum_);
    (*strmap)["capcnt"] = strprintf("%lld", (long long)capcnt_);
    (*strmap)["capsiz"] = strprintf("%lld", (long long)capsiz_);
    (*strmap)["recovered"] = strprintf("%d", false);
    (*strmap)["reorganized"] = strprintf("%d", false);
    if (strmap->count("opaque") > 0)
      (*strmap)["opaque"] = std::string(opaque_, sizeof(opaque_));
    if (strmap->count("bnum_used") > 0) {
      int64_t cnt = 0;
      for (int32_t i = 0; i < SLOTNUM; i++) {
        Slot* slot = slots_ + i;
        Record** buckets = slot->buckets;
        size_t bnum = slot->bnum;
        for (size_t j = 0; j < bnum; j++) {
          if (buckets[j]) cnt++;
        }
      }
      (*strmap)["bnum_used"] = strprintf("%lld", (long long)cnt);
    }
    (*strmap)["count"] = strprintf("%lld", (long long)count_impl());
    (*strmap)["size"] = strprintf("%lld", (long long)size_impl());
    return true;
  }
  /**
   * Create a cursor object.
   * @return the return value is the created cursor object.
   * @note Because the object of the return value is allocated by the constructor, it should be
   * released with the delete operator when it is no longer in use.
   */
  Cursor* cursor() {
    _assert_(true);
    return new Cursor(this);
  }
  /**
   * Write a log message.
   * @param file the file name of the program source code.
   * @param line the line number of the program source code.
   * @param func the function name of the program source code.
   * @param kind the kind of the event.  Logger::DEBUG for debugging, Logger::INFO for normal
   * information, Logger::WARN for warning, and Logger::ERROR for fatal error.
   * @param message the supplement message.
   */
  void log(const char* file, int32_t line, const char* func, Logger::Kind kind,
           const char* message) {
    _assert_(file && line > 0 && func && message);
    ScopedRWLock lock(&mlock_, false);
    if (!logger_) return;
    logger_->log(file, line, func, kind, message);
  }
  /**
   * Set the internal logger.
   * @param logger the logger object.
   * @param kinds kinds of logged messages by bitwise-or: Logger::DEBUG for debugging,
   * Logger::INFO for normal information, Logger::WARN for warning, and Logger::ERROR for fatal
   * error.
   * @return true on success, or false on failure.
   */
  bool tune_logger(Logger* logger, uint32_t kinds = Logger::WARN | Logger::ERROR) {
    _assert_(logger);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    logger_ = logger;
    logkinds_ = kinds;
    return true;
  }
  /**
   * Set the internal meta operation trigger.
   * @param trigger the trigger object.
   * @return true on success, or false on failure.
   */
  bool tune_meta_trigger(MetaTrigger* trigger) {
    _assert_(trigger);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    mtrigger_ = trigger;
    return true;
  }
  /**
   * Set the optional features.
   * @param opts the optional features by bitwise-or: DirDB::TCOMPRESS to compress each record.
   * @return true on success, or false on failure.
   */
  bool tune_options(int8_t opts) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    opts_ = opts;
    return true;
  }
  /**
   * Set the number of buckets of the hash table.
   * @param bnum the number of buckets of the hash table.
   * @return true on success, or false on failure.
   */
  bool tune_buckets(int64_t bnum) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    bnum_ = bnum >= 0 ? bnum : DEFBNUM;
    return true;
  }
  /**
   * Set the data compressor.
   * @param comp the data compressor object.
   * @return true on success, or false on failure.
   */
  bool tune_compressor(Compressor* comp) {
    _assert_(comp);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    embcomp_ = comp;
    return true;
  }
  /**
   * Set the capacity by record number.
   * @param count the maximum number of records.
   * @return true on success, or false on failure.
   */
  bool cap_count(int64_t count) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    capcnt_ = count;
    return true;
  }
  /**
   * Set the capacity by memory usage.
   * @param size the maximum size of memory usage.
   * @return true on success, or false on failure.
   */
  bool cap_size(int64_t size) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    capsiz_ = size;
    return true;
  }
  /**
   * Switch the mode of LRU rotation.
   * @param rttmode true to enable LRU rotation, false to disable LRU rotation.
   * @return true on success, or false on failure.
   * @note This function can be called while the database is opened.
   */
  bool switch_rotation(bool rttmode) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    rttmode_ = rttmode;
    return true;
  }
  /**
   * Get the opaque data.
   * @return the pointer to the opaque data region, whose size is 16 bytes.
   */
  char* opaque() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return NULL;
    }
    return opaque_;
  }
  /**
   * Synchronize the opaque data.
   * @return true on success, or false on failure.
   */
  bool synchronize_opaque() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (!(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    return true;
  }
 protected:
  /**
   * Report a message for debugging.
   * @param file the file name of the program source code.
   * @param line the line number of the program source code.
   * @param func the function name of the program source code.
   * @param kind the kind of the event.  Logger::DEBUG for debugging, Logger::INFO for normal
   * information, Logger::WARN for warning, and Logger::ERROR for fatal error.
   * @param format the printf-like format string.
   * @param ... used according to the format string.
   */
  void report(const char* file, int32_t line, const char* func, Logger::Kind kind,
              const char* format, ...) {
    _assert_(file && line > 0 && func && format);
    if (!logger_ || !(kind & logkinds_)) return;
    std::string message;
    strprintf(&message, "%s: ", path_.empty() ? "-" : path_.c_str());
    va_list ap;
    va_start(ap, format);
    vstrprintf(&message, format, ap);
    va_end(ap);
    logger_->log(file, line, func, kind, message.c_str());
  }
  /**
   * Report a message for debugging with variable number of arguments.
   * @param file the file name of the program source code.
   * @param line the line number of the program source code.
   * @param func the function name of the program source code.
   * @param kind the kind of the event.  Logger::DEBUG for debugging, Logger::INFO for normal
   * information, Logger::WARN for warning, and Logger::ERROR for fatal error.
   * @param format the printf-like format string.
   * @param ap used according to the format string.
   */
  void report_valist(const char* file, int32_t line, const char* func, Logger::Kind kind,
                     const char* format, va_list ap) {
    _assert_(file && line > 0 && func && format);
    if (!logger_ || !(kind & logkinds_)) return;
    std::string message;
    strprintf(&message, "%s: ", path_.empty() ? "-" : path_.c_str());
    vstrprintf(&message, format, ap);
    logger_->log(file, line, func, kind, message.c_str());
  }
  /**
   * Report the content of a binary buffer for debugging.
   * @param file the file name of the epicenter.
   * @param line the line number of the epicenter.
   * @param func the function name of the program source code.
   * @param kind the kind of the event.  Logger::DEBUG for debugging, Logger::INFO for normal
   * information, Logger::WARN for warning, and Logger::ERROR for fatal error.
   * @param name the name of the information.
   * @param buf the binary buffer.
   * @param size the size of the binary buffer
   */
  void report_binary(const char* file, int32_t line, const char* func, Logger::Kind kind,
                     const char* name, const char* buf, size_t size) {
    _assert_(file && line > 0 && func && name && buf && size <= MEMMAXSIZ);
    if (!logger_) return;
    char* hex = hexencode(buf, size);
    report(file, line, func, kind, "%s=%s", name, hex);
    delete[] hex;
  }
  /**
   * Trigger a meta database operation.
   * @param kind the kind of the event.  MetaTrigger::OPEN for opening, MetaTrigger::CLOSE for
   * closing, MetaTrigger::CLEAR for clearing, MetaTrigger::ITERATE for iteration,
   * MetaTrigger::SYNCHRONIZE for synchronization, MetaTrigger::BEGINTRAN for beginning
   * transaction, MetaTrigger::COMMITTRAN for committing transaction, MetaTrigger::ABORTTRAN
   * for aborting transaction, and MetaTrigger::MISC for miscellaneous operations.
   * @param message the supplement message.
   */
  void trigger_meta(MetaTrigger::Kind kind, const char* message) {
    _assert_(message);
    if (mtrigger_) mtrigger_->trigger(kind, message);
  }
  /**
   * Set the database type.
   * @param type the database type.
   * @return true on success, or false on failure.
   */
  bool tune_type(int8_t type) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    type_ = type;
    return true;
  }
  /**
   * Get the library version.
   * @return the library version, or 0 on failure.
   */
  uint8_t libver() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return LIBVER;
  }
  /**
   * Get the library revision.
   * @return the library revision, or 0 on failure.
   */
  uint8_t librev() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return LIBREV;
  }
  /**
   * Get the format version.
   * @return the format version, or 0 on failure.
   */
  uint8_t fmtver() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return FMTVER;
  }
  /**
   * Get the module checksum.
   * @return the module checksum, or 0 on failure.
   */
  uint8_t chksum() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return 0xff;
  }
  /**
   * Get the database type.
   * @return the database type, or 0 on failure.
   */
  uint8_t type() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return type_;
  }
  /**
   * Get the options.
   * @return the options, or 0 on failure.
   */
  uint8_t opts() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return opts_;
  }
  /**
   * Get the data compressor.
   * @return the data compressor, or NULL on failure.
   */
  Compressor* comp() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return NULL;
    }
    return comp_;
  }
  /**
   * Check whether the database was recovered or not.
   * @return true if recovered, or false if not.
   */
  bool recovered() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return false;
  }
  /**
   * Check whether the database was reorganized or not.
   * @return true if reorganized, or false if not.
   */
  bool reorganized() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return false;
  }
 private:
  /**
   * Set the power of the alignment of record size.
   * @note This is a dummy implementation for compatibility.
   */
  bool tune_alignment(int8_t apow) {
    return true;
  }
  /**
   * Set the power of the capacity of the free block pool.
   * @note This is a dummy implementation for compatibility.
   */
  bool tune_fbp(int8_t fpow) {
    return true;
  }
  /**
   * Set the size of the internal memory-mapped region.
   * @note This is a dummy implementation for compatibility.
   */
  bool tune_map(int64_t msiz) {
    return true;
  }
  /**
   * Set the unit step number of auto defragmentation.
   * @note This is a dummy implementation for compatibility.
   */
  bool tune_defrag(int64_t dfunit) {
    return true;
  }
  /**
   * Perform defragmentation of the file.
   * @note This is a dummy implementation for compatibility.
   */
  bool defrag(int64_t step = 0) {
    return true;
  }
  /**
   * Get the status flags.
   * @note This is a dummy implementation for compatibility.
   */
  uint8_t flags() {
    return 0;
  }
  /**
   * Get the alignment power.
   * @note This is a dummy implementation for compatibility.
   */
  uint8_t apow() {
    return 0;
  }
  /**
   * Get the free block pool power.
   * @note This is a dummy implementation for compatibility.
   */
  uint8_t fpow() {
    return 0;
  }
  /**
   * Get the bucket number.
   * @note This is a dummy implementation for compatibility.
   */
  int64_t bnum() {
    return 1;
  }
  /**
   * Get the size of the internal memory-mapped region.
   * @note This is a dummy implementation for compatibility.
   */
  int64_t msiz() {
    return 0;
  }
  /**
   * Get the unit step number of auto defragmentation.
   * @note This is a dummy implementation for compatibility.
   */
  int64_t dfunit() {
    return 0;
  }
 private:
  /**
   * Record data.
   */
  struct Record {
    uint32_t ksiz;                       ///< size of the key
    uint32_t vsiz;                       ///< size of the value
    Record* left;                        ///< left child record
    Record* right;                       ///< right child record
    Record* prev;                        ///< privious record
    Record* next;                        ///< next record
  };
  /**
   * Transaction log.
   */
  struct TranLog {
    bool full;                           ///< flag whether full
    std::string key;                     ///< old key
    std::string value;                   ///< old value
    /** constructor for a full record */
    explicit TranLog(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) :
        full(true), key(kbuf, ksiz), value(vbuf, vsiz) {
      _assert_(true);
    }
    /** constructor for an empty record */
    explicit TranLog(const char* kbuf, size_t ksiz) : full(false), key(kbuf, ksiz) {
      _assert_(true);
    }
  };
  /**
   * Slot table.
   */
  struct Slot {
    Mutex lock;                       ///< lock
    Record** buckets;                    ///< bucket array
    size_t bnum;                         ///< number of buckets
    size_t capcnt;                       ///< cap of record number
    size_t capsiz;                       ///< cap of memory usage
    Record* first;                       ///< first record
    Record* last;                        ///< last record
    size_t count;                        ///< number of records
    size_t size;                         ///< total size of records
    TranLogList trlogs;                  ///< transaction logs
    size_t trsize;                       ///< size before transaction
  };
  /**
   * Repeating visitor.
   */
  class Repeater : public Visitor {
   public:
    /** constructor */
    explicit Repeater(const char* vbuf, size_t vsiz) : vbuf_(vbuf), vsiz_(vsiz) {}
   private:
    /** process a full record */
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ && sp);
      *sp = vsiz_;
      return vbuf_;
    }
    const char* vbuf_;                   ///< region of the value
    size_t vsiz_;                        ///< size of the value
  };
  /**
   * Setting visitor.
   */
  class Setter : public Visitor {
   public:
    /** constructor */
    explicit Setter(const char* vbuf, size_t vsiz) : vbuf_(vbuf), vsiz_(vsiz) {}
   private:
    /** process a full record */
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ && sp);
      *sp = vsiz_;
      return vbuf_;
    }
    /** process an empty record */
    const char* visit_empty(const char* kbuf, size_t ksiz, size_t* sp) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ && sp);
      *sp = vsiz_;
      return vbuf_;
    }
    const char* vbuf_;                   ///< region of the value
    size_t vsiz_;                        ///< size of the value
  };
  /**
   * Removing visitor.
   */
  class Remover : public Visitor {
   private:
    /** visit a record */
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ && sp);
      return REMOVE;
    }
  };
  /**
   * Scoped visitor.
   */
  class ScopedVisitor {
   public:
    /** constructor */
    explicit ScopedVisitor(Visitor* visitor) : visitor_(visitor) {
      _assert_(visitor);
      visitor_->visit_before();
    }
    /** destructor */
    ~ScopedVisitor() {
      _assert_(true);
      visitor_->visit_after();
    }
   private:
    Visitor* visitor_;                   ///< visitor
  };
  /**
   * Accept a visitor to a record.
   * @param slot the slot of the record.
   * @param hash the hash value of the key.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param visitor a visitor object.
   * @param comp the data compressor.
   * @param rtt whether to move the record to the last.
   */
  void accept_impl(Slot* slot, uint64_t hash, const char* kbuf, size_t ksiz, Visitor* visitor,
                   Compressor* comp, bool rtt) {
    _assert_(slot && kbuf && ksiz <= MEMMAXSIZ && visitor);
    size_t bidx = hash % slot->bnum;
    Record* rec = slot->buckets[bidx];
    Record** entp = slot->buckets + bidx;
    uint32_t fhash = fold_hash(hash) & ~KSIZMAX;
    while (rec) {
      uint32_t rhash = rec->ksiz & ~KSIZMAX;
      uint32_t rksiz = rec->ksiz & KSIZMAX;
      if (fhash > rhash) {
        entp = &rec->left;
        rec = rec->left;
      } else if (fhash < rhash) {
        entp = &rec->right;
        rec = rec->right;
      } else {
        char* dbuf = (char*)rec + sizeof(*rec);
        int32_t kcmp = compare_keys(kbuf, ksiz, dbuf, rksiz);
        if (kcmp < 0) {
          entp = &rec->left;
          rec = rec->left;
        } else if (kcmp > 0) {
          entp = &rec->right;
          rec = rec->right;
        } else {
          const char* rvbuf = dbuf + rksiz;
          size_t rvsiz = rec->vsiz;
          char* zbuf = NULL;
          size_t zsiz = 0;
          if (comp) {
            zbuf = comp->decompress(rvbuf, rvsiz, &zsiz);
            if (zbuf) {
              rvbuf = zbuf;
              rvsiz = zsiz;
            }
          }
          size_t vsiz;
          const char* vbuf = visitor->visit_full(dbuf, rksiz, rvbuf, rvsiz, &vsiz);
          delete[] zbuf;
          if (vbuf == Visitor::REMOVE) {
            if (tran_) {
              TranLog log(kbuf, ksiz, dbuf + rksiz, rec->vsiz);
              slot->trlogs.push_back(log);
            }
            if (!curs_.empty()) escape_cursors(rec);
            if (rec == slot->first) slot->first = rec->next;
            if (rec == slot->last) slot->last = rec->prev;
            if (rec->prev) rec->prev->next = rec->next;
            if (rec->next) rec->next->prev = rec->prev;
            if (rec->left && !rec->right) {
              *entp = rec->left;
            } else if (!rec->left && rec->right) {
              *entp = rec->right;
            } else if (!rec->left) {
              *entp = NULL;
            } else {
              Record* pivot = rec->left;
              if (pivot->right) {
                Record** pentp = &pivot->right;
                pivot = pivot->right;
                while (pivot->right) {
                  pentp = &pivot->right;
                  pivot = pivot->right;
                }
                *entp = pivot;
                *pentp = pivot->left;
                pivot->left = rec->left;
                pivot->right = rec->right;
              } else {
                *entp = pivot;
                pivot->right = rec->right;
              }
            }
            slot->count--;
            slot->size -= sizeof(Record) + rksiz + rec->vsiz;
            xfree(rec);
          } else {
            bool adj = false;
            if (vbuf != Visitor::NOP) {
              char* zbuf = NULL;
              size_t zsiz = 0;
              if (comp) {
                zbuf = comp->compress(vbuf, vsiz, &zsiz);
                if (zbuf) {
                  vbuf = zbuf;
                  vsiz = zsiz;
                }
              }
              if (tran_) {
                TranLog log(kbuf, ksiz, dbuf + rksiz, rec->vsiz);
                slot->trlogs.push_back(log);
              } else {
                adj = vsiz > rec->vsiz;
              }
              slot->size -= rec->vsiz;
              slot->size += vsiz;
              if (vsiz > rec->vsiz) {
                Record* old = rec;
                rec = (Record*)xrealloc(rec, sizeof(*rec) + ksiz + vsiz);
                if (rec != old) {
                  if (!curs_.empty()) adjust_cursors(old, rec);
                  if (slot->first == old) slot->first = rec;
                  if (slot->last == old) slot->last = rec;
                  *entp = rec;
                  if (rec->prev) rec->prev->next = rec;
                  if (rec->next) rec->next->prev = rec;
                  dbuf = (char*)rec + sizeof(*rec);
                }
              }
              std::memcpy(dbuf + ksiz, vbuf, vsiz);
              rec->vsiz = vsiz;
              delete[] zbuf;
            }
            if (rtt && slot->last != rec) {
              if (!curs_.empty()) escape_cursors(rec);
              if (slot->first == rec) slot->first = rec->next;
              if (rec->prev) rec->prev->next = rec->next;
              if (rec->next) rec->next->prev = rec->prev;
              rec->prev = slot->last;
              rec->next = NULL;
              slot->last->next = rec;
              slot->last = rec;
            }
            if (adj) adjust_slot_capacity(slot);
          }
          return;
        }
      }
    }
    size_t vsiz;
    const char* vbuf = visitor->visit_empty(kbuf, ksiz, &vsiz);
    if (vbuf != Visitor::NOP && vbuf != Visitor::REMOVE) {
      char* zbuf = NULL;
      size_t zsiz = 0;
      if (comp) {
        zbuf = comp->compress(vbuf, vsiz, &zsiz);
        if (zbuf) {
          vbuf = zbuf;
          vsiz = zsiz;
        }
      }
      if (tran_) {
        TranLog log(kbuf, ksiz);
        slot->trlogs.push_back(log);
      }
      slot->size += sizeof(Record) + ksiz + vsiz;
      rec = (Record*)xmalloc(sizeof(*rec) + ksiz + vsiz);
      char* dbuf = (char*)rec + sizeof(*rec);
      std::memcpy(dbuf, kbuf, ksiz);
      rec->ksiz = ksiz | fhash;
      std::memcpy(dbuf + ksiz, vbuf, vsiz);
      rec->vsiz = vsiz;
      rec->left = NULL;
      rec->right = NULL;
      rec->prev = slot->last;
      rec->next = NULL;
      *entp = rec;
      if (!slot->first) slot->first = rec;
      if (slot->last) slot->last->next = rec;
      slot->last = rec;
      slot->count++;
      if (!tran_) adjust_slot_capacity(slot);
      delete[] zbuf;
    }
  }
  /**
   * Get the number of records.
   * @return the number of records, or -1 on failure.
   */
  int64_t count_impl() {
    _assert_(true);
    int64_t sum = 0;
    for (int32_t i = 0; i < SLOTNUM; i++) {
      Slot* slot = slots_ + i;
      ScopedMutex lock(&slot->lock);
      sum += slot->count;
    }
    return sum;
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes.
   */
  int64_t size_impl() {
    _assert_(true);
    int64_t sum = sizeof(*this);
    for (int32_t i = 0; i < SLOTNUM; i++) {
      Slot* slot = slots_ + i;
      ScopedMutex lock(&slot->lock);
      sum += slot->bnum * sizeof(Record*);
      sum += slot->size;
    }
    return sum;
  }
  /**
   * Initialize a slot table.
   * @param slot the slot table.
   * @param bnum the number of buckets.
   * @param capcnt the capacity of record number.
   * @param capsiz the capacity of memory usage.
   */
  void initialize_slot(Slot* slot, size_t bnum, size_t capcnt, size_t capsiz) {
    _assert_(slot);
    Record** buckets;
    if (bnum >= ZMAPBNUM) {
      buckets = (Record**)mapalloc(sizeof(*buckets) * bnum);
    } else {
      buckets = new Record*[bnum];
      for (size_t i = 0; i < bnum; i++) {
        buckets[i] = NULL;
      }
    }
    slot->buckets = buckets;
    slot->bnum = bnum;
    slot->capcnt = capcnt;
    slot->capsiz = capsiz;
    slot->first = NULL;
    slot->last = NULL;
    slot->count = 0;
    slot->size = 0;
  }
  /**
   * Destroy a slot table.
   * @param slot the slot table.
   */
  void destroy_slot(Slot* slot) {
    _assert_(slot);
    slot->trlogs.clear();
    Record* rec = slot->last;
    while (rec) {
      Record* prev = rec->prev;
      xfree(rec);
      rec = prev;
    }
    if (slot->bnum >= ZMAPBNUM) {
      mapfree(slot->buckets);
    } else {
      delete[] slot->buckets;
    }
  }
  /**
   * Clear a slot table.
   * @param slot the slot table.
   */
  void clear_slot(Slot* slot) {
    _assert_(slot);
    Record* rec = slot->last;
    while (rec) {
      if (tran_) {
        uint32_t rksiz = rec->ksiz & KSIZMAX;
        char* dbuf = (char*)rec + sizeof(*rec);
        TranLog log(dbuf, rksiz, dbuf + rksiz, rec->vsiz);
        slot->trlogs.push_back(log);
      }
      Record* prev = rec->prev;
      xfree(rec);
      rec = prev;
    }
    Record** buckets = slot->buckets;
    size_t bnum = slot->bnum;
    for (size_t i = 0; i < bnum; i++) {
      buckets[i] = NULL;
    }
    slot->first = NULL;
    slot->last = NULL;
    slot->count = 0;
    slot->size = 0;
  }
  /**
   * Apply transaction logs of a slot table.
   * @param slot the slot table.
   */
  void apply_slot_trlogs(Slot* slot) {
    _assert_(slot);
    const TranLogList& logs = slot->trlogs;
    TranLogList::const_iterator it = logs.end();
    TranLogList::const_iterator itbeg = logs.begin();
    while (it != itbeg) {
      --it;
      const char* kbuf = it->key.c_str();
      size_t ksiz = it->key.size();
      const char* vbuf = it->value.c_str();
      size_t vsiz = it->value.size();
      uint64_t hash = hash_record(kbuf, ksiz) / SLOTNUM;
      if (it->full) {
        Setter setter(vbuf, vsiz);
        accept_impl(slot, hash, kbuf, ksiz, &setter, NULL, false);
      } else {
        Remover remover;
        accept_impl(slot, hash, kbuf, ksiz, &remover, NULL, false);
      }
    }
  }
  /**
   * Addjust a slot table to the capacity.
   * @param slot the slot table.
   */
  void adjust_slot_capacity(Slot* slot) {
    _assert_(slot);
    if ((slot->count > slot->capcnt || slot->size > slot->capsiz) && slot->first) {
      Record* rec = slot->first;
      uint32_t rksiz = rec->ksiz & KSIZMAX;
      char* dbuf = (char*)rec + sizeof(*rec);
      char stack[RECBUFSIZ];
      char* kbuf = rksiz > sizeof(stack) ? new char[rksiz] : stack;
      std::memcpy(kbuf, dbuf, rksiz);
      uint64_t hash = hash_record(kbuf, rksiz) / SLOTNUM;
      Remover remover;
      accept_impl(slot, hash, dbuf, rksiz, &remover, NULL, false);
      if (kbuf != stack) delete[] kbuf;
    }
  }
  /**
   * Get the hash value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @return the hash value.
   */
  uint64_t hash_record(const char* kbuf, size_t ksiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    return hashmurmur(kbuf, ksiz);
  }
  /**
   * Fold a hash value into a small number.
   * @param hash the hash number.
   * @return the result number.
   */
  uint32_t fold_hash(uint64_t hash) {
    _assert_(true);
    return ((hash & 0xffffffff00000000ULL) >> 32) ^ ((hash & 0x0000ffffffff0000ULL) >> 16) ^
        ((hash & 0x000000000000ffffULL) << 16) ^ ((hash & 0x00000000ffff0000ULL) >> 0);
  }
  /**
   * Compare two keys in lexical order.
   * @param abuf one key.
   * @param asiz the size of the one key.
   * @param bbuf the other key.
   * @param bsiz the size of the other key.
   * @return positive if the former is big, or negative if the latter is big, or 0 if both are
   * equivalent.
   */
  int32_t compare_keys(const char* abuf, size_t asiz, const char* bbuf, size_t bsiz) {
    _assert_(abuf && asiz <= MEMMAXSIZ && bbuf && bsiz <= MEMMAXSIZ);
    if (asiz != bsiz) return (int32_t)asiz - (int32_t)bsiz;
    return std::memcmp(abuf, bbuf, asiz);
  }
  /**
   * Escape cursors on a shifted or removed records.
   * @param rec the record.
   */
  void escape_cursors(Record* rec) {
    _assert_(rec);
    ScopedMutex lock(&flock_);
    if (curs_.empty()) return;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->rec_ == rec) cur->step_impl();
      ++cit;
    }
  }
  /**
   * Adjust cursors on re-allocated records.
   * @param orec the old address.
   * @param nrec the new address.
   */
  void adjust_cursors(Record* orec, Record* nrec) {
    _assert_(orec && nrec);
    ScopedMutex lock(&flock_);
    if (curs_.empty()) return;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->rec_ == orec) cur->rec_ = nrec;
      ++cit;
    }
  }
  /**
   * Disable all cursors.
   */
  void disable_cursors() {
    _assert_(true);
    ScopedMutex lock(&flock_);
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      cur->sidx_ = -1;
      cur->rec_ = NULL;
      ++cit;
    }
  }
  /** Dummy constructor to forbid the use. */
  CacheDB(const CacheDB&);
  /** Dummy Operator to forbid the use. */
  CacheDB& operator =(const CacheDB&);
  /** The method lock. */
  RWLock mlock_;
  /** The file lock. */
  Mutex flock_;
  /** The last happened error. */
  TSD<Error> error_;
  /** The internal logger. */
  Logger* logger_;
  /** The kinds of logged messages. */
  uint32_t logkinds_;
  /** The internal meta operation trigger. */
  MetaTrigger* mtrigger_;
  /** The open mode. */
  uint32_t omode_;
  /** The cursor objects. */
  CursorList curs_;
  /** The path of the database file. */
  std::string path_;
  /** The database type. */
  uint8_t type_;
  /** The options. */
  uint8_t opts_;
  /** The bucket number. */
  int64_t bnum_;
  /** The capacity of record number. */
  int64_t capcnt_;
  /** The capacity of memory usage. */
  int64_t capsiz_;
  /** The opaque data. */
  char opaque_[OPAQUESIZ];
  /** The embedded data compressor. */
  Compressor* embcomp_;
  /** The data compressor. */
  Compressor* comp_;
  /** The slot tables. */
  Slot slots_[SLOTNUM];
  /** The flag whether in LRU rotation. */
  bool rttmode_;
  /** The flag whether in transaction. */
  bool tran_;
};


/** An alias of the cache tree database. */
typedef PlantDB<CacheDB, BasicDB::TYPEGRASS> GrassDB;


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
