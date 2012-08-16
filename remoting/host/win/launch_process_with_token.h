// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_WIN_LAUNCH_PROCESS_WITH_TOKEN_H_
#define REMOTING_HOST_WIN_LAUNCH_PROCESS_WITH_TOKEN_H_

#include <windows.h>
#include <string>

#include "base/command_line.h"
#include "base/file_path.h"
#include "base/win/scoped_handle.h"

namespace remoting {

// Creates a copy of the current process token for the given |session_id| so
// it can be used to launch a process in that session.
bool CreateSessionToken(uint32 session_id, base::win::ScopedHandle* token_out);

// Launches |binary| in the security context of the user represented by
// |user_token|. The session ID specified by the token is respected as well.
bool LaunchProcessWithToken(const FilePath& binary,
                            const CommandLine::StringType& command_line,
                            HANDLE user_token,
                            DWORD creation_flags,
                            base::win::ScopedHandle* process_out,
                            base::win::ScopedHandle* thread_out);

} // namespace remoting

#endif  // REMOTING_HOST_WIN_LAUNCH_PROCESS_WITH_TOKEN_H_
