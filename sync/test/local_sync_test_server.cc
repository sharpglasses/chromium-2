// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sync/test/local_sync_test_server.h"

#include "base/command_line.h"
#include "base/path_service.h"
#include "base/string_number_conversions.h"
#include "base/values.h"
#include "net/test/python_utils.h"
#include "net/test/spawned_test_server.h"

namespace syncer {

LocalSyncTestServer::LocalSyncTestServer()
    : LocalTestServer(
        net::SpawnedTestServer::TYPE_HTTP,  // Sync uses the HTTP scheme.
        net::SpawnedTestServer::kLocalhost,
        base::FilePath()),
      xmpp_port_(0) {}

LocalSyncTestServer::LocalSyncTestServer(uint16 port, uint16 xmpp_port)
    : LocalTestServer(
        net::SpawnedTestServer::TYPE_HTTP,  // Sync uses the HTTP scheme.
        net::SpawnedTestServer::kLocalhost,
        base::FilePath()),
      xmpp_port_(xmpp_port) {
  SetPort(port);
}

LocalSyncTestServer::~LocalSyncTestServer() {}

bool LocalSyncTestServer::AddCommandLineArguments(
    CommandLine* command_line) const {
  if (!LocalTestServer::AddCommandLineArguments(command_line))
    return false;
  if (xmpp_port_ != 0) {
    std::string xmpp_port_str = base::IntToString(xmpp_port_);
    command_line->AppendArg("--xmpp-port=" + xmpp_port_str);
  }
  return true;
}

bool LocalSyncTestServer::GetTestServerPath(
    base::FilePath* testserver_path) const {
  base::FilePath testserver_dir;
  if (!PathService::Get(base::DIR_SOURCE_ROOT, &testserver_dir)) {
    LOG(ERROR) << "Failed to get DIR_SOURCE_ROOT";
    return false;
  }
  testserver_dir = testserver_dir.Append(FILE_PATH_LITERAL("sync"))
                                 .Append(FILE_PATH_LITERAL("tools"))
                                 .Append(FILE_PATH_LITERAL("testserver"));

  *testserver_path =
      testserver_dir.Append(FILE_PATH_LITERAL("sync_testserver.py"));
  return true;
}

bool LocalSyncTestServer::GetTestScriptPath(
    const base::FilePath::StringType& test_script_name,
    base::FilePath* test_script_path) const {
  base::FilePath testserver_path;
  if (!GetTestServerPath(&testserver_path))
    return false;
  *test_script_path = testserver_path.DirName().Append(test_script_name);
  return true;
}

bool LocalSyncTestServer::SetPythonPath() const {
  if (!LocalTestServer::SetPythonPath())
    return false;

  // Add the net/tools/testserver directory to the path, so that testserver_base
  // can be imported.
  base::FilePath net_testserver_path;
  if (!LocalTestServer::GetTestServerPath(&net_testserver_path)) {
    LOG(ERROR) << "Failed to get net testserver path.";
    return false;
  }
  AppendToPythonPath(net_testserver_path.DirName());

  // Locate the Python code generated by the sync protocol buffers compiler.
  base::FilePath pyproto_dir;
  if (!GetPyProtoPath(&pyproto_dir)) {
    LOG(WARNING) << "Cannot find pyproto dir for generated code. "
                 << "Testserver features that rely on it will not work";
    return true;
  }
  AppendToPythonPath(pyproto_dir.AppendASCII("sync").AppendASCII("protocol"));
  return true;
}

}  // namespace syncer
