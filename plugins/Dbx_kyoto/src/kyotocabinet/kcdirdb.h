/*************************************************************************************************
 * Directory hash database
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


#ifndef _KCDIRDB_H                       // duplication check
#define _KCDIRDB_H

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

#define KCDDBMAGICFILE  "__KCDIR__"      ///< magic file of the directory
#define KCDDBMETAFILE  "__meta__"        ///< meta data file of the directory
#define KCDDBOPAQUEFILE  "__opq__"       ///< opaque file of the directory
#define KCDDBATRANPREFIX  "_x"           ///< prefix of files for auto transaction
#define KCDDBCHKSUMSEED  "__kyotocabinet__"  ///< seed of the module checksum
#define KCDDBMAGICEOF  "_EOF_"           ///< magic data for the end of file
#define KCDDBWALPATHEXT  "wal"           ///< extension of the WAL directory
#define KCDDBTMPPATHEXT  "tmp"           ///< extension of the temporary directory

namespace kyotocabinet {                 // common namespace


/**
 * Directory hash database.
 * @note This class is a concrete class to operate a hash database in a directory.  This class
 * can be inherited but overwriting methods is forbidden.  Before every database operation, it is
 * necessary to call the DirDB::open method in order to open a database file and connect the
 * database object to it.  To avoid data missing or corruption, it is important to close every
 * database file by the DirDB::close method when the database is no longer in use.  It is
 * forbidden for multible database objects in a process to open the same database at the same
 * time.  It is forbidden to share a database object with child processes.
 */
