// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/net/gaia/gaia_auth_fetcher.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "base/json/json_reader.h"
#include "base/string_split.h"
#include "base/string_util.h"
#include "base/stringprintf.h"
#include "base/values.h"
#include "chrome/common/net/gaia/gaia_auth_consumer.h"
#include "chrome/common/net/gaia/gaia_constants.h"
#include "chrome/common/net/gaia/gaia_urls.h"
#include "chrome/common/net/gaia/google_service_auth_error.h"
#include "content/public/common/url_fetcher.h"
#include "net/base/escape.h"
#include "net/base/load_flags.h"
#include "net/http/http_status_code.h"
#include "net/url_request/url_request_context_getter.h"
#include "net/url_request/url_request_status.h"

namespace {
const int kLoadFlagsIgnoreCookies = net::LOAD_DO_NOT_SEND_COOKIES |
                                    net::LOAD_DO_NOT_SAVE_COOKIES;

static bool CookiePartsContains(const std::vector<std::string>& parts,
                                const char* part) {
  return std::find(parts.begin(), parts.end(), part) != parts.end();
}

bool ExtractOAuth2TokenPairResponse(DictionaryValue* dict,
                                    std::string* refresh_token,
                                    std::string* access_token,
                                    int* expires_in_secs) {
  DCHECK(refresh_token);
  DCHECK(access_token);
  DCHECK(expires_in_secs);

  if (!dict->GetStringWithoutPathExpansion("refresh_token", refresh_token) ||
      !dict->GetStringWithoutPathExpansion("access_token", access_token) ||
      !dict->GetIntegerWithoutPathExpansion("expires_in", expires_in_secs)) {
    return false;
  }

  return true;
}

}  // namespace

// TODO(chron): Add sourceless version of this formatter.
// static
const char GaiaAuthFetcher::kClientLoginFormat[] =
    "Email=%s&"
    "Passwd=%s&"
    "PersistentCookie=%s&"
    "accountType=%s&"
    "source=%s&"
    "service=%s";
// static
const char GaiaAuthFetcher::kClientLoginCaptchaFormat[] =
    "Email=%s&"
    "Passwd=%s&"
    "PersistentCookie=%s&"
    "accountType=%s&"
    "source=%s&"
    "service=%s&"
    "logintoken=%s&"
    "logincaptcha=%s";
// static
const char GaiaAuthFetcher::kIssueAuthTokenFormat[] =
    "SID=%s&"
    "LSID=%s&"
    "service=%s&"
    "Session=%s";
// static
const char GaiaAuthFetcher::kClientLoginToOAuth2BodyFormat[] =
    "scope=%s&client_id=%s";
// static
const char GaiaAuthFetcher::kOAuth2CodeToTokenPairBodyFormat[] =
    "scope=%s&"
    "grant_type=authorization_code&"
    "client_id=%s&"
    "client_secret=%s&"
    "code=%s";
// static
const char GaiaAuthFetcher::kGetUserInfoFormat[] =
    "LSID=%s";
// static
const char GaiaAuthFetcher::kMergeSessionFormat[] =
    "uberauth=%s&"
    "continue=%s&"
    "source=%s";
// static
const char GaiaAuthFetcher::kUberAuthTokenURLFormat[] =
    "%s?source=%s&"
    "issueuberauth=1";
// static
const char GaiaAuthFetcher::kClientOAuthFormat[] =
    "{"
    "\"email\": \"%s\","
    "\"password\": \"%s\","
    "\"scopes\": [%s],"
    "\"oauth2_client_id\": \"%s\","
    "\"friendly_device_name\": \"%s\","
    "\"accepts_challenges\": [\"Captcha\", \"TwoStep\"],"
    "\"locale\": \"%s\","
    "%s"  // persistent_id
    "\"fallback\": { \"name\": \"GetOAuth2Token\" }"
    "}";

const char GaiaAuthFetcher::kClientOAuthChallengeResponseFormat[] =
    "{"
    "  \"challenge_reply\" : {"
    "    \"name\" : \"%s\","
    "    \"challenge_token\" : \"%s\","
    "    \"%s\" : \"%s\""
    "  }"
    "}";

const char GaiaAuthFetcher::kOAuthLoginFormat[] = "service=%s&source=%s";

// static
const char GaiaAuthFetcher::kAccountDeletedError[] = "AccountDeleted";
const char GaiaAuthFetcher::kAccountDeletedErrorCode[] = "adel";
// static
const char GaiaAuthFetcher::kAccountDisabledError[] = "AccountDisabled";
const char GaiaAuthFetcher::kAccountDisabledErrorCode[] = "adis";
// static
const char GaiaAuthFetcher::kBadAuthenticationError[] = "BadAuthentication";
const char GaiaAuthFetcher::kBadAuthenticationErrorCode[] = "badauth";
// static
const char GaiaAuthFetcher::kCaptchaError[] = "CaptchaRequired";
const char GaiaAuthFetcher::kCaptchaErrorCode[] = "cr";
// static
const char GaiaAuthFetcher::kServiceUnavailableError[] =
    "ServiceUnavailable";
