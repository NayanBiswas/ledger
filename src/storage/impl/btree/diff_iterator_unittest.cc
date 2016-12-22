// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/src/storage/impl/btree/diff_iterator.h"

#include "apps/ledger/src/glue/crypto/rand.h"
#include "apps/ledger/src/storage/fake/fake_page_storage.h"
#include "apps/ledger/src/storage/impl/btree/commit_contents_impl.h"
#include "apps/ledger/src/storage/impl/btree/tree_node.h"
#include "apps/ledger/src/storage/public/constants.h"
#include "apps/ledger/src/storage/public/types.h"
#include "gtest/gtest.h"
#include "lib/ftl/logging.h"

namespace storage {

namespace {

ObjectId RandomId() {
  std::string result;
  result.resize(kObjectIdSize);
  glue::RandBytes(&result[0], kObjectIdSize);
  return result;
}

class DiffIteratorTest : public ::testing::Test {
 public:
  DiffIteratorTest() : fake_storage_("page_id") {}

  ~DiffIteratorTest() override {}

  // Test:
  void SetUp() override {
    ::testing::Test::SetUp();
    std::srand(0);
  }

 protected:
  fake::FakePageStorage fake_storage_;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(DiffIteratorTest);
};

TEST_F(DiffIteratorTest, IterateEmptyDiff) {
  Entry entry1 = Entry{"key1", RandomId(), storage::KeyPriority::EAGER};
  Entry entry2 = Entry{"key2", RandomId(), storage::KeyPriority::EAGER};
  Entry entry3 = Entry{"key3", RandomId(), storage::KeyPriority::LAZY};
  Entry entry4 = Entry{"key4", RandomId(), storage::KeyPriority::LAZY};
  ObjectId node_id1;
  EXPECT_EQ(Status::OK, TreeNode::FromEntries(
                            &fake_storage_,
                            std::vector<Entry>{entry1, entry2, entry3, entry4},
                            std::vector<ObjectId>(5), &node_id1));

  std::unique_ptr<const TreeNode> left;
  EXPECT_EQ(Status::OK, TreeNode::FromIdSynchronous(&fake_storage_, node_id1, &left));
  std::unique_ptr<const TreeNode> right;
  EXPECT_EQ(Status::OK, TreeNode::FromIdSynchronous(&fake_storage_, node_id1, &right));

  DiffIterator it(std::move(left), std::move(right));
  EXPECT_FALSE(it.Valid());
  EXPECT_EQ(Status::OK, it.GetStatus());
}

TEST_F(DiffIteratorTest, IterateOneNode) {
  Entry entry1 = Entry{"key1", RandomId(), KeyPriority::EAGER};
  Entry entry2 = Entry{"key2", RandomId(), KeyPriority::EAGER};
  Entry entry3 = Entry{"key3", RandomId(), KeyPriority::LAZY};
  Entry entry4 = Entry{"key4", RandomId(), KeyPriority::LAZY};
  Entry entry5 = Entry{"key5", RandomId(), KeyPriority::LAZY};
  Entry entry6 = Entry{"key6", RandomId(), KeyPriority::LAZY};
  ObjectId node_id1;
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_,
                                  std::vector<Entry>{entry1, entry2, entry3,
                                                     entry4, entry5, entry6},
                                  std::vector<ObjectId>(7), &node_id1));

  Entry entry11 = Entry{"key11", RandomId(), KeyPriority::EAGER};
  Entry entry22 = Entry{"key22", RandomId(), KeyPriority::EAGER};
  Entry entry4bis = Entry{entry4.key, RandomId(), entry4.priority};
  Entry entry5bis = Entry{entry5.key, entry5.object_id, KeyPriority::EAGER};
  Entry entry6bis = Entry{"key6b", RandomId(), KeyPriority::LAZY};
  ObjectId node_id2;
  EXPECT_EQ(Status::OK, TreeNode::FromEntries(
                            &fake_storage_,
                            std::vector<Entry>{entry1, entry11, entry22,
                                               entry4bis, entry5bis, entry6bis},
                            std::vector<ObjectId>(7), &node_id2));

  std::unique_ptr<const TreeNode> left;
  EXPECT_EQ(Status::OK, TreeNode::FromIdSynchronous(&fake_storage_, node_id1, &left));
  std::unique_ptr<const TreeNode> right;
  EXPECT_EQ(Status::OK, TreeNode::FromIdSynchronous(&fake_storage_, node_id2, &right));

  DiffIterator it(std::move(left), std::move(right));

  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry11, it->entry);
  EXPECT_FALSE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry2, it->entry);
  EXPECT_TRUE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry22, it->entry);
  EXPECT_FALSE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry3, it->entry);
  EXPECT_TRUE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry4bis, it->entry);
  EXPECT_FALSE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry5bis, it->entry);
  EXPECT_FALSE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry6, it->entry);
  EXPECT_TRUE(it->deleted);

  it.Next();
  EXPECT_TRUE(it.Valid());
  EXPECT_EQ(entry6bis, it->entry);
  EXPECT_FALSE(it->deleted);

  it.Next();
  EXPECT_FALSE(it.Valid());
  EXPECT_EQ(Status::OK, it.GetStatus());
}

}  // namespace
}  // namespace storage
