/*
 * Copyright (C) 2005-2015 Christoph Rupp (chris@crupp.de).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Test gateway for LsnManager
 *
 * @exception_safe: nothrow
 * @thread_safe: no
 */
 
#ifndef HAM_LSN_MANAGER_TEST_H
#define HAM_LSN_MANAGER_TEST_H

#include "0root/root.h"

#include "2lsn_manager/lsn_manager.h"

#ifndef HAM_ROOT_H
#  error "root.h was not included"
#endif

namespace hamsterdb {

struct LsnManagerTest
{
  // Constructor
  LsnManagerTest(LsnManager *lsn_manager)
    : m_state(lsn_manager->m_state) {
  }

  // Returns the current lsn
  uint64_t lsn() const {
    return (m_state);
  }

  uint64_t &m_state;
};

} // namespace hamsterdb

#endif /* HAM_LSN_MANAGER_TEST_H */