const char GaiaAuthFetcher::kServiceUnavailableErrorCode[] =
    "ire";
// static
const char GaiaAuthFetcher::kErrorParam[] = "Error";
// static
const char GaiaAuthFetcher::kErrorUrlParam[] = "Url";
// static
const char GaiaAuthFetcher::kCaptchaUrlParam[] = "CaptchaUrl";
// static
const char GaiaAuthFetcher::kCaptchaTokenParam[] = "CaptchaToken";

// static
const char GaiaAuthFetcher::kNeedsAdditional[] = "NeedsAdditional";
// static
const char GaiaAuthFetcher::kCaptcha[] = "Captcha";
// static
const char GaiaAuthFetcher::kTwoFactor[] = "TwoStep";

// static
const char GaiaAuthFetcher::kCookiePersistence[] = "true";
// static
// TODO(johnnyg): When hosted accounts are supported by sync,
// we can always use "HOSTED_OR_GOOGLE"
const char GaiaAuthFetcher::kAccountTypeHostedOrGoogle[] =
    "HOSTED_OR_GOOGLE";
const char GaiaAuthFetcher::kAccountTypeGoogle[] =
    "GOOGLE";

// static
const char GaiaAuthFetcher::kSecondFactor[] = "Info=InvalidSecondFactor";

// static
const char GaiaAuthFetcher::kAuthHeaderFormat[] =
    "Authorization: GoogleLogin auth=%s";
// static
const char GaiaAuthFetcher::kOAuthHeaderFormat[] = "Authorization: OAuth %s";
// static
const char GaiaAuthFetcher::kClientLoginToOAuth2CookiePartSecure[] = "Secure";
// static
const char GaiaAuthFetcher::kClientLoginToOAuth2CookiePartHttpOnly[] =
    "HttpOnly";
// static
const char GaiaAuthFetcher::kClientLoginToOAuth2CookiePartCodePrefix[] =
    "oauth_code=";
// static
const int GaiaAuthFetcher::kClientLoginToOAuth2CookiePartCodePrefixLength =
    arraysize(GaiaAuthFetcher::kClientLoginToOAuth2CookiePartCodePrefix) - 1;

GaiaAuthFetcher::GaiaAuthFetcher(GaiaAuthConsumer* consumer,
                                 const std::string& source,
                                 net::URLRequestContextGetter* getter)
    : consumer_(consumer),
      getter_(getter),
      source_(source),
      client_login_gurl_(GaiaUrls::GetInstance()->client_login_url()),
      issue_auth_token_gurl_(GaiaUrls::GetInstance()->issue_auth_token_url()),
      oauth2_token_gurl_(GaiaUrls::GetInstance()->oauth2_token_url()),
      get_user_info_gurl_(GaiaUrls::GetInstance()->get_user_info_url()),
      merge_session_gurl_(GaiaUrls::GetInstance()->merge_session_url()),
      uberauth_token_gurl_(base::StringPrintf(kUberAuthTokenURLFormat,
          GaiaUrls::GetInstance()->oauth1_login_url().c_str(), source.c_str())),
      client_oauth_gurl_(GaiaUrls::GetInstance()->client_oauth_url()),
      oauth_login_gurl_(GaiaUrls::GetInstance()->oauth1_login_url()),
      client_login_to_oauth2_gurl_(
          GaiaUrls::GetInstance()->client_login_to_oauth2_url()),
      fetch_pending_(false) {}

GaiaAuthFetcher::~GaiaAuthFetcher() {}

bool GaiaAuthFetcher::HasPendingFetch() {
  return fetch_pending_;
}

void GaiaAuthFetcher::CancelRequest() {
  fetcher_.reset();
  fetch_pending_ = false;
}

// static
net::URLFetcher* GaiaAuthFetcher::CreateGaiaFetcher(
    net::URLRequestContextGetter* getter,
    const std::string& body,
    const std::string& headers,
    const GURL& gaia_gurl,
    int load_flags,
    net::URLFetcherDelegate* delegate) {
  net::URLFetcher* to_return = content::URLFetcher::Create(
      0, gaia_gurl,
      body == "" ? net::URLFetcher::GET : net::URLFetcher::POST,
      delegate);
  to_return->SetRequestContext(getter);
  to_return->SetUploadData("application/x-www-form-urlencoded", body);

  // The Gaia token exchange requests do not require any cookie-based
  // identification as part of requests.  We suppress sending any cookies to
  // maintain a separation between the user's browsing and Chrome's internal
  // services.  Where such mixing is desired (MergeSession), it will be done
  // explicitly.
  to_return->SetLoadFlags(load_flags);

  if (!headers.empty())
    to_return->SetExtraRequestHeaders(headers);

  return to_return;
}

