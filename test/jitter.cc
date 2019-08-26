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

bool testcase_jitter::run() {
  while (should_continue()) {
    jitter_delay();
    db_open();

    if (flipcoin()) {
      jitter_delay();
      txn_begin(true);
      fetch_canary();
      jitter_delay();
      txn_end(flipcoin());
    }

    jitter_delay();
    txn_begin(mode_readonly());
    jitter_delay();
    if (!mode_readonly()) {
      fetch_canary();
      update_canary(1);
      /* TODO:
       *  - db_setsize()
       *  ...
       */
    }
    txn_end(flipcoin());

    if (flipcoin()) {
      jitter_delay();
      txn_begin(true);
      jitter_delay();
      txn_end(flipcoin());
    }

    jitter_delay();
    db_close();

    /* just 'align' nops with other tests with batching */
    const auto batching =
        std::max(config.params.batch_read, config.params.batch_write);
    report(std::max(1u, batching / 2));
  }
  return true;
}
