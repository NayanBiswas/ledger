// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_STORAGE_IMPL_BTREE_COMMIT_CONTENTS_IMPL_H_
#define APPS_LEDGER_SRC_STORAGE_IMPL_BTREE_COMMIT_CONTENTS_IMPL_H_

#include <memory>
#include <string>

#include "apps/ledger/src/convert/convert.h"
#include "apps/ledger/src/storage/impl/btree/btree_iterator.h"
#include "apps/ledger/src/storage/public/commit_contents.h"
#include "apps/ledger/src/storage/public/page_storage.h"
#include "apps/ledger/src/storage/public/types.h"

namespace storage {

// B-Tree implementation of |CommitContents|.
class CommitContentsImpl : public CommitContents {
 public:
  CommitContentsImpl(ObjectIdView root_id, PageStorage* page_storage);
  ~CommitContentsImpl() override;

  // CommitContents:
  std::unique_ptr<Iterator<const Entry>> begin() const override;

  std::unique_ptr<Iterator<const Entry>> find(
      convert::ExtendedStringView key) const override;

  void diff(
      std::unique_ptr<CommitContents> other,
      std::function<void(Status, std::unique_ptr<Iterator<const EntryChange>>)>
          callback) const override;

  ObjectId GetBaseObjectId() const override;

 private:
  std::unique_ptr<BTreeIterator> NewIterator() const;

  const ObjectId root_id_;
  PageStorage* page_storage_;
};

}  // namespace storage

#endif  // APPS_LEDGER_SRC_STORAGE_IMPL_BTREE_COMMIT_CONTENTS_IMPL_H_
