// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SEARCH_INSTANT_IO_CONTEXT_H_
#define CHROME_BROWSER_SEARCH_INSTANT_IO_CONTEXT_H_

#include <map>
#include <set>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/ref_counted.h"
#include "chrome/common/instant_restricted_id_cache.h"

class GURL;

namespace content {
class ResourceContext;
}

namespace net {
class URLRequest;
}

// IO thread data held for Instant.  This reflects the data held in
// InstantService for use on the IO thread.  Owned by ResourceContext
// as user data.
class InstantIOContext : public base::RefCountedThreadSafe<InstantIOContext> {
 public:
  InstantIOContext();

  // Key name for context UserData.  UserData is created by InstantService
  // but accessed by InstantIOContext.
  static const char kInstantIOContextKeyName[];

  // Installs the |instant_io_context| into the UserData of the
  // |resource_context|.
  static void SetUserDataOnIO(
      content::ResourceContext* resource_context,
      scoped_refptr<InstantIOContext> instant_io_context);

  // Add and remove RenderProcessHost IDs that are associated with Instant
  // processes.  Used to keep process IDs in sync with InstantService.
  static void AddInstantProcessOnIO(
      scoped_refptr<InstantIOContext> instant_io_context,
      int process_id);
  static void RemoveInstantProcessOnIO(
      scoped_refptr<InstantIOContext> instant_io_context,
      int process_id);
  static void ClearInstantProcessesOnIO(
      scoped_refptr<InstantIOContext> instant_io_context);

  // Associates the |most_visited_item_id| with the |url|.
  static void AddMostVisitedItemsOnIO(
      scoped_refptr<InstantIOContext> instant_io_context,
      std::vector<InstantMostVisitedItemIDPair> items);

  // Determine if this chrome-search: request is coming from an Instant render
  // process.
  static bool ShouldServiceRequest(const net::URLRequest* request);

  // If there is a mapping for the |most_visited_item_id|, sets |url| and
  // returns true.
  static bool GetURLForMostVisitedItemID(
      const net::URLRequest* request,
      InstantRestrictedID most_visited_item_id,
      GURL* url);

 protected:
   virtual ~InstantIOContext();

 private:
  friend class base::RefCountedThreadSafe<InstantIOContext>;

  // Check that |process_id| is in the known set of Instant processes, ie.
  // |process_ids_|.
  bool IsInstantProcess(int process_id) const;

  bool GetURLForMostVisitedItemID(InstantRestrictedID most_visited_item_id,
                                  GURL* url) const;

  // The process IDs associated with Instant processes.  Mirror of the process
  // IDs in InstantService.  Duplicated here for synchronous access on the IO
  // thread.
  std::set<int> process_ids_;

  // The Most Visited item cache. Mirror of the Most Visited item cache in
  // InstantService. Duplicated here for synchronous access on the IO thread.
  InstantRestrictedIDCache<InstantMostVisitedItem> most_visited_item_cache_;

  DISALLOW_COPY_AND_ASSIGN(InstantIOContext);
};

#endif  // CHROME_BROWSER_SEARCH_INSTANT_IO_CONTEXT_H_
