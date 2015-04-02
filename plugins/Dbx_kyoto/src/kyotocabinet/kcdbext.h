/*************************************************************************************************
 * Database extension
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


#ifndef _KCDBEXT_H                       // duplication check
#define _KCDBEXT_H

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
#include <kcpolydb.h>

namespace kyotocabinet {                 // common namespace


/**
 * MapReduce framework.
 * @note Although this framework is not distributed or concurrent, it is useful for aggregate
 * calculation with less CPU loading and less memory usage.
 */
class MapReduce {
 public:
  class ValueIterator;
 private:
  class FlushThread;
  class ReduceTaskQueue;
  class MapVisitor;
  struct MergeLine;
  /** An alias of vector of loaded values. */
  typedef std::vector<std::string> Values;
  /** The default number of temporary databases. */
  static const size_t DEFDBNUM = 8;
  /** The maxinum number of temporary databases. */
  static const size_t MAXDBNUM = 256;
  /** The default cache limit. */
  static const int64_t DEFCLIM = 512LL << 20;
  /** The default cache bucket numer. */
  static const int64_t DEFCBNUM = 1048583LL;
  /** The bucket number of temprary databases. */
  static const int64_t DBBNUM = 512LL << 10;
  /** The page size of temprary databases. */
  static const int32_t DBPSIZ = 32768;
  /** The mapped size of temprary databases. */
  static const int64_t DBMSIZ = 516LL * 4096;
  /** The page cache capacity of temprary databases. */
  static const int64_t DBPCCAP = 16LL << 20;
  /** The default number of threads in parallel mode. */
  static const size_t DEFTHNUM = 8;
  /** The number of slots of the record lock. */
  static const int32_t RLOCKSLOT = 256;
 public:
  /**
   * Value iterator for the reducer.
   */
  class ValueIterator {
    friend class MapReduce;
   public:
    /**
     * Get the next value.
     * @param sp the pointer to the variable into which the size of the region of the return
     * value is assigned.
     * @return the pointer to the next value region, or NULL if no value remains.
     */
    const char* next(size_t* sp) {
      _assert_(sp);
      if (!vptr_) {
        if (vit_ == vend_) return NULL;
        vptr_ = vit_->data();
        vsiz_ = vit_->size();
        vit_++;
      }
      uint64_t vsiz;
      size_t step = readvarnum(vptr_, vsiz_, &vsiz);
      vptr_ += step;
      vsiz_ -= step;
      const char* vbuf = vptr_;
      *sp = vsiz;
      vptr_ += vsiz;
      vsiz_ -= vsiz;
      if (vsiz_ < 1) vptr_ = NULL;
      return vbuf;
    }
   private:
    /**
     * Default constructor.
     */
    explicit ValueIterator(Values::const_iterator vit, Values::const_iterator vend) :
        vit_(vit), vend_(vend), vptr_(NULL), vsiz_(0) {
      _assert_(true);
    }
    /**
     * Destructor.
     */
    ~ValueIterator() {
      _assert_(true);
    }
    /** Dummy constructor to forbid the use. */
    ValueIterator(const ValueIterator&);
    /** Dummy Operator to forbid the use. */
    ValueIterator& operator =(const ValueIterator&);
    /** The current iterator of loaded values. */
    Values::const_iterator vit_;
    /** The ending iterator of loaded values. */
    Values::const_iterator vend_;
    /** The pointer of the current value. */
    const char* vptr_;
    /** The size of the current value. */
    size_t vsiz_;
  };
  /**
   * Execution options.
   */
  enum Option {
    XNOLOCK = 1 << 0,                    ///< avoid locking against update operations
    XPARAMAP = 1 << 1,                   ///< run mappers in parallel
    XPARARED = 1 << 2,                   ///< run reducers in parallel
    XPARAFLS = 1 << 3,                   ///< run cache flushers in parallel
    XNOCOMP = 1 << 8                     ///< avoid compression of temporary databases
  };
  /**
   * Default constructor.
   */
  explicit MapReduce() :
      db_(NULL), rcomp_(NULL), tmpdbs_(NULL), dbnum_(DEFDBNUM), dbclock_(0),
      mapthnum_(DEFTHNUM), redthnum_(DEFTHNUM), flsthnum_(DEFTHNUM),
      cache_(NULL), csiz_(0), clim_(DEFCLIM), cbnum_(DEFCBNUM), flsths_(NULL),
      redtasks_(NULL), redaborted_(false), rlocks_(NULL) {
    _assert_(true);
  }
  /**
   * Destructor.
   */
  virtual ~MapReduce() {
    _assert_(true);
  }
  /**
   * Map a record data.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   * @note This function can call the MapReduce::emit method to emit a record.  To avoid
   * deadlock, any explicit database operation must not be performed in this function.
   */
  virtual bool map(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) = 0;
  /**
   * Reduce a record data.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param iter the iterator to get the values.
   * @return true on success, or false on failure.
   * @note To avoid deadlock, any explicit database operation must not be performed in this
   * function.
   */
  virtual bool reduce(const char* kbuf, size_t ksiz, ValueIterator* iter) = 0;
  /**
   * Preprocess the map operations.
   * @return true on success, or false on failure.
   * @note This function can call the MapReduce::emit method to emit a record.  To avoid
   * deadlock, any explicit database operation must not be performed in this function.
   */
  virtual bool preprocess() {
    _assert_(true);
    return true;
  }
  /**
   * Mediate between the map and the reduce phases.
   * @return true on success, or false on failure.
   * @note This function can call the MapReduce::emit method to emit a record.  To avoid
   * deadlock, any explicit database operation must not be performed in this function.
   */
  virtual bool midprocess() {
    _assert_(true);
    return true;
  }
  /**
   * Postprocess the reduce operations.
   * @return true on success, or false on failure.
   * @note To avoid deadlock, any explicit database operation must not be performed in this
   * function.
   */
  virtual bool postprocess() {
    _assert_(true);
    return true;
  }
  /**
   * Process a log message.
   * @param name the name of the event.
   * @param message a supplement message.
   * @return true on success, or false on failure.
   */
  virtual bool log(const char* name, const char* message) {
    _assert_(name && message);
    return true;
  }
  /**
   * Execute the MapReduce process about a database.
   * @param db the source database.
   * @param tmppath the path of a directory for the temporary data storage.  If it is an empty
   * string, temporary data are handled on memory.
   * @param opts the optional features by bitwise-or: MapReduce::XNOLOCK to avoid locking
   * against update operations by other threads, MapReduce::XPARAMAP to run the mapper in
   * parallel, MapReduce::XPARARED to run the reducer in parallel, MapReduce::XNOCOMP to avoid
   * compression of temporary databases.
   * @return true on success, or false on failure.
   */
  bool execute(BasicDB* db, const std::string& tmppath = "", uint32_t opts = 0) {
    int64_t count = db->count();
    if (count < 0) {
      if (db->error() != BasicDB::Error::NOIMPL) return false;
      count = 0;
    }
    bool err = false;
    double stime, etime;
    db_ = db;
    rcomp_ = LEXICALCOMP;
    BasicDB* idb = db;
    if (typeid(*db) == typeid(PolyDB)) {
      PolyDB* pdb = (PolyDB*)idb;
      idb = pdb->reveal_inner_db();
    }
    const std::type_info& info = typeid(*idb);
    if (info == typeid(GrassDB)) {
      GrassDB* gdb = (GrassDB*)idb;
      rcomp_ = gdb->rcomp();
    } else if (info == typeid(TreeDB)) {
      TreeDB* tdb = (TreeDB*)idb;
      rcomp_ = tdb->rcomp();
    } else if (info == typeid(ForestDB)) {
      ForestDB* fdb = (ForestDB*)idb;
      rcomp_ = fdb->rcomp();
    }
    tmpdbs_ = new BasicDB*[dbnum_];
    if (tmppath.empty()) {
      if (!logf("prepare", "started to open temporary databases on memory")) err = true;
      stime = time();
      for (size_t i = 0; i < dbnum_; i++) {
        GrassDB* gdb = new GrassDB;
        int32_t myopts = 0;
        if (!(opts & XNOCOMP)) myopts |= GrassDB::TCOMPRESS;
        gdb->tune_options(myopts);
        gdb->tune_buckets(DBBNUM / 2);
        gdb->tune_page(DBPSIZ);
        gdb->tune_page_cache(DBPCCAP);
        gdb->tune_comparator(rcomp_);
        gdb->open("%", GrassDB::OWRITER | GrassDB::OCREATE | GrassDB::OTRUNCATE);
        tmpdbs_[i] = gdb;
      }
      etime = time();
      if (!logf("prepare", "opening temporary databases finished: time=%.6f", etime - stime))
        err = true;
      if (err) {
        delete[] tmpdbs_;
        return false;
      }
    } else {
      File::Status sbuf;
      if (!File::status(tmppath, &sbuf) || !sbuf.isdir) {
        db->set_error(_KCCODELINE_, BasicDB::Error::NOREPOS, "no such directory");
        delete[] tmpdbs_;
        return false;
      }
      if (!logf("prepare", "started to open temporary databases under %s", tmppath.c_str()))
        err = true;
      stime = time();
      uint32_t pid = getpid() & UINT16MAX;
      uint32_t tid = Thread::hash() & UINT16MAX;
      uint32_t ts = time() * 1000;
      for (size_t i = 0; i < dbnum_; i++) {
        std::string childpath =
            strprintf("%s%cmr-%04x-%04x-%08x-%03d%ckct",
                      tmppath.c_str(), File::PATHCHR, pid, tid, ts, (int)(i + 1), File::EXTCHR);
        TreeDB* tdb = new TreeDB;
        int32_t myopts = TreeDB::TSMALL | TreeDB::TLINEAR;
        if (!(opts & XNOCOMP)) myopts |= TreeDB::TCOMPRESS;
        tdb->tune_options(myopts);
        tdb->tune_buckets(DBBNUM);
        tdb->tune_page(DBPSIZ);
        tdb->tune_map(DBMSIZ);
        tdb->tune_page_cache(DBPCCAP);
        tdb->tune_comparator(rcomp_);
        if (!tdb->open(childpath, TreeDB::OWRITER | TreeDB::OCREATE | TreeDB::OTRUNCATE)) {
          const BasicDB::Error& e = tdb->error();
          db->set_error(_KCCODELINE_, e.code(), e.message());
          err = true;
        }
        tmpdbs_[i] = tdb;
      }
      etime = time();
      if (!logf("prepare", "opening temporary databases finished: time=%.6f", etime - stime))
        err = true;
      if (err) {
        for (size_t i = 0; i < dbnum_; i++) {
          delete tmpdbs_[i];
        }
        delete[] tmpdbs_;
        return false;
      }
    }
    if (opts & XPARARED) redtasks_ = new ReduceTaskQueue;
    if (opts & XPARAFLS) flsths_ = new std::deque<FlushThread*>;
    if (opts & XNOLOCK) {
      MapChecker mapchecker;
      MapVisitor mapvisitor(this, &mapchecker, count);
      mapvisitor.visit_before();
      if (!err) {
        BasicDB::Cursor* cur = db->cursor();
        if (!cur->jump() && cur->error() != BasicDB::Error::NOREC) err = true;
        while (!err) {
          if (!cur->accept(&mapvisitor, false, true)) {
            if (cur->error() != BasicDB::Error::NOREC) err = true;
            break;
          }
        }
        delete cur;
      }
      if (mapvisitor.error()) {
        db_->set_error(_KCCODELINE_, BasicDB::Error::LOGIC, "mapper failed");
        err = true;
      }
      mapvisitor.visit_after();
    } else if (opts & XPARAMAP) {
      MapChecker mapchecker;
      MapVisitor mapvisitor(this, &mapchecker, count);
      rlocks_ = new SlottedMutex(RLOCKSLOT);
      if (!err && !db->scan_parallel(&mapvisitor, mapthnum_, &mapchecker)) {
        db_->set_error(_KCCODELINE_, BasicDB::Error::LOGIC, "mapper failed");
        err = true;
      }
      delete rlocks_;
      rlocks_ = NULL;
      if (mapvisitor.error()) err = true;
    } else {
      MapChecker mapchecker;
      MapVisitor mapvisitor(this, &mapchecker, count);
      if (!err && !db->iterate(&mapvisitor, false, &mapchecker)) err = true;
      if (mapvisitor.error()) {
        db_->set_error(_KCCODELINE_, BasicDB::Error::LOGIC, "mapper failed");
        err = true;
      }
    }
    if (flsths_) {
      delete flsths_;
      flsths_ = NULL;
    }
    if (redtasks_) {
      delete redtasks_;
      redtasks_ = NULL;
    }
    if (!logf("clean", "closing the temporary databases")) err = true;
    stime = time();
    for (size_t i = 0; i < dbnum_; i++) {
      const std::string& path = tmpdbs_[i]->path();
      if (!tmpdbs_[i]->clear()) {
        const BasicDB::Error& e = tmpdbs_[i]->error();
        db->set_error(_KCCODELINE_, e.code(), e.message());
        err = true;
      }
      if (!tmpdbs_[i]->close()) {
        const BasicDB::Error& e = tmpdbs_[i]->error();
        db->set_error(_KCCODELINE_, e.code(), e.message());
        err = true;
      }
      if (!tmppath.empty()) File::remove(path);
      delete tmpdbs_[i];
    }
    etime = time();
    if (!logf("clean", "closing the temporary databases finished: time=%.6f",
              etime - stime)) err = true;
    delete[] tmpdbs_;
    return !err;
  }
  /**
   * Set the storage configurations.
   * @param dbnum the number of temporary databases.
   * @param clim the limit size of the internal cache.
   * @param cbnum the bucket number of the internal cache.
   */
  void tune_storage(int32_t dbnum, int64_t clim, int64_t cbnum) {
    _assert_(true);
    dbnum_ = dbnum > 0 ? dbnum : DEFDBNUM;
    if (dbnum_ > MAXDBNUM) dbnum_ = MAXDBNUM;
    clim_ = clim > 0 ? clim : DEFCLIM;
    cbnum_ = cbnum > 0 ? cbnum : DEFCBNUM;
    if (cbnum_ > INT16MAX) cbnum_ = nearbyprime(cbnum_);
  }
  /**
   * Set the thread configurations.
   * @param mapthnum the number of threads for the mapper.
   * @param redthnum the number of threads for the reducer.
   * @param flsthnum the number of threads for the internal flusher.
   */
  void tune_thread(int32_t mapthnum, int32_t redthnum, int32_t flsthnum) {
    _assert_(true);
    mapthnum_ = mapthnum > 0 ? mapthnum : DEFTHNUM;
    redthnum_ = redthnum > 0 ? redthnum : DEFTHNUM;
    flsthnum_ = flsthnum > 0 ? flsthnum : DEFTHNUM;
  }
 protected:
  /**
   * Emit a record from the mapper.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   */
  bool emit(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    bool err = false;
    size_t rsiz = sizevarnum(vsiz) + vsiz;
    char stack[NUMBUFSIZ*4];
    char* rbuf = rsiz > sizeof(stack) ? new char[rsiz] : stack;
    char* wp = rbuf;
    wp += writevarnum(rbuf, vsiz);
    std::memcpy(wp, vbuf, vsiz);
    if (rlocks_) {
      size_t bidx = TinyHashMap::hash_record(kbuf, ksiz) % cbnum_;
      size_t lidx = bidx % RLOCKSLOT;
      rlocks_->lock(lidx);
      cache_->append(kbuf, ksiz, rbuf, rsiz);
      rlocks_->unlock(lidx);
    } else {
      cache_->append(kbuf, ksiz, rbuf, rsiz);
    }
    if (rbuf != stack) delete[] rbuf;
    csiz_ += sizevarnum(ksiz) + ksiz + rsiz;
    return !err;
  }
 private:
  /**
   * Cache flusher.
   */
  class FlushThread : public Thread {
   public:
    /** constructor */
    explicit FlushThread(MapReduce* mr, BasicDB* tmpdb,
                         TinyHashMap* cache, size_t csiz, bool cown) :
        mr_(mr), tmpdb_(tmpdb), cache_(cache), csiz_(csiz), cown_(cown), err_(false) {}
    /** perform the concrete process */
    void run() {
      if (!mr_->logf("map", "started to flushing the cache: count=%lld size=%lld",
                     (long long)cache_->count(), (long long)csiz_)) err_ = true;
      double stime = time();
      BasicDB* tmpdb = tmpdb_;
      TinyHashMap* cache = cache_;
      bool cown = cown_;
      TinyHashMap::Sorter sorter(cache);
      const char* kbuf, *vbuf;
      size_t ksiz, vsiz;
      while ((kbuf = sorter.get(&ksiz, &vbuf, &vsiz)) != NULL) {
        if (!tmpdb->append(kbuf, ksiz, vbuf, vsiz)) {
          const BasicDB::Error& e = tmpdb->error();
          mr_->db_->set_error(_KCCODELINE_, e.code(), e.message());
          err_ = true;
        }
        sorter.step();
        if (cown) cache->remove(kbuf, ksiz);
      }
      double etime = time();
      if (!mr_->logf("map", "flushing the cache finished: time=%.6f", etime - stime))
        err_ = true;
      if (cown) delete cache;
    }
    /** check the error flag. */
    bool error() {
      return err_;
    }
   private:
    MapReduce* mr_;                          ///< driver
    BasicDB* tmpdb_;                         ///< temprary database
    TinyHashMap* cache_;                     ///< cache for emitter
    size_t csiz_;                            ///< current cache size
    bool cown_;                              ///< cache ownership flag
    bool err_;                               ///< error flag
  };
  /**
   * Task queue for parallel reducer.
   */
  class ReduceTaskQueue : public TaskQueue {
   public:
    /**
     * Task for parallel reducer.
     */
    class ReduceTask : public Task {
      friend class ReduceTaskQueue;
     public:
      /** constructor */
      explicit ReduceTask(MapReduce* mr, const char* kbuf, size_t ksiz, const Values& values) :
          mr_(mr), key_(kbuf, ksiz), values_(values) {}
     private:
      MapReduce* mr_;                    ///< driver
      std::string key_;                  ///< key
      Values values_;                    ///< values
    };
    /** constructor */
    explicit ReduceTaskQueue() {}
   private:
    /** process a task */
    void do_task(Task* task) {
      ReduceTask* rtask = (ReduceTask*)task;
      ValueIterator iter(rtask->values_.begin(), rtask->values_.end());
      if (!rtask->mr_->reduce(rtask->key_.data(), rtask->key_.size(), &iter))
        rtask->mr_->redaborted_ = true;
      delete rtask;
    }
  };
  /**
   * Checker for the map process.
   */
  class MapChecker : public BasicDB::ProgressChecker {
   public:
    /** constructor */
    explicit MapChecker() : stop_(false) {}
    /** stop the process */
    void stop() {
      stop_ = true;
    }
    /** check whether stopped */
    bool stopped() {
      return stop_;
    }
   private:
    /** check whether stopped */
    bool check(const char* name, const char* message, int64_t curcnt, int64_t allcnt) {
      return !stop_;
    }
    bool stop_;                          ///< flag for stop
  };
  /**
   * Visitor for the map process.
   */
  class MapVisitor : public BasicDB::Visitor {
   public:
    /** constructor */
    explicit MapVisitor(MapReduce* mr, MapChecker* checker, int64_t scale) :
        mr_(mr), checker_(checker), scale_(scale), stime_(0), err_(false) {}
    /** get the error flag */
    bool error() {
      return err_;
    }
    /** preprocess the mappter */
    void visit_before() {
      mr_->dbclock_ = 0;
      mr_->cache_ = new TinyHashMap(mr_->cbnum_);
      mr_->csiz_ = 0;
      if (!mr_->preprocess()) err_ = true;
      if (mr_->csiz_ > 0 && !mr_->flush_cache()) err_ = true;
      if (!mr_->logf("map", "started the map process: scale=%lld", (long long)scale_))
        err_ = true;
      stime_ = time();
    }
    /** postprocess the mappter and call the reducer */
    void visit_after() {
      if (mr_->csiz_ > 0 && !mr_->flush_cache()) err_ = true;
      double etime = time();
      if (!mr_->logf("map", "the map process finished: time=%.6f", etime - stime_))
        err_ = true;
      if (!mr_->midprocess()) err_ = true;
      if (mr_->csiz_ > 0 && !mr_->flush_cache()) err_ = true;
      delete mr_->cache_;
      if (mr_->flsths_ && !mr_->flsths_->empty()) {
        std::deque<FlushThread*>::iterator flthit = mr_->flsths_->begin();
        std::deque<FlushThread*>::iterator flthitend = mr_->flsths_->end();
        while (flthit != flthitend) {
          FlushThread* flth = *flthit;
          flth->join();
          if (flth->error()) err_ = true;
          delete flth;
          ++flthit;
        }
      }
      if (!err_ && !mr_->execute_reduce()) err_ = true;
      if (!mr_->postprocess()) err_ = true;
    }
   private:
    /** visit a record */
    const char* visit_full(const char* kbuf, size_t ksiz,
                           const char* vbuf, size_t vsiz, size_t* sp) {
      if (!mr_->map(kbuf, ksiz, vbuf, vsiz)) {
        checker_->stop();
        err_ = true;
      }
      if (mr_->rlocks_) {
        if (mr_->csiz_ >= mr_->clim_) {
          mr_->rlocks_->lock_all();
          if (mr_->csiz_ >= mr_->clim_ && !mr_->flush_cache()) {
            checker_->stop();
            err_ = true;
          }
          mr_->rlocks_->unlock_all();
        }
      } else {
        if (mr_->csiz_ >= mr_->clim_ && !mr_->flush_cache()) {
          checker_->stop();
          err_ = true;
        }
      }
      return NOP;
    }
    MapReduce* mr_;                      ///< driver
    MapChecker* checker_;                ///< checker
    int64_t scale_;                      ///< number of records
    double stime_;                       ///< start time
    bool err_;                           ///< error flag
  };
  /**
   * Front line of a merging list.
   */
  struct MergeLine {
    BasicDB::Cursor* cur;                ///< cursor
    Comparator* rcomp;                   ///< record comparator
    char* kbuf;                          ///< pointer to the key
    size_t ksiz;                         ///< size of the key
    const char* vbuf;                    ///< pointer to the value
    size_t vsiz;                         ///< size of the value
    /** comparing operator */
    bool operator <(const MergeLine& right) const {
      return rcomp->compare(kbuf, ksiz, right.kbuf, right.ksiz) > 0;
    }
  };
  /**
   * Process a log message.
   * @param name the name of the event.
   * @param format the printf-like format string.
   * @param ... used according to the format string.
   * @return true on success, or false on failure.
   */
  bool logf(const char* name, const char* format, ...) {
    _assert_(name && format);
    va_list ap;
    va_start(ap, format);
    std::string message;
    vstrprintf(&message, format, ap);
    va_end(ap);
    return log(name, message.c_str());
  }
  /**
   * Flush all cache records.
   * @return true on success, or false on failure.
   */
  bool flush_cache() {
    _assert_(true);
    bool err = false;
    BasicDB* tmpdb = tmpdbs_[dbclock_];
    dbclock_ = (dbclock_ + 1) % dbnum_;
    if (flsths_) {
      size_t num = flsths_->size();
      if (num >= flsthnum_ || num >= dbnum_) {
        FlushThread* flth = flsths_->front();
        flsths_->pop_front();
        flth->join();
        if (flth->error()) err = true;
        delete flth;
      }
      FlushThread* flth = new FlushThread(this, tmpdb, cache_, csiz_, true);
      cache_ = new TinyHashMap(cbnum_);
      csiz_ = 0;
      flth->start();
      flsths_->push_back(flth);
    } else {
      FlushThread flth(this, tmpdb, cache_, csiz_, false);
      flth.run();
      if (flth.error()) err = true;
      cache_->clear();
      csiz_ = 0;
    }
    return !err;
  }
  /**
   * Execute the reduce part.
   * @return true on success, or false on failure.
   */
  bool execute_reduce() {
    bool err = false;
    int64_t scale = 0;
    for (size_t i = 0; i < dbnum_; i++) {
      scale += tmpdbs_[i]->count();
    }
    if (!logf("reduce", "started the reduce process: scale=%lld", (long long)scale)) err = true;
    double stime = time();
    if (redtasks_) redtasks_->start(redthnum_);
    std::priority_queue<MergeLine> lines;
    for (size_t i = 0; i < dbnum_; i++) {
      MergeLine line;
      line.cur = tmpdbs_[i]->cursor();
      line.rcomp = rcomp_;
      line.cur->jump();
      line.kbuf = line.cur->get(&line.ksiz, &line.vbuf, &line.vsiz, true);
      if (line.kbuf) {
        lines.push(line);
      } else {
        delete line.cur;
      }
    }
    char* lkbuf = NULL;
    size_t lksiz = 0;
    Values values;
    while (!err && !lines.empty()) {
      MergeLine line = lines.top();
      lines.pop();
      if (lkbuf && (lksiz != line.ksiz || std::memcmp(lkbuf, line.kbuf, lksiz))) {
        if (!call_reducer(lkbuf, lksiz, values)) {
          db_->set_error(_KCCODELINE_, BasicDB::Error::LOGIC, "reducer failed");
          err = true;
        }
        values.clear();
      }
      delete[] lkbuf;
      lkbuf = line.kbuf;
      lksiz = line.ksiz;
      values.push_back(std::string(line.vbuf, line.vsiz));
      line.kbuf = line.cur->get(&line.ksiz, &line.vbuf, &line.vsiz, true);
      if (line.kbuf) {
        lines.push(line);
      } else {
        delete line.cur;
      }
    }
    if (lkbuf) {
      if (!err && !call_reducer(lkbuf, lksiz, values)) {
        db_->set_error(_KCCODELINE_, BasicDB::Error::LOGIC, "reducer failed");
        err = true;
      }
      delete[] lkbuf;
    }
    while (!lines.empty()) {
      MergeLine line = lines.top();
      lines.pop();
      delete[] line.kbuf;
      delete line.cur;
    }
    if (redtasks_) redtasks_->finish();
    double etime = time();
    if (!logf("reduce", "the reduce process finished: time=%.6f", etime - stime)) err = true;
    return !err;
  }
  /**
   * Call the reducer.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param values a vector of the values.
   * @return true on success, or false on failure.
   */
  bool call_reducer(const char* kbuf, size_t ksiz, const Values& values) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    if (redtasks_) {
      if (redaborted_) return false;
      ReduceTaskQueue::ReduceTask* task =
          new ReduceTaskQueue::ReduceTask(this, kbuf, ksiz, values);
      redtasks_->add_task(task);
      return true;
    }
    bool err = false;
    ValueIterator iter(values.begin(), values.end());
    if (!reduce(kbuf, ksiz, &iter)) err = true;
    return !err;
  }
  /** Dummy constructor to forbid the use. */
  MapReduce(const MapReduce&);
  /** Dummy Operator to forbid the use. */
  MapReduce& operator =(const MapReduce&);
  /** The internal database. */
  BasicDB* db_;
  /** The record comparator. */
  Comparator* rcomp_;
  /** The temporary databases. */
  BasicDB** tmpdbs_;
  /** The number of temporary databases. */
  size_t dbnum_;
  /** The logical clock for temporary databases. */
  int64_t dbclock_;
  /** The number of the mapper threads. */
  size_t mapthnum_;
  /** The number of the reducer threads. */
  size_t redthnum_;
  /** The number of the flusher threads. */
  size_t flsthnum_;
  /** The cache for emitter. */
  TinyHashMap* cache_;
  /** The current size of the cache for emitter. */
  AtomicInt64 csiz_;
  /** The limit size of the cache for emitter. */
  int64_t clim_;
  /** The bucket number of the cache for emitter. */
  int64_t cbnum_;
  /** The flush threads. */
  std::deque<FlushThread*>* flsths_;
  /** The task queue for parallel reducer. */
  TaskQueue* redtasks_;
  /** The flag whether aborted. */
  bool redaborted_;
  /** The whole lock. */
  SlottedMutex* rlocks_;
};


