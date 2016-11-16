// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_BACKOFF_BACKOFF_H_
#define APPS_LEDGER_SRC_BACKOFF_BACKOFF_H_

#include "lib/ftl/macros.h"
#include "lib/ftl/time/time_delta.h"

namespace backoff {

// Interface for a backoff policy.
class Backoff {
 public:
  Backoff() {}
  virtual ~Backoff() {}

  virtual ftl::TimeDelta GetNext() = 0;
  virtual void Reset() = 0;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(Backoff);
};

}  // namespace backoff

#endif  // APPS_LEDGER_SRC_BACKOFF_BACKOFF_H_
