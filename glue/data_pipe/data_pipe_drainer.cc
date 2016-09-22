// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/glue/data_pipe/data_pipe_drainer.h"

#include <mojo/system/result.h>

#include <utility>

#include "lib/ftl/logging.h"

namespace glue {

DataPipeDrainer::DataPipeDrainer(Client* client, const MojoAsyncWaiter* waiter)
    : client_(client), waiter_(waiter), wait_id_(0) {
  FTL_DCHECK(client_);
}

DataPipeDrainer::~DataPipeDrainer() {
  if (wait_id_) {
    waiter_->CancelWait(wait_id_);
  }
}

void DataPipeDrainer::Start(mojo::ScopedDataPipeConsumerHandle source) {
  source_ = std::move(source);
  ReadData();
}

void DataPipeDrainer::ReadData() {
  const void* buffer = nullptr;
  uint32_t num_bytes = 0;
  MojoResult rv = BeginReadDataRaw(source_.get(), &buffer, &num_bytes,
                                   MOJO_READ_DATA_FLAG_NONE);
  if (rv == MOJO_RESULT_OK) {
    client_->OnDataAvailable(buffer, num_bytes);
    EndReadDataRaw(source_.get(), num_bytes);
    WaitForData();
  } else if (rv == MOJO_SYSTEM_RESULT_SHOULD_WAIT) {
    WaitForData();
  } else if (rv == MOJO_SYSTEM_RESULT_FAILED_PRECONDITION) {
    client_->OnDataComplete();
  } else {
    FTL_DCHECK(false) << "Unhandled MojoResult: " << rv;
  }
}

void DataPipeDrainer::WaitForData() {
  FTL_DCHECK(!wait_id_);
  MojoHandle handle = source_.get().value();
  wait_id_ = waiter_->AsyncWait(handle, MOJO_HANDLE_SIGNAL_READABLE,
                                MOJO_DEADLINE_INDEFINITE, &WaitComplete, this);
}

void DataPipeDrainer::WaitComplete(void* context, MojoResult result) {
  DataPipeDrainer* drainer = static_cast<DataPipeDrainer*>(context);
  drainer->wait_id_ = 0;
  drainer->ReadData();
}

}  // namespace mtl
