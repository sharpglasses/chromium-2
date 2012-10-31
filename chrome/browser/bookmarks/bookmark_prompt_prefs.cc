// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/bookmarks/bookmark_prompt_prefs.h"

#include "base/metrics/field_trial.h"
#include "base/prefs/public/pref_service_base.h"
#include "chrome/common/pref_names.h"

namespace {

// This function checks field trial "BookmarkPrompt" managed by
// VariationsService.
bool IsBookmarkFieldTrialActive() {
  base::FieldTrial* trial = base::FieldTrialList::Find("BookmarkPrompt");
  if (!trial) {
    DVLOG(1) << "BookmarkPrompt field trial was not found.";
    return false;
  }
  return trial->group_name() == "V1";
}

}  // namespace

BookmarkPromptPrefs::BookmarkPromptPrefs(PrefServiceBase* user_prefs)
    : prefs_(user_prefs) {
}

BookmarkPromptPrefs::~BookmarkPromptPrefs() {
}

void BookmarkPromptPrefs::DisableBookmarkPrompt() {
  prefs_->SetBoolean(prefs::kBookmarkPromptEnabled, false);
}

int BookmarkPromptPrefs::GetPromptImpressionCount() const {
  return prefs_->GetInteger(prefs::kBookmarkPromptImpressionCount);
}

void BookmarkPromptPrefs::IncrementPromptImpressionCount() {
  prefs_->SetInteger(prefs::kBookmarkPromptImpressionCount,
                     GetPromptImpressionCount() + 1);
}

bool BookmarkPromptPrefs::IsBookmarkPromptEnabled() const {
  return prefs_->GetBoolean(prefs::kBookmarkPromptEnabled) &&
         IsBookmarkFieldTrialActive();
}

// static
void BookmarkPromptPrefs::RegisterUserPrefs(PrefServiceBase* user_prefs) {
  // We always register preferences without checking FieldTrial, because
  // we may not receive field trial list from the server yet.
  user_prefs->RegisterBooleanPref(prefs::kBookmarkPromptEnabled, true,
                                  PrefServiceBase::UNSYNCABLE_PREF);
  user_prefs->RegisterIntegerPref(prefs::kBookmarkPromptImpressionCount, 0,
                                  PrefServiceBase::UNSYNCABLE_PREF);
}
