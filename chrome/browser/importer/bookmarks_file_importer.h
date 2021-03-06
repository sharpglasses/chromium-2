// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_IMPORTER_BOOKMARKS_FILE_IMPORTER_H_
#define CHROME_BROWSER_IMPORTER_BOOKMARKS_FILE_IMPORTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/browser/importer/importer.h"

// Importer for bookmarks files.
class BookmarksFileImporter : public Importer {
 public:
  BookmarksFileImporter();

  virtual void StartImport(const importer::SourceProfile& source_profile,
                           uint16 items,
                           ImporterBridge* bridge) OVERRIDE;

 private:
  virtual ~BookmarksFileImporter();

  DISALLOW_COPY_AND_ASSIGN(BookmarksFileImporter);
};

#endif  // CHROME_BROWSER_IMPORTER_BOOKMARKS_FILE_IMPORTER_H_
