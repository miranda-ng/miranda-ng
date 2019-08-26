﻿/*
 * Copyright 2017-2019 Leonid Yuriev <leo@yuriev.ru>
 * and other libmdbx authors: please see AUTHORS file.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#include "test.h"

void __noreturn usage(void) {
  printf("usage:\n"
         "\tFIXME\n");
  exit(EXIT_FAILURE);
}

//-----------------------------------------------------------------------------

void actor_params::set_defaults(const std::string &tmpdir) {
  pathname_log = "";
  loglevel =
#ifdef NDEBUG
      logging::info;
#elif defined(_WIN32) || defined(_WIN64)
      logging::verbose;
#else
      logging::trace;
#endif

  pathname_db = tmpdir + "mdbx-test.db";
  mode_flags = MDBX_NOSUBDIR | MDBX_WRITEMAP | MDBX_MAPASYNC | MDBX_NORDAHEAD |
               MDBX_NOMEMINIT | MDBX_COALESCE | MDBX_LIFORECLAIM;
  table_flags = MDBX_DUPSORT;

  size_lower = -1;
  size_now =
      intptr_t(1024) * 1024 * ((table_flags & MDBX_DUPSORT) ? 256 : 1024);
  size_upper = -1;
  shrink_threshold = -1;
  growth_step = -1;
  pagesize = -1;

  keygen.seed = 1;
  keygen.keycase = kc_random;
  keygen.width = (table_flags & MDBX_DUPSORT) ? 32 : 64;
  keygen.mesh = keygen.width;
  keygen.split = keygen.width / 2;
  keygen.rotate = 3;
  keygen.offset = 41;

  test_duration = 0;
  test_nops = 1000;
  nrepeat = 1;
  nthreads = 1;

  keylen_min = mdbx_keylen_min();
  keylen_max = mdbx_keylen_max();
  datalen_min = mdbx_datalen_min();
  datalen_max = std::min(mdbx_datalen_max(), 256u * 1024 + 42);

  batch_read = 42;
  batch_write = 42;

  delaystart = 0;
  waitfor_nops = 0;
  inject_writefaultn = 0;

  drop_table = false;
  ignore_dbfull = false;

  max_readers = 42;
  max_tables = 42;

  global::config::timeout_duration_seconds = 0 /* infinite */;
  global::config::dump_config = true;
  global::config::cleanup_before = true;
  global::config::cleanup_after = true;
  global::config::failfast = true;
  global::config::progress_indicator = osal_istty(STDERR_FILENO);
}

namespace global {

std::vector<actor_config> actors;
std::unordered_map<unsigned, actor_config *> events;
std::unordered_map<mdbx_pid_t, actor_config *> pid2actor;
std::set<std::string> databases;
unsigned nactors;
chrono::time start_motonic;
chrono::time deadline_motonic;
bool singlemode;

namespace config {
unsigned timeout_duration_seconds;
bool dump_config;
bool cleanup_before;
bool cleanup_after;
bool failfast;
bool progress_indicator;
} /* namespace config */

} /* namespace global */

//-----------------------------------------------------------------------------

const char global::thunk_param_prefix[] = "--execute=";

std::string thunk_param(const actor_config &config) {
  return config.serialize(global::thunk_param_prefix);
}

void cleanup() {
  log_trace(">> cleanup");
  /* TODO: remove each database */
  log_trace("<< cleanup");
}

