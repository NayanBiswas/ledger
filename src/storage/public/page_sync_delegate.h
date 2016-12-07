// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_STORAGE_PUBLIC_PAGE_SYNC_DELEGATE_H_
#define APPS_LEDGER_SRC_STORAGE_PUBLIC_PAGE_SYNC_DELEGATE_H_

#include <functional>

#include <mx/socket.h>

#include "apps/ledger/src/storage/public/types.h"
#include "lib/ftl/macros.h"

namespace storage {

// Delegate interface for PageStorage responsible for retrieving on-demand
// storage objects from the cloud.
class PageSyncDelegate {
 public:
  PageSyncDelegate() {}
  virtual ~PageSyncDelegate() {}

  // Retrieves the object of the given id from the cloud. The size of the object
  // is passed to the callback along with the socket handle, so that the
  // client can verify that all data was streamed when draining the socket.
  virtual void GetObject(
      ObjectIdView object_id,
      std::function<void(Status status, uint64_t size, mx::socket data)>
          callback) = 0;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(PageSyncDelegate);
};

}  // namespace storage

#endif  // APPS_LEDGER_SRC_STORAGE_PUBLIC_PAGE_SYNC_DELEGATE_H_
