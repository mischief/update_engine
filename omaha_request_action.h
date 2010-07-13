// Copyright (c) 2009 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_PLATFORM_UPDATE_ENGINE_OMAHA_REQUEST_ACTION_H__
#define CHROMEOS_PLATFORM_UPDATE_ENGINE_OMAHA_REQUEST_ACTION_H__

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include <string>

#include <curl/curl.h>

#include "base/scoped_ptr.h"
#include "update_engine/action.h"
#include "update_engine/http_fetcher.h"

// The Omaha Request action makes a request to Omaha and can output
// the response on the output ActionPipe.

namespace chromeos_update_engine {

// Encodes XML entities in a given string with libxml2. input must be
// UTF-8 formatted. Output will be UTF-8 formatted.
std::string XmlEncode(const std::string& input);

// This struct encapsulates the data Omaha gets for the request.
// These strings in this struct should not be XML escaped.
struct OmahaRequestParams {
  OmahaRequestParams()
      : os_platform(kOsPlatform), os_version(kOsVersion), app_id(kAppId) {}
  OmahaRequestParams(const std::string& in_machine_id,
                     const std::string& in_user_id,
                     const std::string& in_os_platform,
                     const std::string& in_os_version,
                     const std::string& in_os_sp,
                     const std::string& in_os_board,
                     const std::string& in_app_id,
                     const std::string& in_app_version,
                     const std::string& in_app_lang,
                     const std::string& in_app_track,
                     const std::string& in_update_url)
      : machine_id(in_machine_id),
        user_id(in_user_id),
        os_platform(in_os_platform),
        os_version(in_os_version),
        os_sp(in_os_sp),
        os_board(in_os_board),
        app_id(in_app_id),
        app_version(in_app_version),
        app_lang(in_app_lang),
        app_track(in_app_track),
        update_url(in_update_url) {}

  std::string machine_id;
  std::string user_id;
  std::string os_platform;
  std::string os_version;
  std::string os_sp;
  std::string os_board;
  std::string app_id;
  std::string app_version;
  std::string app_lang;
  std::string app_track;

  std::string update_url;

  // Suggested defaults
  static const char* const kAppId;
  static const char* const kOsPlatform;
  static const char* const kOsVersion;
  static const char* const kUpdateUrl;
};

// This struct encapsulates the data Omaha's response for the request.
// These strings in this struct are not XML escaped.
struct OmahaResponse {
  OmahaResponse()
      : update_exists(false), size(0), needs_admin(false), prompt(false) {}
  // True iff there is an update to be downloaded.
  bool update_exists;

  // These are only valid if update_exists is true:
  std::string display_version;
  std::string codebase;
  std::string more_info_url;
  std::string hash;
  off_t size;
  bool needs_admin;
  bool prompt;
};
COMPILE_ASSERT(sizeof(off_t) == 8, off_t_not_64bit);

class OmahaRequestAction;
class NoneType;

template<>
class ActionTraits<OmahaRequestAction> {
 public:
  // Takes parameters on the input pipe
  typedef OmahaRequestParams InputObjectType;
  // On success, puts the output path on output
  typedef OmahaResponse OutputObjectType;
};

class OmahaRequestAction : public Action<OmahaRequestAction>,
                           public HttpFetcherDelegate {
 public:
  // The ctor takes in all the parameters that will be used for
  // making the request to Omaha. For some of them we have constants
  // that should be used.
  // Takes ownership of the passed in HttpFetcher. Useful for testing.
  // A good calling pattern is:
  // OmahaRequestAction(..., new WhateverHttpFetcher);
  OmahaRequestAction(HttpFetcher* http_fetcher);
  virtual ~OmahaRequestAction();
  typedef ActionTraits<OmahaRequestAction>::InputObjectType InputObjectType;
  typedef ActionTraits<OmahaRequestAction>::OutputObjectType OutputObjectType;
  void PerformAction();
  void TerminateProcessing();

  // Debugging/logging
  static std::string StaticType() { return "OmahaRequestAction"; }
  std::string Type() const { return StaticType(); }

  // Delegate methods (see http_fetcher.h)
  virtual void ReceivedBytes(HttpFetcher *fetcher,
                             const char* bytes, int length);
  virtual void TransferComplete(HttpFetcher *fetcher, bool successful);

 private:
  // These are data that are passed in the request to the Omaha server
  OmahaRequestParams params_;

  // pointer to the HttpFetcher that does the http work
  scoped_ptr<HttpFetcher> http_fetcher_;

  // Stores the response from the omaha server
  std::vector<char> response_buffer_;

  DISALLOW_COPY_AND_ASSIGN(OmahaRequestAction);
};

}  // namespace chromeos_update_engine

#endif  // CHROMEOS_PLATFORM_UPDATE_ENGINE_OMAHA_REQUEST_ACTION_H__