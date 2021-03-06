// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/quic/test_tools/quic_session_peer.h"

#include "net/quic/quic_session.h"

namespace net {
namespace test {

// static
void QuicSessionPeer::SetNextStreamId(QuicSession* session, QuicStreamId id) {
  session->next_stream_id_ = id;
}

// static
void QuicSessionPeer::SetMaxOpenStreams(QuicSession* session,
                                        uint32 max_streams) {
  session->max_open_streams_ = max_streams;
}

}  // namespace test
}  // namespace net