// static
std::string GaiaAuthFetcher::MakeClientLoginBody(
    const std::string& username,
    const std::string& password,
    const std::string& source,
    const char* service,
    const std::string& login_token,
    const std::string& login_captcha,
    HostedAccountsSetting allow_hosted_accounts) {
  std::string encoded_username = net::EscapeUrlEncodedData(username, true);
  std::string encoded_password = net::EscapeUrlEncodedData(password, true);
  std::string encoded_login_token = net::EscapeUrlEncodedData(login_token,
                                                              true);
  std::string encoded_login_captcha = net::EscapeUrlEncodedData(login_captcha,
                                                                true);

  const char* account_type = allow_hosted_accounts == HostedAccountsAllowed ?
      kAccountTypeHostedOrGoogle :
      kAccountTypeGoogle;

  if (login_token.empty() || login_captcha.empty()) {
    return base::StringPrintf(kClientLoginFormat,
                              encoded_username.c_str(),
                              encoded_password.c_str(),
                              kCookiePersistence,
                              account_type,
                              source.c_str(),
                              service);
  }

  return base::StringPrintf(kClientLoginCaptchaFormat,
                            encoded_username.c_str(),
                            encoded_password.c_str(),
                            kCookiePersistence,
                            account_type,
                            source.c_str(),
                            service,
                            encoded_login_token.c_str(),
                            encoded_login_captcha.c_str());
}

// static
std::string GaiaAuthFetcher::MakeIssueAuthTokenBody(
    const std::string& sid,
    const std::string& lsid,
    const char* const service) {
  std::string encoded_sid = net::EscapeUrlEncodedData(sid, true);
  std::string encoded_lsid = net::EscapeUrlEncodedData(lsid, true);

  // All tokens should be session tokens except the gaia auth token.
  bool session = true;
  if (!strcmp(service, GaiaConstants::kGaiaService))
    session = false;

  return base::StringPrintf(kIssueAuthTokenFormat,
                            encoded_sid.c_str(),
                            encoded_lsid.c_str(),
                            service,
                            session ? "true" : "false");
}

// static
std::string GaiaAuthFetcher::MakeGetAuthCodeBody() {
  std::string encoded_scope = net::EscapeUrlEncodedData(
      GaiaUrls::GetInstance()->oauth1_login_scope(), true);
  std::string encoded_client_id = net::EscapeUrlEncodedData(
      GaiaUrls::GetInstance()->oauth2_chrome_client_id(), true);
  return StringPrintf(kClientLoginToOAuth2BodyFormat,
                      encoded_scope.c_str(),
                      encoded_client_id.c_str());
}

// static
std::string GaiaAuthFetcher::MakeGetTokenPairBody(
    const std::string& auth_code) {
  std::string encoded_scope = net::EscapeUrlEncodedData(
      GaiaUrls::GetInstance()->oauth1_login_scope(), true);
  std::string encoded_client_id = net::EscapeUrlEncodedData(
      GaiaUrls::GetInstance()->oauth2_chrome_client_id(), true);
  std::string encoded_client_secret = net::EscapeUrlEncodedData(
      GaiaUrls::GetInstance()->oauth2_chrome_client_secret(), true);
  std::string encoded_auth_code = net::EscapeUrlEncodedData(auth_code, true);
  return StringPrintf(kOAuth2CodeToTokenPairBodyFormat,
                      encoded_scope.c_str(),
                      encoded_client_id.c_str(),
                      encoded_client_secret.c_str(),
                      encoded_auth_code.c_str());
}

// static
std::string GaiaAuthFetcher::MakeGetUserInfoBody(const std::string& lsid) {
  std::string encoded_lsid = net::EscapeUrlEncodedData(lsid, true);
  return base::StringPrintf(kGetUserInfoFormat, encoded_lsid.c_str());
}

// static
std::string GaiaAuthFetcher::MakeMergeSessionBody(
    const std::string& auth_token,
    const std::string& continue_url,
    const std::string& source) {
  std::string encoded_auth_token = net::EscapeUrlEncodedData(auth_token, true);
  std::string encoded_continue_url = net::EscapeUrlEncodedData(continue_url,
                                                               true);
  std::string encoded_source = net::EscapeUrlEncodedData(source, true);
  return base::StringPrintf(kMergeSessionFormat,
                            encoded_auth_token.c_str(),
                            encoded_continue_url.c_str(),
                            encoded_source.c_str());
}

// static
std::string GaiaAuthFetcher::MakeGetAuthCodeHeader(
    const std::string& auth_token) {
  return StringPrintf(kAuthHeaderFormat, auth_token.c_str());
}

