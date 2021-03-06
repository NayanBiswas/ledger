// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_STORAGE_PUBLIC_OBJECT_H_
#define APPS_LEDGER_SRC_STORAGE_PUBLIC_OBJECT_H_

#include <vector>

#include "apps/ledger/src/storage/public/types.h"
#include "lib/ftl/macros.h"
#include "lib/ftl/strings/string_view.h"

namespace storage {

class Object {
 public:
  Object() {}
  virtual ~Object() {}

  // Returns the id of this storage object.
  virtual ObjectId GetId() const = 0;

  // Returns the data of this object.
  virtual Status GetData(ftl::StringView* data) const = 0;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(Object);
};

}  // namespace storage

#endif  // APPS_LEDGER_SRC_STORAGE_PUBLIC_OBJECT_H_
