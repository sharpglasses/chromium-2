// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/json/json_reader.h"
#include "base/utf_string_conversions.h"
#include "base/values.h"
#include "chrome/browser/spellchecker/spellcheck_misspelling.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(SpellcheckMisspellingTest, SerializeTest) {
  SpellcheckMisspelling misspelling;
  misspelling.context = ASCIIToUTF16("How doe sit know");
  misspelling.location = 4;
  misspelling.length = 7;
  misspelling.timestamp = base::Time::FromJsTime(42);
  misspelling.suggestions.push_back(ASCIIToUTF16("does it"));

  scoped_ptr<base::Value> expected(base::JSONReader::Read(
      "{\"originalText\": \"How doe sit know\","
      "\"misspelledStart\": 4,"
      "\"misspelledLength\": 7,"
      "\"timestamp\": 42.0,"
      "\"suggestions\": [\"does it\"],"
      "\"userActions\": [{\"actionType\": \"PENDING\"}]}"));

  scoped_ptr<base::DictionaryValue> serialized(misspelling.Serialize());
  EXPECT_TRUE(serialized->Equals(expected.get()));
}
