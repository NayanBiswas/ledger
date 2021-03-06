// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/ledger/src/storage/impl/btree/btree_iterator.h"

#include "apps/ledger/src/glue/crypto/rand.h"
#include "apps/ledger/src/storage/fake/fake_page_storage.h"
#include "apps/ledger/src/storage/impl/btree/commit_contents_impl.h"
#include "apps/ledger/src/storage/impl/btree/tree_node.h"
#include "apps/ledger/src/storage/public/commit_contents.h"
#include "apps/ledger/src/storage/public/constants.h"
#include "apps/ledger/src/storage/public/iterator.h"
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

class BTreeIteratorTest : public ::testing::Test {
 public:
  BTreeIteratorTest() : fake_storage_("page_id") {}

  ~BTreeIteratorTest() override {}

  // Test:
  void SetUp() override {
    ::testing::Test::SetUp();
    std::srand(0);
  }

 protected:
  fake::FakePageStorage fake_storage_;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(BTreeIteratorTest);
};

TEST_F(BTreeIteratorTest, IterateOneNode) {
  Entry entry1 = Entry{"key1", RandomId(), storage::KeyPriority::EAGER};
  Entry entry2 = Entry{"key2", RandomId(), storage::KeyPriority::EAGER};
  Entry entry3 = Entry{"key3", RandomId(), storage::KeyPriority::LAZY};
  ObjectId node_id;
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_,
                                  std::vector<Entry>{entry1, entry2, entry3},
                                  std::vector<ObjectId>(4), &node_id));

  CommitContentsImpl reader(node_id, &fake_storage_);

  std::unique_ptr<Iterator<const Entry>> it = reader.begin();

  EXPECT_TRUE(it->Valid());
  EXPECT_EQ(entry1, **it);

  it->Next();
  EXPECT_TRUE(it->Valid());
  EXPECT_EQ(entry2, **it);

  it->Next();
  EXPECT_TRUE(it->Valid());
  EXPECT_EQ(entry3, **it);

  it->Next();
  EXPECT_FALSE(it->Valid());
  EXPECT_EQ(Status::OK, it->GetStatus());
}

TEST_F(BTreeIteratorTest, IterateEmptyTree) {
  ObjectId node_id;
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_, std::vector<Entry>{},
                                  std::vector<ObjectId>(1), &node_id));

  CommitContentsImpl reader(node_id, &fake_storage_);

  std::unique_ptr<Iterator<const Entry>> it = reader.begin();

  EXPECT_FALSE(it->Valid());
  EXPECT_EQ(Status::OK, it->GetStatus());
}

TEST_F(BTreeIteratorTest, IterateTree) {
  std::vector<Entry> entries{
      Entry{"key0", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key1", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key2", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key3", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key4", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key5", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key6", RandomId(), storage::KeyPriority::LAZY}};

  // We create a tree with one root and three leaves, as follows:
  //              D:[2, 3]
  //           /      |    \
  //  A:[0, 1]   B:[]   C:[4, 5, 6]

  ObjectId node_A, node_B, node_C, node_D;
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_,
                                  std::vector<Entry>{entries[0], entries[1]},
                                  std::vector<ObjectId>(3), &node_A));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_, std::vector<Entry>(),
                                  std::vector<ObjectId>(1), &node_B));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(
                &fake_storage_,
                std::vector<Entry>{entries[4], entries[5], entries[6]},
                std::vector<ObjectId>(4), &node_C));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(
                &fake_storage_, std::vector<Entry>{entries[2], entries[3]},
                std::vector<ObjectId>{node_A, node_B, node_C}, &node_D));

  CommitContentsImpl reader(node_D, &fake_storage_);

  std::unique_ptr<Iterator<const Entry>> it = reader.begin();
  EXPECT_TRUE(it->Valid());

  for (const Entry& entry : entries) {
    EXPECT_TRUE(it->Valid());
    EXPECT_EQ(entry, **it);
    it->Next();
  }
  EXPECT_FALSE(it->Valid());
  EXPECT_EQ(Status::OK, it->GetStatus());
}

TEST_F(BTreeIteratorTest, Seek) {
  std::vector<Entry> entries{
      Entry{"key0", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key1", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key2", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key3", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key4", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key5", RandomId(), storage::KeyPriority::EAGER},
      Entry{"key6", RandomId(), storage::KeyPriority::LAZY}};

  // We create a tree with one root and three leaves, as follows:
  //              D:[2, 3]
  //           /      |    \
  //  A:[0, 1]   B:[]   C:[4, 5, 6]

  ObjectId node_A, node_B, node_C, node_D;
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_,
                                  std::vector<Entry>{entries[0], entries[1]},
                                  std::vector<ObjectId>(3), &node_A));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(&fake_storage_, std::vector<Entry>(),
                                  std::vector<ObjectId>(1), &node_B));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(
                &fake_storage_,
                std::vector<Entry>{entries[4], entries[5], entries[6]},
                std::vector<ObjectId>(4), &node_C));
  EXPECT_EQ(Status::OK,
            TreeNode::FromEntries(
                &fake_storage_, std::vector<Entry>{entries[2], entries[3]},
                std::vector<ObjectId>{node_A, node_B, node_C}, &node_D));

  CommitContentsImpl reader(node_D, &fake_storage_);

  std::unique_ptr<Iterator<const Entry>> it = reader.find("key");
  EXPECT_EQ(entries[0], **it);

  it = reader.find("key2");
  EXPECT_EQ(entries[2], **it);
  EXPECT_TRUE(it->Valid());

  it = reader.find("key5");
  EXPECT_EQ(entries[5], **it);
  EXPECT_TRUE(it->Valid());

  it = reader.find("key11");
  EXPECT_EQ(entries[2], **it);
  EXPECT_TRUE(it->Valid());

  it = reader.find("key6");
  EXPECT_EQ(entries[6], **it);
  EXPECT_TRUE(it->Valid());

  it = reader.find("key9");
  EXPECT_FALSE(it->Valid());
  EXPECT_EQ(Status::OK, it->GetStatus());
}

}  // namespace
}  // namespace storage
