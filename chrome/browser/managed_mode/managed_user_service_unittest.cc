// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/path_service.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/extensions/extension_service_unittest.h"
#include "chrome/browser/extensions/unpacked_installer.h"
#include "chrome/browser/managed_mode/managed_user_service.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/test/test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"

using content::MessageLoopRunner;

namespace {

class ManagedModeURLFilterObserver : public ManagedModeURLFilter::Observer {
 public:
  explicit ManagedModeURLFilterObserver(ManagedModeURLFilter* url_filter)
      : url_filter_(url_filter) {
    Reset();
    url_filter_->AddObserver(this);
  }

  ~ManagedModeURLFilterObserver() {
    url_filter_->RemoveObserver(this);
  }

  void Wait() {
    message_loop_runner_->Run();
    Reset();
  }

  // ManagedModeURLFilter::Observer
  virtual void OnSiteListUpdated() OVERRIDE {
    message_loop_runner_->Quit();
  }

 private:
  void Reset() {
    message_loop_runner_ = new MessageLoopRunner;
  }

  ManagedModeURLFilter* url_filter_;
  scoped_refptr<MessageLoopRunner> message_loop_runner_;
};

}  // namespace

TEST(ManagedUserServiceTest, ExtensionManagementPolicyProvider) {
  MessageLoop message_loop;
  TestingProfile profile;
  {
    ManagedUserService managed_user_service(&profile);
    EXPECT_FALSE(managed_user_service.ProfileIsManaged());

    string16 error_1;
    EXPECT_TRUE(managed_user_service.UserMayLoad(NULL, &error_1));
    EXPECT_EQ(string16(), error_1);

    string16 error_2;
    EXPECT_TRUE(managed_user_service.UserMayModifySettings(NULL, &error_2));
    EXPECT_EQ(string16(), error_2);
  }

  profile.GetPrefs()->SetBoolean(prefs::kProfileIsManaged, true);
  {
    ManagedUserService managed_user_service(&profile);
    EXPECT_TRUE(managed_user_service.ProfileIsManaged());

    string16 error_1;
    EXPECT_FALSE(managed_user_service.UserMayLoad(NULL, &error_1));
    EXPECT_FALSE(error_1.empty());

    string16 error_2;
    EXPECT_FALSE(managed_user_service.UserMayModifySettings(NULL, &error_2));
    EXPECT_FALSE(error_2.empty());

#ifndef NDEBUG
  EXPECT_FALSE(managed_user_service.GetDebugPolicyProviderName().empty());
#endif
  }
}

class ManagedUserServiceExtensionTest : public ExtensionServiceTestBase {
 public:
  ManagedUserServiceExtensionTest() {}
  virtual ~ManagedUserServiceExtensionTest() {}

  virtual void SetUp() OVERRIDE {
    ExtensionServiceTestBase::SetUp();
    InitializeEmptyExtensionService();
  }

 protected:
  ScopedVector<ManagedModeSiteList> GetActiveSiteLists(
      ManagedUserService* managed_user_service) {
    return managed_user_service->GetActiveSiteLists();
  }
};

TEST_F(ManagedUserServiceExtensionTest, NoContentPacks) {
  ManagedUserService managed_user_service(profile_.get());
  ManagedModeURLFilter* url_filter =
      managed_user_service.GetURLFilterForUIThread();

  GURL url("http://youtube.com");
  ScopedVector<ManagedModeSiteList> site_lists =
      GetActiveSiteLists(&managed_user_service);
  ASSERT_EQ(0u, site_lists.size());
  EXPECT_EQ(ManagedModeURLFilter::ALLOW,
            url_filter->GetFilteringBehaviorForURL(url));
}