// Helper method that extracts tokens from a successful reply.
// static
void GaiaAuthFetcher::ParseClientLoginResponse(const std::string& data,
                                               std::string* sid,
                                               std::string* lsid,
                                               std::string* token) {
  using std::vector;
  using std::pair;
  using std::string;

  vector<pair<string, string> > tokens;
  base::SplitStringIntoKeyValuePairs(data, '=', '\n', &tokens);
  for (vector<pair<string, string> >::iterator i = tokens.begin();
      i != tokens.end(); ++i) {
    if (i->first == "SID") {
      sid->assign(i->second);
    } else if (i->first == "LSID") {
      lsid->assign(i->second);
    } else if (i->first == "Auth") {
      token->assign(i->second);
    }
  }
}

// static
std::string GaiaAuthFetcher::MakeClientOAuthBody(
    const std::string& username,
    const std::string& password,
    const std::vector<std::string>& scopes,
    const std::string& persistent_id,
    const std::string& friendly_name,
    const std::string& locale) {
  // Convert the scope list info a comma-separated list of strings, surrounded
  // by double quotes.
  std::string scope_list;
  for (size_t i = 0; i < scopes.size(); ++i) {
    base::StringAppendF(&scope_list, "\"%s\"", scopes[i].c_str());
    if (i < scopes.size() - 1)
      scope_list += ',';
  }

  std::string persistent_id_line;
  if (!persistent_id.empty())
    persistent_id_line = base::StringPrintf("\"persistent_id\": \"%s\",",
                                            persistent_id.c_str());

  return StringPrintf(
      kClientOAuthFormat, username.c_str(), password.c_str(),
      scope_list.c_str(),
      GaiaUrls::GetInstance()->oauth2_chrome_client_id().c_str(),
      friendly_name.c_str(), locale.c_str(), persistent_id_line.c_str());
}

// static
std::string GaiaAuthFetcher::MakeClientOAuthChallengeResponseBody(
    const std::string& name,
    const std::string& token,
    const std::string& solution) {
  std::string field_name = name == kTwoFactor ? "otp" : "solution";

  return StringPrintf(kClientOAuthChallengeResponseFormat, name.c_str(),
                      token.c_str(), field_name.c_str(), solution.c_str());
}

// static
std::string GaiaAuthFetcher::MakeOAuthLoginBody(const std::string& service,
                                                const std::string& source) {
  std::string encoded_service = net::EscapeUrlEncodedData(service, true);
  std::string encoded_source = net::EscapeUrlEncodedData(source, true);
  return StringPrintf(kOAuthLoginFormat, encoded_service.c_str(),
                      encoded_source.c_str());
}

// static
void GaiaAuthFetcher::ParseClientLoginFailure(const std::string& data,
                                              std::string* error,
                                              std::string* error_url,
                                              std::string* captcha_url,
                                              std::string* captcha_token) {
  using std::vector;
  using std::pair;
  using std::string;

  vector<pair<string, string> > tokens;
  base::SplitStringIntoKeyValuePairs(data, '=', '\n', &tokens);
  for (vector<pair<string, string> >::iterator i = tokens.begin();
       i != tokens.end(); ++i) {
    if (i->first == kErrorParam) {
      error->assign(i->second);
    } else if (i->first == kErrorUrlParam) {
      error_url->assign(i->second);
    } else if (i->first == kCaptchaUrlParam) {
      captcha_url->assign(i->second);
    } else if (i->first == kCaptchaTokenParam) {
      captcha_token->assign(i->second);
    }
  }
}

// static
bool GaiaAuthFetcher::ParseClientLoginToOAuth2Response(
    const net::ResponseCookies& cookies,
    std::string* auth_code) {
  DCHECK(auth_code);
  net::ResponseCookies::const_iterator iter;
  for (iter = cookies.begin(); iter != cookies.end(); ++iter) {
    if (ParseClientLoginToOAuth2Cookie(*iter, auth_code))
      return true;
  }
  return false;
}

// static
bool GaiaAuthFetcher::ParseClientLoginToOAuth2Cookie(const std::string& cookie,
                                                     std::string* auth_code) {
  std::vector<std::string> parts;
  base::SplitString(cookie, ';', &parts);
  // Per documentation, the cookie should have Secure and HttpOnly.
  if (!CookiePartsContains(parts, kClientLoginToOAuth2CookiePartSecure) ||
      !CookiePartsContains(parts, kClientLoginToOAuth2CookiePartHttpOnly)) {
    return false;
  }

  std::vector<std::string>::const_iterator iter;
  for (iter = parts.begin(); iter != parts.end(); ++iter) {
    const std::string& part = *iter;
    if (StartsWithASCII(
        part, kClientLoginToOAuth2CookiePartCodePrefix, false)) {
      auth_code->assign(part.substr(
          kClientLoginToOAuth2CookiePartCodePrefixLength));
      return true;
    }
  }
  return false;
}

