// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/common/fileapi/file_system_dispatcher.h"

#include "base/callback.h"
#include "base/file_util.h"
#include "base/process.h"
#include "content/common/child_thread.h"
#include "content/common/fileapi/file_system_messages.h"

namespace content {

class FileSystemDispatcher::CallbackDispatcher {
 public:
  typedef CallbackDispatcher self;
  typedef FileSystemDispatcher::StatusCallback StatusCallback;
  typedef FileSystemDispatcher::MetadataCallback MetadataCallback;
  typedef FileSystemDispatcher::ReadDirectoryCallback ReadDirectoryCallback;
  typedef FileSystemDispatcher::OpenFileSystemCallback OpenFileSystemCallback;
  typedef FileSystemDispatcher::WriteCallback WriteCallback;
  typedef FileSystemDispatcher::OpenFileCallback OpenFileCallback;

  static CallbackDispatcher* Create(const StatusCallback& callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->status_callback_ = callback;
    dispatcher->error_callback_ = callback;
    return dispatcher;
  }
  static CallbackDispatcher* Create(const MetadataCallback& callback,
                                    const StatusCallback& error_callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->metadata_callback_ = callback;
    dispatcher->error_callback_ = error_callback;
    return dispatcher;
  }
  static CallbackDispatcher* Create(const ReadDirectoryCallback& callback,
                                    const StatusCallback& error_callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->directory_callback_ = callback;
    dispatcher->error_callback_ = error_callback;
    return dispatcher;
  }
  static CallbackDispatcher* Create(const OpenFileSystemCallback& callback,
                                    const StatusCallback& error_callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->filesystem_callback_ = callback;
    dispatcher->error_callback_ = error_callback;
    return dispatcher;
  }
  static CallbackDispatcher* Create(const WriteCallback& callback,
                                    const StatusCallback& error_callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->write_callback_ = callback;
    dispatcher->error_callback_ = error_callback;
    return dispatcher;
  }
  static CallbackDispatcher* Create(const OpenFileCallback& callback,
                                    const StatusCallback& error_callback) {
    CallbackDispatcher* dispatcher = new CallbackDispatcher;
    dispatcher->open_callback_ = callback;
    dispatcher->error_callback_ = error_callback;
    return dispatcher;
  }

  ~CallbackDispatcher() {}

  void DidSucceed() {
    status_callback_.Run(base::PLATFORM_FILE_OK);
  }

  void DidFail(base::PlatformFileError error_code) {
    error_callback_.Run(error_code);
  }

  void DidReadMetadata(
      const base::PlatformFileInfo& file_info,
      const base::FilePath& platform_path) {
    metadata_callback_.Run(file_info, platform_path);
  }

  void DidCreateSnapshotFile(
      const base::PlatformFileInfo& file_info,
      const base::FilePath& platform_path) {
    metadata_callback_.Run(file_info, platform_path);
  }

  void DidReadDirectory(
      const std::vector<fileapi::DirectoryEntry>& entries,
      bool has_more) {
    directory_callback_.Run(entries, has_more);
  }

  void DidOpenFileSystem(const std::string& name,
                         const GURL& root) {
    filesystem_callback_.Run(name, root);
  }

  void DidWrite(int64 bytes, bool complete) {
    write_callback_.Run(bytes, complete);
  }

  void DidOpenFile(base::PlatformFile file,
                   int file_open_id,
                   quota::QuotaLimitType quota_policy) {
    open_callback_.Run(file, file_open_id, quota_policy);
  }

 private:
  CallbackDispatcher() {}

  StatusCallback status_callback_;
  MetadataCallback metadata_callback_;
  ReadDirectoryCallback directory_callback_;
  OpenFileSystemCallback filesystem_callback_;
  WriteCallback write_callback_;
  OpenFileCallback open_callback_;

  StatusCallback error_callback_;

  DISALLOW_COPY_AND_ASSIGN(CallbackDispatcher);
};

FileSystemDispatcher::FileSystemDispatcher() {
}

FileSystemDispatcher::~FileSystemDispatcher() {
  // Make sure we fire all the remaining callbacks.
  for (IDMap<CallbackDispatcher, IDMapOwnPointer>::iterator
           iter(&dispatchers_); !iter.IsAtEnd(); iter.Advance()) {
    int request_id = iter.GetCurrentKey();
    CallbackDispatcher* dispatcher = iter.GetCurrentValue();
    DCHECK(dispatcher);
    dispatcher->DidFail(base::PLATFORM_FILE_ERROR_ABORT);
    dispatchers_.Remove(request_id);
  }
}

bool FileSystemDispatcher::OnMessageReceived(const IPC::Message& msg) {
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(FileSystemDispatcher, msg)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidOpenFileSystem, OnDidOpenFileSystem)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidSucceed, OnDidSucceed)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidReadDirectory, OnDidReadDirectory)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidReadMetadata, OnDidReadMetadata)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidCreateSnapshotFile,
                        OnDidCreateSnapshotFile)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidFail, OnDidFail)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidWrite, OnDidWrite)
    IPC_MESSAGE_HANDLER(FileSystemMsg_DidOpenFile, OnDidOpenFile)
    IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