TEST_F(ManagedUserServiceExtensionTest, InstallContentPacks) {
  profile_->GetPrefs()->SetBoolean(prefs::kProfileIsManaged, true);
  ManagedUserService managed_user_service(profile_.get());
  managed_user_service.SetElevatedForTesting(true);
  ManagedModeURLFilter* url_filter =
      managed_user_service.GetURLFilterForUIThread();
  ManagedModeURLFilterObserver observer(url_filter);
  observer.Wait();

  GURL example_url("http://example.com");
  GURL moose_url("http://moose.org");
  EXPECT_EQ(ManagedModeURLFilter::BLOCK,
            url_filter->GetFilteringBehaviorForURL(example_url));

  profile_->GetPrefs()->SetInteger(prefs::kDefaultManagedModeFilteringBehavior,
                                   ManagedModeURLFilter::WARN);
  EXPECT_EQ(ManagedModeURLFilter::WARN,
            url_filter->GetFilteringBehaviorForURL(example_url));

  // Load a content pack.
  scoped_refptr<extensions::UnpackedInstaller> installer(
      extensions::UnpackedInstaller::Create(service_));
  installer->set_prompt_for_plugins(false);
  FilePath test_data_dir;
  ASSERT_TRUE(PathService::Get(chrome::DIR_TEST_DATA, &test_data_dir));
  FilePath extension_path =
      test_data_dir.AppendASCII("extensions/managed_mode/content_pack");
  content::WindowedNotificationObserver extension_load_observer(
      chrome::NOTIFICATION_EXTENSION_LOADED,
      content::Source<Profile>(profile_.get()));
  installer->Load(extension_path);
  extension_load_observer.Wait();
  observer.Wait();
  content::Details<extensions::Extension> details =
      extension_load_observer.details();
  scoped_refptr<extensions::Extension> extension =
      make_scoped_refptr(details.ptr());
  ASSERT_TRUE(extension);

  ScopedVector<ManagedModeSiteList> site_lists =
      GetActiveSiteLists(&managed_user_service);
  ASSERT_EQ(1u, site_lists.size());
  std::vector<ManagedModeSiteList::Site> sites;
  site_lists[0]->GetSites(&sites);
  ASSERT_EQ(3u, sites.size());
  EXPECT_EQ(ASCIIToUTF16("YouTube"), sites[0].name);
  EXPECT_EQ(ASCIIToUTF16("Homestar Runner"), sites[1].name);
  EXPECT_EQ(string16(), sites[2].name);

  EXPECT_EQ(ManagedModeURLFilter::ALLOW,
            url_filter->GetFilteringBehaviorForURL(example_url));
  EXPECT_EQ(ManagedModeURLFilter::WARN,
            url_filter->GetFilteringBehaviorForURL(moose_url));

  // Load a second content pack.
  installer = extensions::UnpackedInstaller::Create(service_);
  extension_path =
      test_data_dir.AppendASCII("extensions/managed_mode/content_pack_2");
  installer->Load(extension_path);
  observer.Wait();

  site_lists = GetActiveSiteLists(&managed_user_service);
  ASSERT_EQ(2u, site_lists.size());
  sites.clear();
  site_lists[0]->GetSites(&sites);
  site_lists[1]->GetSites(&sites);
  ASSERT_EQ(4u, sites.size());
  // The site lists might be returned in any order, so we put them into a set.
  std::set<std::string> site_names;
  for (std::vector<ManagedModeSiteList::Site>::const_iterator it =
      sites.begin(); it != sites.end(); ++it) {
    site_names.insert(UTF16ToUTF8(it->name));
  }
  EXPECT_TRUE(site_names.count("YouTube") == 1u);
  EXPECT_TRUE(site_names.count("Homestar Runner") == 1u);
  EXPECT_TRUE(site_names.count(std::string()) == 1u);
  EXPECT_TRUE(site_names.count("Moose") == 1u);

  EXPECT_EQ(ManagedModeURLFilter::ALLOW,
            url_filter->GetFilteringBehaviorForURL(example_url));
  EXPECT_EQ(ManagedModeURLFilter::ALLOW,
            url_filter->GetFilteringBehaviorForURL(moose_url));

  // Disable the first content pack.
  service_->DisableExtension(extension->id(),
                             extensions::Extension::DISABLE_USER_ACTION);
  observer.Wait();

  site_lists = GetActiveSiteLists(&managed_user_service);
  ASSERT_EQ(1u, site_lists.size());
  sites.clear();
  site_lists[0]->GetSites(&sites);
  ASSERT_EQ(1u, sites.size());
  EXPECT_EQ(ASCIIToUTF16("Moose"), sites[0].name);

  EXPECT_EQ(ManagedModeURLFilter::WARN,
            url_filter->GetFilteringBehaviorForURL(example_url));
  EXPECT_EQ(ManagedModeURLFilter::ALLOW,
            url_filter->GetFilteringBehaviorForURL(moose_url));
}
