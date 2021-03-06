// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/src/storage/fake/fake_journal.h"

#include <string>

namespace storage {
namespace fake {

FakeJournal::FakeJournal(FakeJournalDelegate* delegate) : delegate_(delegate) {}

FakeJournal::~FakeJournal() {}

Status FakeJournal::Put(convert::ExtendedStringView key,
                        ObjectIdView object_id,
                        KeyPriority priority) {
  return delegate_->SetValue(key, object_id, priority);
}

Status FakeJournal::Delete(convert::ExtendedStringView key) {
  return delegate_->Delete(key);
}

void FakeJournal::Commit(
    std::function<void(Status, const CommitId&)> callback) {
  delegate_->Commit(callback);
}

Status FakeJournal::Rollback() {
  return delegate_->Rollback();
}

}  // namespace fake
}  // namespace storage
