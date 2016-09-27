// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/glue/data_pipe/data_pipe_drainer.h"

#include "gtest/gtest.h"
#include "lib/mtl/data_pipe/strings.h"
#include "lib/mtl/tasks/message_loop.h"
#include "mojo/public/cpp/system/data_pipe.h"

namespace glue {
namespace {

class Client : public DataPipeDrainer::Client {
 public:
  Client(const std::function<void()>& callback) : callback_(callback) {}
  ~Client() override {}

  std::string GetValue() { return value_; }

 private:
  void OnDataAvailable(const void* data, size_t num_bytes) override {
    value_.append(static_cast<const char*>(data), num_bytes);
  }
  void OnDataComplete() override { callback_(); }

  std::string value_;
  std::function<void()> callback_;
};

TEST(DataPipeDrainer, ReadData) {
  mtl::MessageLoop message_loop;
  Client client([&message_loop]() { message_loop.QuitNow(); });
  DataPipeDrainer drainer(&client);
  drainer.Start(mtl::WriteStringToConsumerHandle("Hello"));
  message_loop.Run();
  EXPECT_EQ("Hello", client.GetValue());
}

}  // namespace
}  // namespace glue
