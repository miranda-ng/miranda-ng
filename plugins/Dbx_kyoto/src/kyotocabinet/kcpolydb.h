/*************************************************************************************************
 * Polymorphic database
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


#ifndef _KCPOLYDB_H                      // duplication check
#define _KCPOLYDB_H

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
#include <kcprotodb.h>
#include <kcstashdb.h>
#include <kccachedb.h>
#include <kchashdb.h>
#include <kcdirdb.h>
#include <kctextdb.h>

namespace kyotocabinet {                 // common namespace


/**
 * Polymorphic database.
 * @note This class is a concrete class to operate an arbitrary database whose type is determined
 * in runtime.  This class can be inherited but overwriting methods is forbidden.  Before every
 * database operation, it is necessary to call the PolyDB::open method in order to open a
 * database file and connect the database object to it.  To avoid data missing or corruption, it
 * is important to close every database file by the PolyDB::close method when the database is no
 * longer in use.  It is forbidden for multible database objects in a process to open the same
 * database at the same time.  It is forbidden to share a database object with child processes.
 */
class PolyDB : public BasicDB {
 public:
  class Cursor;
 private:
  class StreamLogger;
  class StreamMetaTrigger;
  struct SimilarKey;
  struct MergeLine;
 public:
  /**
   * Cursor to indicate a record.
   */
  class Cursor : public BasicDB::Cursor {
    friend class PolyDB;
   public:
    /**
     * Constructor.
     * @param db the container database object.
     */
    explicit Cursor(PolyDB* db) : db_(db), cur_(NULL) {
      _assert_(db);
      if (db_->type_ == TYPEVOID) {
        ProtoHashDB tmpdb;
        cur_ = tmpdb.cursor();
      } else {
        cur_ = db->db_->cursor();
      }
    }
    /**
     * Destructor.
     */
    virtual ~Cursor() {
      _assert_(true);
      delete cur_;
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
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->accept(visitor, writable, step);
    }
    /**
     * Jump the cursor to the first record for forward scan.
     * @return true on success, or false on failure.
     */
    bool jump() {
      _assert_(true);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->jump();
    }
    /**
     * Jump the cursor to a record for forward scan.
     * @param kbuf the pointer to the key region.
     * @param ksiz the size of the key region.
     * @return true on success, or false on failure.
     */
    bool jump(const char* kbuf, size_t ksiz) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->jump(kbuf, ksiz);
    }
    /**
     * Jump the cursor to a record for forward scan.
     * @note Equal to the original Cursor::jump method except that the parameter is std::string.
     */
    bool jump(const std::string& key) {
      _assert_(true);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return jump(key.c_str(), key.size());
    }
    /**
     * Jump the cursor to the last record for backward scan.
     * @return true on success, or false on failure.
     * @note This method is dedicated to tree databases.  Some database types, especially hash
     * databases, may provide a dummy implementation.
     */
    bool jump_back() {
      _assert_(true);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->jump_back();
    }
    /**
     * Jump the cursor to a record for backward scan.
     * @param kbuf the pointer to the key region.
     * @param ksiz the size of the key region.
     * @return true on success, or false on failure.
     * @note This method is dedicated to tree databases.  Some database types, especially hash
     * databases, will provide a dummy implementation.
     */
    bool jump_back(const char* kbuf, size_t ksiz) {
      _assert_(kbuf && ksiz <= MEMMAXSIZ);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->jump_back(kbuf, ksiz);
    }
    /**
     * Jump the cursor to a record for backward scan.
     * @note Equal to the original Cursor::jump_back method except that the parameter is
     * std::string.
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
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->step();
    }
    /**
     * Step the cursor to the previous record.
     * @return true on success, or false on failure.
     * @note This method is dedicated to tree databases.  Some database types, especially hash
     * databases, may provide a dummy implementation.
     */
    bool step_back() {
      _assert_(true);
      if (db_->type_ == TYPEVOID) {
        db_->set_error(_KCCODELINE_, Error::INVALID, "not opened");
        return false;
      }
      return cur_->step_back();
    }
    /**
     * Get the database object.
     * @return the database object.
     */
    PolyDB* db() {
      _assert_(true);
      return db_;
    }
   private:
    /** Dummy constructor to forbid the use. */
    Cursor(const Cursor&);
    /** Dummy Operator to forbid the use. */
    Cursor& operator =(const Cursor&);
    /** The inner database. */
    PolyDB* db_;
    /** The inner cursor. */
    BasicDB::Cursor* cur_;
  };
  /**
   * Merge modes.
   */
  enum MergeMode {
    MSET,                                ///< overwrite the existing value
    MADD,                                ///< keep the existing value
    MREPLACE,                            ///< modify the existing record only
    MAPPEND                              ///< append the new value
  };
  /**
   * Default constructor.
   */
  explicit PolyDB() :
      type_(TYPEVOID), db_(NULL), error_(),
      stdlogstrm_(NULL), stdlogger_(NULL), logger_(NULL), logkinds_(0),
      stdmtrgstrm_(NULL), stdmtrigger_(NULL), mtrigger_(NULL), zcomp_(NULL) {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  virtual ~PolyDB() {
    _assert_(true);
    if (type_ != TYPEVOID) close();
    delete zcomp_;
    delete stdmtrigger_;
    delete stdmtrgstrm_;
    delete stdlogger_;
    delete stdlogstrm_;
  }
  /**
   * Set the internal database object.
   * @param db the internal database object.  Its possession is transferred inside and the
   * object is deleted automatically.
   * @return true on success, or false on failure.
   */
  bool set_internal_db(BasicDB* db) {
    _assert_(db);
    if (type_ != TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    type_ = TYPEMISC;
    db_ = db;
    return true;
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->accept(kbuf, ksiz, visitor, writable);
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->accept_bulk(keys, visitor, writable);
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->iterate(visitor, writable, checker);
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->scan_parallel(visitor, thnum, checker);
  }
  /**
   * Get the last happened error.
   * @return the last happened error.
   */
  Error error() const {
    _assert_(true);
    if (type_ == TYPEVOID) return error_;
    return db_->error();
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
    if (type_ == TYPEVOID) {
      error_.set(code, message);
      return;
    }
    db_->set_error(file, line, func, code, message);
  }
  /**
   * Set the error information without source code information.
   * @param code an error code.
   * @param message a supplement message.
   */
  void set_error(Error::Code code, const char* message) {
    _assert_(message);
    if (type_ == TYPEVOID) {
      error_.set(code, message);
      return;
    }
    db_->set_error(_KCCODELINE_, code, message);
  }
  /**
   * Open a database file.
   * @param path the path of a database file.  If it is "-", the database will be a prototype
   * hash database.  If it is "+", the database will be a prototype tree database.  If it is ":",
   * the database will be a stash database.  If it is "*", the database will be a cache hash
   * database.  If it is "%", the database will be a cache tree database.  If its suffix is
   * ".kch", the database will be a file hash database.  If its suffix is ".kct", the database
   * will be a file tree database.  If its suffix is ".kcd", the database will be a directory
   * hash database.  If its suffix is ".kcf", the database will be a directory tree database.
   * If its suffix is ".kcx", the database will be a plain text database.  Otherwise, this
   * function fails.  Tuning parameters can trail the name, separated by "#".  Each parameter is
   * composed of the name and the value, separated by "=".  If the "type" parameter is specified,
   * the database type is determined by the value in "-", "+", ":", "*", "%", "kch", "kct",
   * "kcd", kcf", and "kcx".  All database types support the logging parameters of "log",
   * "logkinds", and "logpx".  The prototype hash database and the prototype tree database do
   * not support any other tuning parameter.  The stash database supports "bnum".  The cache
   * hash database supports "opts", "bnum", "zcomp", "capcnt", "capsiz", and "zkey".  The cache
   * tree database supports all parameters of the cache hash database except for capacity
   * limitation, and supports "psiz", "rcomp", "pccap" in addition.  The file hash database
   * supports "apow", "fpow", "opts", "bnum", "msiz", "dfunit", "zcomp", and "zkey".  The file
   * tree database supports all parameters of the file hash database and "psiz", "rcomp",
   * "pccap" in addition.  The directory hash database supports "opts", "zcomp", and "zkey".
   * The directory tree database supports all parameters of the directory hash database and
   * "psiz", "rcomp", "pccap" in addition.  The plain text database does not support any other
   * tuning parameter.
   * @param mode the connection mode.  PolyDB::OWRITER as a writer, PolyDB::OREADER as a
   * reader.  The following may be added to the writer mode by bitwise-or: PolyDB::OCREATE,
   * which means it creates a new database if the file does not exist, PolyDB::OTRUNCATE, which
   * means it creates a new database regardless if the file exists, PolyDB::OAUTOTRAN, which
   * means each updating operation is performed in implicit transaction, PolyDB::OAUTOSYNC,
   * which means each updating operation is followed by implicit synchronization with the file
   * system.  The following may be added to both of the reader mode and the writer mode by
   * bitwise-or: PolyDB::ONOLOCK, which means it opens the database file without file locking,
   * PolyDB::OTRYLOCK, which means locking is performed without blocking, PolyDB::ONOREPAIR,
   * which means the database file is not repaired implicitly even if file destruction is
   * detected.
   * @return true on success, or false on failure.
   * @note The tuning parameter "log" is for the original "tune_logger" and the value specifies
   * the path of the log file, or "-" for the standard output, or "+" for the standard error.
   * "logkinds" specifies kinds of logged messages and the value can be "debug", "info", "warn",
   * or "error".  "logpx" specifies the prefix of each log message.  "opts" is for "tune_options"
   * and the value can contain "s" for the small option, "l" for the linear option, and "c" for
   * the compress option.  "bnum" corresponds to "tune_bucket".  "zcomp" is for "tune_compressor"
   * and the value can be "zlib" for the ZLIB raw compressor, "def" for the ZLIB deflate
   * compressor, "gz" for the ZLIB gzip compressor, "lzo" for the LZO compressor, "lzma" for the
   * LZMA compressor, or "arc" for the Arcfour cipher.  "zkey" specifies the cipher key of the
   * compressor.  "capcnt" is for "cap_count".  "capsiz" is for "cap_size".  "psiz" is for
   * "tune_page".  "rcomp" is for "tune_comparator" and the value can be "lex" for the lexical
   * comparator, "dec" for the decimal comparator, "lexdesc" for the lexical descending
   * comparator, or "decdesc" for the decimal descending comparator.  "pccap" is for
   * "tune_page_cache".  "apow" is for "tune_alignment".  "fpow" is for "tune_fbp".  "msiz" is
   * for "tune_map".  "dfunit" is for "tune_defrag".  Every opened database must be closed by
   * the PolyDB::close method when it is no longer in use.  It is not allowed for two or more
   * database objects in the same process to keep their connections to the same database file at
   * the same time.
   */
  bool open(const std::string& path = ":", uint32_t mode = OWRITER | OCREATE) {
    _assert_(true);
    if (type_ == TYPEMISC) {
      if (logger_) db_->tune_logger(logger_, logkinds_);
      if (mtrigger_) db_->tune_meta_trigger(mtrigger_);
      return db_->open(path, mode);
    }
    if (type_ != TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    std::vector<std::string> elems;
    strsplit(path, '#', &elems);
    std::string fpath;
    Type type = TYPEVOID;
    std::string logname = "";
    std::string logpx = "";
    uint32_t logkinds = Logger::WARN | Logger::ERROR;
    std::string mtrgname = "";
    std::string mtrgpx = "";
    int64_t bnum = -1;
    int64_t capcnt = -1;
    int64_t capsiz = -1;
    int32_t apow = -1;
    int32_t fpow = -1;
    bool tsmall = false;
    bool tlinear = false;
    bool tcompress = false;
    int64_t msiz = -1;
    int64_t dfunit = -1;
    std::string zcompname = "";
    int64_t psiz = -1;
    Comparator* rcomp = NULL;
    int64_t pccap = 0;
    std::string zkey = "";
    std::vector<std::string>::iterator it = elems.begin();
    std::vector<std::string>::iterator itend = elems.end();
    if (it != itend) {
      fpath = *it;
      ++it;
    }
    const char* fstr = fpath.c_str();
    const char* pv = std::strrchr(fstr, File::PATHCHR);
    if (pv) fstr = pv + 1;
    if (!std::strcmp(fstr, "-")) {
      type = TYPEPHASH;
    } else if (!std::strcmp(fstr, "+")) {
      type = TYPEPTREE;
    } else if (!std::strcmp(fstr, ":")) {
      type = TYPESTASH;
    } else if (!std::strcmp(fstr, "*")) {
      type = TYPECACHE;
    } else if (!std::strcmp(fstr, "%")) {
      type = TYPEGRASS;
    } else {
      pv = std::strrchr(fstr, File::EXTCHR);
      if (pv) {
        pv++;
        if (!std::strcmp(pv, "kcph") || !std::strcmp(pv, "phdb")) {
          type = TYPEPHASH;
        } else if (!std::strcmp(pv, "kcpt") || !std::strcmp(pv, "ptdb")) {
          type = TYPEPTREE;
        } else if (!std::strcmp(pv, "kcs") || !std::strcmp(pv, "sdb")) {
          type = TYPESTASH;
        } else if (!std::strcmp(pv, "kcc") || !std::strcmp(pv, "cdb")) {
          type = TYPECACHE;
        } else if (!std::strcmp(pv, "kcg") || !std::strcmp(pv, "gdb")) {
          type = TYPEGRASS;
        } else if (!std::strcmp(pv, "kch") || !std::strcmp(pv, "hdb")) {
          type = TYPEHASH;
        } else if (!std::strcmp(pv, "kct") || !std::strcmp(pv, "tdb")) {
          type = TYPETREE;
        } else if (!std::strcmp(pv, "kcd") || !std::strcmp(pv, "ddb")) {
          type = TYPEDIR;
        } else if (!std::strcmp(pv, "kcf") || !std::strcmp(pv, "fdb")) {
          type = TYPEFOREST;
        } else if (!std::strcmp(pv, "kcx") || !std::strcmp(pv, "xdb") ||
                   !std::strcmp(pv, "txt") || !std::strcmp(pv, "text") ||
                   !std::strcmp(pv, "tsv") || !std::strcmp(pv, "csv")) {
          type = TYPETEXT;
        }
      }
    }
    while (it != itend) {
      std::vector<std::string> fields;
      if (strsplit(*it, '=', &fields) > 1) {
        const char* key = fields[0].c_str();
        const char* value = fields[1].c_str();
        if (!std::strcmp(key, "type")) {
          if (!std::strcmp(value, "-") || !std::strcmp(value, "kcph") ||
              !std::strcmp(value, "phdb") || !std::strcmp(value, "phash")) {
            type = TYPEPHASH;
          } else if (!std::strcmp(value, "+") || !std::strcmp(value, "kcpt") ||
                     !std::strcmp(value, "ptdb") || !std::strcmp(value, "ptree")) {
            type = TYPEPTREE;
          } else if (!std::strcmp(value, ":") || !std::strcmp(value, "kcs") ||
                     !std::strcmp(value, "sdb") || !std::strcmp(value, "stash")) {
            type = TYPESTASH;
          } else if (!std::strcmp(value, "*") || !std::strcmp(value, "kcc") ||
                     !std::strcmp(value, "cdb") || !std::strcmp(value, "cache")) {
            type = TYPECACHE;
          } else if (!std::strcmp(value, "%") || !std::strcmp(value, "kcg") ||
                     !std::strcmp(value, "gdb") || !std::strcmp(value, "grass")) {
            type = TYPEGRASS;
          } else if (!std::strcmp(value, "kch") || !std::strcmp(value, "hdb") ||
                     !std::strcmp(value, "hash")) {
            type = TYPEHASH;
          } else if (!std::strcmp(value, "kct") || !std::strcmp(value, "tdb") ||
                     !std::strcmp(value, "tree")) {
            type = TYPETREE;
          } else if (!std::strcmp(value, "kcd") || !std::strcmp(value, "ddb") ||
                     !std::strcmp(value, "dir") || !std::strcmp(value, "directory")) {
            type = TYPEDIR;
          } else if (!std::strcmp(value, "kcf") || !std::strcmp(value, "fdb") ||
                     !std::strcmp(value, "for") || !std::strcmp(value, "forest")) {
            type = TYPEFOREST;
          } else if (!std::strcmp(value, "kcx") || !std::strcmp(value, "xdb") ||
                     !std::strcmp(value, "txt") || !std::strcmp(value, "text")) {
            type = TYPETEXT;
          }
        } else if (!std::strcmp(key, "log") || !std::strcmp(key, "logger")) {
          logname = value;
        } else if (!std::strcmp(key, "logkinds") || !std::strcmp(key, "logkind")) {
          if (!std::strcmp(value, "debug") || !std::strcmp(value, "debugging")) {
            logkinds = Logger::DEBUG | Logger::INFO | Logger::WARN | Logger::ERROR;
          } else if (!std::strcmp(value, "info") || !std::strcmp(value, "information")) {
            logkinds = Logger::INFO | Logger::WARN | Logger::ERROR;
          } else if (!std::strcmp(value, "warn") || !std::strcmp(value, "warning")) {
            logkinds = Logger::WARN | Logger::ERROR;
          } else if (!std::strcmp(value, "error") || !std::strcmp(value, "fatal")) {
            logkinds = Logger::ERROR;
          } else {
            logkinds = atoix(value);
          }
        } else if (!std::strcmp(key, "logpx") || !std::strcmp(key, "lpx")) {
          logpx = value;
        } else if (!std::strcmp(key, "mtrg") || !std::strcmp(key, "metatrigger") ||
                   !std::strcmp(key, "meta_trigger")) {
          mtrgname = value;
        } else if (!std::strcmp(key, "mtrgpx") || !std::strcmp(key, "mtpx")) {
          mtrgpx = value;
        } else if (!std::strcmp(key, "bnum") || !std::strcmp(key, "buckets")) {
          bnum = atoix(value);
        } else if (!std::strcmp(key, "capcnt") || !std::strcmp(key, "capcount") ||
                   !std::strcmp(key, "cap_count")) {
          capcnt = atoix(value);
        } else if (!std::strcmp(key, "capsiz") || !std::strcmp(key, "capsize") ||
                   !std::strcmp(key, "cap_size")) {
          capsiz = atoix(value);
        } else if (!std::strcmp(key, "apow") || !std::strcmp(key, "alignment")) {
          apow = atoix(value);
        } else if (!std::strcmp(key, "fpow") || !std::strcmp(key, "fbp")) {
          fpow = atoix(value);
        } else if (!std::strcmp(key, "opts") || !std::strcmp(key, "options")) {
          if (std::strchr(value, 's')) tsmall = true;
          if (std::strchr(value, 'l')) tlinear = true;
          if (std::strchr(value, 'c')) tcompress = true;
        } else if (!std::strcmp(key, "msiz") || !std::strcmp(key, "map")) {
          msiz = atoix(value);
        } else if (!std::strcmp(key, "dfunit") || !std::strcmp(key, "defrag")) {
          dfunit = atoix(value);
        } else if (!std::strcmp(key, "zcomp") || !std::strcmp(key, "compressor")) {
          zcompname = value;
        } else if (!std::strcmp(key, "psiz") || !std::strcmp(key, "page")) {
          psiz = atoix(value);
        } else if (!std::strcmp(key, "pccap") || !std::strcmp(key, "cache")) {
          pccap = atoix(value);
        } else if (!std::strcmp(key, "rcomp") || !std::strcmp(key, "comparator")) {
          if (!std::strcmp(value, "lex") || !std::strcmp(value, "lexical")) {
            rcomp = LEXICALCOMP;
          } else if (!std::strcmp(value, "dec") || !std::strcmp(value, "decimal")) {
            rcomp = DECIMALCOMP;
          } else if (!std::strcmp(value, "lexdesc") || !std::strcmp(value, "lexicaldesc")) {
            rcomp = LEXICALDESCCOMP;
          } else if (!std::strcmp(value, "decdesc") || !std::strcmp(value, "decimaldesc")) {
            rcomp = DECIMALDESCCOMP;
          }
        } else if (!std::strcmp(key, "zkey") || !std::strcmp(key, "pass") ||
                   !std::strcmp(key, "password")) {
          zkey = value;
        }
      }
      ++it;
    }
    delete stdlogger_;
    delete stdlogstrm_;
    stdlogstrm_ = NULL;
    stdlogger_ = NULL;
    if (!logname.empty()) {
      std::ostream* stdlogstrm = NULL;
      if (logname == "-" || logname == "[stdout]" || logname == "[cout]") {
        stdlogstrm = &std::cout;
      } else if (logname == "+" || logname == "[stderr]" || logname == "[cerr]") {
        stdlogstrm = &std::cerr;
      } else {
        std::ofstream *ofs = new std::ofstream;
        ofs->open(logname.c_str(),
                  std::ios_base::out | std::ios_base::binary | std::ios_base::app);
        if (!*ofs) ofs->open(logname.c_str(), std::ios_base::out | std::ios_base::binary);
        if (ofs) {
          stdlogstrm = ofs;
          stdlogstrm_ = ofs;
        } else {
          delete ofs;
        }
      }
      if (stdlogstrm) stdlogger_ = new StreamLogger(stdlogstrm, logpx.c_str());
    }
    delete stdmtrigger_;
    delete stdmtrgstrm_;
    stdmtrgstrm_ = NULL;
    stdmtrigger_ = NULL;
    if (!mtrgname.empty()) {
      std::ostream* stdmtrgstrm = NULL;
      if (mtrgname == "-" || mtrgname == "[stdout]" || mtrgname == "[cout]") {
        stdmtrgstrm = &std::cout;
      } else if (mtrgname == "+" || mtrgname == "[stderr]" || mtrgname == "[cerr]") {
        stdmtrgstrm = &std::cerr;
      } else {
        std::ofstream *ofs = new std::ofstream;
        ofs->open(mtrgname.c_str(),
                  std::ios_base::out | std::ios_base::binary | std::ios_base::app);
        if (!*ofs) ofs->open(mtrgname.c_str(), std::ios_base::out | std::ios_base::binary);
        if (ofs) {
          stdmtrgstrm = ofs;
          stdmtrgstrm_ = ofs;
        } else {
          delete ofs;
        }
      }
      if (stdmtrgstrm) stdmtrigger_ = new StreamMetaTrigger(stdmtrgstrm, mtrgpx.c_str());
    }
    delete zcomp_;
    zcomp_ = NULL;
    ArcfourCompressor* arccomp = NULL;
    if (!zcompname.empty()) {
      if (zcompname == "zlib" || zcompname == "raw") {
        zcomp_ = new ZLIBCompressor<ZLIB::RAW>;
      } else if (zcompname == "def" || zcompname == "deflate") {
        zcomp_ = new ZLIBCompressor<ZLIB::DEFLATE>;
      } else if (zcompname == "gz" || zcompname == "gzip") {
        zcomp_ = new ZLIBCompressor<ZLIB::GZIP>;
      } else if (zcompname == "lzo" || zcompname == "oz") {
        zcomp_ = new LZOCompressor<LZO::RAW>;
      } else if (zcompname == "lzocrc" || zcompname == "ozcrc") {
        zcomp_ = new LZOCompressor<LZO::CRC>;
      } else if (zcompname == "lzma" || zcompname == "xz") {
        zcomp_ = new LZMACompressor<LZMA::RAW>;
      } else if (zcompname == "lzmacrc" || zcompname == "xzcrc") {
        zcomp_ = new LZMACompressor<LZMA::CRC>;
      } else if (zcompname == "lzmasha" || zcompname == "xzsha") {
        zcomp_ = new LZMACompressor<LZMA::SHA>;
      } else if (zcompname == "arc" || zcompname == "rc4") {
        arccomp = new ArcfourCompressor();
        zcomp_ = arccomp;
      } else if (zcompname == "arcz" || zcompname == "rc4z") {
        arccomp = new ArcfourCompressor();
        arccomp->set_compressor(ZLIBRAWCOMP);
        zcomp_ = arccomp;
      }
    }
    BasicDB *db;
    switch (type) {
      default: {
        set_error(_KCCODELINE_, Error::INVALID, "unknown database type");
        return false;
      }
      case TYPEPHASH: {
        ProtoHashDB* phdb = new ProtoHashDB();
        if (stdlogger_) {
          phdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          phdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          phdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          phdb->tune_meta_trigger(mtrigger_);
        }
        db = phdb;
        break;
      }
      case TYPEPTREE: {
        ProtoTreeDB *ptdb = new ProtoTreeDB();
        if (stdlogger_) {
          ptdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          ptdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          ptdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          ptdb->tune_meta_trigger(mtrigger_);
        }
        db = ptdb;
        break;
      }
      case TYPESTASH: {
        StashDB* sdb = new StashDB();
        if (stdlogger_) {
          sdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          sdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          sdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          sdb->tune_meta_trigger(mtrigger_);
        }
        if (bnum > 0) sdb->tune_buckets(bnum);
        db = sdb;
        break;
      }
      case TYPECACHE: {
        int8_t opts = 0;
        if (tcompress) opts |= CacheDB::TCOMPRESS;
        CacheDB* cdb = new CacheDB();
        if (stdlogger_) {
          cdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          cdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          cdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          cdb->tune_meta_trigger(mtrigger_);
        }
        if (opts > 0) cdb->tune_options(opts);
        if (bnum > 0) cdb->tune_buckets(bnum);
        if (zcomp_) cdb->tune_compressor(zcomp_);
        if (capcnt > 0) cdb->cap_count(capcnt);
        if (capsiz > 0) cdb->cap_size(capsiz);
        db = cdb;
        break;
      }
      case TYPEGRASS: {
        int8_t opts = 0;
        if (tcompress) opts |= GrassDB::TCOMPRESS;
        GrassDB* gdb = new GrassDB();
        if (stdlogger_) {
          gdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          gdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          gdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          gdb->tune_meta_trigger(mtrigger_);
        }
        if (opts > 0) gdb->tune_options(opts);
        if (bnum > 0) gdb->tune_buckets(bnum);
        if (psiz > 0) gdb->tune_page(psiz);
        if (zcomp_) gdb->tune_compressor(zcomp_);
        if (pccap > 0) gdb->tune_page_cache(pccap);
        if (rcomp) gdb->tune_comparator(rcomp);
        db = gdb;
        break;
      }
      case TYPEHASH: {
        int8_t opts = 0;
        if (tsmall) opts |= HashDB::TSMALL;
        if (tlinear) opts |= HashDB::TLINEAR;
        if (tcompress) opts |= HashDB::TCOMPRESS;
        HashDB* hdb = new HashDB();
        if (stdlogger_) {
          hdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          hdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          hdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          hdb->tune_meta_trigger(mtrigger_);
        }
        if (apow >= 0) hdb->tune_alignment(apow);
        if (fpow >= 0) hdb->tune_fbp(fpow);
        if (opts > 0) hdb->tune_options(opts);
        if (bnum > 0) hdb->tune_buckets(bnum);
        if (msiz >= 0) hdb->tune_map(msiz);
        if (dfunit > 0) hdb->tune_defrag(dfunit);
        if (zcomp_) hdb->tune_compressor(zcomp_);
        db = hdb;
        break;
      }
      case TYPETREE: {
        int8_t opts = 0;
        if (tsmall) opts |= TreeDB::TSMALL;
        if (tlinear) opts |= TreeDB::TLINEAR;
        if (tcompress) opts |= TreeDB::TCOMPRESS;
        TreeDB* tdb = new TreeDB();
        if (stdlogger_) {
          tdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          tdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          tdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          tdb->tune_meta_trigger(mtrigger_);
        }
        if (apow >= 0) tdb->tune_alignment(apow);
        if (fpow >= 0) tdb->tune_fbp(fpow);
        if (opts > 0) tdb->tune_options(opts);
        if (bnum > 0) tdb->tune_buckets(bnum);
        if (psiz > 0) tdb->tune_page(psiz);
        if (msiz >= 0) tdb->tune_map(msiz);
        if (dfunit > 0) tdb->tune_defrag(dfunit);
        if (zcomp_) tdb->tune_compressor(zcomp_);
        if (pccap > 0) tdb->tune_page_cache(pccap);
        if (rcomp) tdb->tune_comparator(rcomp);
        db = tdb;
        break;
      }
      case TYPEDIR: {
        int8_t opts = 0;
        if (tcompress) opts |= DirDB::TCOMPRESS;
        DirDB* ddb = new DirDB();
        if (stdlogger_) {
          ddb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          ddb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          ddb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          ddb->tune_meta_trigger(mtrigger_);
        }
        if (opts > 0) ddb->tune_options(opts);
        if (zcomp_) ddb->tune_compressor(zcomp_);
        db = ddb;
        break;
      }
      case TYPEFOREST: {
        int8_t opts = 0;
        if (tcompress) opts |= TreeDB::TCOMPRESS;
        ForestDB* fdb = new ForestDB();
        if (stdlogger_) {
          fdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          fdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          fdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          fdb->tune_meta_trigger(mtrigger_);
        }
        if (opts > 0) fdb->tune_options(opts);
        if (bnum > 0) fdb->tune_buckets(bnum);
        if (psiz > 0) fdb->tune_page(psiz);
        if (zcomp_) fdb->tune_compressor(zcomp_);
        if (pccap > 0) fdb->tune_page_cache(pccap);
        if (rcomp) fdb->tune_comparator(rcomp);
        db = fdb;
        break;
      }
      case TYPETEXT: {
        TextDB* xdb = new TextDB();
        if (stdlogger_) {
          xdb->tune_logger(stdlogger_, logkinds);
        } else if (logger_) {
          xdb->tune_logger(logger_, logkinds_);
        }
        if (stdmtrigger_) {
          xdb->tune_meta_trigger(stdmtrigger_);
        } else if (mtrigger_) {
          xdb->tune_meta_trigger(mtrigger_);
        }
        db = xdb;
        break;
      }
    }
    if (arccomp) arccomp->set_key(zkey.c_str(), zkey.size());
    if (!db->open(fpath, mode)) {
      const Error& error = db->error();
      set_error(_KCCODELINE_, error.code(), error.message());
      delete db;
      return false;
    }
    if (arccomp) {
      const std::string& apath = File::absolute_path(fpath);
      uint64_t hash = (hashmurmur(apath.c_str(), apath.size()) >> 16) << 40;
      hash += (uint64_t)(time() * 256);
      arccomp->begin_cycle(hash);
    }
    type_ = type;
    db_ = db;
    return true;
  }
  /**
   * Close the database file.
   * @return true on success, or false on failure.
   */
  bool close() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    bool err = false;
    if (!db_->close()) {
      const Error& error = db_->error();
      set_error(_KCCODELINE_, error.code(), error.message());
      err = true;
    }
    delete zcomp_;
    delete stdmtrigger_;
    delete stdmtrgstrm_;
    delete stdlogger_;
    delete stdlogstrm_;
    delete db_;
    type_ = TYPEVOID;
    db_ = NULL;
    stdlogstrm_ = NULL;
    stdlogger_ = NULL;
    stdmtrgstrm_ = NULL;
    stdmtrigger_ = NULL;
    zcomp_ = NULL;
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->synchronize(hard, proc, checker);
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
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->occupy(writable, proc);
  }
  /**
   * Begin transaction.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @return true on success, or false on failure.
   */
  bool begin_transaction(bool hard = false) {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->begin_transaction(hard);
  }
  /**
   * Try to begin transaction.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @return true on success, or false on failure.
   */
  bool begin_transaction_try(bool hard = false) {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->begin_transaction_try(hard);
  }
  /**
   * End transaction.
   * @param commit true to commit the transaction, or false to abort the transaction.
   * @return true on success, or false on failure.
   */
  bool end_transaction(bool commit = true) {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->end_transaction(commit);
  }
  /**
   * Remove all records.
   * @return true on success, or false on failure.
   */
  bool clear() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->clear();
  }
  /**
   * Get the number of records.
   * @return the number of records, or -1 on failure.
   */
  int64_t count() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return -1;
    }
    return db_->count();
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes, or -1 on failure.
   */
  int64_t size() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return -1;
    }
    return db_->size();
  }
  /**
   * Get the path of the database file.
   * @return the path of the database file, or an empty string on failure.
   */
  std::string path() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return "";
    }
    return db_->path();
  }
  /**
   * Get the miscellaneous status information.
   * @param strmap a string map to contain the result.
   * @return true on success, or false on failure.
   */
  bool status(std::map<std::string, std::string>* strmap) {
    _assert_(strmap);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    return db_->status(strmap);
  }
  /**
   * Reveal the inner database object.
   * @return the inner database object, or NULL on failure.
   */
  BasicDB* reveal_inner_db() {
    _assert_(true);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return NULL;
    }
    return db_;
  }
  /**
   * Get keys matching a prefix string.
   * @param prefix the prefix string.
   * @param strvec a string vector to contain the result.
   * @param max the maximum number to retrieve.  If it is negative, no limit is specified.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return the number of retrieved keys or -1 on failure.
   */
  int64_t match_prefix(const std::string& prefix, std::vector<std::string>* strvec,
                       int64_t max = -1, ProgressChecker* checker = NULL) {
    _assert_(strvec);
    const char* pbuf = prefix.data();
    size_t psiz = prefix.size();
    if (max < 0) max = INT64MAX;
    Comparator* comp;
    switch (type_) {
      case TYPEPTREE: {
        comp = LEXICALCOMP;
        break;
      }
      case TYPEGRASS: {
        comp = ((GrassDB*)db_)->rcomp();
        break;
      }
      case TYPETREE: {
        comp = ((TreeDB*)db_)->rcomp();
        break;
      }
      case TYPEFOREST: {
        comp = ((ForestDB*)db_)->rcomp();
        break;
      }
      default: {
        comp = NULL;
        break;
      }
    }
    bool err = false;
    int64_t allcnt = count();
    if (checker && !checker->check("match_prefix", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    strvec->clear();
    Cursor* cur = cursor();
    int64_t curcnt = 0;
    if (comp == LEXICALCOMP) {
      if (cur->jump(pbuf, psiz)) {
        while ((int64_t)strvec->size() < max) {
          size_t ksiz;
          char* kbuf = cur->get_key(&ksiz, true);
          if (kbuf) {
            if (ksiz >= psiz && !std::memcmp(kbuf, pbuf, psiz)) {
              strvec->push_back(std::string(kbuf, ksiz));
            } else {
              delete[] kbuf;
              break;
            }
            delete[] kbuf;
          } else {
            if (cur->error() != Error::NOREC) err = true;
            break;
          }
          curcnt++;
          if (checker && !checker->check("match_prefix", "processing", curcnt, allcnt)) {
            set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
            err = true;
          }
        }
      } else if (cur->error() != Error::NOREC) {
        err = true;
      }
    } else {
      if (cur->jump()) {
        while ((int64_t)strvec->size() < max) {
          size_t ksiz;
          char* kbuf = cur->get_key(&ksiz, true);
          if (kbuf) {
            if (ksiz >= psiz && !std::memcmp(kbuf, pbuf, psiz))
              strvec->push_back(std::string(kbuf, ksiz));
            delete[] kbuf;
          } else {
            if (cur->error() != Error::NOREC) err = true;
            break;
          }
          curcnt++;
          if (checker && !checker->check("match_prefix", "processing", curcnt, allcnt)) {
            set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
            err = true;
          }
        }
      } else if (cur->error() != Error::NOREC) {
        err = true;
      }
    }
    if (checker && !checker->check("match_prefix", "ending", strvec->size(), allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    delete cur;
    return err ? -1 : strvec->size();
  }
  /**
   * Get keys matching a regular expression string.
   * @param regex the regular expression string.
   * @param strvec a string vector to contain the result.
   * @param max the maximum number to retrieve.  If it is negative, no limit is specified.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return the number of retrieved keys or -1 on failure.
   */
  int64_t match_regex(const std::string& regex, std::vector<std::string>* strvec,
                      int64_t max = -1, ProgressChecker* checker = NULL) {
    _assert_(strvec);
    if (max < 0) max = INT64MAX;
    Regex reg;
    if (!reg.compile(regex, Regex::MATCHONLY)) {
      set_error(_KCCODELINE_, Error::LOGIC, "compilation failed");
      return -1;
    }
    bool err = false;
    int64_t allcnt = count();
    if (checker && !checker->check("match_regex", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    strvec->clear();
    Cursor* cur = cursor();
    int64_t curcnt = 0;
    if (cur->jump()) {
      while ((int64_t)strvec->size() < max) {
        size_t ksiz;
        char* kbuf = cur->get_key(&ksiz, true);
        if (kbuf) {
          std::string key(kbuf, ksiz);
          if (reg.match(key)) strvec->push_back(key);
          delete[] kbuf;
        } else {
          if (cur->error() != Error::NOREC) err = true;
          break;
        }
        curcnt++;
        if (checker && !checker->check("match_regex", "processing", curcnt, allcnt)) {
          set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
          err = true;
        }
      }
    } else if (cur->error() != Error::NOREC) {
      err = true;
    }
    if (checker && !checker->check("match_regex", "ending", strvec->size(), allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    delete cur;
    return err ? -1 : strvec->size();
  }
  /**
   * Get keys similar to a string in terms of the levenshtein distance.
   * @param origin the origin string.
   * @param range the maximum distance of keys to adopt.
   * @param utf flag to treat keys as UTF-8 strings.
   * @param strvec a string vector to contain the result.
   * @param max the maximum number to retrieve.  If it is negative, no limit is specified.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return the number of retrieved keys or -1 on failure.
   */
  int64_t match_similar(const std::string& origin, size_t range, bool utf,
                        std::vector<std::string>* strvec,
                        int64_t max = -1, ProgressChecker* checker = NULL) {
    _assert_(strvec);
    if (max < 0) max = INT64MAX;
    bool err = false;
    int64_t allcnt = count();
    if (checker && !checker->check("match_similar", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    strvec->clear();
    uint32_t ostack[128];
    uint32_t* oary = NULL;
    size_t onum = 0;
    if (utf) {
      const char* ostr = origin.c_str();
      onum = strutflen(ostr);
      oary = onum > sizeof(ostack) / sizeof(*ostack) ? new uint32_t[onum] : ostack;
      strutftoucs(ostr, oary, &onum);
    }
    Cursor* cur = cursor();
    int64_t curcnt = 0;
    std::priority_queue<SimilarKey> queue;
    if (cur->jump()) {
      if (max > 0) {
        while (true) {
          size_t ksiz;
          char* kbuf = cur->get_key(&ksiz, true);
          if (kbuf) {
            size_t kdist;
            if (oary) {
              uint32_t kstack[128];
              uint32_t* kary = ksiz > sizeof(kstack) / sizeof(*kstack) ?
                  new uint32_t[ksiz] : kstack;
              size_t knum;
              strutftoucs(kbuf, ksiz, kary, &knum);
              kdist = std::labs((long)onum - (long)knum) > (long)range ?
                  UINT32MAX : strucsdist(oary, onum, kary, knum);
              if (kary != kstack) delete[] kary;
            } else {
              kdist = std::labs((long)origin.size() - (long)ksiz) > (long)range ?
                  UINT32MAX : memdist(origin.data(), origin.size(), kbuf, ksiz);
            }
            if (kdist <= range) {
              std::string key(kbuf, ksiz);
              if ((int64_t)queue.size() < max) {
                SimilarKey skey = { kdist, key, curcnt };
                queue.push(skey);
              } else {
                const SimilarKey& top = queue.top();
                if (!top.less(kdist, key, curcnt)) {
                  queue.pop();
                  SimilarKey skey = { kdist, key, curcnt };
                  queue.push(skey);
                }
              }
            }
            delete[] kbuf;
          } else {
            if (cur->error() != Error::NOREC) err = true;
            break;
          }
          curcnt++;
          if (checker && !checker->check("match_similar", "processing", curcnt, allcnt)) {
            set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
            err = true;
          }
        }
        while (!queue.empty()) {
          const SimilarKey& top = queue.top();
          strvec->push_back(top.key);
          queue.pop();
        }
        size_t end = strvec->size() - 1;
        size_t mid = strvec->size() / 2;
        for (size_t i = 0; i < mid; i++) {
          (*strvec)[i].swap((*strvec)[end-i]);
        }
      }
    } else if (cur->error() != Error::NOREC) {
      err = true;
    }
    if (checker && !checker->check("match_similar", "ending", strvec->size(), allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    delete cur;
    if (oary && oary != ostack) delete[] oary;
    return err ? -1 : strvec->size();
  }
  /**
   * Merge records from other databases.
   * @param srcary an array of the source detabase objects.
   * @param srcnum the number of the elements of the source array.
   * @param mode the merge mode.  PolyDB::MSET to overwrite the existing value, PolyDB::MADD to
   * keep the existing value, PolyDB::MREPLACE to modify the existing record only,
   * PolyDB::MAPPEND to append the new value.
   * @param checker a progress checker object.  If it is NULL, no checking is performed.
   * @return true on success, or false on failure.
   */
  bool merge(BasicDB** srcary, size_t srcnum, MergeMode mode = MSET,
             ProgressChecker* checker = NULL) {
    _assert_(srcary && srcnum <= MEMMAXSIZ);
    if (type_ == TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "not opened");
      return false;
    }
    bool err = false;
    Comparator* comp;
    switch (type_) {
      case TYPEGRASS: {
        comp = ((GrassDB*)db_)->rcomp();
        break;
      }
      case TYPETREE: {
        comp = ((TreeDB*)db_)->rcomp();
        break;
      }
      case TYPEFOREST: {
        comp = ((ForestDB*)db_)->rcomp();
        break;
      }
      default: {
        comp = NULL;
        break;
      }
    }
    if (!comp) comp = LEXICALCOMP;
    std::priority_queue<MergeLine> lines;
    int64_t allcnt = 0;
    for (size_t i = 0; i < srcnum; i++) {
      MergeLine line;
      line.cur = srcary[i]->cursor();
      line.comp = comp;
      line.cur->jump();
      line.kbuf = line.cur->get(&line.ksiz, &line.vbuf, &line.vsiz, true);
      if (line.kbuf) {
        lines.push(line);
        int64_t count = srcary[i]->count();
        if (count > 0) allcnt += count;
      } else {
        delete line.cur;
      }
    }
    if (checker && !checker->check("merge", "beginning", 0, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    int64_t curcnt = 0;
    while (!err && !lines.empty()) {
      MergeLine line = lines.top();
      lines.pop();
      switch (mode) {
        case MSET: {
          if (!set(line.kbuf, line.ksiz, line.vbuf, line.vsiz)) err = true;
          break;
        }
        case MADD: {
          if (!add(line.kbuf, line.ksiz, line.vbuf, line.vsiz) &&
              error() != Error::DUPREC) err = true;
          break;
        }
        case MREPLACE: {
          if (!replace(line.kbuf, line.ksiz, line.vbuf, line.vsiz) &&
              error() != Error::NOREC) err = true;
          break;
        }
        case MAPPEND: {
          if (!append(line.kbuf, line.ksiz, line.vbuf, line.vsiz)) err = true;
          break;
        }
      }
      delete[] line.kbuf;
      line.kbuf = line.cur->get(&line.ksiz, &line.vbuf, &line.vsiz, true);
      if (line.kbuf) {
        lines.push(line);
      } else {
        delete line.cur;
      }
      curcnt++;
      if (checker && !checker->check("merge", "processing", curcnt, allcnt)) {
        set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
        err = true;
        break;
      }
    }
    if (checker && !checker->check("merge", "ending", -1, allcnt)) {
      set_error(_KCCODELINE_, Error::LOGIC, "checker failed");
      err = true;
    }
    while (!lines.empty()) {
      MergeLine line = lines.top();
      lines.pop();
      delete[] line.kbuf;
      delete line.cur;
    }
    return !err;
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
    if (logger_) {
      logger_->log(file, line, func, kind, message);
    } else if (type_ != TYPEVOID) {
      db_->log(file, line, func, kind, message);
    }
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
    if (type_ != TYPEVOID) {
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
    if (type_ != TYPEVOID) {
      set_error(_KCCODELINE_, Error::INVALID, "already opened");
      return false;
    }
    mtrigger_ = trigger;
    return true;
  }
 private:
  /**
   * Stream logger implementation.
   */
  class StreamLogger : public Logger {
   public:
    /** constructor */
    StreamLogger(std::ostream* strm, const char* prefix) : strm_(strm), prefix_(prefix) {}
    /** print a log message */
    void log(const char* file, int32_t line, const char* func, Kind kind,
             const char* message) {
      _assert_(file && line > 0 && func && message);
      const char* kstr = "MISC";
      switch (kind) {
        case Logger::DEBUG: kstr = "DEBUG"; break;
        case Logger::INFO: kstr = "INFO"; break;
        case Logger::WARN: kstr = "WARN"; break;
        case Logger::ERROR: kstr = "ERROR"; break;
      }
      if (!prefix_.empty()) *strm_ << prefix_ << ": ";
      *strm_ << "[" << kstr << "]: " << file << ": " << line << ": " << func << ": " <<
          message << std::endl;
    }
   private:
    std::ostream* strm_;                 ///< output stream
    std::string prefix_;                 ///< prefix of each message
  };
  /**
   * Stream meta operation trigger implementation.
   */
  class StreamMetaTrigger : public MetaTrigger {
   public:
    /** constructor */
    StreamMetaTrigger(std::ostream* strm, const char* prefix) : strm_(strm), prefix_(prefix) {}
    /** print a meta operation */
    void trigger(Kind kind, const char* message) {
      _assert_(message);
      const char* kstr = "unknown";
      switch (kind) {
        case MetaTrigger::OPEN: kstr = "OPEN"; break;
        case MetaTrigger::CLOSE: kstr = "CLOSE"; break;
        case MetaTrigger::CLEAR: kstr = "CLEAR"; break;
        case MetaTrigger::ITERATE: kstr = "ITERATE"; break;
        case MetaTrigger::SYNCHRONIZE: kstr = "SYNCHRONIZE"; break;
        case MetaTrigger::OCCUPY: kstr = "OCCUPY"; break;
        case MetaTrigger::BEGINTRAN: kstr = "BEGINTRAN"; break;
        case MetaTrigger::COMMITTRAN: kstr = "COMMITTRAN"; break;
        case MetaTrigger::ABORTTRAN: kstr = "ABORTTRAN"; break;
        case MetaTrigger::MISC: kstr = "MISC"; break;
      }
      if (!prefix_.empty()) *strm_ << prefix_ << ": ";
      *strm_ << "[" << kstr << "]: " << message << std::endl;
    }
   private:
    std::ostream* strm_;                 ///< output stream
    std::string prefix_;                 ///< prefix of each message
  };
  /**
   * Key for similarity search.
   */
  struct SimilarKey {
    size_t dist;
    std::string key;
    int64_t order;
    bool operator <(const SimilarKey& right) const {
      if (dist != right.dist) return dist < right.dist;
      if (key != right.key) return key < right.key;
      return order < right.order;
    }
    bool less(size_t rdist, const std::string& rkey, uint32_t rorder) const {
      if (dist != rdist) return dist < rdist;
      if (key != rkey) return key < rkey;
      return order < rorder;
    }
  };
  /**
   * Front line of a merging list.
   */
  struct MergeLine {
    BasicDB::Cursor* cur;                ///< cursor
    Comparator* comp;                    ///< comparator
    char* kbuf;                          ///< pointer to the key
    size_t ksiz;                         ///< size of the key
    const char* vbuf;                    ///< pointer to the value
    size_t vsiz;                         ///< size of the value
    /** comparing operator */
    bool operator <(const MergeLine& right) const {
      return comp->compare(kbuf, ksiz, right.kbuf, right.ksiz) > 0;
    }
  };
  /** Dummy constructor to forbid the use. */
  PolyDB(const PolyDB&);
  /** Dummy Operator to forbid the use. */
  PolyDB& operator =(const PolyDB&);
  /** The database type. */
  Type type_;
  /** The internal database. */
  BasicDB* db_;
  /** The last happened error. */
  Error error_;
  /** The standard log stream. */
  std::ostream* stdlogstrm_;
  /** The standard logger. */
  Logger* stdlogger_;
  /** The internal logger. */
  Logger* logger_;
  /** The kinds of logged messages. */
  uint32_t logkinds_;
  /** The standard meta operation trigger stream. */
  std::ostream* stdmtrgstrm_;
  /** The standard meta operation trigger. */
  MetaTrigger* stdmtrigger_;
  /** The internal meta operation trigger. */
  MetaTrigger* mtrigger_;
  /** The custom compressor. */
  Compressor* zcomp_;
};


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
