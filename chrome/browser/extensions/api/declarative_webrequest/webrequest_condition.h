// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_
#define CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/linked_ptr.h"
#include "chrome/browser/extensions/api/declarative/declarative_rule.h"
#include "chrome/browser/extensions/api/declarative_webrequest/webrequest_condition_attribute.h"
#include "chrome/common/extensions/matcher/url_matcher.h"
#include "net/http/http_response_headers.h"

namespace extensions {

// Container for information about a URLRequest to determine which
// rules apply to the request.
struct DeclarativeWebRequestData {
  DeclarativeWebRequestData(net::URLRequest* request, RequestStage stage)
      : request(request), stage(stage),
        original_response_headers(NULL) {}
  DeclarativeWebRequestData(
      net::URLRequest* request, RequestStage stage,
      const net::HttpResponseHeaders* original_response_headers)
      : request(request), stage(stage),
        original_response_headers(original_response_headers) {}
  net::URLRequest* request;
  RequestStage stage;
  // Additional information about requests that is not
  // available in all request stages.
  const net::HttpResponseHeaders* original_response_headers;
};

// Representation of a condition in the Declarative WebRequest API. A condition
// consists of several attributes. Each of these attributes needs to be
// fulfilled in order for the condition to be fulfilled.
//
// We distinguish between two types of conditions:
// - URL Matcher conditions are conditions that test the URL of a request.
//   These are treated separately because we use a URLMatcher to efficiently
//   test many of these conditions in parallel by using some advanced
//   data structures. The URLMatcher tells us if all URL Matcher conditions
//   are fulfilled for a WebRequestCondition.
// - All other conditions are represented as WebRequestConditionAttributes.
//   These conditions are probed linearly (only if the URL Matcher found a hit).
//
// TODO(battre) Consider making the URLMatcher an owner of the
// URLMatcherConditionSet and only pass a pointer to URLMatcherConditionSet
// in url_matcher_condition_set(). This saves some copying in
// WebRequestConditionSet::GetURLMatcherConditionSets.
class WebRequestCondition {
 public:
  typedef DeclarativeWebRequestData MatchData;

  WebRequestCondition(
      scoped_refptr<URLMatcherConditionSet> url_matcher_conditions,
      const WebRequestConditionAttributes& condition_attributes);
  ~WebRequestCondition();

  // Factory method that instantiates a WebRequestCondition according to
  // the description |condition| passed by the extension API.
  static scoped_ptr<WebRequestCondition> Create(
      URLMatcherConditionFactory* url_matcher_condition_factory,
      const base::Value& condition,
      std::string* error);

  // Returns whether the request matches this condition.  |url_matches| lists
  // the IDs that match the request's URL.
  bool IsFulfilled(const std::set<URLMatcherConditionSet::ID> &url_matches,
                   const DeclarativeWebRequestData& request_data) const;

  // True if this condition has a url filter.
  bool has_url_matcher_condition_set() const {
    return url_matcher_conditions_ != NULL;
  }

  // If this Condition has a url filter, appends it to |condition_sets|.
  void GetURLMatcherConditionSets(
      URLMatcherConditionSet::Vector* condition_sets) const {
    if (url_matcher_conditions_)
      condition_sets->push_back(url_matcher_conditions_);
  }

  // Returns the condition attributes checked by this condition.
  const WebRequestConditionAttributes condition_attributes() const {
    return condition_attributes_;
  }

  // Returns a bit vector representing extensions::RequestStage. The bit vector
  // contains a 1 for each request stage during which the condition can be
  // tested.
  int stages() const { return applicable_request_stages_; }

 private:
  // Represents the 'url' attribute of this condition. If NULL, then there was
  // no 'url' attribute in this condition.
  scoped_refptr<URLMatcherConditionSet> url_matcher_conditions_;

  // All non-UrlFilter attributes of this condition.
  WebRequestConditionAttributes condition_attributes_;

  // Bit vector indicating all RequestStage during which all
  // |condition_attributes_| can be evaluated.
  int applicable_request_stages_;

  DISALLOW_COPY_AND_ASSIGN(WebRequestCondition);
};

typedef DeclarativeConditionSet<WebRequestCondition> WebRequestConditionSet;

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_