/**
 * Index database.
 * @note This class is designed to implement an indexing storage with an efficient appending
 * operation for the existing record values.  This class is a wrapper of the polymorphic
 * database, featuring buffering mechanism to alleviate IO overhead in the database layer.  This
 * class can be inherited but overwriting methods is forbidden.  Before every database operation,
 * it is necessary to call the IndexDB::open method in order to open a database file and connect
 * the database object to it.  To avoid data missing or corruption, it is important to close
 * every database file by the IndexDB::close method when the database is no longer in use.  It
 * is forbidden for multible database objects in a process to open the same database at the same
 * time.  It is forbidden to share a database object with child processes.
 */
class IndexDB {
 private:
  /** The default number of temporary databases. */
  static const size_t DEFDBNUM = 8;
  /** The maxinum number of temporary databases. */
  static const size_t MAXDBNUM = 256;
  /** The default cache limit size. */
  static const int64_t DEFCLIM = 256LL << 20;
  /** The default cache bucket number. */
  static const int64_t DEFCBNUM = 1048583LL;
  /** The bucket number of temprary databases. */
  static const int64_t DBBNUM = 512LL << 10;
  /** The page size of temprary databases. */
  static const int32_t DBPSIZ = 32768;
  /** The mapped size of temprary databases. */
  static const int64_t DBMSIZ = 516LL * 4096;
  /** The page cache capacity of temprary databases. */
  static const int64_t DBPCCAP = 16LL << 20;
 public:
  /**
   * Default constructor.
   */
  explicit IndexDB() :
      mlock_(), db_(), omode_(0),
      rcomp_(NULL), tmppath_(""), tmpdbs_(NULL), dbnum_(DEFDBNUM), dbclock_(0),
      cache_(NULL), csiz_(0), clim_(0) {
    _assert_(true);
  }
  /**
   * Destructor.
   * @note If the database is not closed, it is closed implicitly.
   */
  virtual ~IndexDB() {
    _assert_(true);
    if (omode_ != 0) close();
  }
  /**
   * Get the last happened error.
   * @return the last happened error.
   */
  BasicDB::Error error() const {
    _assert_(true);
    return db_.error();
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
                 BasicDB::Error::Code code, const char* message) {
    _assert_(file && line > 0 && func && message);
    db_.set_error(file, line, func, code, message);
  }
  /**
   * Set the error information without source code information.
   * @param code an error code.
   * @param message a supplement message.
   */
  void set_error(BasicDB::Error::Code code, const char* message) {
    _assert_(message);
    db_.set_error(_KCCODELINE_, code, message);
  }
  /**
   * Open a database file.
   * @param path the path of a database file.  The same as with PolyDB.  In addition, the
   * following tuning parameters are supported.  "idxclim" specifies the limit size of the
   * internal cache.  "idxcbnum" the bucket number of the internal cache.  "idxdbnum" specifies
   * the number of internal databases.  "idxtmppath' specifies the path of the temporary
   * directory.
   * @param mode the connection mode.  The same as with PolyDB.
   * @return true on success, or false on failure.
   * @note Every opened database must be closed by the IndexDB::close method when it is no longer
   * in use.  It is not allowed for two or more database objects in the same process to keep
   * their connections to the same database file at the same time.
   */
  bool open(const std::string& path = ":",
            uint32_t mode = BasicDB::OWRITER | BasicDB::OCREATE) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ != 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "already opened");
      return false;
    }
    std::vector<std::string> elems;
    strsplit(path, '#', &elems);
    int64_t clim = 0;
    int64_t cbnum = 0;
    size_t dbnum = 0;
    std::string tmppath = "";
    std::vector<std::string>::iterator it = elems.begin();
    std::vector<std::string>::iterator itend = elems.end();
    if (it != itend) ++it;
    while (it != itend) {
      std::vector<std::string> fields;
      if (strsplit(*it, '=', &fields) > 1) {
        const char* key = fields[0].c_str();
        const char* value = fields[1].c_str();
        if (!std::strcmp(key, "idxclim") || !std::strcmp(key, "idxcachelimit")) {
          clim = atoix(value);
        } else if (!std::strcmp(key, "idxcbnum") || !std::strcmp(key, "idxcachebuckets")) {
          cbnum = atoix(value);
        } else if (!std::strcmp(key, "idxdbnum")) {
          dbnum = atoix(value);
        } else if (!std::strcmp(key, "idxtmppath")) {
          tmppath = value;
        }
      }
      ++it;
    }
    if (!db_.open(path, mode)) return false;
    tmppath_ = tmppath;
    rcomp_ = LEXICALCOMP;
    BasicDB* idb = &db_;
    if (typeid(db_) == typeid(PolyDB)) {
      PolyDB* pdb = (PolyDB*)idb;
      idb = pdb->reveal_inner_db();
    }
    const std::type_info& info = typeid(*idb);
    if (info == typeid(GrassDB)) {
      GrassDB* gdb = (GrassDB*)idb;
      rcomp_ = gdb->rcomp();
    } else if (info == typeid(TreeDB)) {
      TreeDB* tdb = (TreeDB*)idb;
      rcomp_ = tdb->rcomp();
    } else if (info == typeid(ForestDB)) {
      ForestDB* fdb = (ForestDB*)idb;
      rcomp_ = fdb->rcomp();
    }
    dbnum_ = dbnum < MAXDBNUM ? dbnum : MAXDBNUM;
    dbclock_ = 0;
    if ((mode & BasicDB::OWRITER) && dbnum > 0) {
      tmpdbs_ = new BasicDB*[dbnum_];
      if (tmppath_.empty()) {
        report(_KCCODELINE_, "started to open temporary databases on memory");
        double stime = time();
        for (size_t i = 0; i < dbnum_; i++) {
          GrassDB* gdb = new GrassDB;
          gdb->tune_options(GrassDB::TCOMPRESS);
          gdb->tune_buckets(DBBNUM / 2);
          gdb->tune_page(DBPSIZ);
          gdb->tune_page_cache(DBPCCAP);
          gdb->tune_comparator(rcomp_);
          gdb->open("%", GrassDB::OWRITER | GrassDB::OCREATE | GrassDB::OTRUNCATE);
          tmpdbs_[i] = gdb;
        }
        double etime = time();
        report(_KCCODELINE_, "opening temporary databases finished: time=%.6f", etime - stime);
      } else {
        File::Status sbuf;
        if (!File::status(tmppath_, &sbuf) || !sbuf.isdir) {
          set_error(_KCCODELINE_, BasicDB::Error::NOREPOS, "no such directory");
          delete[] tmpdbs_;
          db_.close();
          return false;
        }
        report(_KCCODELINE_, "started to open temporary databases under %s", tmppath.c_str());
        double stime = time();
        uint32_t pid = getpid() & UINT16MAX;
        uint32_t tid = Thread::hash() & UINT16MAX;
        uint32_t ts = time() * 1000;
        bool err = false;
        for (size_t i = 0; i < dbnum_; i++) {
          std::string childpath =
              strprintf("%s%cidx-%04x-%04x-%08x-%03d%ckct",
                        tmppath_.c_str(), File::PATHCHR, pid, tid, ts,
                        (int)(i + 1), File::EXTCHR);
          TreeDB* tdb = new TreeDB;
          tdb->tune_options(TreeDB::TSMALL | TreeDB::TLINEAR);
          tdb->tune_buckets(DBBNUM);
          tdb->tune_page(DBPSIZ);
          tdb->tune_map(DBMSIZ);
          tdb->tune_page_cache(DBPCCAP);
          tdb->tune_comparator(rcomp_);
          if (!tdb->open(childpath, TreeDB::OWRITER | TreeDB::OCREATE | TreeDB::OTRUNCATE)) {
            const BasicDB::Error& e = tdb->error();
            set_error(_KCCODELINE_, e.code(), e.message());
            err = true;
          }
          tmpdbs_[i] = tdb;
        }
        double etime = time();
        report(_KCCODELINE_, "opening temporary databases finished: time=%.6f", etime - stime);
        if (err) {
          for (size_t i = 0; i < dbnum_; i++) {
            delete tmpdbs_[i];
          }
          delete[] tmpdbs_;
          db_.close();
          return false;
        }
      }
    } else {
      tmpdbs_ = NULL;
    }
    if (mode & BasicDB::OWRITER) {
      cache_ = new TinyHashMap(cbnum > 0 ? cbnum : DEFCBNUM);
    } else {
      cache_ = NULL;
    }
    clim_ = clim > 0 ? clim : DEFCLIM;
    csiz_ = 0;
    omode_ = mode;
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
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    bool err = false;
    if (cache_) {
      if (!flush_cache()) err = true;
      delete cache_;
      if (tmpdbs_) {
        if (!merge_tmpdbs()) err = true;
        report(_KCCODELINE_, "closing the temporary databases");
        double stime = time();
        for (size_t i = 0; i < dbnum_; i++) {
          BasicDB* tmpdb = tmpdbs_[i];
          const std::string& path = tmpdb->path();
          if (!tmpdb->close()) {
            const BasicDB::Error& e = tmpdb->error();
            set_error(_KCCODELINE_, e.code(), e.message());
            err = true;
          }
          if (!tmppath_.empty()) File::remove(path);
          delete tmpdb;
        }
        double etime = time();
        report(_KCCODELINE_, "closing the temporary databases finished: %.6f", etime - stime);
        delete[] tmpdbs_;
      }
    }
    if (!db_.close()) err = true;
    omode_ = 0;
    return !err;
  }
  /**
   * Set the value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   * @note If no record corresponds to the key, a new record is created.  If the corresponding
   * record exists, the value is overwritten.
   */
  bool set(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    bool err = false;
    if (!clean_dbs(kbuf, ksiz)) err = true;
    cache_->set(kbuf, ksiz, vbuf, vsiz);
    csiz_ += ksiz + vsiz;
    if (csiz_ > clim_ && !flush_cache()) err = false;
    return !err;
  }
  /**
   * Set the value of a record.
   * @note Equal to the original DB::set method except that the parameters are std::string.
   */
  bool set(const std::string& key, const std::string& value) {
    _assert_(true);
    return set(key.c_str(), key.size(), value.c_str(), value.size());
  }
  /**
   * Add a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   * @note If no record corresponds to the key, a new record is created.  If the corresponding
   * record exists, the record is not modified and false is returned.
   */
  bool add(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    if (check_impl(kbuf, ksiz)) {
      set_error(_KCCODELINE_, BasicDB::Error::DUPREC, "record duplication");
      return false;
    }
    bool err = false;
    cache_->set(kbuf, ksiz, vbuf, vsiz);
    csiz_ += ksiz + vsiz;
    if (csiz_ > clim_ && !flush_cache()) err = false;
    return !err;
  }
  /**
   * Set the value of a record.
   * @note Equal to the original DB::add method except that the parameters are std::string.
   */
  bool add(const std::string& key, const std::string& value) {
    _assert_(true);
    return add(key.c_str(), key.size(), value.c_str(), value.size());
  }
  /**
   * Replace the value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   * @note If no record corresponds to the key, no new record is created and false is returned.
   * If the corresponding record exists, the value is modified.
   */
  bool replace(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    if (!check_impl(kbuf, ksiz)) {
      set_error(_KCCODELINE_, BasicDB::Error::NOREC, "no record");
      return false;
    }
    bool err = false;
    if (!clean_dbs(kbuf, ksiz)) err = true;
    cache_->set(kbuf, ksiz, vbuf, vsiz);
    csiz_ += ksiz + vsiz;
    if (csiz_ > clim_ && !flush_cache()) err = false;
    return !err;
  }
  /**
   * Replace the value of a record.
   * @note Equal to the original DB::replace method except that the parameters are std::string.
   */
  bool replace(const std::string& key, const std::string& value) {
    _assert_(true);
    return replace(key.c_str(), key.size(), value.c_str(), value.size());
  }
  /**
   * Append the value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param vbuf the pointer to the value region.
   * @param vsiz the size of the value region.
   * @return true on success, or false on failure.
   * @note If no record corresponds to the key, a new record is created.  If the corresponding
   * record exists, the given value is appended at the end of the existing value.
   */
  bool append(const char* kbuf, size_t ksiz, const char* vbuf, size_t vsiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && vbuf && vsiz <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    bool err = false;
    cache_->append(kbuf, ksiz, vbuf, vsiz);
    csiz_ += ksiz + vsiz;
    if (csiz_ > clim_ && !flush_cache()) err = false;
    return !err;
  }
  /**
   * Set the value of a record.
   * @note Equal to the original DB::append method except that the parameters are std::string.
   */
  bool append(const std::string& key, const std::string& value) {
    _assert_(true);
    return append(key.c_str(), key.size(), value.c_str(), value.size());
  }
  /**
   * Remove a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @return true on success, or false on failure.
   * @note If no record corresponds to the key, false is returned.
   */
  bool remove(const char* kbuf, size_t ksiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    bool err = false;
    if (!clean_dbs(kbuf, ksiz)) err = true;
    cache_->remove(kbuf, ksiz);
    return !err;
  }
  /**
   * Remove a record.
   * @note Equal to the original DB::remove method except that the parameter is std::string.
   */
  bool remove(const std::string& key) {
    _assert_(true);
    return remove(key.c_str(), key.size());
  }
  /**
   * Retrieve the value of a record.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @param sp the pointer to the variable into which the size of the region of the return
   * value is assigned.
   * @return the pointer to the value region of the corresponding record, or NULL on failure.
   * @note If no record corresponds to the key, NULL is returned.  Because an additional zero
   * code is appended at the end of the region of the return value, the return value can be
   * treated as a C-style string.  Because the region of the return value is allocated with the
   * the new[] operator, it should be released with the delete[] operator when it is no longer
   * in use.
   */
  char* get(const char* kbuf, size_t ksiz, size_t* sp) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ && sp);
    ScopedRWLock lock(&mlock_, false);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      *sp = 0;
      return false;
    }
    if (!cache_) return db_.get(kbuf, ksiz, sp);
    size_t dvsiz = 0;
    char* dvbuf = db_.get(kbuf, ksiz, &dvsiz);
    size_t cvsiz = 0;
    const char* cvbuf = cache_->get(kbuf, ksiz, &cvsiz);
    struct Record {
      char* buf;
      size_t size;
    };
    Record* recs = NULL;
    bool hit = false;
    size_t rsiz = 0;
    if (tmpdbs_) {
      recs = new Record[dbnum_];
      for (size_t i = 0; i < dbnum_; i++) {
        BasicDB* tmpdb = tmpdbs_[i];
        Record* rp = recs + i;
        rp->buf = tmpdb->get(kbuf, ksiz, &rp->size);
        if (rp->buf) {
          rsiz += rp->size;
          hit = true;
        }
      }
    }
    if (!hit) {
      delete[] recs;
      if (!dvbuf && !cvbuf) {
        *sp = 0;
        return NULL;
      }
      if (!dvbuf) {
        dvbuf = new char[cvsiz+1];
        std::memcpy(dvbuf, cvbuf, cvsiz);
        *sp = cvsiz;
        return dvbuf;
      }
      if (!cvbuf) {
        *sp = dvsiz;
        return dvbuf;
      }
      char* rbuf = new char[dvsiz+cvsiz+1];
      std::memcpy(rbuf, dvbuf, dvsiz);
      std::memcpy(rbuf + dvsiz, cvbuf, cvsiz);
      delete[] dvbuf;
      *sp = dvsiz + cvsiz;
      return rbuf;
    }
    if (dvbuf) rsiz += dvsiz;
    if (cvbuf) rsiz += cvsiz;
    char* rbuf = new char[rsiz+1];
    char* wp = rbuf;
    if (dvbuf) {
      std::memcpy(wp, dvbuf, dvsiz);
      wp += dvsiz;
      delete[] dvbuf;
    }
    if (cvbuf) {
      std::memcpy(wp, cvbuf, cvsiz);
      wp += cvsiz;
    }
    if (recs) {
      for (size_t i = 0; i < dbnum_; i++) {
        Record* rp = recs + i;
        if (rp->buf) {
          std::memcpy(wp, rp->buf, rp->size);
          wp += rp->size;
          delete[] rp->buf;
        }
      }
      delete[] recs;
    }
    *sp = rsiz;
    return rbuf;
  }
  /**
   * Retrieve the value of a record.
   * @note Equal to the original DB::get method except that the first parameters is the key
   * string and the second parameter is a string to contain the result and the return value is
   * bool for success.
   */
  bool get(const std::string& key, std::string* value) {
    _assert_(value);
    size_t vsiz;
    char* vbuf = get(key.c_str(), key.size(), &vsiz);
    if (!vbuf) return false;
    value->clear();
    value->append(vbuf, vsiz);
    delete[] vbuf;
    return true;
  }
  /**
   * Synchronize updated contents with the file and the device.
   * @param hard true for physical synchronization with the device, or false for logical
   * synchronization with the file system.
   * @param proc a postprocessor object.  If it is NULL, no postprocessing is performed.
   * @return true on success, or false on failure.
   * @note The operation of the postprocessor is performed atomically and other threads accessing
   * the same record are blocked.  To avoid deadlock, any explicit database operation must not
   * be performed in this function.
   */
  bool synchronize(bool hard = false, BasicDB::FileProcessor* proc = NULL) {
    _assert_(true);
    ScopedRWLock lock(&mlock_, true);
    if (omode_ == 0) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    bool err = false;
    if (!flush_cache()) err = true;
    if (tmpdbs_ && !merge_tmpdbs()) err = true;
    if (!db_.synchronize(hard, proc)) err = true;
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
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "not opened");
      return false;
    }
    if (!cache_) {
      set_error(_KCCODELINE_, BasicDB::Error::INVALID, "permission denied");
      return false;
    }
    cache_->clear();
    csiz_ = 0;
    return db_.clear();
  }
  /**
   * Get the number of records.
   * @return the number of records, or -1 on failure.
   */
  int64_t count() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    return count_impl();
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes, or -1 on failure.
   */
  int64_t size() {
    _assert_(true);
    ScopedRWLock lock(&mlock_, false);
    return size_impl();
  }
  /**
   * Get the path of the database file.
   * @return the path of the database file, or an empty string on failure.
   */
  std::string path() {
    _assert_(true);
    return db_.path();
  }
  /**
   * Get the miscellaneous status information.
   * @param strmap a string map to contain the result.
   * @return true on success, or false on failure.
   */
  bool status(std::map<std::string, std::string>* strmap) {
    _assert_(strmap);
    return db_.status(strmap);
  }
  /**
   * Reveal the inner database object.
   * @return the inner database object, or NULL on failure.
   */
  PolyDB* reveal_inner_db() {
    _assert_(true);
    return &db_;
  }
  /**
   * Create a cursor object.
   * @return the return value is the created cursor object.
   * @note Because the object of the return value is allocated by the constructor, it should be
   * released with the delete operator when it is no longer in use.
   */
  BasicDB::Cursor* cursor() {
    _assert_(true);
    return db_.cursor();
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
  void log(const char* file, int32_t line, const char* func, BasicDB::Logger::Kind kind,
           const char* message) {
    _assert_(file && line > 0 && func && message);
    db_.log(file, line, func, kind, message);
  }
  /**
   * Set the internal logger.
   * @param logger the logger object.
   * @param kinds kinds of logged messages by bitwise-or: Logger::DEBUG for debugging,
   * Logger::INFO for normal information, Logger::WARN for warning, and Logger::ERROR for fatal
   * error.
   * @return true on success, or false on failure.
   */
  bool tune_logger(BasicDB::Logger* logger,
                   uint32_t kinds = BasicDB::Logger::WARN | BasicDB::Logger::ERROR) {
    _assert_(logger);
    return db_.tune_logger(logger, kinds);
  }
  /**
   * Set the internal meta operation trigger.
   * @param trigger the trigger object.
   * @return true on success, or false on failure.
   */
  bool tune_meta_trigger(BasicDB::MetaTrigger* trigger) {
    _assert_(trigger);
    return db_.tune_meta_trigger(trigger);
  }
 protected:
  /**
   * Report a message for debugging.
   * @param file the file name of the program source code.
   * @param line the line number of the program source code.
   * @param func the function name of the program source code.
   * @param format the printf-like format string.
   * @param ... used according to the format string.
   */
  void report(const char* file, int32_t line, const char* func, const char* format, ...) {
    _assert_(file && line > 0 && func && format);
    std::string message;
    va_list ap;
    va_start(ap, format);
    vstrprintf(&message, format, ap);
    va_end(ap);
    db_.log(file, line, func, BasicDB::Logger::INFO, message.c_str());
  }
 private:
  /**
   * Flush all cache records.
   * @return true on success, or false on failure.
   */
  bool flush_cache() {
    _assert_(true);
    bool err = false;
    double stime = time();
    report(_KCCODELINE_, "flushing the cache");
    if (tmpdbs_) {
      BasicDB* tmpdb = tmpdbs_[dbclock_];
      TinyHashMap::Sorter sorter(cache_);
      const char* kbuf, *vbuf;
      size_t ksiz, vsiz;
      while ((kbuf = sorter.get(&ksiz, &vbuf, &vsiz)) != NULL) {
        if (!tmpdb->append(kbuf, ksiz, vbuf, vsiz)) {
          const BasicDB::Error& e = tmpdb->error();
          db_.set_error(_KCCODELINE_, e.code(), e.message());
          err = true;
        }
        sorter.step();
      }
      dbclock_ = (dbclock_ + 1) % dbnum_;
    } else {
      TinyHashMap::Sorter sorter(cache_);
      const char* kbuf, *vbuf;
      size_t ksiz, vsiz;
      while ((kbuf = sorter.get(&ksiz, &vbuf, &vsiz)) != NULL) {
        if (!db_.append(kbuf, ksiz, vbuf, vsiz)) err = true;
        sorter.step();
      }
    }
    cache_->clear();
    csiz_ = 0;
    double etime = time();
    report(_KCCODELINE_, "flushing the cache finished: time=%.6f", etime - stime);
    return !err;
  }
  /**
   * Merge temporary databases.
   * @return true on success, or false on failure.
   */
  bool merge_tmpdbs() {
    _assert_(true);
    bool err = false;
    report(_KCCODELINE_, "merging the temporary databases");
    double stime = time();
    if (!db_.merge(tmpdbs_, dbnum_, PolyDB::MAPPEND)) err = true;
    dbclock_ = 0;
    for (size_t i = 0; i < dbnum_; i++) {
      BasicDB* tmpdb = tmpdbs_[i];
      if (!tmpdb->clear()) {
        const BasicDB::Error& e = tmpdb->error();
        set_error(_KCCODELINE_, e.code(), e.message());
        err = true;
      }
    }
    double etime = time();
    report(_KCCODELINE_, "merging the temporary databases finished: %.6f", etime - stime);
    return !err;
  }
  /**
   * Remove a record from databases.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @return true on success, or false on failure.
   */
  bool clean_dbs(const char* kbuf, size_t ksiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    if (db_.remove(kbuf, ksiz)) return true;
    bool err = false;
    if (db_.error() != BasicDB::Error::NOREC) err = true;
    if (tmpdbs_) {
      for (size_t i = 0; i < dbnum_; i++) {
        BasicDB* tmpdb = tmpdbs_[i];
        if (!tmpdb->remove(kbuf, ksiz)) {
          const BasicDB::Error& e = tmpdb->error();
          if (e != BasicDB::Error::NOREC) {
            set_error(_KCCODELINE_, e.code(), e.message());
            err = true;
          }
        }
      }
    }
    return !err;
  }
  /**
   * Check whether a record exists.
   * @param kbuf the pointer to the key region.
   * @param ksiz the size of the key region.
   * @return true if the record exists, or false if not.
   */
  bool check_impl(const char* kbuf, size_t ksiz) {
    _assert_(kbuf && ksiz <= MEMMAXSIZ);
    char vbuf;
    if (db_.get(kbuf, ksiz, &vbuf, 1) >= 0) return true;
    if (cache_) {
      size_t vsiz;
      if (cache_->get(kbuf, ksiz, &vsiz)) return true;
      if (tmpdbs_) {
        for (size_t i = 0; i < dbnum_; i++) {
          BasicDB* tmpdb = tmpdbs_[i];
          if (tmpdb->get(kbuf, ksiz, &vbuf, 1)) return true;
        }
      }
    }
    return false;
  }
  /**
   * Get the number of records.
   * @return the number of records, or -1 on failure.
   */
  int64_t count_impl() {
    _assert_(true);
    int64_t dbcnt = db_.count();
    if (dbcnt < 0) return -1;
    if (!cache_) return dbcnt;
    int64_t ccnt = cache_->count();
    return dbcnt > ccnt ? dbcnt : ccnt;
  }
  /**
   * Get the size of the database file.
   * @return the size of the database file in bytes.
   */
  int64_t size_impl() {
    _assert_(true);
    int64_t dbsiz = db_.size();
    if (dbsiz < 0) return -1;
    return dbsiz + csiz_;
  }
  /** Dummy constructor to forbid the use. */
  IndexDB(const IndexDB&);
  /** Dummy Operator to forbid the use. */
  IndexDB& operator =(const IndexDB&);
  /** The method lock. */
  RWLock mlock_;
  /** The internal database. */
  PolyDB db_;
  /** The open mode. */
  uint32_t omode_;
  /** The record comparator. */
  Comparator* rcomp_;
  /** The base path of temporary databases. */
  std::string tmppath_;
  /** The temporary databases. */
  BasicDB** tmpdbs_;
  /** The number of temporary databases. */
  size_t dbnum_;
  /** The logical clock for temporary databases. */
  int64_t dbclock_;
  /** The internal cache. */
  TinyHashMap* cache_;
  /** The current size of the internal cache. */
  int64_t csiz_;
  /** The limit size of the internal cache. */
  int64_t clim_;
};


}                                        // common namespace

#endif                                   // duplication check

// END OF FILE
