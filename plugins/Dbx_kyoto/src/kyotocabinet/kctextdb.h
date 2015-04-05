/*************************************************************************************************
 * Plain text database
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


#ifndef _KCTEXTDB_H                      // duplication check
#define _KCTEXTDB_H

#include <kccommon.h>
#include <kcutil.h>
#include <kcthread.h>
#include <kcfile.h>
#include <kccompress.h>
#include <kccompare.h>
#include <kcmap.h>
#include <kcregex.h>
#include <kcdb.h>

namespace kyotocabinet {                 // common namespace


/**
 * Plain text database.
 * @note Although this class is designed to use a text file as a database file, not all methods
 * are implemented.  Each line in the text is treated as a record.  When storing a record, the
 * key is ignored and the value only is appended at the end of the file.  Records can be
 * retrieved only by the iterator and the cursor mechanisms.  No record can be retrieved by
 * specifying the key.  When accessing a record by the iterator, the key is given as the offset
 * from the beginning of the file for descriptive purposes.  Any existing record cannot be
 * modified and deleted.
 */
class TextDB : public BasicDB {
 public:
  class Cursor;
 private:
  class ScopedVisitor;
  /** An alias of list of cursors. */
  typedef std::list<Cursor*> CursorList;
  /** An alias of a past record. */
  typedef std::pair<int64_t, std::string> Record;
  /** The size of the IO buffer. */
  static const size_t IOBUFSIZ = 1024;
 public:
  /**
   * Cursor to indicate a record.
   */
  class Cursor : public BasicDB::Cursor {
    friend class TextDB;
   public:
    /**
     * Constructor.
     * @param db the container database object.
     */
    explicit Cursor(TextDB* db) : db_(db), off_(INT64MAX), end_(0), queue_(), line_() {
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
     * @note The key is generated from the offset of each record.  To avoid deadlock, any
     * explicit database operation must not be performed in this function.
     */
    bool accept(Visitor* visitor, bool writable = true, bool step = false) {
      _assert_(visitor);
      // ScopedRWLock lock(&db_->mlock_, false);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (writable && !db_->writer_) {
        db_->set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        return false;
      }
      bool err = false;
      if (!accept_impl(visitor, step)) err = true;
      return !err;
    }
    /**
     * Jump the cursor to the first record for forward scan.
     * @return true on success, or false on failure.
     */
    bool jump() {
      _assert_(true);
      // ScopedRWLock lock(&db_->mlock_, false);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      off_ = 0;
      end_ = db_->file_.size();
      queue_.clear();
      line_.clear();
      if (off_ >= end_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      return true;
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
      off_ = atoin(kbuf, ksiz);
      end_ = db_->file_.size();
      queue_.clear();
      line_.clear();
      if (off_ >= end_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      return true;
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
      return jump_back(key.c_str(), key.size());
    }
    /**
     * Step the cursor to the next record.
     * @return true on success, or false on failure.
     */
    bool step() {
      _assert_(true);
      // ScopedRWLock lock(&db_->mlock_, false);
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (queue_.empty() && !read_next()) return false;
      if (queue_.empty()) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      queue_.pop_front();
      return true;
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
    TextDB* db() {
      _assert_(true);
      return db_;
    }
   private:
    /**
     * Accept a visitor to the current record.
     * @param visitor a visitor object.
     * @param step true to move the cursor to the next record, or false for no move.
     * @return true on success, or false on failure.
     */
    bool accept_impl(Visitor* visitor, bool step) {
      _assert_(visitor);
      if (queue_.empty() && !read_next()) return false;
      if (queue_.empty()) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      bool err = false;
      const Record& rec = queue_.front();
      char kbuf[NUMBUFSIZ];
      size_t ksiz = db_->write_key(kbuf, rec.first);
      size_t vsiz;
      const char* vbuf = visitor->visit_full(kbuf, ksiz,
                                             rec.second.data(), rec.second.size(), &vsiz);
      if (vbuf != Visitor::NOP && vbuf != Visitor::REMOVE) {
        char stack[IOBUFSIZ];
        size_t rsiz = vsiz + 1;
        char* rbuf = rsiz > sizeof(stack) ? new char[rsiz] : stack;
        std::memcpy(rbuf, vbuf, vsiz);
        rbuf[vsiz] = '\n';
        if (!db_->file_.append(rbuf, rsiz)) {
          db_->set_error(_KCCODELINE_, Error::SYSTEM, db_->file_.error());
          err = true;
        }
        if (rbuf != stack) delete[] rbuf;
        if (db_->autosync_ && !db_->file_.synchronize(true)) {
          db_->set_error(_KCCODELINE_, Error::SYSTEM, db_->file_.error());
          err = true;
        }
      }
      if (step) queue_.pop_front();
      return !err;
    }
    /**
     * Read the next record.
     * @return true on success, or false on failure.
     */
    bool read_next() {
      _assert_(true);
      while (off_ < end_) {
        char stack[IOBUFSIZ];
        int64_t rsiz = end_ - off_;
        if (rsiz > (int64_t)sizeof(stack)) rsiz = sizeof(stack);
        if (!db_->file_.read_fast(off_, stack, rsiz)) {
          db_->set_error(_KCCODELINE_, Error::SYSTEM, db_->file_.error());
          return false;
        }
        const char* rp = stack;
        const char* pv = rp;
        const char* ep = rp + rsiz;
        while (rp < ep) {
          if (*rp == '\n') {
            line_.append(pv, rp - pv);
            Record rec;
            rec.first = off_ + pv - stack;
            rec.second = line_;
            queue_.push_back(rec);
            line_.clear();
            rp++;
            pv = rp;
          } else {
            rp++;
          }
        }
        line_.append(pv, rp - pv);
        off_ += rsiz;
        if (!queue_.empty()) break;
      }
      return true;
    }
    /** Dummy constructor to forbid the use. */
    Cursor(const Cursor&);
    /** Dummy Operator to forbid the use. */
    Cursor& operator =(const Cursor&);
    /** The inner database. */
    TextDB* db_;
    /** The current offset. */
    int64_t off_;
    /** The end offset. */
    int64_t end_;
    /** The queue of read lines. */
    std::deque<Record> queue_;
    /** The current line. */
    std::string line_;
  };
  /**
   * Default constructor.
   */
  explicit TextDB() :
      error_(), logger_(NULL), logkinds_(0), mtrigger_(NULL),
      omode_(0), writer_(false), autotran_(false), autosync_(false),
      file_(), curs_(), path_("") {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  virtual ~TextDB() {
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
   * @note No record can be retrieved by specifying the key and the Visitor::visit_empty method
   * is always called.  To avoid deadlock, any explicit database operation must not be performed
   * in this function.
   */
  bool accept(const char* kbuf, size_t ksiz, Visitor* visitor, bool writable = true) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor);
    // ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    bool err = false;
    if (!accept_impl(kbuf, ksiz, visitor)) err = true;
    return !err;
  }
  /**
   * Accept a visitor to multiple records at once.
   * @param keys specifies a string vector of the keys.
   * @param visitor a visitor object.
   * @param writable true for writable operation, or false for read-only operation.
   * @return true on success, or false on failure.
   * @note No record can be retrieved by specifying the key and the Visitor::visit_empty method
   * is always called.  To avoid deadlock, any explicit database operation must not be performed
   * in this function.
   */
  bool accept_bulk(const std::vector<std::string>& keys, Visitor* visitor,
                   bool writable = true) {
    _assert_(visitor);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    bool err = false;
    std::vector<std::string>::const_iterator kit = keys.begin();
    std::vector<std::string>::const_iterator kitend = keys.end();
    while (kit != kitend) {
      if (!accept_impl(kit->data(), kit->size(), visitor)) err = true;
      ++kit;
    }
    return !err;
  }
  /**
   * Iterate to accept a visitor for each record.
   * @param visitor a visitor object.
   * @param writable true for writable operation, or false for read-only operation.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   * @note The key is generated from the offset of each record.  To avoid deadlock, any explicit
   * database operation must not be performed in this function.
   */
  bool iterate(Visitor *visitor, bool writable = true, ProgressChecker* checker = NULL) {
    _assert_(visitor);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    if (writable && !writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    bool err = false;
    if (!iterate_impl(visitor, checker)) err = true;
    trigger_meta(MetaTrigger::ITERATE, "iterate");
    return !err;
  }
  /**
   * Scan each record in parallel.
   * @param visitor a visitor object.
   * @param thnum the number of worker threads.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   * @note This function is for reading records and not for updating ones.  The return value of
   * the visitor is just ignored.  The key is generated from the offset of each record.  To
   * avoid deadlock, any explicit database operation must not be performed in this function.
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
    ScopedVisitor svis(visitor);
    bool err = false;
    if (!scan_parallel_impl(visitor, thnum, checker)) err = true;
    trigger_meta(MetaTrigger::ITERATE, "scan_parallel");
    return !err;
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
   * @param mode the connection mode.  TextDB::OWRITER as a writer, TextDB::OREADER as a
   * reader.  The following may be added to the writer mode by bitwise-or: TextDB::OCREATE,
   * which means it creates a new database if the file does not exist, TextDB::OTRUNCATE, which
   * means it creates a new database regardless if the file exists, TextDB::OAUTOTRAN, which
   * means each updating operation is performed in implicit transaction, TextDB::OAUTOSYNC,
   * which means each updating operation is followed by implicit synchronization with the file
   * system.  The following may be added to both of the reader mode and the writer mode by
   * bitwise-or: TextDB::ONOLOCK, which means it opens the database file without file locking,
   * TextDB::OTRYLOCK, which means locking is performed without blocking, TextDB::ONOREPAIR,
   * which means the database file is not repaired implicitly even if file destruction is
   * detected.
   * @return true on success, or false on failure.
   * @note Every opened database must be closed by the TextDB::close method when it is no
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
    writer_ = false;
    autotran_ = false;
    autosync_ = false;
    uint32_t fmode = File::OREADER;
    if (mode & OWRITER) {
      writer_ = true;
      fmode = File::OWRITER;
      if (mode & OCREATE) fmode |= File::OCREATE;
      if (mode & OTRUNCATE) fmode |= File::OTRUNCATE;
      if (mode & OAUTOTRAN) autotran_ = true;
      if (mode & OAUTOSYNC) autosync_ = true;
    }
    if (mode & ONOLOCK) fmode |= File::ONOLOCK;
    if (mode & OTRYLOCK) fmode |= File::OTRYLOCK;
    if (!file_.open(path, fmode, 0)) {
      const char* emsg = file_.error();
      Error::Code code = Error::SYSTEM;
      if (std::strstr(emsg, "(permission denied)") || std::strstr(emsg, "(directory)")) {
        code = Error::NOPERM;
      } else if (std::strstr(emsg, "(file not found)") || std::strstr(emsg, "(invalid path)")) {
        code = Error::NOREPOS;
      }
      set_error(_KCCODELINE_, code, emsg);
      return false;
    }
    if (autosync_ && !File::synchronize_whole()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
      file_.close();
      return false;
    }
    path_.append(path);
    omode_ = mode;
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
    bool err = false;
    disable_cursors();
    if (!file_.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      err = true;
    }
    omode_ = 0;
    path_.clear();
    trigger_meta(MetaTrigger::CLOSE, "close");
    return !err;
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
    if (!synchronize_impl(hard, proc, checker)) err = true;
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
    if (proc && !proc->process(path_, -1, file_.size())) {
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
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
    return false;
  }
  /**
   * Try to begin transaction.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @return true on success, or false on failure.
   */
  bool begin_transaction_try(bool hard = false) {
    _assert_(true);
    // ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
    return false;
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
    set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
    return false;
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
    if (!writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    disable_cursors();
    if (!file_.truncate(0)) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      return false;
    }
    if (autosync_ && !file_.synchronize(true)) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      return false;
    }
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
    set_error(_KCCODELINE_, Error::NOIMPL, "not implemented");
    return -1;
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
    return file_.size();
  }
  /**
   * Get the path of the database file.
   * @return the path of the database file, or an empty string on failure.
   */
  std::string path() {
    _assert_(true);
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
    (*strmap)["type"] = strprintf("%u", (unsigned)TYPETEXT);
    (*strmap)["realtype"] = strprintf("%u", (unsigned)TYPETEXT);
    (*strmap)["path"] = path_;
    (*strmap)["size"] = strprintf("%lld", (long long)file_.size());
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
   * @return true on success, or false on failure.
   */
  bool accept_impl(const char* kbuf, size_t ksiz, Visitor* visitor) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor);
    bool err = false;
    size_t vsiz;
    const char* vbuf = visitor->visit_empty(kbuf, ksiz, &vsiz);
    if (vbuf != Visitor::NOP && vbuf != Visitor::REMOVE) {
      size_t rsiz = vsiz + 1;
      char stack[IOBUFSIZ];
      char* rbuf = rsiz > sizeof(stack) ? new char[rsiz] : stack;
      std::memcpy(rbuf, vbuf, vsiz);
      rbuf[vsiz] = '\n';
      if (!file_.append(rbuf, rsiz)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        err = true;
      }
      if (rbuf != stack) delete[] rbuf;
      if (autosync_ && !file_.synchronize(true)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        err = true;
      }
    }
    return !err;
  }
  /**
   * Iterate to accept a visitor for each record.
   * @param visitor a visitor object.
   * @param checker a progress checker object.
   * @return true on success, or false on failure.
   */
  bool iterate_impl(Visitor* visitor, ProgressChecker* checker) {
    _assert_(visitor);
    if (checker && !checker->check("iterate", "beginning", 0, -1)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    int64_t off = 0;
    int64_t end = file_.size();
    int64_t curcnt = 0;
    std::string line;
    char stack[IOBUFSIZ*4];
    while (off < end) {
      int64_t rsiz = end - off;
      if (rsiz > (int64_t)sizeof(stack)) rsiz = sizeof(stack);
      if (!file_.read_fast(off, stack, rsiz)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        return false;
      }
      const char* rp = stack;
      const char* pv = rp;
      const char* ep = rp + rsiz;
      while (rp < ep) {
        if (*rp == '\n') {
          char kbuf[NUMBUFSIZ];
          size_t ksiz = write_key(kbuf, off + pv - stack);
          const char* vbuf;
          size_t vsiz;
          if (line.empty()) {
            vbuf = visitor->visit_full(kbuf, ksiz, pv, rp - pv, &vsiz);
          } else {
            line.append(pv, rp - pv);
            vbuf = visitor->visit_full(kbuf, ksiz, line.data(), line.size(), &vsiz);
            line.clear();
          }
          if (vbuf != Visitor::NOP && vbuf != Visitor::REMOVE) {
            char tstack[IOBUFSIZ];
            size_t trsiz = vsiz + 1;
            char* trbuf = trsiz > sizeof(tstack) ? new char[trsiz] : tstack;
            std::memcpy(trbuf, vbuf, vsiz);
            trbuf[vsiz] = '\n';
            if (!file_.append(trbuf, trsiz)) {
              set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
              if (trbuf != stack) delete[] trbuf;
              return false;
            }
            if (trbuf != tstack) delete[] trbuf;
          }
          curcnt++;
          if (checker && !checker->check("iterate", "processing", curcnt, -1)) {
            set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
            return false;
          }
          rp++;
          pv = rp;
        } else {
          rp++;
        }
      }
      line.append(pv, rp - pv);
      off += rsiz;
    }
    if (checker && !checker->check("iterate", "ending", -1, -1)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    return true;
  }
  /**
   * Scan each record in parallel.
   * @param visitor a visitor object.
   * @param thnum the number of worker threads.
   * @param checker a progress checker object.
   * @return true on success, or false on failure.
   */
  bool scan_parallel_impl(Visitor *visitor, size_t thnum, ProgressChecker* checker) {
    _assert_(visitor && thnum <= MEMMAXSIZ);
    if (checker && !checker->check("scan_parallel", "beginning", -1, -1)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    int64_t off = 0;
    int64_t end = file_.size();
    int64_t gap = (end - off) / thnum;
    std::vector<int64_t> offs;
    while (off < end) {
      offs.push_back(off);
      int64_t edge = off + gap;
      while (true) {
        if (edge >= end) {
          off = end;
          break;
        }
        char rbuf[IOBUFSIZ];
        int64_t rsiz = end - edge;
        if (rsiz > (int64_t)sizeof(rbuf)) rsiz = sizeof(rbuf);
        if (!file_.read_fast(edge, rbuf, rsiz)) {
          set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
          return false;
        }
        int64_t noff = -1;
        const char* rp = rbuf;
        const char* ep = rp + rsiz;
        while (rp < ep) {
          if (*rp == '\n') {
            noff = edge + (rp - rbuf);
            break;
          }
          rp++;
        }
        if (noff >= 0) {
          off = noff + 1;
          break;
        }
        edge += rsiz;
      }
    }
    bool err = false;
    size_t onum = offs.size();
    if (onum > 0) {
      class ThreadImpl : public Thread {
       public:
        explicit ThreadImpl() :
            db_(NULL), visitor_(NULL), checker_(NULL), begoff_(0), endoff_(0), error_() {}
        void init(TextDB* db, Visitor* visitor, ProgressChecker* checker,
                  int64_t begoff, int64_t endoff) {
          db_ = db;
          visitor_ = visitor;
          checker_ = checker;
          begoff_ = begoff;
          endoff_ = endoff;
        }
        const Error& error() {
          return error_;
        }
       private:
        void run() {
          TextDB* db = db_;
          File* file = &db->file_;
          Visitor* visitor = visitor_;
          ProgressChecker* checker = checker_;
          int64_t off = begoff_;
          int64_t end = endoff_;
          std::string line;
          char stack[IOBUFSIZ*4];
          while (off < end) {
            int64_t rsiz = end - off;
            if (rsiz > (int64_t)sizeof(stack)) rsiz = sizeof(stack);
            if (!file->read_fast(off, stack, rsiz)) {
              db->set_error(_KCCODELINE_, Error::SYSTEM, file->error());
              return;
            }
            const char* rp = stack;
            const char* pv = rp;
            const char* ep = rp + rsiz;
            while (rp < ep) {
              if (*rp == '\n') {
                char kbuf[NUMBUFSIZ];
                size_t ksiz = db->write_key(kbuf, off + pv - stack);
                if (line.empty()) {
                  size_t vsiz;
                  visitor->visit_full(kbuf, ksiz, pv, rp - pv, &vsiz);
                } else {
                  line.append(pv, rp - pv);
                  size_t vsiz;
                  visitor->visit_full(kbuf, ksiz, line.data(), line.size(), &vsiz);
                  line.clear();
                }
                if (checker && !checker->check("iterate", "processing", -1, -1)) {
                  db->set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
                  return;
                }
                rp++;
                pv = rp;
              } else {
                rp++;
              }
            }
            line.append(pv, rp - pv);
            off += rsiz;
          }
        }
        TextDB* db_;
        Visitor* visitor_;
        ProgressChecker* checker_;
        int64_t begoff_;
        int64_t endoff_;
        Error error_;
      };
      ThreadImpl* threads = new ThreadImpl[onum];
      for (size_t i = 0; i < onum; i++) {
        int64_t begoff = offs[i];
        int64_t endoff = i < onum - 1 ? offs[i+1] : end;
        ThreadImpl* thread = threads + i;
        thread->init(this, visitor, checker, begoff, endoff);
        thread->start();
      }
      for (size_t i = 0; i < onum; i++) {
        ThreadImpl* thread = threads + i;
        thread->join();
        if (thread->error() != Error::SUCCESS) {
          *error_ = thread->error();
          err = true;
        }
      }
      delete[] threads;
    }
    if (checker && !checker->check("scan_parallel", "ending", -1, -1)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    return !err;
  }
  /**
   * Synchronize updated contents with the file and the device.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @param proc a postprocessor object.
   * @param checker a progress checker object.
   * @return true on success, or false on failure.
   */
  bool synchronize_impl(bool hard, FileProcessor* proc, ProgressChecker* checker) {
    _assert_(true);
    bool err = false;
    if (writer_) {
      if (checker && !checker->check("synchronize", "synchronizing the file", -1, -1)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        return false;
      }
      if (!file_.synchronize(hard)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        err = true;
      }
    }
    if (proc) {
      if (checker && !checker->check("synchronize", "running the post processor", -1, -1)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        return false;
      }
      if (!proc->process(path_, -1, file_.size())) {
        set_error(_KCCODELINE_, Error::LOGIC, "postprocessing failed");
        err = true;
      }
    }
    return !err;
  }
  /**
   * Disable all cursors.
   */
  void disable_cursors() {
    _assert_(true);
    if (curs_.empty()) return;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      cur->off_ = INT64MAX;
      ++cit;
    }
  }
  /**
   * Write the key pattern into a buffer.
   * @param kbuf the destination buffer.
   * @param off the offset of the record.
   * @return the size of the key pattern.
   */
  size_t write_key(char* kbuf, int64_t off) {
    _assert_(kbuf && off >= 0);
    for (size_t i = 0; i < sizeof(off); i++) {
      uint8_t c = off >> ((sizeof(off) - 1 - i) * 8);
      uint8_t h = c >> 4;
      if (h < 10) {
        *(kbuf++) = '0' + h;
      } else {
        *(kbuf++) = 'A' - 10 + h;
      }
      uint8_t l = c & 0xf;
      if (l < 10) {
        *(kbuf++) = '0' + l;
      } else {
        *(kbuf++) = 'A' - 10 + l;
      }
    }
    return sizeof(off) * 2;
  }
  /** Dummy constructor to forbid the use. */
  TextDB(const TextDB&);
  /** Dummy Operator to forbid the use. */
  TextDB& operator =(const TextDB&);
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
  /** The flag for writer. */
  bool writer_;
  /** The flag for auto transaction. */
  bool autotran_;
  /** The flag for auto synchronization. */
  bool autosync_;
  /** The file for data. */
  File file_;
  /** The cursor objects. */
  CursorList curs_;
  /** The path of the database file. */
  std::string path_;
};


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