bool FileSystemDispatcher::OpenFileSystem(
    const GURL& origin_url, fileapi::FileSystemType type,
    long long size, bool create,
    const OpenFileSystemCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_Open(
          request_id, origin_url, type, size, create))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::DeleteFileSystem(
    const GURL& origin_url,
    fileapi::FileSystemType type,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_DeleteFileSystem(
          request_id, origin_url, type))) {
    dispatchers_.Remove(request_id);
    return false;
  }
  return true;
}

bool FileSystemDispatcher::Move(
    const GURL& src_path,
    const GURL& dest_path,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_Move(
          request_id, src_path, dest_path))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::Copy(
    const GURL& src_path,
    const GURL& dest_path,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_Copy(
          request_id, src_path, dest_path))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::Remove(
    const GURL& path,
    bool recursive,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(
          new FileSystemMsg_Remove(request_id, path, recursive))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::ReadMetadata(
    const GURL& path,
    const MetadataCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_ReadMetadata(request_id, path))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::Create(
    const GURL& path,
    bool exclusive,
    bool is_directory,
    bool recursive,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_Create(
          request_id, path, exclusive, is_directory, recursive))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::Exists(
    const GURL& path,
    bool is_directory,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_Exists(request_id, path, is_directory))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::ReadDirectory(
    const GURL& path,
    const ReadDirectoryCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_ReadDirectory(request_id, path))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::Truncate(
    const GURL& path,
    int64 offset,
    int* request_id_out,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_Truncate(request_id, path, offset))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  if (request_id_out)
    *request_id_out = request_id;
  return true;
}

bool FileSystemDispatcher::Write(
    const GURL& path,
    const GURL& blob_url,
    int64 offset,
    int* request_id_out,
    const WriteCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_Write(request_id, path, blob_url, offset))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  if (request_id_out)
    *request_id_out = request_id;
  return true;
}

bool FileSystemDispatcher::Cancel(
    int request_id_to_cancel,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(new FileSystemHostMsg_CancelWrite(
          request_id, request_id_to_cancel))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::TouchFile(
    const GURL& path,
    const base::Time& last_access_time,
    const base::Time& last_modified_time,
    const StatusCallback& callback) {
  int request_id = dispatchers_.Add(CallbackDispatcher::Create(callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_TouchFile(
              request_id, path, last_access_time, last_modified_time))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::OpenFile(
    const GURL& file_path,
    int file_flags,
    const OpenFileCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_OpenFile(
              request_id, file_path, file_flags))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }

  return true;
}

bool FileSystemDispatcher::NotifyCloseFile(int file_open_id) {
  return ChildThread::current()->Send(
      new FileSystemHostMsg_NotifyCloseFile(file_open_id));
}

bool FileSystemDispatcher::CreateSnapshotFile(
    const GURL& file_path,
    const CreateSnapshotFileCallback& success_callback,
    const StatusCallback& error_callback) {
  int request_id = dispatchers_.Add(
      CallbackDispatcher::Create(success_callback, error_callback));
  if (!ChildThread::current()->Send(
          new FileSystemHostMsg_CreateSnapshotFile(
              request_id, file_path))) {
    dispatchers_.Remove(request_id);  // destroys |dispatcher|
    return false;
  }
  return true;
}

void FileSystemDispatcher::OnDidOpenFileSystem(int request_id,
                                               const std::string& name,
                                               const GURL& root) {
  DCHECK(root.is_valid());
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidOpenFileSystem(name, root);
  dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidSucceed(int request_id) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidSucceed();
  dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidReadMetadata(
    int request_id, const base::PlatformFileInfo& file_info,
    const base::FilePath& platform_path) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidReadMetadata(file_info, platform_path);
  dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidCreateSnapshotFile(
    int request_id, const base::PlatformFileInfo& file_info,
    const base::FilePath& platform_path) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidCreateSnapshotFile(file_info, platform_path);
  dispatchers_.Remove(request_id);
  ChildThread::current()->Send(
      new FileSystemHostMsg_DidReceiveSnapshotFile(request_id));
}

void FileSystemDispatcher::OnDidReadDirectory(
    int request_id,
    const std::vector<fileapi::DirectoryEntry>& entries,
    bool has_more) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidReadDirectory(entries, has_more);
  dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidFail(
    int request_id, base::PlatformFileError error_code) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidFail(error_code);
  dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidWrite(
    int request_id, int64 bytes, bool complete) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidWrite(bytes, complete);
  if (complete)
    dispatchers_.Remove(request_id);
}

void FileSystemDispatcher::OnDidOpenFile(
    int request_id,
    IPC::PlatformFileForTransit file,
    int file_open_id,
    quota::QuotaLimitType quota_policy) {
  CallbackDispatcher* dispatcher = dispatchers_.Lookup(request_id);
  DCHECK(dispatcher);
  dispatcher->DidOpenFile(IPC::PlatformFileForTransitToPlatformFile(file),
                          file_open_id,
                          quota_policy);
  dispatchers_.Remove(request_id);
}

}  // namespace content
