// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef APPS_LEDGER_SRC_STORAGE_IMPL_PAGE_STORAGE_IMPL_H_
#define APPS_LEDGER_SRC_STORAGE_IMPL_PAGE_STORAGE_IMPL_H_

#include "apps/ledger/src/storage/public/page_storage.h"

#include <set>

#include "apps/ledger/src/convert/convert.h"
#include "apps/ledger/src/storage/impl/db_impl.h"
#include "apps/ledger/src/storage/public/page_sync_delegate.h"
#include "lib/ftl/memory/ref_ptr.h"
#include "lib/ftl/strings/string_view.h"
#include "lib/ftl/tasks/task_runner.h"

namespace storage {

class PageStorageImpl : public PageStorage {
 public:
  PageStorageImpl(ftl::RefPtr<ftl::TaskRunner> main_runner,
                  ftl::RefPtr<ftl::TaskRunner> io_runner,
                  std::string page_dir,
                  PageId page_id);
  ~PageStorageImpl() override;

  // Initializes this PageStorageImpl. This includes initializing the underlying
  // database, adding the default page head if the page is empty, removing
  // uncommitted explicit and committing implicit journals.
  Status Init();

  // Adds the given locally created |commit| in this |PageStorage|.
  void AddCommitFromLocal(std::unique_ptr<const Commit> commit,
                          std::function<void(Status)> callback);

  // Returns true if the given |object_id| is untracked, i.e. has been  created
  // using |AddObjectFromLocal()|, but is not yet part of any commit. Untracked
  // objects are invalid after the PageStorageImpl object is destroyed.
  bool ObjectIsUntracked(ObjectIdView object_id);

  // Marks the given object as tracked.
  void MarkObjectTracked(ObjectIdView object_id);

  // PageStorage:
  PageId GetId() override;
  void SetSyncDelegate(PageSyncDelegate* page_sync) override;
  Status GetHeadCommitIds(std::vector<CommitId>* commit_ids) override;
  Status GetCommit(const CommitId& commit_id,
                   std::unique_ptr<const Commit>* commit) override;
  void AddCommitsFromSync(std::vector<CommitIdAndBytes> ids_and_bytes,
                          std::function<void(Status)>) override;
  Status StartCommit(const CommitId& commit_id,
                     JournalType journal_type,
                     std::unique_ptr<Journal>* journal) override;
  Status StartMergeCommit(const CommitId& left,
                          const CommitId& right,
                          std::unique_ptr<Journal>* journal) override;
  Status AddCommitWatcher(CommitWatcher* watcher) override;
  Status RemoveCommitWatcher(CommitWatcher* watcher) override;
  Status GetUnsyncedCommits(
      std::vector<std::unique_ptr<const Commit>>* commits) override;
  Status MarkCommitSynced(const CommitId& commit_id) override;
  Status GetDeltaObjects(const CommitId& commit_id,
                         std::vector<ObjectId>* objects) override;
  void GetUnsyncedObjectIds(
      const CommitId& commit_id,
      std::function<void(Status, std::vector<ObjectId>)> callback) override;
  Status MarkObjectSynced(ObjectIdView object_id) override;
  void AddObjectFromSync(ObjectIdView object_id,
                         mx::socket data,
                         size_t size,
                         const std::function<void(Status)>& callback) override;
  void AddObjectFromLocal(
      mx::socket data,
      int64_t size,
      const std::function<void(Status, ObjectId)>& callback) override;
  void GetObject(
      ObjectIdView object_id,
      const std::function<void(Status, std::unique_ptr<const Object>)>&
          callback) override;
  Status GetObjectSynchronous(ObjectIdView object_id,
                              std::unique_ptr<const Object>* object) override;
  Status AddObjectSynchronous(convert::ExtendedStringView data,
                              std::unique_ptr<const Object>* object) override;
  Status SetSyncMetadata(ftl::StringView sync_state) override;
  Status GetSyncMetadata(std::string* sync_state) override;

 private:
  friend class PageStorageImplAccessorForTest;
  class FileWriter;

  void AddCommits(std::vector<std::unique_ptr<const Commit>> commits,
                  ChangeSource source,
                  std::function<void(Status)> callback);
  Status ContainsCommit(const CommitId& id);
  bool IsFirstCommit(const CommitId& id);
  void AddObject(mx::socket data,
                 int64_t size,
                 const std::function<void(Status, ObjectId)>& callback);
  void GetObjectFromSync(
      ObjectIdView object_id,
      const std::function<void(Status, std::unique_ptr<const Object>)>&
          callback);
  std::string GetFilePath(ObjectIdView object_id) const;

  // Notifies the registered watchers with the given |commits|.
  void NotifyWatchers(const std::vector<std::unique_ptr<const Commit>>& commits,
                      ChangeSource source);

  ftl::RefPtr<ftl::TaskRunner> main_runner_;
  ftl::RefPtr<ftl::TaskRunner> io_runner_;
  std::string page_dir_;
  PageId page_id_;
  DbImpl db_;
  std::vector<CommitWatcher*> watchers_;
  std::set<ObjectId, convert::StringViewComparator> untracked_objects_;
  std::string objects_dir_;
  std::string staging_dir_;
  std::vector<std::unique_ptr<FileWriter>> writers_;
  PageSyncDelegate* page_sync_;
};

}  // namespace storage

#endif  // APPS_LEDGER_SRC_STORAGE_IMPL_PAGE_STORAGE_IMPL_H_
