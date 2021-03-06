// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Some helpers for quic

#ifndef NET_QUIC_QUIC_UTILS_H_
#define NET_QUIC_QUIC_UTILS_H_

#include "net/base/int128.h"
#include "net/base/net_export.h"
#include "net/quic/quic_protocol.h"

namespace net {

class NET_EXPORT_PRIVATE QuicUtils {
 public:
  enum Priority {
    LOCAL_PRIORITY,
    PEER_PRIORITY,
  };

  // returns the 64 bit FNV1a hash of the data.  See
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
  static uint64 FNV1a_64_Hash(const char* data, int len);

  // returns the 128 bit FNV1a hash of the data.  See
  // http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param
  static uint128 FNV1a_128_Hash(const char* data, int len);

  // FindMutualTag sets |out_result| to the first tag in the priority list that
  // is also in the other list and returns true. If there is no intersection it
  // returns false.
  //
  // Which list has priority is determined by |priority|.
  //
  // If |out_index| is non-NULL and a match is found then the index of that
  // match in |their_tags| is written to |out_index|.
  static bool FindMutualTag(const QuicTagVector& our_tags,
                            const QuicTag* their_tags,
                            size_t num_their_tags,
                            Priority priority,
                            QuicTag* out_result,
                            size_t* out_index);

  // SerializeUint128 writes |v| in little-endian form to |out|.
  static void SerializeUint128(uint128 v, uint8* out);

  // ParseUint128 parses a little-endian uint128 from |in| and returns it.
  static uint128 ParseUint128(const uint8* in);

  // Returns the name of the QuicRstStreamErrorCode as a char*
  static const char* StreamErrorToString(QuicRstStreamErrorCode error);

  // Returns the name of the QuicErrorCode as a char*
  static const char* ErrorToString(QuicErrorCode error);

  // TagToString is a utility function for pretty-printing handshake messages
  // that converts a tag to a string. It will try to maintain the human friendly
  // name if possible (i.e. kABCD -> "ABCD"), or will just treat it as a number
  // if not.
  static std::string TagToString(QuicTag tag);
};

}  // namespace net

#endif  // NET_QUIC_QUIC_UTILS_H_
