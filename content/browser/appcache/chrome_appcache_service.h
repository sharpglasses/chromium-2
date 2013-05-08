// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_APPCACHE_CHROME_APPCACHE_SERVICE_H_
#define CONTENT_BROWSER_APPCACHE_CHROME_APPCACHE_SERVICE_H_

#include "base/compiler_specific.h"
#include "base/memory/ref_counted.h"
#include "base/task/sequenced_task_runner_helpers.h"
#include "content/common/content_export.h"
#include "webkit/appcache/appcache_policy.h"
#include "webkit/appcache/appcache_service.h"
#include "webkit/quota/special_storage_policy.h"

namespace base {
class FilePath;
}

namespace net {
class URLRequestContextGetter;
}

namespace content {
class ResourceContext;

struct ChromeAppCacheServiceDeleter;

// An AppCacheService subclass used by the chrome. There is an instance
// associated with each BrowserContext. This derivation adds refcounting
// semantics since a browser context has multiple URLRequestContexts which refer
// to the same object, and those URLRequestContexts are refcounted independently
// of the owning browser context.
//
// All methods, except the ctor, are expected to be called on
// the IO thread (unless specifically called out in doc comments).
//
// TODO(dpranke): Fix dependencies on AppCacheService so that we don't have
// to worry about clients calling AppCacheService methods.
class CONTENT_EXPORT ChromeAppCacheService
    : public base::RefCountedThreadSafe<ChromeAppCacheService,
                                        ChromeAppCacheServiceDeleter>,
      NON_EXPORTED_BASE(public appcache::AppCacheService),
      NON_EXPORTED_BASE(public appcache::AppCachePolicy) {
 public:
  explicit ChromeAppCacheService(quota::QuotaManagerProxy* proxy);

  void InitializeOnIOThread(
      const base::FilePath& cache_path,  // May be empty to use in-memory structs.
      ResourceContext* resource_context,
      net::URLRequestContextGetter* request_context_getter,
      scoped_refptr<quota::SpecialStoragePolicy> special_storage_policy);

  // AppCachePolicy overrides
  virtual bool CanLoadAppCache(const GURL& manifest_url,
                               const GURL& first_party) OVERRIDE;
  virtual bool CanCreateAppCache(const GURL& manifest_url,
                                 const GURL& first_party) OVERRIDE;

 protected:
  virtual ~ChromeAppCacheService();

 private:
  friend class base::DeleteHelper<ChromeAppCacheService>;
  friend class base::RefCountedThreadSafe<ChromeAppCacheService,
                                          ChromeAppCacheServiceDeleter>;
  friend struct ChromeAppCacheServiceDeleter;

  void DeleteOnCorrectThread() const;

  ResourceContext* resource_context_;
  base::FilePath cache_path_;

  DISALLOW_COPY_AND_ASSIGN(ChromeAppCacheService);
};

struct ChromeAppCacheServiceDeleter {
  static void Destruct(const ChromeAppCacheService* service) {
    service->DeleteOnCorrectThread();
  }
};

}  // namespace content

#endif  // CONTENT_BROWSER_APPCACHE_CHROME_APPCACHE_SERVICE_H_