// static
GoogleServiceAuthError
GaiaAuthFetcher::GenerateClientOAuthError(const std::string& data,
                                          const net::URLRequestStatus& status) {
  scoped_ptr<base::Value> value(base::JSONReader::Read(data));
  if (!value.get() || value->GetType() != base::Value::TYPE_DICTIONARY)
    return GenerateAuthError(data, status);
  DictionaryValue* dict = static_cast<DictionaryValue*>(value.get());

  std::string cause;
  if (!dict->GetStringWithoutPathExpansion("cause", &cause))
    return GenerateAuthError(data, status);

  if (cause == kBadAuthenticationError) {
    return GoogleServiceAuthError(
        GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS);
  } else if (cause != kNeedsAdditional) {
    return GenerateAuthError(data, status);
  }

  DictionaryValue* challenge;
  if (!dict->GetDictionaryWithoutPathExpansion("challenge", &challenge))
    return GenerateAuthError(data, status);

  std::string name;
  if (!challenge->GetStringWithoutPathExpansion("name", &name))
    return GenerateAuthError(data, status);

  if (name == kCaptcha) {
    std::string token;
    std::string audio_url;
    std::string image_url;
    int image_width;
    int image_height;
    if (!challenge->GetStringWithoutPathExpansion("challenge_token", &token) ||
        !challenge->GetStringWithoutPathExpansion("audio_url", &audio_url) ||
        !challenge->GetStringWithoutPathExpansion("image_url", &image_url) ||
        !challenge->GetIntegerWithoutPathExpansion("image_width",
                                                   &image_width) ||
        !challenge->GetIntegerWithoutPathExpansion("image_height",
                                                   &image_height)) {
      return GenerateAuthError(data, status);
    }
    return GoogleServiceAuthError::FromCaptchaChallenge(token, GURL(audio_url),
                                                        GURL(image_url),
                                                        image_width,
                                                        image_height);
  } else if (name == kTwoFactor) {
    std::string token;
    std::string prompt_text;
    std::string alternate_text;
    int field_length;

    // The protocol doc says these are required, but in practice they are not
    // returned.  So only a missing challenge token will cause an error here.
    challenge->GetStringWithoutPathExpansion("prompt_text", &prompt_text);
    challenge->GetStringWithoutPathExpansion("alternate_text", &alternate_text);
    challenge->GetIntegerWithoutPathExpansion("field_length", &field_length);
    if (!challenge->GetStringWithoutPathExpansion("challenge_token", &token))
      return GenerateAuthError(data, status);

    return GoogleServiceAuthError::FromSecondFactorChallenge(token, prompt_text,
                                                             alternate_text,
                                                             field_length);
  }

  return GenerateAuthError(data, status);
}