class DirDB : public BasicDB {
  friend class PlantDB<DirDB, BasicDB::TYPEFOREST>;
 public:
  class Cursor;
 private:
  struct Record;
  class ScopedVisitor;
  /** An alias of list of cursors. */
  typedef std::list<Cursor*> CursorList;
  /** An alias of vector of strings. */
  typedef std::vector<std::string> StringVector;
  /** The size of the meta data buffer. */
  static const int64_t METABUFSIZ = 128;
  /** The magic data for record. */
  static const uint8_t RECMAGIC = 0xcc;
  /** The number of slots of the record lock. */
  static const int32_t RLOCKSLOT = 2048;
  /** The unit size of a record. */
  static const int32_t RECUNITSIZ = 32;
  /** The size of the opaque buffer. */
  static const size_t OPAQUESIZ = 16;
  /** The threshold of busy loop and sleep for locking. */
  static const uint32_t LOCKBUSYLOOP = 8192;
 public:
  /**
   * Cursor to indicate a record.
   */
  class Cursor : public BasicDB::Cursor {
    friend class DirDB;
   public:
    /**
     * Constructor.
     * @param db the container database object.
     */
    explicit Cursor(DirDB* db) : db_(db), dir_(), alive_(false), name_("") {
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
      if (writable && !(db_->writer_)) {
        db_->set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
        return false;
      }
      if (!alive_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      bool err = false;
      const std::string& rpath = db_->path_ + File::PATHCHR + name_;
      int64_t cnt = db_->count_;
      Record rec;
      if (db_->read_record(rpath, &rec)) {
        if (!db_->accept_visit_full(rec.kbuf, rec.ksiz, rec.vbuf, rec.vsiz, rec.rsiz,
                                    visitor, rpath, name_.c_str())) err = true;
        delete[] rec.rbuf;
        if (alive_ && step && db_->count_ == cnt) {
          do {
            if (!dir_.read(&name_)) {
              if (!disable()) err = true;
              break;
            }
          } while (*name_.c_str() == *KCDDBMAGICFILE);
        }
      } else {
        while (true) {
          if (!dir_.read(&name_)) {
            db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
            disable();
            break;
          }
          if (*name_.c_str() == *KCDDBMAGICFILE) continue;
          const std::string& npath = db_->path_ + File::PATHCHR + name_;
          if (!File::status(npath)) continue;
          if (db_->read_record(npath, &rec)) {
            if (!db_->accept_visit_full(rec.kbuf, rec.ksiz, rec.vbuf, rec.vsiz, rec.rsiz,
                                        visitor, npath, name_.c_str())) err = true;
            delete[] rec.rbuf;
            if (alive_ && step && db_->count_ == cnt) {
              do {
                if (!dir_.read(&name_)) {
                  if (!disable()) err = true;
                  break;
                }
              } while (*name_.c_str() == *KCDDBMAGICFILE);
            }
          } else {
            db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
            err = true;
          }
          break;
        }
      }
      return !err;
    }
    /**
     * Jump the cursor to the first record for forward scan.
     * @return true on success, or false on failure.
     */
    bool jump() {
      _assert_(true);
      ScopedRWLock lock(&db_->mlock_, true);
      if (alive_ && !disable()) return false;
      if (db_->omode_ == 0) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      if (!dir_.open(db_->path_)) {
        db_->set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
        return false;
      }
      alive_ = true;
      do {
        if (!dir_.read(&name_)) {
          db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
          disable();
          return false;
        }
      } while (*name_.c_str() == *KCDDBMAGICFILE);
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
      ScopedRWLock lock(&db_->mlock_, true);
      if (alive_ && !disable()) return false;
      if (!dir_.open(db_->path_)) {
        db_->set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
        return false;
      }
      alive_ = true;
      while (true) {
        if (!dir_.read(&name_)) {
          db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
          disable();
          return false;
        }
        if (*name_.c_str() == *KCDDBMAGICFILE) continue;
        const std::string& rpath = db_->path_ + File::PATHCHR + name_;
        Record rec;
        if (db_->read_record(rpath, &rec)) {
          if (rec.ksiz == ksiz && !std::memcmp(rec.kbuf, kbuf, ksiz)) {
            delete[] rec.rbuf;
            break;
          }
          delete[] rec.rbuf;
        } else {
          db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
          disable();
          return false;
        }
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
      if (!alive_) {
        db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
        return false;
      }
      do {
        if (!dir_.read(&name_)) {
          db_->set_error(_KCCODELINE_, Error::NOREC, "no record");
          disable();
          return false;
        }
      } while (*name_.c_str() == *KCDDBMAGICFILE);
      return true;
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
    DirDB* db() {
      _assert_(true);
      return db_;
    }
   private:
    /**
     * Disable the cursor.
     * @return true on success, or false on failure.
     */
    bool disable() {
      bool err = false;
      if (!dir_.close()) {
        db_->set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
        err = true;
      }
      alive_ = false;
      return !err;
    }
    /** Dummy constructor to forbid the use. */
    Cursor(const Cursor&);
    /** Dummy Operator to forbid the use. */
    Cursor& operator =(const Cursor&);
    /** The inner database. */
    DirDB* db_;
    /** The inner directory stream. */
    DirStream dir_;
    /** The flag if alive. */
    bool alive_;
    /** The current name. */
    std::string name_;
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
  explicit DirDB() :
      mlock_(), rlock_(RLOCKSLOT), error_(),
      logger_(NULL), logkinds_(0), mtrigger_(NULL),
      omode_(0), writer_(false), autotran_(false), autosync_(false),
      recov_(false), reorg_(false),
      file_(), curs_(), path_(""),
      libver_(LIBVER), librev_(LIBREV), fmtver_(FMTVER), chksum_(0), type_(TYPEDIR),
      flags_(0), opts_(0), count_(0), size_(0), opaque_(), embcomp_(ZLIBRAWCOMP), comp_(NULL),
      tran_(false), trhard_(false), trcount_(0), trsize_(0), walpath_(""), tmppath_("") {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  virtual ~DirDB() {
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
    if (writable && !writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    bool err = false;
    char name[NUMBUFSIZ];
    size_t lidx = hashpath(kbuf, ksiz, name) % RLOCKSLOT;
    if (writable) {
      rlock_.lock_writer(lidx);
    } else {
      rlock_.lock_reader(lidx);
    }
    if (!accept_impl(kbuf, ksiz, visitor, name)) err = true;
    rlock_.unlock(lidx);
    return !err;
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
    if (writable && !writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    ScopedVisitor svis(visitor);
    size_t knum = keys.size();
    if (knum < 1) return true;
    bool err = false;
    struct RecordKey {
      const char* kbuf;
      size_t ksiz;
      char name[NUMBUFSIZ];
    };
    RecordKey* rkeys = new RecordKey[knum];
    std::set<size_t> lidxs;
    for (size_t i = 0; i < knum; i++) {
      const std::string& key = keys[i];
      RecordKey* rkey = rkeys + i;
      rkey->kbuf = key.data();
      rkey->ksiz = key.size();
      lidxs.insert(hashpath(rkey->kbuf, rkey->ksiz, rkey->name) % RLOCKSLOT);
    }
    std::set<size_t>::iterator lit = lidxs.begin();
    std::set<size_t>::iterator litend = lidxs.end();
    while (lit != litend) {
      if (writable) {
        rlock_.lock_writer(*lit);
      } else {
        rlock_.lock_reader(*lit);
      }
      ++lit;
    }
    for (size_t i = 0; i < knum; i++) {
      RecordKey* rkey = rkeys + i;
      if (!accept_impl(rkey->kbuf, rkey->ksiz, visitor, rkey->name)) {
        err = true;
        break;
      }
    }
    lit = lidxs.begin();
    litend = lidxs.end();
    while (lit != litend) {
      rlock_.unlock(*lit);
      ++lit;
    }
    delete[] rkeys;
    return !err;
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
    if (thnum < 1) thnum = 0;
    if (thnum > (size_t)INT8MAX) thnum = INT8MAX;
    ScopedVisitor svis(visitor);
    rlock_.lock_reader_all();
    bool err = false;
    if (!scan_parallel_impl(visitor, thnum, checker)) err = true;
    rlock_.unlock_all();
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
    if (code == Error::BROKEN || code == Error::SYSTEM) flags_ |= FFATAL;
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
   * @param mode the connection mode.  DirDB::OWRITER as a writer, DirDB::OREADER as a
   * reader.  The following may be added to the writer mode by bitwise-or: DirDB::OCREATE,
   * which means it creates a new database if the file does not exist, DirDB::OTRUNCATE, which
   * means it creates a new database regardless if the file exists, DirDB::OAUTOTRAN, which
   * means each updating operation is performed in implicit transaction, DirDB::OAUTOSYNC,
   * which means each updating operation is followed by implicit synchronization with the file
   * system.  The following may be added to both of the reader mode and the writer mode by
   * bitwise-or: DirDB::ONOLOCK, which means it opens the database file without file locking,
   * DirDB::OTRYLOCK, which means locking is performed without blocking, DirDB::ONOREPAIR,
   * which means the database file is not repaired implicitly even if file destruction is
   * detected.
   * @return true on success, or false on failure.
   * @note Every opened database must be closed by the DirDB::close method when it is no
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
    writer_ = false;
    autotran_ = false;
    autosync_ = false;
    recov_ = false;
    reorg_ = false;
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
    size_t psiz = path.size();
    while (psiz > 0 && path[psiz-1] == File::PATHCHR) {
      psiz--;
    }
    const std::string& cpath = path.substr(0, psiz);
    const std::string& magicpath = cpath + File::PATHCHR + KCDDBMAGICFILE;
    const std::string& metapath = cpath + File::PATHCHR + KCDDBMETAFILE;
    const std::string& opqpath = cpath + File::PATHCHR + KCDDBOPAQUEFILE;
    const std::string& walpath = cpath + File::EXTCHR + KCDDBWALPATHEXT;
    const std::string& tmppath = cpath + File::EXTCHR + KCDDBTMPPATHEXT;
    bool hot = false;
    if (writer_ && (mode & OTRUNCATE) && File::status(magicpath)) {
      if (!file_.open(magicpath, fmode)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        return false;
      }
      if (!remove_files(cpath)) {
        file_.close();
        return false;
      }
      if (File::status(walpath)) {
        remove_files(walpath);
        File::remove_directory(walpath);
      }
      if (!file_.close()) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        return false;
      }
      const std::string& buf = format_magic(0, 0);
      if (!File::write_file(magicpath, buf.c_str(), buf.size())) {
        set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
        return false;
      }
      if (File::status(metapath) && !File::remove(metapath)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
        return false;
      }
      if (File::status(opqpath) && !File::remove(opqpath)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
        return false;
      }
      hot = true;
    }
    File::Status sbuf;
    if (File::status(cpath, &sbuf)) {
      if (!sbuf.isdir) {
        set_error(_KCCODELINE_, Error::NOPERM, "invalid path (not directory)");
        return false;
      }
      if (!File::status(magicpath)) {
        set_error(_KCCODELINE_, Error::BROKEN, "invalid magic data");
        return false;
      }
      if (!file_.open(magicpath, fmode)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        return false;
      }
    } else if (writer_ && (mode & OCREATE)) {
      hot = true;
      if (!File::make_directory(cpath)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "making a directory failed");
        return false;
      }
      if (!file_.open(magicpath, fmode)) {
        set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
        return false;
      }
    } else {
      set_error(_KCCODELINE_, Error::NOREPOS, "open failed (file not found)");
      return false;
    }
    if (hot) {
      count_ = 0;
      size_ = 0;
      comp_ = (opts_ & TCOMPRESS) ? embcomp_ : NULL;
      libver_ = LIBVER;
      librev_ = LIBREV;
      fmtver_ = FMTVER;
      chksum_ = calc_checksum();
      if (!dump_meta(metapath)) {
        file_.close();
        return false;
      }
      std::memset(opaque_, 0, sizeof(opaque_));
      if (autosync_ && !File::synchronize_whole()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
        file_.close();
        return false;
      }
    } else {
      if (File::status(walpath, &sbuf)) {
        if (writer_) {
          file_.truncate(0);
        } else {
          File::write_file(magicpath, "", 0);
          file_.refresh();
        }
        DirStream dir;
        if (dir.open(walpath)) {
          std::string name;
          while (dir.read(&name)) {
            const std::string& srcpath = walpath + File::PATHCHR + name;
            const std::string& destpath = cpath + File::PATHCHR + name;
            File::Status sbuf;
            if (File::status(srcpath, &sbuf)) {
              if (sbuf.size > 1) {
                File::rename(srcpath, destpath);
              } else {
                if (File::remove(destpath) || !File::status(destpath)) File::remove(srcpath);
              }
            }
          }
          dir.close();
          File::remove_directory(walpath);
          recov_ = true;
          report(_KCCODELINE_, Logger::WARN, "recovered by the WAL directory");
        }
      }
      if (!load_meta(metapath)) {
        file_.close();
        return false;
      }
      comp_ = (opts_ & TCOMPRESS) ? embcomp_ : NULL;
      uint8_t chksum = calc_checksum();
      if (chksum != chksum_) {
        set_error(_KCCODELINE_, Error::INVALID, "invalid module checksum");
        report(_KCCODELINE_, Logger::WARN, "saved=%02X calculated=%02X",
               (unsigned)chksum_, (unsigned)chksum);
        file_.close();
        return false;
      }
      if (!load_magic()) {
        if (!calc_magic(cpath)) {
          file_.close();
          return false;
        }
        reorg_ = true;
        if (!writer_ && !(mode & ONOLOCK)) {
          const std::string& buf = format_magic(count_, size_);
          if (!File::write_file(magicpath, buf.c_str(), buf.size())) {
            set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
            file_.close();
            return false;
          }
          if (!file_.refresh()) {
            set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
            file_.close();
            return false;
          }
        }
        report(_KCCODELINE_, Logger::WARN, "re-calculated magic data");
      }
    }
    if (writer_ && !file_.truncate(0)) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      file_.close();
      return false;
    }
    if (File::status(walpath)) {
      remove_files(walpath);
      File::remove_directory(walpath);
    }
    if (File::status(tmppath)) {
      remove_files(tmppath);
      File::remove_directory(tmppath);
    }
    omode_ = mode;
    path_ = cpath;
    tran_ = false;
    walpath_ = walpath;
    tmppath_ = tmppath;
    load_opaque();
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
    bool err = false;
    if (tran_ && !abort_transaction()) err = true;
    if (!disable_cursors()) err = true;
    if (writer_) {
      if (!dump_magic()) err = true;
      if (!dump_opaque()) err = true;
    }
    if (!file_.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      err = true;
    }
    omode_ = 0;
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
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    rlock_.lock_reader_all();
    bool err = false;
    if (!synchronize_impl(hard, proc, checker)) err = true;
    trigger_meta(MetaTrigger::SYNCHRONIZE, "synchronize");
    rlock_.unlock_all();
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
      mlock_.lock_writer();
      if (omode_ == 0) {
        set_error(_KCCODELINE_, Error::INVALID, "not opened");
        mlock_.unlock();
        return false;
      }
      if (!writer_) {
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
    trhard_ = hard;
    if (!begin_transaction_impl()) {
      mlock_.unlock();
      return false;
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
    if (!writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      mlock_.unlock();
      return false;
    }
    if (tran_) {
      set_error(_KCCODELINE_, Error::LOGIC, "competition avoided");
      mlock_.unlock();
      return false;
    }
    trhard_ = hard;
    if (!begin_transaction_impl()) {
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
    bool err = false;
    if (commit) {
      if (!commit_transaction()) err = true;
    } else {
      if (!abort_transaction()) err = true;
    }
    tran_ = false;
    trigger_meta(commit ? MetaTrigger::COMMITTRAN : MetaTrigger::ABORTTRAN, "end_transaction");
    return !err;
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
    if (!writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    bool err = false;
    if (!disable_cursors()) err = true;
    if (tran_) {
      DirStream dir;
      if (dir.open(path_)) {
        std::string name;
        while (dir.read(&name)) {
          if (*name.c_str() == *KCDDBMAGICFILE) continue;
          const std::string& rpath = path_ + File::PATHCHR + name;
          const std::string& walpath = walpath_ + File::PATHCHR + name;
          if (File::status(walpath)) {
            if (!File::remove(rpath)) {
              set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
              err = true;
            }
          } else if (!File::rename(rpath, walpath)) {
            set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
            err = true;
          }
        }
        if (!dir.close()) {
          set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
          err = true;
        }
      } else {
        set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
        err = true;
      }
    } else {
      if (!remove_files(path_)) err = true;
    }
    recov_ = false;
    reorg_ = false;
    flags_ = 0;
    std::memset(opaque_, 0, sizeof(opaque_));
    count_ = 0;
    size_ = 0;
    trigger_meta(MetaTrigger::CLEAR, "clear");
    return !err;
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
    return count_;
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
    (*strmap)["type"] = strprintf("%u", (unsigned)TYPEDIR);
    (*strmap)["realtype"] = strprintf("%u", (unsigned)type_);
    (*strmap)["path"] = path_;
    (*strmap)["libver"] = strprintf("%u", libver_);
    (*strmap)["librev"] = strprintf("%u", librev_);
    (*strmap)["fmtver"] = strprintf("%u", fmtver_);
    (*strmap)["chksum"] = strprintf("%u", chksum_);
    (*strmap)["flags"] = strprintf("%u", flags_);
    (*strmap)["opts"] = strprintf("%u", opts_);
    (*strmap)["recovered"] = strprintf("%d", recov_);
    (*strmap)["reorganized"] = strprintf("%d", reorg_);
    if (strmap->count("opaque") > 0)
      (*strmap)["opaque"] = std::string(opaque_, sizeof(opaque_));
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
    if (!writer_) {
      set_error(_KCCODELINE_, Error::NOPERM, "permission denied");
      return false;
    }
    bool err = false;
    if (!dump_opaque()) err = true;
    return !err;
  }
  /**
   * Get the status flags.
   * @note This is a dummy implementation for compatibility.
   */
  uint8_t flags() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return 0;
    }
    return 0;
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
    return libver_;
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
    return librev_;
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
    return fmtver_;
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
    return chksum_;
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
    return recov_;
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
    return reorg_;
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
   * Set the number of buckets of the hash table.
   * @note This is a dummy implementation for compatibility.
   */
  bool tune_buckets(int64_t bnum) {
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
    char* rbuf;                          ///< record buffer
    size_t rsiz;                         ///< record size
    const char* kbuf;                    ///< key buffer
    size_t ksiz;                         ///< key size
    const char* vbuf;                    ///< value buffer
    size_t vsiz;                         ///< value size
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
   * Dump the magic data into the file.
   * @return true on success, or false on failure.
   */
  bool dump_magic() {
    _assert_(true);
    const std::string& buf = format_magic(count_, size_);
    if (!file_.write(0, buf.c_str(), buf.size())) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      return false;
    }
    return true;
  }
  /**
   * Format the magic data.
   * @return the result string.
   */
  std::string format_magic(int64_t count, int64_t size) {
    return strprintf("%lld\n%lld\n%s\n", (long long)count, (long long)size, KCDDBMAGICEOF);
  }
  /**
   * Load the magic data from the file.
   * @return true on success, or false on failure.
   */
  bool load_magic() {
    _assert_(true);
    char buf[NUMBUFSIZ*3];
    size_t len = file_.size();
    if (len > sizeof(buf) - 1) len = sizeof(buf) - 1;
    if (!file_.read(0, buf, len)) return false;
    buf[len] = '\0';
    char* rp = buf;
    int64_t count = atoi(rp);
    char* pv = std::strchr(rp, '\n');
    if (!pv) return false;
    rp = pv + 1;
    int64_t size = atoi(rp);
    pv = std::strchr(rp, '\n');
    if (!pv) return false;
    rp = pv + 1;
    if (std::strlen(rp) < sizeof(KCDDBMAGICEOF) - 1 ||
        std::memcmp(rp, KCDDBMAGICEOF, sizeof(KCDDBMAGICEOF) - 1)) return false;
    flags_ = 0;
    count_ = count;
    size_ = size;
    return true;
  }
  /**
   * Calculate magic data.
   * @param cpath the path of the database file.
   * @return true on success, or false on failure.
   */
  bool calc_magic(const std::string& cpath) {
    _assert_(true);
    count_ = 0;
    size_ = 0;
    DirStream dir;
    if (!dir.open(cpath)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
      return false;
    }
    bool err = false;
    std::string name;
    while (dir.read(&name)) {
      if (*name.c_str() == *KCDDBMAGICFILE) continue;
      const std::string& rpath = cpath + File::PATHCHR + name;
      File::Status sbuf;
      if (File::status(rpath, &sbuf)) {
        if (sbuf.size >= 4) {
          count_ += 1;
          size_ += sbuf.size - 4;
        } else {
          File::remove(rpath);
        }
      } else {
        set_error(_KCCODELINE_, Error::SYSTEM, "checking the status of a file failed");
        err = true;
      }
    }
    if (!dir.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
      err = true;
    }
    return !err;
  }
  /**
   * Calculate the module checksum.
   * @return the module checksum.
   */
  uint8_t calc_checksum() {
    _assert_(true);
    const char* kbuf = KCDDBCHKSUMSEED;
    size_t ksiz = sizeof(KCDDBCHKSUMSEED) - 1;
    char* zbuf = NULL;
    size_t zsiz = 0;
    if (comp_) {
      zbuf = comp_->compress(kbuf, ksiz, &zsiz);
      if (!zbuf) return 0;
      kbuf = zbuf;
      ksiz = zsiz;
    }
    char name[NUMBUFSIZ];
    uint32_t hash = hashpath(kbuf, ksiz, name);
    hash += hashmurmur(name, std::strlen(name));
    delete[] zbuf;
    return hash;
  }
  /**
   * Dump the meta data into the file.
   * @param metapath the path of the meta data file.
   * @return true on success, or false on failure.
   */
  bool dump_meta(const std::string& metapath) {
    _assert_(true);
    bool err = false;
    char buf[METABUFSIZ];
    char* wp = buf;
    wp += std::sprintf(wp, "%u\n", libver_);
    wp += std::sprintf(wp, "%u\n", librev_);
    wp += std::sprintf(wp, "%u\n", fmtver_);
    wp += std::sprintf(wp, "%u\n", chksum_);
    wp += std::sprintf(wp, "%u\n", type_);
    wp += std::sprintf(wp, "%u\n", opts_);
    wp += std::sprintf(wp, "%s\n", KCDDBMAGICEOF);
    if (!File::write_file(metapath, buf, wp - buf)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
      err = true;
    }
    return !err;
  }
  /**
   * Load the meta data from the file.
   * @param metapath the path of the meta data file.
   * @return true on success, or false on failure.
   */
  bool load_meta(const std::string& metapath) {
    _assert_(true);
    int64_t size;
    char* buf = File::read_file(metapath, &size, METABUFSIZ);
    if (!buf) {
      set_error(_KCCODELINE_, Error::SYSTEM, "reading a file failed");
      return false;
    }
    std::string str(buf, size);
    delete[] buf;
    std::vector<std::string> elems;
    if (strsplit(str, '\n', &elems) < 7 || elems[6] != KCDDBMAGICEOF) {
      set_error(_KCCODELINE_, Error::BROKEN, "invalid meta data file");
      return false;
    }
    libver_ = atoi(elems[0].c_str());
    librev_ = atoi(elems[1].c_str());
    fmtver_ = atoi(elems[2].c_str());
    chksum_ = atoi(elems[3].c_str());
    type_ = atoi(elems[4].c_str());
    opts_ = atoi(elems[5].c_str());
    return true;
  }
  /**
   * Dump the opaque data into the file.
   * @return true on success, or false on failure.
   */
  bool dump_opaque() {
    _assert_(true);
    bool err = false;
    const std::string& opath = path_ + File::PATHCHR + KCDDBOPAQUEFILE;
    if (!File::write_file(opath, opaque_, sizeof(opaque_))) {
      set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
      err = true;
    }
    return !err;
  }
  /**
   * Load the opaque data from the file.
   * @return true on success, or false on failure.
   */
  void load_opaque() {
    _assert_(true);
    std::memset(opaque_, 0, sizeof(opaque_));
    const std::string& opath = path_ + File::PATHCHR + KCDDBOPAQUEFILE;
    int64_t size;
    char* buf = File::read_file(opath, &size, sizeof(opaque_));
    if (buf) {
      std::memcpy(opaque_, buf, size);
      delete[] buf;
    }
  }
  /**
   * Remove inner files.
   * @param cpath the path of the database file.
   * @return true on success, or false on failure.
   */
  bool remove_files(const std::string& cpath) {
    _assert_(true);
    DirStream dir;
    if (!dir.open(cpath)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
      return false;
    }
    bool err = false;
    std::string name;
    while (dir.read(&name)) {
      if (*name.c_str() == *KCDDBMAGICFILE) continue;
      const std::string& rpath = cpath + File::PATHCHR + name;
      if (!File::remove(rpath)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
        err = true;
      }
    }
    if (!dir.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
      err = true;
    }
    return !err;
  }
  /**
   * Read a record.
   * @param rpath the path of the record.
   * @param rec the record structure.
   * @return true on success, or false on failure.
   */
  bool read_record(const std::string& rpath, Record* rec) {
    _assert_(rec);
    int64_t rsiz;
    char* rbuf = File::read_file(rpath, &rsiz);
    if (!rbuf) return false;
    rec->rsiz = rsiz;
    if (comp_) {
      size_t zsiz;
      char* zbuf = comp_->decompress(rbuf, rsiz, &zsiz);
      if (!zbuf) {
        set_error(_KCCODELINE_, Error::SYSTEM, "data decompression failed");
        delete[] rbuf;
        return false;
      }
      delete[] rbuf;
      rbuf = zbuf;
      rsiz = zsiz;
    }
    const char* rp = rbuf;
    if (rsiz < 4 || *(const unsigned char*)rp != RECMAGIC) {
      set_error(_KCCODELINE_, Error::BROKEN, "invalid magic data of a record");
      report(_KCCODELINE_, Logger::WARN, "rpath=%s", rpath.c_str());
      report_binary(_KCCODELINE_, Logger::WARN, "rbuf", rbuf, rsiz);
      delete[] rbuf;
      return false;
    }
    rp++;
    uint64_t num;
    size_t step = readvarnum(rp, rsiz, &num);
    rp += step;
    rsiz -= step;
    size_t ksiz = num;
    if (rsiz < 2) {
      report(_KCCODELINE_, Logger::WARN, "rpath=%s", rpath.c_str());
      delete[] rbuf;
      return false;
    }
    step = readvarnum(rp, rsiz, &num);
    rp += step;
    rsiz -= step;
    size_t vsiz = num;
    if (rsiz < 1 + (int64_t)ksiz + (int64_t)vsiz ||
        ((const unsigned char*)rp)[ksiz+vsiz] != RECMAGIC) {
      set_error(_KCCODELINE_, Error::BROKEN, "too short record");
      report(_KCCODELINE_, Logger::WARN, "rpath=%s", rpath.c_str());
      delete[] rbuf;
      return false;
    }
    rec->rbuf = rbuf;
    rec->kbuf = rp;
    rec->ksiz = ksiz;
    rec->vbuf = rp + ksiz;
    rec->vsiz = vsiz;
    return true;
  }
  /**
   * Write a record.
   * @param rpath the path of the record.
   * @param name the file name of the record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @param wsp the pointer to the variable into which the size of the written record is
   * assigned.
   * @return true on success, or false on failure.
   */
  bool write_record(const std::string& rpath, const char* name, const char* kbuf, size_t ksiz,
                    const char* vbuf, size_t vsiz, size_t* wsp) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ && wsp);
    bool err = false;
    char* rbuf = new char[NUMBUFSIZ*2+ksiz+vsiz];
    char* wp = rbuf;
    *(wp++) = RECMAGIC;
    wp += writevarnum(wp, ksiz);
    wp += writevarnum(wp, vsiz);
    std::memcpy(wp, kbuf, ksiz);
    wp += ksiz;
    std::memcpy(wp, vbuf, vsiz);
    wp += vsiz;
    *(wp++) = RECMAGIC;
    size_t rsiz = wp - rbuf;
    if (comp_) {
      size_t zsiz;
      char* zbuf = comp_->compress(rbuf, rsiz, &zsiz);
      if (!zbuf) {
        set_error(_KCCODELINE_, Error::SYSTEM, "data compression failed");
        delete[] rbuf;
        *wsp = 0;
        return false;
      }
      delete[] rbuf;
      rbuf = zbuf;
      rsiz = zsiz;
    }
    if (autotran_ && !tran_) {
      const std::string& tpath = path_ + File::PATHCHR + KCDDBATRANPREFIX + name;
      if (!File::write_file(tpath, rbuf, rsiz)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
        err = true;
      }
      if (!File::rename(tpath, rpath)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
        err = true;
        File::remove(tpath);
      }
    } else {
      if (!File::write_file(rpath, rbuf, rsiz)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "writing a file failed");
        err = true;
      }
    }
    delete[] rbuf;
    *wsp = rsiz;
    return !err;
  }
  /**
   * Disable all cursors.
   * @return true on success, or false on failure.
   */
  bool disable_cursors() {
    _assert_(true);
    if (curs_.empty()) return true;
    bool err = false;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->alive_ && !cur->disable()) err = true;
      ++cit;
    }
    return !err;
  }
  /**
   * Escape cursors on a free block.
   * @param rpath the file path of the record.
   * @param name the file name of the record.
   * @return true on success, or false on failure.
   */
  bool escape_cursors(const std::string& rpath, const char* name) {
    bool err = false;
    if (curs_.empty()) return true;
    CursorList::const_iterator cit = curs_.begin();
    CursorList::const_iterator citend = curs_.end();
    while (cit != citend) {
      Cursor* cur = *cit;
      if (cur->alive_ && cur->name_ == name) {
        do {
          if (!cur->dir_.read(&cur->name_)) {
            if (!cur->disable()) err = true;
            break;
          }
        } while (*cur->name_.c_str() == *KCDDBMAGICFILE);
      }
      ++cit;
    }
    return !err;
  }
  /**
   * Accept a visitor to a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param visitor a visitor object.
   * @param name the encoded key.
   * @return true on success, or false on failure.
   */
  bool accept_impl(const char* kbuf, size_t ksiz, Visitor* visitor, const char* name) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor && name);
    bool err = false;
    const std::string& rpath = path_ + File::PATHCHR + name;
    Record rec;
    if (read_record(rpath, &rec)) {
      if (rec.ksiz == ksiz || !std::memcmp(rec.kbuf, kbuf, ksiz)) {
        if (!accept_visit_full(kbuf, ksiz, rec.vbuf, rec.vsiz, rec.rsiz,
                               visitor, rpath, name)) err = true;
      } else {
        set_error(_KCCODELINE_, Error::LOGIC, "collision of the hash values");
        err = true;
      }
      delete[] rec.rbuf;
    } else {
      if (!accept_visit_empty(kbuf, ksiz, visitor, rpath, name)) err = true;
    }
    return !err;
  }
  /**
   * Accept the visit_full method.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @param osiz the old size of the record.
   * @param visitor a visitor object.
   * @param rpath the file path of the record.
   * @param name the file name of the record.
   * @return true on success, or false on failure.
   */
  bool accept_visit_full(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz,
                         size_t osiz, Visitor *visitor, const std::string& rpath,
                         const char* name) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ && visitor);
    bool err = false;
    size_t rsiz;
    const char* rbuf = visitor->visit_full(kbuf, ksiz, vbuf, vsiz, &rsiz);
    if (rbuf == Visitor::REMOVE) {
      if (tran_) {
        const std::string& walpath = walpath_ + File::PATHCHR + name;
        if (File::status(walpath)) {
          if (!File::remove(rpath)) {
            set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
            err = true;
          }
        } else if (!File::rename(rpath, walpath)) {
          set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
          err = true;
        }
      } else {
        if (!File::remove(rpath)) {
          set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
          err = true;
        }
      }
      if (!escape_cursors(rpath, name)) err = true;
      count_ -= 1;
      size_ -= osiz;
      if (autosync_ && !File::synchronize_whole()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
        err = true;
      }
    } else if (rbuf != Visitor::NOP) {
      if (tran_) {
        const std::string& walpath = walpath_ + File::PATHCHR + name;
        if (!File::status(walpath) && !File::rename(rpath, walpath)) {
          set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
          err = true;
        }
      }
      size_t wsiz;
      if (!write_record(rpath, name, kbuf, ksiz, rbuf, rsiz, &wsiz)) err = true;
      size_ += (int64_t)wsiz - (int64_t)osiz;
      if (autosync_ && !File::synchronize_whole()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
        err = true;
      }
    }
    return !err;
  }
  /**
   * Accept the visit_empty method.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param visitor a visitor object.
   * @param rpath the file path of the record.
   * @param name the file name of the record.
   * @return true on success, or false on failure.
   */
  bool accept_visit_empty(const char* kbuf, size_t ksiz,
                          Visitor *visitor, const std::string& rpath, const char* name) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && visitor);
    bool err = false;
    size_t rsiz;
    const char* rbuf = visitor->visit_empty(kbuf, ksiz, &rsiz);
    if (rbuf != Visitor::NOP && rbuf != Visitor::REMOVE) {
      if (tran_) {
        const std::string& walpath = walpath_ + File::PATHCHR + name;
        if (!File::status(walpath) && !File::write_file(walpath, "", 0)) {
          set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
          err = true;
        }
      }
      size_t wsiz;
      if (!write_record(rpath, name, kbuf, ksiz, rbuf, rsiz, &wsiz)) err = true;
      count_ += 1;
      size_ += wsiz;
      if (autosync_ && !File::synchronize_whole()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
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
    int64_t allcnt = count_;
    if (checker && !checker->check("iterate", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    DirStream dir;
    if (!dir.open(path_)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
      return false;
    }
    bool err = false;
    std::string name;
    int64_t curcnt = 0;
    while (dir.read(&name)) {
      if (*name.c_str() == *KCDDBMAGICFILE) continue;
      const std::string& rpath = path_ + File::PATHCHR + name;
      Record rec;
      if (read_record(rpath, &rec)) {
        if (!accept_visit_full(rec.kbuf, rec.ksiz, rec.vbuf, rec.vsiz, rec.rsiz,
                               visitor, rpath, name.c_str())) err = true;
        delete[] rec.rbuf;
      } else {
        set_error(_KCCODELINE_, Error::BROKEN, "missing record");
        err = true;
      }
      curcnt++;
      if (checker && !checker->check("iterate", "processing", curcnt, allcnt)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        err = true;
        break;
      }
    }
    if (!dir.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
      err = true;
    }
    if (checker && !checker->check("iterate", "ending", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    return !err;
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
    int64_t allcnt = count_;
    if (checker && !checker->check("scan_parallel", "beginning", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      return false;
    }
    DirStream dir;
    if (!dir.open(path_)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
      return false;
    }
    class ThreadImpl : public Thread {
     public:
      explicit ThreadImpl() :
          db_(NULL), visitor_(NULL), checker_(NULL), allcnt_(0),
          dir_(NULL), itmtx_(NULL), error_() {}
      void init(DirDB* db, Visitor* visitor, ProgressChecker* checker, int64_t allcnt,
                DirStream* dir, Mutex* itmtx) {
        db_ = db;
        visitor_ = visitor;
        checker_ = checker;
        allcnt_ = allcnt;
        dir_ = dir;
        itmtx_ = itmtx;
      }
      const Error& error() {
        return error_;
      }
     private:
      void run() {
        DirDB* db = db_;
        Visitor* visitor = visitor_;
        ProgressChecker* checker = checker_;
        int64_t allcnt = allcnt_;
        DirStream* dir = dir_;
        Mutex* itmtx = itmtx_;
        const std::string& path = db->path_;
        while (true) {
          itmtx->lock();
          std::string name;
          if (!dir->read(&name)) {
            itmtx->unlock();
            break;
          }
          itmtx->unlock();
          if (*name.c_str() == *KCDDBMAGICFILE) continue;
          const std::string& rpath = path + File::PATHCHR + name;
          Record rec;
          if (db->read_record(rpath, &rec)) {
            size_t vsiz;
            visitor->visit_full(rec.kbuf, rec.ksiz, rec.vbuf, rec.vsiz, &vsiz);
            delete[] rec.rbuf;
          } else {
            error_ = db->error();
            break;
          }
          if (checker && !checker->check("scan_parallel", "processing", -1, allcnt)) {
            db->set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
            error_ = db->error();
            break;
          }
        }
      }
      DirDB* db_;
      Visitor* visitor_;
      ProgressChecker* checker_;
      int64_t allcnt_;
      DirStream* dir_;
      Mutex* itmtx_;
      Error error_;
    };
    bool err = false;
    Mutex itmtx;
    ThreadImpl* threads = new ThreadImpl[thnum];
    for (size_t i = 0; i < thnum; i++) {
      ThreadImpl* thread = threads + i;
      thread->init(this, visitor, checker, allcnt, &dir, &itmtx);
    }
    for (size_t i = 0; i < thnum; i++) {
      ThreadImpl* thread = threads + i;
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
    if (!dir.close()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
      err = true;
    }
    if (checker && !checker->check("scan_parallel", "ending", -1, allcnt)) {
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
      if (checker && !checker->check("synchronize", "dumping the magic data", -1, -1)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        return false;
      }
      if (!dump_magic()) err = true;
      if (checker && !checker->check("synchronize", "synchronizing the directory", -1, -1)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        return false;
      }
      if (hard && !File::synchronize_whole()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
        err = true;
      }
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
    if (writer_ && !file_.truncate(0)) {
      set_error(_KCCODELINE_, Error::SYSTEM, file_.error());
      err = true;
    }
    return !err;
  }
  /**
   * Begin transaction.
   * @return true on success, or false on failure.
   */
  bool begin_transaction_impl() {
    _assert_(true);
    if (!File::make_directory(walpath_)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "making a directory failed");
      return false;
    }
    if (trhard_ && !File::synchronize_whole()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
      return false;
    }
    trcount_ = count_;
    trsize_ = size_;
    return true;
  }
  /**
   * Commit transaction.
   * @return true on success, or false on failure.
   */
  bool commit_transaction() {
    _assert_(true);
    bool err = false;
    if (!File::rename(walpath_, tmppath_)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "renaming a directory failed");
      err = true;
    }
    if (!remove_files(tmppath_)) err = true;
    if (!File::remove_directory(tmppath_)) {
      set_error(_KCCODELINE_, Error::SYSTEM, "removing a directory failed");
      return false;
    }
    if (trhard_ && !File::synchronize_whole()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
      err = true;
    }
    return !err;
  }
  /**
   * Abort transaction.
   * @return true on success, or false on failure.
   */
  bool abort_transaction() {
    _assert_(true);
    bool err = false;
    if (!disable_cursors()) err = true;
    DirStream dir;
    if (dir.open(walpath_)) {
      std::string name;
      while (dir.read(&name)) {
        const std::string& srcpath = walpath_ + File::PATHCHR + name;
        const std::string& destpath = path_ + File::PATHCHR + name;
        File::Status sbuf;
        if (File::status(srcpath, &sbuf)) {
          if (sbuf.size > 1) {
            if (!File::rename(srcpath, destpath)) {
              set_error(_KCCODELINE_, Error::SYSTEM, "renaming a file failed");
              err = true;
            }
          } else {
            if (File::remove(destpath) || !File::status(destpath)) {
              if (!File::remove(srcpath)) {
                set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
                err = true;
              }
            } else {
              set_error(_KCCODELINE_, Error::SYSTEM, "removing a file failed");
              err = true;
            }
          }
        } else {
          set_error(_KCCODELINE_, Error::SYSTEM, "checking a file failed");
          err = true;
        }
      }
      if (!dir.close()) {
        set_error(_KCCODELINE_, Error::SYSTEM, "closing a directory failed");
        err = true;
      }
      if (!File::remove_directory(walpath_)) {
        set_error(_KCCODELINE_, Error::SYSTEM, "removing a directory failed");
        err = true;
      }
    } else {
      set_error(_KCCODELINE_, Error::SYSTEM, "opening a directory failed");
      err = true;
    }
    count_ = trcount_;
    size_ = trsize_;
    if (trhard_ && !File::synchronize_whole()) {
      set_error(_KCCODELINE_, Error::SYSTEM, "synchronizing the file system failed");
      err = true;
    }
    return !err;
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes.
   */
  int64_t size_impl() {
    return size_ + count_ * RECUNITSIZ;
  }
  /** Dummy constructor to forbid the use. */
  DirDB(const DirDB&);
  /** Dummy Operator to forbid the use. */
  DirDB& operator =(const DirDB&);
  /** The method lock. */
  RWLock mlock_;
  /** The record locks. */
  SlottedRWLock rlock_;
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
  /** The flag for recovered. */
  bool recov_;
  /** The flag for reorganized. */
  bool reorg_;
  /** The file for magic data. */
  File file_;
  /** The cursor objects. */
  CursorList curs_;
  /** The path of the database file. */
  std::string path_;
  /** The library version. */
  uint8_t libver_;
  /** The library revision. */
  uint8_t librev_;
  /** The format revision. */
  uint8_t fmtver_;
  /** The module checksum. */
  uint8_t chksum_;
  /** The database type. */
  uint8_t type_;
  /** The status flags. */
  uint8_t flags_;
  /** The options. */
  uint8_t opts_;
  /** The record number. */
  AtomicInt64 count_;
  /** The total size of records. */
  AtomicInt64 size_;
  /** The opaque data. */
  char opaque_[OPAQUESIZ];
  /** The embedded data compressor. */
  Compressor* embcomp_;
  /** The data compressor. */
  Compressor* comp_;
  /** The compression checksum. */
  bool tran_;
  /** The flag whether hard transaction. */
  bool trhard_;
  /** The old count before transaction. */
  int64_t trcount_;
  /** The old size before transaction. */
  int64_t trsize_;
  /** The WAL directory for transaction. */
  std::string walpath_;
  /** The temporary directory. */
  std::string tmppath_;
};


/** An alias of the directory tree database. */
typedef PlantDB<DirDB, BasicDB::TYPEFOREST> ForestDB;


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
