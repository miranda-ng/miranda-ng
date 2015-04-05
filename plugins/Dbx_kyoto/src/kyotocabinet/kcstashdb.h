/*************************************************************************************************
 * Stash database
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


#ifndef _KCSTASHDB_H                     // duplication check
#define _KCSTASHDB_H

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
 * Economical on-memory hash database.
 * @note This class is a concrete class to operate a hash database on memory.  This class can be
 * inherited but overwriting methods is forbidden.  Before every database operation, it is
 * necessary to call the StashDB::open method in order to open a database file and connect the
 * database object to it.  To avoid data missing or corruption, it is important to close every
 * database file by the StashDB::close method when the database is no longer in use.  It is
 * forbidden for multible database objects in a process to open the same database at the same
 * time.  It is forbidden to share a database object with child processes.
 */
class StashDB : public BasicDB {
 public:
  class Cursor;
 private:
  struct Record;
  struct TranLog;
  class Repeater;
  class Setter;
  class Remover;
  class ScopedVisitor;
  /** An alias of list of cursors. */
  typedef std::list<Cursor*> CursorList;
  /** An alias of list of transaction logs. */
  typedef std::list<TranLog> TranLogList;
  /** The number of slots of the record lock. */
  static const int32_t RLOCKSLOT = 1024;
  /** The default bucket number. */
  static const size_t DEFBNUM = 1048583LL;
  /** The size of the opaque buffer. */
  static const size_t OPAQUESIZ = 16;
  /** The threshold of busy loop and sleep for locking. */
  static const uint32_t LOCKBUSYLOOP = 8192;
  /** The mininum number of buckets to use mmap. */
  static const size_t MAPZMAPBNUM = 32768;
 public:
  /**
   * Cursor to indicate a record.
   */
  class Cursor : public BasicDB::Cursor {
    friend class StashDB;
   public:
    /**
     * Constructor.
     * @param db the container database object.
     */
    explicit Cursor(StashDB* db) : db_(db), bidx_(-1), rbuf_(NULL) {
      _assert_(db);
      // ScopedRWLock lock(&db_->mlock_, true);
      db_->curs_.push_back(this);
    }
    /**
     * Destructor.
     */
    virtual ~Cursor() {
      _assert_(true);
      if (!db_) return;
      // ScopedRWLock lock(&db_->mlock_, true);
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
      // ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (writable && !(db_->omode_ & OWRITER)) {
        db_->set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        return false;
      }
      if (bidx_ < 0) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      Record rec(rbuf_);
      size_t vsiz;
      const char* vbuf = visitor->visit_full(rec.kbuf_, rec.ksiz_, rec.vbuf_, rec.vsiz_, &vsiz);
      if (vbuf == Visitor::REMOVE) {
        Repeater repeater(Visitor::REMOVE, 0);
        db_->accept_impl(rec.kbuf_, rec.ksiz_, &repeater, bidx_);
      } else if (vbuf == Visitor::NOP) {
        if (step) step_impl();
      } else {
        Repeater repeater(vbuf, vsiz);
        db_->accept_impl(rec.kbuf_, rec.ksiz_, &repeater, bidx_);
        if (step && rbuf_) step_impl();
      }
      return true;
    }
    /**
     * Jump the cursor to the first record for forward scan.
     * @return true on success, or false on failure.
     */
    bool jump() {
      _assert_(true);
      // ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      bidx_ = 0;
      rbuf_ = NULL;
      while (bidx_ < (int64_t)db_->bnum_) {
        if (db_->buckets_[bidx_]) {
          rbuf_ = db_->buckets_[bidx_];
          return true;
        }
        bidx_++;
      }
      db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
      bidx_ = -1;
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
      // ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      bidx_ = -1;
      rbuf_ = NULL;
      size_t bidx = db_->hash_record(kbuf, ksiz) % db_->bnum_;
      char* rbuf = db_->buckets_[bidx];
      while (rbuf) {
        Record rec(rbuf);
        if (rec.ksiz_ == ksiz && !std::memcmp(rec.kbuf_, kbuf, ksiz)) {
          bidx_ = bidx;
          rbuf_ = rbuf;
          return true;
        }
        rbuf = rec.child_;
      }
      db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
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
      // ScopedRWLock lock(&db_->mlock_, true);
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
      // ScopedRWLock lock(&db_->mlock_, true);
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
      // ScopedRWLock lock(&db_->mlock_, true);
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
      // ScopedRWLock lock(&db_->mlock_, true);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (bidx_ < 0) {
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
      // ScopedRWLock lock(&db_->mlock_, true);
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
    StashDB* db() {
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
      Record rec(rbuf_);
      rbuf_ = rec.child_;
      if (!rbuf_) {
        while (++bidx_ < (int64_t)db_->bnum_) {
          if (db_->buckets_[bidx_]) {
            rbuf_ = db_->buckets_[bidx_];
            return true;
          }
        }
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        bidx_ = -1;
        return false;
      }
      return true;
    }
    /** Dummy constructor to forbid the use. */
    Cursor(const Cursor&);
    /** Dummy Operator to forbid the use. */
    Cursor& operator =(const Cursor&);
    /** The inner database. */
    StashDB* db_;
    /** The index of the current bucket. */
    int64_t bidx_;
    /** The buffer of the current record. */
    char* rbuf_;
  };
  /**
   * Default constructor.
   */
  explicit StashDB() :
      flock_(), error_(),
      logger_(NULL), logkinds_(0), mtrigger_(NULL),
      omode_(0), curs_(), path_(""), bnum_(DEFBNUM), opaque_(),
      count_(0), size_(0), buckets_(NULL),
      tran_(false), trlogs_(), trcount_(0), trsize_(0) {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  ~StashDB() {
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
    // ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    size_t bidx = hash_record(kbuf, ksiz) % bnum_;
    accept_impl(kbuf, ksiz, visitor, bidx);
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
    // ScopedRWLock lock(&mlock_, false);
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
      size_t bidx;
    };
    RecordKey* rkeys = new RecordKey[knum];
    std::set<size_t> lidxs;
    for (size_t i = 0; i < knum; i++) {
      const std::string& key = keys[i];
      RecordKey* rkey = rkeys + i;
      rkey->kbuf = key.data();
      rkey->ksiz = key.size();
      rkey->bidx = hash_record(rkey->kbuf, rkey->ksiz) % bnum_;
      lidxs.insert(rkey->bidx % RLOCKSLOT);
    }
    std::set<size_t>::iterator lit = lidxs.begin();
    std::set<size_t>::iterator litend = lidxs.end();
    while (lit != litend) {
      ++lit;
    }
    for (size_t i = 0; i < knum; i++) {
      RecordKey* rkey = rkeys + i;
      accept_impl(rkey->kbuf, rkey->ksiz, visitor, rkey->bidx);
    }
    lit = lidxs.begin();
    litend = lidxs.end();
    while (lit != litend) {
      ++lit;
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    int64_t allcnt = count_;
    if (checker && !checker->check("iterate", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    int64_t curcnt = 0;
    for (size_t i = 0; i < bnum_; i++) {
      char* rbuf = buckets_[i];
      while (rbuf) {
        curcnt++;
        Record rec(rbuf);
        rbuf = rec.child_;
        size_t vsiz;
        const char* vbuf = visitor->visit_full(rec.kbuf_, rec.ksiz_,
                                               rec.vbuf_, rec.vsiz_, &vsiz);
        if (vbuf == Visitor::REMOVE) {
          Repeater repeater(Visitor::REMOVE, 0);
          accept_impl(rec.kbuf_, rec.ksiz_, &repeater, i);
        } else if (vbuf != Visitor::NOP) {
          Repeater repeater(vbuf, vsiz);
          accept_impl(rec.kbuf_, rec.ksiz_, &repeater, i);
        }
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
    // ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (thnum < 1) thnum = 1;
    if (thnum > (size_t)INT8MAX) thnum = INT8MAX;
    if (thnum > bnum_) thnum = bnum_;
    ScopedVisitor svis(visitor);
    int64_t allcnt = count_;
    if (checker && !checker->check("scan_parallel", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    class ThreadImpl : public Thread {
     public:
      explicit ThreadImpl() :
          db_(NULL), visitor_(NULL), checker_(NULL), allcnt_(0),
          begidx_(0), endidx_(0), error_() {}
      void init(StashDB* db, Visitor* visitor, ProgressChecker* checker, int64_t allcnt,
                size_t begidx, size_t endidx) {
        db_ = db;
        visitor_ = visitor;
        checker_ = checker;
        allcnt_ = allcnt;
        begidx_ = begidx;
        endidx_ = endidx;
      }
      const Error& error() {
        return error_;
      }
     private:
      void run() {
        StashDB* db = db_;
        Visitor* visitor = visitor_;
        ProgressChecker* checker = checker_;
        int64_t allcnt = allcnt_;
        size_t endidx = endidx_;
        char** buckets = db->buckets_;
        for (size_t i = begidx_; i < endidx; i++) {
          char* rbuf = buckets[i];
          while (rbuf) {
            Record rec(rbuf);
            rbuf = rec.child_;
            size_t vsiz;
            visitor->visit_full(rec.kbuf_, rec.ksiz_, rec.vbuf_, rec.vsiz_, &vsiz);
            if (checker && !checker->check("scan_parallel", "processing", -1, allcnt)) {
              db->set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
              error_ = db->error();
              break;
            }
          }
        }
      }
      StashDB* db_;
      Visitor* visitor_;
      ProgressChecker* checker_;
      int64_t allcnt_;
      size_t begidx_;
      size_t endidx_;
      Error error_;
    };
    bool err = false;
    ThreadImpl* threads = new ThreadImpl[thnum];
    double range = (double)bnum_ / thnum;
    for (size_t i = 0; i < thnum; i++) {
      size_t cidx = i * range;
      size_t nidx = (i + 1) * range;
      if (i < 1) cidx = 0;
      if (i >= thnum - 1) nidx = bnum_;
      ThreadImpl* thread = threads + i;
      thread->init(this, visitor, checker, allcnt, cidx, nidx);
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
   * @param mode the connection mode.  StashDB::OWRITER as a writer, StashDB::OREADER as a
   * reader.  The following may be added to the writer mode by bitwise-or: StashDB::OCREATE,
   * which means it creates a new database if the file does not exist, StashDB::OTRUNCATE, which
   * means it creates a new database regardless if the file exists, StashDB::OAUTOTRAN, which
   * means each updating operation is performed in implicit transaction, StashDB::OAUTOSYNC,
   * which means each updating operation is followed by implicit synchronization with the file
   * system.  The following may be added to both of the reader mode and the writer mode by
   * bitwise-or: StashDB::ONOLOCK, which means it opens the database file without file locking,
   * StashDB::OTRYLOCK, which means locking is performed without blocking, StashDB::ONOREPAIR,
   * which means the database file is not repaired implicitly even if file destruction is
   * detected.
   * @return true on success, or false on failure.
   * @note Every opened database must be closed by the StashDB::close method when it is no
   * longer in use.  It is not allowed for two or more database objects in the same process to
   * keep their connections to the same database file at the same time.
   */
  bool open(const std::string& path, uint32_t mode = OWRITER | OCREATE) {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    report(_KCCODELINE_, Logger::DEBUG, "opening the database (path=%s)", path.c_str());
    omode_ = mode;
    path_.append(path);
    if (bnum_ >= MAPZMAPBNUM) {
      buckets_ = (char**)mapalloc(sizeof(*buckets_) * bnum_);
    } else {
      buckets_ = new char*[bnum_];
      for (size_t i = 0; i < bnum_; i++) {
        buckets_[i] = NULL;
      }
    }
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    report(_KCCODELINE_, Logger::DEBUG, "closing the database (path=%s)", path_.c_str());
    tran_ = false;
    trlogs_.clear();
    for (size_t i = 0; i < bnum_; i++) {
      char* rbuf = buckets_[i];
      while (rbuf) {
        Record rec(rbuf);
        char* child = rec.child_;
        delete[] rbuf;
        rbuf = child;
      }
    }
    if (bnum_ >= MAPZMAPBNUM) {
      mapfree(buckets_);
    } else {
      delete[] buckets_;
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
    // ScopedRWLock lock(&mlock_, false);
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
      if (!proc->process(path_, count_, size_impl())) {
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
    // ScopedRWLock lock(&mlock_, writable);
    bool err = false;
    if (proc && !proc->process(path_, count_, size_impl())) {
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
      // mlock_.lock_writer();
      if (omode_ == 0) {
        set_error(_KCCODELINE_, Error::INVALID, "not opened");
        // mlock_.unlock();
        return false;
      }
      if (!(omode_ & OWRITER)) {
        set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        // mlock_.unlock();
        return false;
      }
      if (!tran_) break;
      // mlock_.unlock();
      if (wcnt >= LOCKBUSYLOOP) {
        Thread::chill();
      } else {
        Thread::yield();
        wcnt++;
      }
    }
    tran_ = true;
    trcount_ = count_;
    trsize_ = size_;
    trigger_meta(MetaTrigger::BEGINTRAN, "begin_transaction");
    // mlock_.unlock();
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
    // mlock_.lock_writer();
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      // mlock_.unlock();
      return false;
    }
    if (!(omode_ & OWRITER)) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      // mlock_.unlock();
      return false;
    }
    if (tran_) {
      set_error(_KCCODELINE_, Error::LOGIC, "competition avoided");
      // mlock_.unlock();
      return false;
    }
    tran_ = true;
    trcount_ = count_;
    trsize_ = size_;
    trigger_meta(MetaTrigger::BEGINTRAN, "begin_transaction_try");
    // mlock_.unlock();
    return true;
  }
  /**
   * End transaction.
   * @param commit true to commit the transaction, or false to abort the transaction.
   * @return true on success, or false on failure.
   */
  bool end_transaction(bool commit = true) {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (!tran_) {
      set_error(_KCCODELINE_, Error::INVALID, "not in transaction");
      return false;
    }
    if (!commit) {
      disable_cursors();
      apply_trlogs();
      count_ = trcount_;
      size_ = trsize_;
    }
    trlogs_.clear();
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    disable_cursors();
    if (count_ > 0) {
      for (size_t i = 0; i < bnum_; i++) {
        char* rbuf = buckets_[i];
        while (rbuf) {
          Record rec(rbuf);
          char* child = rec.child_;
          delete[] rbuf;
          rbuf = child;
        }
        buckets_[i] = NULL;
      }
      count_ = 0;
      size_ = 0;
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
    // ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return -1;
    }
    return count_;
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes, or -1 on failure.
   */
  int64_t size() {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, false);
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
    // ScopedRWLock lock(&mlock_, false);
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    (*strmap)["type"] = strprintf("%u", (unsigned)TYPESTASH);
    (*strmap)["realtype"] = strprintf("%u", (unsigned)TYPESTASH);
    (*strmap)["path"] = path_;
    if (strmap->count("opaque") > 0)
      (*strmap)["opaque"] = std::string(opaque_, sizeof(opaque_));
    if (strmap->count("bnum_used") > 0) {
      int64_t cnt = 0;
      for (size_t i = 0; i < bnum_; i++) {
        if (buckets_[i]) cnt++;
      }
      (*strmap)["bnum_used"] = strprintf("%lld", (long long)cnt);
    }
    (*strmap)["count"] = strprintf("%lld", (long long)count_);
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
    // ScopedRWLock lock(&mlock_, false);
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
    // ScopedRWLock lock(&mlock_, true);
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    mtrigger_ = trigger;
    return true;
  }
  /**
   * Set the number of buckets of the hash table.
   * @param bnum the number of buckets of the hash table.
   * @return true on success, or false on failure.
   */
  bool tune_buckets(int64_t bnum) {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    bnum_ = bnum >= 0 ? bnum : DEFBNUM;
    if (bnum_ > (size_t)INT16MAX) bnum_ = nearbyprime(bnum_);
    return true;
  }
  /**
   * Get the opaque data.
   * @return the pointer to the opaque data region, whose size is 16 bytes.
   */
  char* opaque() {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, false);
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
    // ScopedRWLock lock(&mlock_, true);
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
 private:
  /**
   * Record data.
   */
  struct Record {
    /** constructor */
    Record(char* child, const char* kbuf, uint64_t ksiz, const char* vbuf, uint64_t vsiz) :
        child_(child), kbuf_(kbuf), ksiz_(ksiz), vbuf_(vbuf), vsiz_(vsiz) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    }
    /** constructor */
    Record(const char* rbuf) :
        child_(NULL), kbuf_(NULL), ksiz_(0), vbuf_(NULL), vsiz_(0) {
      _assert_(rbuf);
      deserialize(rbuf);
    }
    /** overwrite the buffer */
    void overwrite(char* rbuf, const char* vbuf, size_t vsiz) {
      _assert_(rbuf && vbuf && vsiz <= MEMMAXSIZ);
      char* wp = rbuf + sizeof(child_) + sizevarnum(ksiz_) + ksiz_;
      wp += writevarnum(wp, vsiz);
      std::memcpy(wp, vbuf, vsiz);
    }
    /** serialize data into a buffer */
    char* serialize() {
      _assert_(true);
      uint64_t rsiz = sizeof(child_) + sizevarnum(ksiz_) + ksiz_ + sizevarnum(vsiz_) + vsiz_;
      char* rbuf = new char[rsiz];
      char* wp = rbuf;
      *(char**)wp = child_;
      wp += sizeof(child_);
      wp += writevarnum(wp, ksiz_);
      std::memcpy(wp, kbuf_, ksiz_);
      wp += ksiz_;
      wp += writevarnum(wp, vsiz_);
      std::memcpy(wp, vbuf_, vsiz_);
      return rbuf;
    }
    /** deserialize a buffer into object */
    void deserialize(const char* rbuf) {
      _assert_(rbuf);
      const char* rp = rbuf;
      child_ = *(char**)rp;
      rp += sizeof(child_);
      rp += readvarnum(rp, sizeof(ksiz_), &ksiz_);
      kbuf_ = rp;
      rp += ksiz_;
      rp += readvarnum(rp, sizeof(vsiz_), &vsiz_);
      vbuf_ = rp;
    }
    /** print debug info */
    void print() {
      std::cout << "child:" << (void*)child_ << std::endl;
      std::cout << "key:" << std::string(kbuf_, ksiz_) << std::endl;
      std::cout << "value:" << std::string(vbuf_, vsiz_) << std::endl;
      std::cout << "ksiz:" << ksiz_ << std::endl;
      std::cout << "vsiz:" << vsiz_ << std::endl;
    }
    char* child_;                        ///< region of the child
    const char* kbuf_;                   ///< region of the key
    uint64_t ksiz_;                      ///< size of the key
    const char* vbuf_;                   ///< region of the value
    uint64_t vsiz_;                      ///< size of the key
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
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param visitor a visitor object.
   * @param bidx the bucket index.
   */
  void accept_impl(const char* kbuf, size_t ksiz, Visitor* visitor, size_t bidx) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor);
    char* rbuf = buckets_[bidx];
    char** entp = buckets_ + bidx;
    while (rbuf) {
      Record rec(rbuf);
      if (rec.ksiz_ == ksiz && !std::memcmp(rec.kbuf_, kbuf, ksiz)) {
        size_t vsiz;
        const char* vbuf = visitor->visit_full(rec.kbuf_, rec.ksiz_,
                                               rec.vbuf_, rec.vsiz_, &vsiz);
        if (vbuf == Visitor::REMOVE) {
          if (tran_) {
            ScopedMutex lock(&flock_);
            TranLog log(rec.kbuf_, rec.ksiz_, rec.vbuf_, rec.vsiz_);
            trlogs_.push_back(log);
          }
          count_ -= 1;
          size_ -= rec.ksiz_ + rec.vsiz_;
          escape_cursors(rbuf);
          *entp = rec.child_;
          delete[] rbuf;
        } else if (vbuf != Visitor::NOP) {
          if (tran_) {
            ScopedMutex lock(&flock_);
            TranLog log(rec.kbuf_, rec.ksiz_, rec.vbuf_, rec.vsiz_);
            trlogs_.push_back(log);
          }
          int32_t oh = (int32_t)sizevarnum(vsiz) - (int32_t)sizevarnum(rec.vsiz_);
          int64_t diff = (int64_t)rec.vsiz_ - (int64_t)(vsiz + oh);
          size_ += (int64_t)vsiz - (int64_t)rec.vsiz_;
          if (diff >= 0) {
            rec.overwrite(rbuf, vbuf, vsiz);
          } else {
            Record nrec(rec.child_, kbuf, ksiz, vbuf, vsiz);
            char* nbuf = nrec.serialize();
            adjust_cursors(rbuf, nbuf);
            *entp = nbuf;
            delete[] rbuf;
          }
        }
        return;
      }
      entp = (char**)rbuf;
      rbuf = rec.child_;
    }
    size_t vsiz;
    const char* vbuf = visitor->visit_empty(kbuf, ksiz, &vsiz);
    if (vbuf != Visitor::REMOVE && vbuf != Visitor::NOP) {
      if (tran_) {
        ScopedMutex lock(&flock_);
        TranLog log(kbuf, ksiz);
        trlogs_.push_back(log);
      }
      Record nrec(NULL, kbuf, ksiz, vbuf, vsiz);
      *entp = nrec.serialize();
      count_ += 1;
      size_ += ksiz + vsiz;
    }
  }
  /**
   * Get the hash value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @return the hash value.
   */
  size_t hash_record(const char* kbuf, size_t ksiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    return hashmurmur(kbuf, ksiz);
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes.
   */
  int64_t size_impl() {
    _assert_(true);
    return bnum_ * sizeof(*buckets_) + count_ * (4 + sizeof(void*)) + size_;
  }
  /**
   * Escape cursors on a shifted or removed records.
   * @param rbuf the record buffer.
   */
  void escape_cursors(char* rbuf) {
    _assert_(rbuf);
    ScopedMutex lock(&flock_);
    if (curs_.empty()) return;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->rbuf_ == rbuf) cur->step_impl();
      ++cit;
    }
  }
  /**
   * Adjust cursors on re-allocated records.
   * @param obuf the old address.
   * @param nbuf the new address.
   */
  void adjust_cursors(char* obuf, char* nbuf) {
    _assert_(obuf && nbuf);
    ScopedMutex lock(&flock_);
    if (curs_.empty()) return;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->rbuf_ == obuf) cur->rbuf_ = nbuf;
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
      cur->bidx_ = -1;
      cur->rbuf_ = NULL;
      ++cit;
    }
  }
  /**
   * Apply transaction logs.
   */
  void apply_trlogs() {
    _assert_(true);
    TranLogList::const_iterator it = trlogs_.end();
    TranLogList::const_iterator itbeg = trlogs_.begin();
    while (it != itbeg) {
      --it;
      const char* kbuf = it->key.c_str();
      size_t ksiz = it->key.size();
      const char* vbuf = it->value.c_str();
      size_t vsiz = it->value.size();
      size_t bidx = hash_record(kbuf, ksiz) % bnum_;
      if (it->full) {
        Setter setter(vbuf, vsiz);
        accept_impl(kbuf, ksiz, &setter, bidx);
      } else {
        Remover remover;
        accept_impl(kbuf, ksiz, &remover, bidx);
      }
    }
  }
  /** Dummy constructor to forbid the use. */
  StashDB(const StashDB&);
  /** Dummy Operator to forbid the use. */
  StashDB& operator =(const StashDB&);
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
  /** The number of buckets. */
  size_t bnum_;
  /** The opaque data. */
  char opaque_[OPAQUESIZ];
  /** The record number. */
  AtomicInt64 count_;
  /** The total size of records. */
  AtomicInt64 size_;
  /** The bucket array. */
  char** buckets_;
  /** The flag whether in transaction. */
  bool tran_;
  /** The list of transaction logs. */
  TranLogList trlogs_;
  /** The count history for transaction. */
  int64_t trcount_;
  /** The size history for transaction. */
  int64_t trsize_;
};


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