int main(int argc, char *const argv[]) {

#ifdef _DEBUG
  log_trace("#argc = %d", argc);
  for (int i = 0; i < argc; ++i)
    log_trace("#argv[%d] = %s", i, argv[i]);
#endif /* _DEBUG */

  if (argc < 2)
    failure("No parameters given\n");

  if (argc == 2 && strncmp(argv[1], global::thunk_param_prefix,
                           strlen(global::thunk_param_prefix)) == 0)
    return test_execute(
               actor_config(argv[1] + strlen(global::thunk_param_prefix)))
               ? EXIT_SUCCESS
               : EXIT_FAILURE;

  actor_params params;
  params.set_defaults(osal_tempdir());
  global::config::dump_config = true;
  logging::setup((logging::loglevel)params.loglevel, "main");
  unsigned last_space_id = 0;

  for (int narg = 1; narg < argc; ++narg) {
    const char *value = nullptr;

    if (config::parse_option(argc, argv, narg, "case", &value)) {
      testcase_setup(value, params, last_space_id);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "pathname", params.pathname_db))
      continue;
    if (config::parse_option(argc, argv, narg, "mode", params.mode_flags,
                             config::mode_bits))
      continue;
    if (config::parse_option(argc, argv, narg, "table", params.table_flags,
                             config::table_bits)) {
      if ((params.table_flags & MDBX_DUPFIXED) == 0)
        params.table_flags &= ~MDBX_INTEGERDUP;
      if ((params.table_flags & MDBX_DUPSORT) == 0)
        params.table_flags &=
            ~(MDBX_DUPFIXED | MDBX_REVERSEDUP | MDBX_INTEGERDUP);
      continue;
    }

    if (config::parse_option(argc, argv, narg, "pagesize", params.pagesize,
                             mdbx_limits_pgsize_min(),
                             mdbx_limits_pgsize_max())) {
      const unsigned keylen_max = params.mdbx_keylen_max();
      if (params.keylen_min > keylen_max)
        params.keylen_min = keylen_max;
      if (params.keylen_max > keylen_max)
        params.keylen_max = keylen_max;
      const unsigned datalen_max = params.mdbx_datalen_max();
      if (params.datalen_min > datalen_max)
        params.datalen_min = datalen_max;
      if (params.datalen_max > datalen_max)
        params.datalen_max = datalen_max;
      continue;
    }
    if (config::parse_option_intptr(argc, argv, narg, "size-lower",
                                    params.size_lower,
                                    mdbx_limits_dbsize_min(params.pagesize),
                                    mdbx_limits_dbsize_max(params.pagesize)))
      continue;
    if (config::parse_option_intptr(argc, argv, narg, "size-upper",
                                    params.size_upper,
                                    mdbx_limits_dbsize_min(params.pagesize),
                                    mdbx_limits_dbsize_max(params.pagesize)))
      continue;
    if (config::parse_option_intptr(argc, argv, narg, "size", params.size_now,
                                    mdbx_limits_dbsize_min(params.pagesize),
                                    mdbx_limits_dbsize_max(params.pagesize)))
      continue;
    if (config::parse_option(
            argc, argv, narg, "shrink-threshold", params.shrink_threshold, 0,
            (int)std::min((intptr_t)INT_MAX,
                          mdbx_limits_dbsize_max(params.pagesize) -
                              mdbx_limits_dbsize_min(params.pagesize))))
      continue;
    if (config::parse_option(
            argc, argv, narg, "growth-step", params.growth_step, 0,
            (int)std::min((intptr_t)INT_MAX,
                          mdbx_limits_dbsize_max(params.pagesize) -
                              mdbx_limits_dbsize_min(params.pagesize))))
      continue;

    if (config::parse_option(argc, argv, narg, "keygen.width",
                             params.keygen.width, 1, 64))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.mesh",
                             params.keygen.mesh, 1, 64))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.seed",
                             params.keygen.seed, config::no_scale))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.split",
                             params.keygen.split, 1, 64))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.rotate",
                             params.keygen.rotate, 1, 64))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.offset",
                             params.keygen.offset, config::binary))
      continue;
    if (config::parse_option(argc, argv, narg, "keygen.case", &value)) {
      keycase_setup(value, params);
      continue;
    }

    if (config::parse_option(argc, argv, narg, "repeat", params.nrepeat,
                             config::no_scale))
      continue;
    if (config::parse_option(argc, argv, narg, "threads", params.nthreads,
                             config::no_scale, 1, 64))
      continue;
    if (config::parse_option(argc, argv, narg, "timeout",
                             global::config::timeout_duration_seconds,
                             config::duration, 1))
      continue;
    if (config::parse_option(argc, argv, narg, "keylen.min", params.keylen_min,
                             config::no_scale, params.mdbx_keylen_min(),
                             params.mdbx_keylen_max())) {
      if ((params.table_flags & MDBX_INTEGERKEY) ||
          params.keylen_max < params.keylen_min)
        params.keylen_max = params.keylen_min;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "keylen.max", params.keylen_max,
                             config::no_scale, params.mdbx_keylen_min(),
                             params.mdbx_keylen_max())) {
      if ((params.table_flags & MDBX_INTEGERKEY) ||
          params.keylen_min > params.keylen_max)
        params.keylen_min = params.keylen_max;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "datalen.min",
                             params.datalen_min, config::no_scale,
                             params.mdbx_datalen_min(),
                             params.mdbx_datalen_max())) {
      if ((params.table_flags & MDBX_DUPFIXED) ||
          params.datalen_max < params.datalen_min)
        params.datalen_max = params.datalen_min;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "datalen.max",
                             params.datalen_max, config::no_scale,
                             params.mdbx_datalen_min(),
                             params.mdbx_datalen_max())) {
      if ((params.table_flags & MDBX_DUPFIXED) ||
          params.datalen_min > params.datalen_max)
        params.datalen_min = params.datalen_max;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "batch.read", params.batch_read,
                             config::no_scale, 1))
      continue;
    if (config::parse_option(argc, argv, narg, "batch.write",
                             params.batch_write, config::no_scale, 1))
      continue;
    if (config::parse_option(argc, argv, narg, "delay", params.delaystart,
                             config::duration))
      continue;
    if (config::parse_option(argc, argv, narg, "wait4ops", params.waitfor_nops,
                             config::decimal))
      continue;
    if (config::parse_option(argc, argv, narg, "inject-writefault",
                             params.inject_writefaultn, config::decimal))
      continue;
    if (config::parse_option(argc, argv, narg, "drop", params.drop_table))
      continue;
    if (config::parse_option(argc, argv, narg, "ignore-dbfull",
                             params.ignore_dbfull))
      continue;
    if (config::parse_option(argc, argv, narg, "dump-config",
                             global::config::dump_config))
      continue;
    if (config::parse_option(argc, argv, narg, "cleanup-before",
                             global::config::cleanup_before))
      continue;
    if (config::parse_option(argc, argv, narg, "cleanup-after",
                             global::config::cleanup_after))
      continue;
    if (config::parse_option(argc, argv, narg, "max-readers",
                             params.max_readers, config::no_scale, 1, 255))
      continue;
    if (config::parse_option(argc, argv, narg, "max-tables", params.max_tables,
                             config::no_scale, 1, INT16_MAX))
      continue;

    if (config::parse_option(argc, argv, narg, "no-delay", nullptr)) {
      params.delaystart = 0;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "no-wait", nullptr)) {
      params.waitfor_nops = 0;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "duration", params.test_duration,
                             config::duration, 1)) {
      params.test_nops = 0;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "nops", params.test_nops,
                             config::decimal, 1)) {
      params.test_duration = 0;
      continue;
    }
    if (config::parse_option(argc, argv, narg, "hill", &value, "auto")) {
      configure_actor(last_space_id, ac_hill, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "jitter", nullptr)) {
      configure_actor(last_space_id, ac_jitter, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "dead.reader", nullptr)) {
      configure_actor(last_space_id, ac_deadread, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "dead.writer", nullptr)) {
      configure_actor(last_space_id, ac_deadwrite, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "copy", nullptr)) {
      configure_actor(last_space_id, ac_copy, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "append", nullptr)) {
      configure_actor(last_space_id, ac_append, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "ttl", nullptr)) {
      configure_actor(last_space_id, ac_ttl, value, params);
      continue;
    }
    if (config::parse_option(argc, argv, narg, "failfast",
                             global::config::failfast))
      continue;
    if (config::parse_option(argc, argv, narg, "progress",
                             global::config::progress_indicator))
      continue;

    if (*argv[narg] != '-')
      testcase_setup(argv[narg], params, last_space_id);
    else
      failure("Unknown option '%s'\n", argv[narg]);
  }

  if (global::config::dump_config)
    config::dump();

  //--------------------------------------------------------------------------

  if (global::actors.empty()) {
    log_notice("no testcase(s) configured, exiting");
    return EXIT_SUCCESS;
  }

  bool failed = false;
  global::start_motonic = chrono::now_motonic();
  global::deadline_motonic.fixedpoint =
      (global::config::timeout_duration_seconds == 0)
          ? chrono::infinite().fixedpoint
          : global::start_motonic.fixedpoint +
                chrono::from_seconds(global::config::timeout_duration_seconds)
                    .fixedpoint;

  if (global::config::cleanup_before)
    cleanup();

  log_trace(">> probe entropy_ticks()");
  entropy_ticks();
  log_trace("<< probe entropy_ticks()");

  if (global::actors.size() == 1) {
    logging::setup("main");
    global::singlemode = true;
    if (!test_execute(global::actors.front()))
      failed = true;
  } else {
    logging::setup("overlord");

    log_trace("=== preparing...");
    log_trace(">> osal_setup");
    osal_setup(global::actors);
    log_trace("<< osal_setup");

    for (auto &a : global::actors) {
      mdbx_pid_t pid;
      log_trace(">> actor_start");
      int rc = osal_actor_start(a, pid);
      log_trace("<< actor_start");
      if (rc) {
        log_trace(">> killall_actors: (%s)", "start failed");
        osal_killall_actors();
        log_trace("<< killall_actors");
        failure("Failed to start actor #%u (%s)\n", a.actor_id,
                test_strerror(rc));
      }
      global::pid2actor[pid] = &a;
    }

    log_trace("=== ready to start...");
    atexit(osal_killall_actors);
    log_trace(">> wait4barrier");
    osal_wait4barrier();
    log_trace("<< wait4barrier");

    size_t left = global::actors.size();
    log_trace("=== polling...");
    while (left > 0) {
      unsigned timeout_seconds_left = INT_MAX;
      chrono::time now_motonic = chrono::now_motonic();
      if (now_motonic.fixedpoint >= global::deadline_motonic.fixedpoint)
        timeout_seconds_left = 0;
      else {
        chrono::time left_motonic;
        left_motonic.fixedpoint =
            global::deadline_motonic.fixedpoint - now_motonic.fixedpoint;
        timeout_seconds_left = left_motonic.seconds();
      }

      mdbx_pid_t pid;
      int rc = osal_actor_poll(pid, timeout_seconds_left);
      if (rc)
        failure("Poll error: %s (%d)\n", test_strerror(rc), rc);

      if (pid) {
        actor_status status = osal_actor_info(pid);
        actor_config *actor = global::pid2actor.at(pid);
        if (!actor)
          continue;

        log_info("actor #%u, id %d, pid %u: %s\n", actor->actor_id,
                 actor->space_id, pid, status2str(status));
        if (status > as_running) {
          left -= 1;
          if (status != as_successful) {
            if (global::config::failfast && !failed) {
              log_trace(">> killall_actors: (%s)", "failfast");
              osal_killall_actors();
              log_trace("<< killall_actors");
            }
            failed = true;
          }
        }
      } else {
        if (timeout_seconds_left == 0)
          failure("Timeout\n");
      }
    }
    log_trace("=== done...");
  }

  log_notice("RESULT: %s\n", failed ? "Failed" : "Successful");
  if (global::config::cleanup_before) {
    if (failed)
      log_info("skip cleanup");
    else
      cleanup();
  }
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
