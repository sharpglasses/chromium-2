// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/time.h"
#include "chrome/browser/browser_thread.h"
#include "chrome/browser/prerender/prerender_contents.h"
#include "chrome/browser/prerender/prerender_manager.h"
#include "googleurl/src/gurl.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class DummyPrerenderContents : public PrerenderContents {
 public:
  DummyPrerenderContents(PrerenderManager* prerender_manager, const GURL& url)
      : PrerenderContents(prerender_manager, NULL, url,
                          std::vector<GURL>()),
        has_started_(false) {
  }

  DummyPrerenderContents(PrerenderManager* prerender_manager, const GURL& url,
                         const std::vector<GURL> alias_urls)
      : PrerenderContents(prerender_manager, NULL, url, alias_urls),
        has_started_(false) {
  }

  virtual void StartPrerendering() {
    has_started_ = true;
  }

  bool has_started() const { return has_started_; }

 private:
  bool has_started_;
};

class TestPrerenderManager : public PrerenderManager {
 public:
  TestPrerenderManager()
      : PrerenderManager(NULL),
        time_(base::Time::Now()),
        next_pc_(NULL) {
  }

  void AdvanceTime(base::TimeDelta delta) {
    time_ += delta;
  }

  void SetNextPrerenderContents(PrerenderContents* pc) {
    next_pc_.reset(pc);
  }

  // Shorthand to add a simple preload with no aliases.
  void AddSimplePreload(const GURL& url) {
    AddPreload(url, std::vector<GURL>());
  }

  PrerenderContents* next_pc() { return next_pc_.get(); }

 protected:
  virtual ~TestPrerenderManager() {
  }

 private:
  virtual base::Time GetCurrentTime() const {
    return time_;
  }

  virtual PrerenderContents* CreatePrerenderContents(
      const GURL& url,
      const std::vector<GURL>& alias_urls) {
    return next_pc_.release();
  }

  base::Time time_;
  scoped_ptr<PrerenderContents> next_pc_;
};

}  // namespace

class PrerenderManagerTest : public testing::Test {
 public:
  PrerenderManagerTest() : prerender_manager_(new TestPrerenderManager()),
                           ui_thread_(BrowserThread::UI, &message_loop_) {
  }

 protected:
  scoped_refptr<TestPrerenderManager> prerender_manager_;

 private:
  // Needed to pass PrerenderManager's DCHECKs.
  MessageLoop message_loop_;
  BrowserThread ui_thread_;
};

TEST_F(PrerenderManagerTest, EmptyTest) {
  GURL url("http://www.google.com/");
  EXPECT_FALSE(prerender_manager_->MaybeUsePreloadedPage(NULL, url));
}

TEST_F(PrerenderManagerTest, FoundTest) {
  GURL url("http://www.google.com/");
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_TRUE(pc->has_started());
  EXPECT_EQ(pc, prerender_manager_->GetEntry(url));
  delete pc;
}

// Make sure that if queue a request, and a second prerender request for the
// same URL comes in, that we drop the second request and keep the first one.
TEST_F(PrerenderManagerTest, DropSecondRequestTest) {
  GURL url("http://www.google.com/");
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  DummyPrerenderContents* null = NULL;
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc->has_started());
  DummyPrerenderContents* pc1 =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  prerender_manager_->SetNextPrerenderContents(pc1);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(pc1, prerender_manager_->next_pc());
  EXPECT_FALSE(pc1->has_started());
  EXPECT_EQ(pc, prerender_manager_->GetEntry(url));
  delete pc;
}

// Ensure that we expire a prerendered page after the max. permitted time.
TEST_F(PrerenderManagerTest, ExpireTest) {
  GURL url("http://www.google.com/");
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  DummyPrerenderContents* null = NULL;
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc->has_started());
  prerender_manager_->AdvanceTime(prerender_manager_->max_prerender_age()
                                  + base::TimeDelta::FromSeconds(1));
  EXPECT_EQ(null, prerender_manager_->GetEntry(url));
}

// LRU Test.  Make sure that if we prerender more than one request, that
// the oldest one will be dropped.
TEST_F(PrerenderManagerTest, DropOldestRequestTest) {
  GURL url("http://www.google.com/");
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  DummyPrerenderContents* null = NULL;
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc->has_started());
  GURL url1("http://news.google.com/");
  DummyPrerenderContents* pc1 =
      new DummyPrerenderContents(prerender_manager_.get(), url1);
  prerender_manager_->SetNextPrerenderContents(pc1);
  prerender_manager_->AddSimplePreload(url1);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc1->has_started());
  EXPECT_EQ(null, prerender_manager_->GetEntry(url));
  EXPECT_EQ(pc1, prerender_manager_->GetEntry(url1));
  delete pc1;
}

// Two element prerender test.  Ensure that the LRU operates correctly if we
// permit 2 elements to be kept prerendered.
TEST_F(PrerenderManagerTest, TwoElementPrerenderTest) {
  prerender_manager_->set_max_elements(2);
  GURL url("http://www.google.com/");
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url);
  DummyPrerenderContents* null = NULL;
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc->has_started());
  GURL url1("http://news.google.com/");
  DummyPrerenderContents* pc1 =
      new DummyPrerenderContents(prerender_manager_.get(),  url1);
  prerender_manager_->SetNextPrerenderContents(pc1);
  prerender_manager_->AddSimplePreload(url1);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc1->has_started());
  GURL url2("http://images.google.com/");
  DummyPrerenderContents* pc2 =
      new DummyPrerenderContents(prerender_manager_.get(), url2);
  prerender_manager_->SetNextPrerenderContents(pc2);
  prerender_manager_->AddSimplePreload(url2);
  EXPECT_EQ(null, prerender_manager_->next_pc());
  EXPECT_TRUE(pc2->has_started());
  EXPECT_EQ(null, prerender_manager_->GetEntry(url));
  EXPECT_EQ(pc1, prerender_manager_->GetEntry(url1));
  EXPECT_EQ(pc2, prerender_manager_->GetEntry(url2));
  delete pc1;
  delete pc2;
}

TEST_F(PrerenderManagerTest, AliasURLTest) {
  GURL url("http://www.google.com/");
  GURL alias_url1("http://www.google.com/index.html");
  GURL alias_url2("http://google.com/");
  GURL not_an_alias_url("http://google.com/index.html");
  std::vector<GURL> alias_urls;
  alias_urls.push_back(alias_url1);
  alias_urls.push_back(alias_url2);
  DummyPrerenderContents* pc =
      new DummyPrerenderContents(prerender_manager_.get(), url, alias_urls);
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(NULL, prerender_manager_->GetEntry(not_an_alias_url));
  EXPECT_EQ(pc, prerender_manager_->GetEntry(alias_url1));
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(pc, prerender_manager_->GetEntry(alias_url2));
  prerender_manager_->SetNextPrerenderContents(pc);
  prerender_manager_->AddSimplePreload(url);
  EXPECT_EQ(pc, prerender_manager_->GetEntry(url));
  delete pc;
}
