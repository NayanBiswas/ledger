// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_STORAGE_PUBLIC_ITERATOR_H_
#define APPS_LEDGER_SRC_STORAGE_PUBLIC_ITERATOR_H_

#include "apps/ledger/src/storage/public/types.h"
#include "lib/ftl/macros.h"

namespace storage {

// A non-copyable iterator over a collection of elements of type T.
template <class T>
class Iterator {
 public:
  Iterator() {}
  virtual ~Iterator() {}

  // Advances to the next element in the collection. Should only be called on a
  // valid iterator.
  virtual Iterator<T>& Next() = 0;
  // Returns false if no elements are available in the collection, or an error
  // occurred. It is invalid to dereference this iterator or to call |Next()| if
  // Valid() returns false.
  virtual bool Valid() const = 0;
  // Returns the current status of the iterator. It returns the error status if
  // an error occurred, Status::OK otherwise. Reaching the end of the iterator
  // is not an error.
  virtual Status GetStatus() const = 0;

  virtual T& operator*() const = 0;
  virtual T* operator->() const = 0;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(Iterator);
};

}  // namespace storage

#endif  // APPS_LEDGER_SRC_STORAGE_PUBLIC_ITERATOR_H_