void GaiaAuthFetcher::StartClientLogin(
    const std::string& username,
    const std::string& password,
    const char* const service,
    const std::string& login_token,
    const std::string& login_captcha,
    HostedAccountsSetting allow_hosted_accounts) {

  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  // This class is thread agnostic, so be sure to call this only on the
  // same thread each time.
  DVLOG(1) << "Starting new ClientLogin fetch for:" << username;

  // Must outlive fetcher_.
  request_body_ = MakeClientLoginBody(username,
                                      password,
                                      source_,
                                      service,
                                      login_token,
                                      login_captcha,
                                      allow_hosted_accounts);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   client_login_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartIssueAuthToken(const std::string& sid,
                                          const std::string& lsid,
                                          const char* const service) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting IssueAuthToken for: " << service;
  requested_service_ = service;
  request_body_ = MakeIssueAuthTokenBody(sid, lsid, service);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   issue_auth_token_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartLsoForOAuthLoginTokenExchange(
    const std::string& auth_token) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting OAuth login token exchange with auth_token";
  request_body_ = MakeGetAuthCodeBody();
  client_login_to_oauth2_gurl_ =
      GURL(GaiaUrls::GetInstance()->client_login_to_oauth2_url());

  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   MakeGetAuthCodeHeader(auth_token),
                                   client_login_to_oauth2_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartCookieForOAuthLoginTokenExchange(
    const std::string& session_index) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting OAuth login token fetch with cookie jar";
  request_body_ = MakeGetAuthCodeBody();

  std::string url = GaiaUrls::GetInstance()->client_login_to_oauth2_url();
  if (!session_index.empty())
    url += "?authuser=" + session_index;

  client_login_to_oauth2_gurl_ = GURL(url);

  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   client_login_to_oauth2_gurl_,
                                   net::LOAD_NORMAL,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartGetUserInfo(const std::string& lsid) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting GetUserInfo for lsid=" << lsid;
  request_body_ = MakeGetUserInfoBody(lsid);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   get_user_info_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartMergeSession(const std::string& uber_token) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting MergeSession with uber_token=" << uber_token;

  // The continue URL is a required parameter of the MergeSession API, but in
  // this case we don't actually need or want to navigate to it.  Setting it to
  // an arbitrary Google URL.
  //
  // In order for the new session to be merged correctly, the server needs to
  // know what sessions already exist in the browser.  The fetcher needs to be
  // created such that it sends the cookies with the request, which is
  // different from all other requests the fetcher can make.
  std::string continue_url("http://www.google.com");
  request_body_ = MakeMergeSessionBody(uber_token, continue_url, source_);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   merge_session_gurl_,
                                   net::LOAD_NORMAL,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartTokenFetchForUberAuthExchange(
    const std::string& access_token) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting StartTokenFetchForUberAuthExchange with access_token="
           << access_token;
  std::string authentication_header =
      base::StringPrintf(kOAuthHeaderFormat, access_token.c_str());
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   "",
                                   authentication_header,
                                   uberauth_token_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartClientOAuth(const std::string& username,
                                       const std::string& password,
                                       const std::vector<std::string>& scopes,
                                       const std::string& persistent_id,
                                       const std::string& locale) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  request_body_ = MakeClientOAuthBody(username, password, scopes, persistent_id,
                                      source_, locale);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   client_oauth_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartClientOAuthChallengeResponse(
    GoogleServiceAuthError::State type,
    const std::string& token,
    const std::string& solution) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  std::string name;
  switch (type) {
    case GoogleServiceAuthError::CAPTCHA_REQUIRED:
      name = kCaptcha;
      break;
    case GoogleServiceAuthError::TWO_FACTOR:
      name = kTwoFactor;
      break;
    default:
      NOTREACHED();
  }

  request_body_ = MakeClientOAuthChallengeResponseBody(name, token, solution);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   client_oauth_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::StartOAuthLogin(const std::string& access_token,
                                      const std::string& service) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  request_body_ = MakeOAuthLoginBody(service, source_);
  std::string authentication_header =
      base::StringPrintf("Authorization: Bearer %s", access_token.c_str());
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   authentication_header,
                                   oauth_login_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

// static
GoogleServiceAuthError GaiaAuthFetcher::GenerateAuthError(
    const std::string& data,
    const net::URLRequestStatus& status) {
  if (!status.is_success()) {
    if (status.status() == net::URLRequestStatus::CANCELED) {
      return GoogleServiceAuthError(GoogleServiceAuthError::REQUEST_CANCELED);
    } else {
      DLOG(WARNING) << "Could not reach Google Accounts servers: errno "
          << status.error();
      return GoogleServiceAuthError::FromConnectionError(status.error());
    }
  } else {
    if (IsSecondFactorSuccess(data)) {
      return GoogleServiceAuthError(GoogleServiceAuthError::TWO_FACTOR);
    }

    std::string error;
    std::string url;
    std::string captcha_url;
    std::string captcha_token;
    ParseClientLoginFailure(data, &error, &url, &captcha_url, &captcha_token);
    DLOG(WARNING) << "ClientLogin failed with " << error;

    if (error == kCaptchaError) {
      GURL image_url(
          GaiaUrls::GetInstance()->captcha_url_prefix() + captcha_url);
      GURL unlock_url(url);
      return GoogleServiceAuthError::FromClientLoginCaptchaChallenge(
          captcha_token, image_url, unlock_url);
    }
    if (error == kAccountDeletedError)
      return GoogleServiceAuthError(GoogleServiceAuthError::ACCOUNT_DELETED);
    if (error == kAccountDisabledError)
      return GoogleServiceAuthError(GoogleServiceAuthError::ACCOUNT_DISABLED);
    if (error == kBadAuthenticationError) {
      return GoogleServiceAuthError(
          GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS);
    }
    if (error == kServiceUnavailableError) {
      return GoogleServiceAuthError(
          GoogleServiceAuthError::SERVICE_UNAVAILABLE);
    }

    DLOG(WARNING) << "Incomprehensible response from Google Accounts servers.";
    return GoogleServiceAuthError(
        GoogleServiceAuthError::SERVICE_UNAVAILABLE);
  }

  NOTREACHED();
  return GoogleServiceAuthError(GoogleServiceAuthError::SERVICE_UNAVAILABLE);
}

// static
GoogleServiceAuthError GaiaAuthFetcher::GenerateOAuthLoginError(
    const std::string& data,
    const net::URLRequestStatus& status) {
  if (!status.is_success()) {
    if (status.status() == net::URLRequestStatus::CANCELED) {
      return GoogleServiceAuthError(GoogleServiceAuthError::REQUEST_CANCELED);
    } else {
      DLOG(WARNING) << "Could not reach Google Accounts servers: errno "
          << status.error();
      return GoogleServiceAuthError::FromConnectionError(status.error());
    }
  } else {
    if (IsSecondFactorSuccess(data)) {
      return GoogleServiceAuthError(GoogleServiceAuthError::TWO_FACTOR);
    }

    std::string error;
    std::string url;
    std::string captcha_url;
    std::string captcha_token;
    ParseClientLoginFailure(data, &error, &url, &captcha_url, &captcha_token);
    LOG(WARNING) << "OAuthLogin failed with " << error;

    if (error == kCaptchaErrorCode) {
      GURL image_url(
          GaiaUrls::GetInstance()->captcha_url_prefix() + captcha_url);
      GURL unlock_url(url);
      return GoogleServiceAuthError::FromClientLoginCaptchaChallenge(
          captcha_token, image_url, unlock_url);
    }
    if (error == kAccountDeletedErrorCode)
      return GoogleServiceAuthError(GoogleServiceAuthError::ACCOUNT_DELETED);
    if (error == kAccountDisabledErrorCode)
      return GoogleServiceAuthError(GoogleServiceAuthError::ACCOUNT_DISABLED);
    if (error == kBadAuthenticationErrorCode) {
      return GoogleServiceAuthError(
          GoogleServiceAuthError::INVALID_GAIA_CREDENTIALS);
    }
    if (error == kServiceUnavailableErrorCode) {
      return GoogleServiceAuthError(
          GoogleServiceAuthError::SERVICE_UNAVAILABLE);
    }

    DLOG(WARNING) << "Incomprehensible response from Google Accounts servers.";
    return GoogleServiceAuthError(
        GoogleServiceAuthError::SERVICE_UNAVAILABLE);
  }

  NOTREACHED();
  return GoogleServiceAuthError(GoogleServiceAuthError::SERVICE_UNAVAILABLE);
}

void GaiaAuthFetcher::OnClientLoginFetched(const std::string& data,
                                           const net::URLRequestStatus& status,
                                           int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    DVLOG(1) << "ClientLogin successful!";
    std::string sid;
    std::string lsid;
    std::string token;
    ParseClientLoginResponse(data, &sid, &lsid, &token);
    consumer_->OnClientLoginSuccess(
        GaiaAuthConsumer::ClientLoginResult(sid, lsid, token, data));
  } else {
    consumer_->OnClientLoginFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnIssueAuthTokenFetched(
    const std::string& data,
    const net::URLRequestStatus& status,
    int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    // Only the bare token is returned in the body of this Gaia call
    // without any padding.
    consumer_->OnIssueAuthTokenSuccess(requested_service_, data);
  } else {
    consumer_->OnIssueAuthTokenFailure(requested_service_,
        GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnClientLoginToOAuth2Fetched(
    const std::string& data,
    const net::ResponseCookies& cookies,
    const net::URLRequestStatus& status,
    int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    std::string auth_code;
    ParseClientLoginToOAuth2Response(cookies, &auth_code);
    StartOAuth2TokenPairFetch(auth_code);
  } else {
    consumer_->OnClientOAuthFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::StartOAuth2TokenPairFetch(const std::string& auth_code) {
  DCHECK(!fetch_pending_) << "Tried to fetch two things at once!";

  DVLOG(1) << "Starting OAuth token pair fetch";
  request_body_ = MakeGetTokenPairBody(auth_code);
  fetcher_.reset(CreateGaiaFetcher(getter_,
                                   request_body_,
                                   "",
                                   oauth2_token_gurl_,
                                   kLoadFlagsIgnoreCookies,
                                   this));
  fetch_pending_ = true;
  fetcher_->Start();
}

void GaiaAuthFetcher::OnOAuth2TokenPairFetched(
    const std::string& data,
    const net::URLRequestStatus& status,
    int response_code) {
  std::string refresh_token;
  std::string access_token;
  int expires_in_secs = 0;

  bool success = false;
  if (status.is_success() && response_code == net::HTTP_OK) {
    scoped_ptr<base::Value> value(base::JSONReader::Read(data));
    if (value.get() && value->GetType() == base::Value::TYPE_DICTIONARY) {
      DictionaryValue* dict = static_cast<DictionaryValue*>(value.get());
      success = ExtractOAuth2TokenPairResponse(dict, &refresh_token,
                                               &access_token, &expires_in_secs);
    }
  }

  if (success) {
    consumer_->OnClientOAuthSuccess(
        GaiaAuthConsumer::ClientOAuthResult(refresh_token, access_token,
                                            expires_in_secs));
  } else {
    consumer_->OnClientOAuthFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnGetUserInfoFetched(
    const std::string& data,
    const net::URLRequestStatus& status,
    int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    std::vector<std::pair<std::string, std::string> > tokens;
    UserInfoMap matches;
    base::SplitStringIntoKeyValuePairs(data, '=', '\n', &tokens);
    std::vector<std::pair<std::string, std::string> >::iterator i;
    for (i = tokens.begin(); i != tokens.end(); ++i) {
      matches[i->first] = i->second;
    }
    consumer_->OnGetUserInfoSuccess(matches);
  } else {
    consumer_->OnGetUserInfoFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnMergeSessionFetched(const std::string& data,
                                            const net::URLRequestStatus& status,
                                            int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    consumer_->OnMergeSessionSuccess(data);
  } else {
    consumer_->OnMergeSessionFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnUberAuthTokenFetch(const std::string& data,
                                           const net::URLRequestStatus& status,
                                           int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    consumer_->OnUberAuthTokenSuccess(data);
  } else {
    consumer_->OnUberAuthTokenFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnClientOAuthFetched(const std::string& data,
                                           const net::URLRequestStatus& status,
                                           int response_code) {
  std::string refresh_token;
  std::string access_token;
  int expires_in_secs = 0;

  bool success = false;
  if (status.is_success() && response_code == net::HTTP_OK) {
    scoped_ptr<base::Value> value(base::JSONReader::Read(data));
    if (value.get() && value->GetType() == base::Value::TYPE_DICTIONARY) {
      DictionaryValue* dict = static_cast<DictionaryValue*>(value.get());
      DictionaryValue* dict_oauth2;
      if (dict->GetDictionaryWithoutPathExpansion("oauth2", &dict_oauth2)) {
        success = ExtractOAuth2TokenPairResponse(dict_oauth2, &refresh_token,
                                                 &access_token,
                                                 &expires_in_secs);
      }
    }
  }

  // TODO(rogerta): for now this reuses the OnOAuthLoginTokenXXX callbacks
  // since the data is exactly the same.  This ignores the optional
  // persistent_id data in the response, which we may need to handle.
  // If we do, we'll need to modify ExtractOAuth2TokenPairResponse() to parse
  // the optional data and declare new consumer callbacks to take it.
  if (success) {
    consumer_->OnClientOAuthSuccess(
        GaiaAuthConsumer::ClientOAuthResult(refresh_token, access_token,
                                            expires_in_secs));
  } else {
    consumer_->OnClientOAuthFailure(GenerateClientOAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnOAuthLoginFetched(const std::string& data,
                                          const net::URLRequestStatus& status,
                                          int response_code) {
  if (status.is_success() && response_code == net::HTTP_OK) {
    DVLOG(1) << "ClientLogin successful!";
    std::string sid;
    std::string lsid;
    std::string token;
    ParseClientLoginResponse(data, &sid, &lsid, &token);
    consumer_->OnClientLoginSuccess(
        GaiaAuthConsumer::ClientLoginResult(sid, lsid, token, data));
  } else {
    consumer_->OnClientLoginFailure(GenerateAuthError(data, status));
  }
}

void GaiaAuthFetcher::OnURLFetchComplete(const net::URLFetcher* source) {
  fetch_pending_ = false;
  // Some of the GAIA requests perform redirects, which results in the final
  // URL of the fetcher not being the original URL requested.  Therefore use
  // the original URL when determining which OnXXX function to call.
  const GURL& url = source->GetOriginalURL();
  const net::URLRequestStatus& status = source->GetStatus();
  int response_code = source->GetResponseCode();
  std::string data;
  source->GetResponseAsString(&data);
  if (url == client_login_gurl_) {
    OnClientLoginFetched(data, status, response_code);
  } else if (url == issue_auth_token_gurl_) {
    OnIssueAuthTokenFetched(data, status, response_code);
  } else if (url == client_login_to_oauth2_gurl_) {
    OnClientLoginToOAuth2Fetched(
        data, source->GetCookies(), status, response_code);
  } else if (url == oauth2_token_gurl_) {
    OnOAuth2TokenPairFetched(data, status, response_code);
  } else if (url == get_user_info_gurl_) {
    OnGetUserInfoFetched(data, status, response_code);
  } else if (url == merge_session_gurl_) {
    OnMergeSessionFetched(data, status, response_code);
  } else if (url == uberauth_token_gurl_) {
    OnUberAuthTokenFetch(data, status, response_code);
  } else if (url == client_oauth_gurl_) {
    OnClientOAuthFetched(data, status, response_code);
  } else if (url == oauth_login_gurl_) {
    OnOAuthLoginFetched(data, status, response_code);
  } else {
    NOTREACHED();
  }
}

// static
bool GaiaAuthFetcher::IsSecondFactorSuccess(
    const std::string& alleged_error) {
  return alleged_error.find(kSecondFactor) !=
      std::string::npos;
}
