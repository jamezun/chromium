// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Maps hostnames to custom content settings.  Written on the UI thread and read
// on any thread.  One instance per profile.

#ifndef CHROME_BROWSER_HOST_CONTENT_SETTINGS_MAP_H_
#define CHROME_BROWSER_HOST_CONTENT_SETTINGS_MAP_H_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "base/basictypes.h"
#include "base/lock.h"
#include "base/ref_counted.h"
#include "chrome/common/content_settings.h"

class DictionaryValue;
class PrefService;
class Profile;

class HostContentSettingsMap
    : public base::RefCountedThreadSafe<HostContentSettingsMap> {
 public:
  // Details for the CONTENT_SETTINGS_CHANGED notification. This is sent when
  // content settings change for at least one host. If settings change for more
  // than one host in one user interaction, this will usually send a single
  // notification with a wildcard host field instead of one notification for
  // each host.
  class ContentSettingsDetails {
   public:
    ContentSettingsDetails(const std::string& host) : host_(host) {}
    // The host whose settings have changed. Empty if many hosts are affected
    // (e.g. if the default settings have changed).
    const std::string& host() { return host_; }

   private:
    std::string host_;
  };

  typedef std::pair<std::string, ContentSetting> HostSettingPair;
  typedef std::vector<HostSettingPair> SettingsForOneType;

  explicit HostContentSettingsMap(Profile* profile);

  static void RegisterUserPrefs(PrefService* prefs);

  // Returns the default setting for a particular content type.
  //
  // This may be called on any thread.
  ContentSetting GetDefaultContentSetting(
      ContentSettingsType content_type) const;

  // Returns a single ContentSetting which applies to a given host.
  //
  // This may be called on any thread.
  ContentSetting GetContentSetting(const std::string& host,
                                   ContentSettingsType content_type) const;

  // Returns all ContentSettings which apply to a given host.
  //
  // This may be called on any thread.
  ContentSettings GetContentSettings(const std::string& host) const;

  // For a given content type, returns all hosts with a non-default setting,
  // mapped to their actual settings, in lexicographical order.  |settings| must
  // be a non-NULL outparam.
  //
  // This may be called on any thread.
  void GetSettingsForOneType(ContentSettingsType content_type,
                             SettingsForOneType* settings) const;

  // Sets the default setting for a particular content type.
  //
  // This should only be called on the UI thread.
  void SetDefaultContentSetting(ContentSettingsType content_type,
                                ContentSetting setting);

  // Sets the blocking setting for a particular hostname and content type.
  // Setting the value to CONTENT_SETTING_DEFAULT causes the default setting for
  // that type to be used when loading pages from this host.
  //
  // This should only be called on the UI thread.
  void SetContentSetting(const std::string& host,
                         ContentSettingsType content_type,
                         ContentSetting setting);

  // Clears all host-specific settings for one content type.
  //
  // This should only be called on the UI thread.
  void ClearSettingsForOneType(ContentSettingsType content_type);

  // This setting trumps any host-specific settings.
  bool BlockThirdPartyCookies() const { return block_third_party_cookies_; }

  // Sets whether we block all third-party cookies.
  //
  // This should only be called on the UI thread.
  void SetBlockThirdPartyCookies(bool block);

  // Resets all settings levels.
  //
  // This should only be called on the UI thread.
  void ResetToDefaults();

 private:
  friend class base::RefCountedThreadSafe<HostContentSettingsMap>;

  typedef std::map<std::string, ContentSettings> HostContentSettings;

  // The names of the ContentSettingsType values, for use with dictionary prefs.
  static const wchar_t* kTypeNames[CONTENT_SETTINGS_NUM_TYPES];

  // The default setting for each content type.
  static const ContentSetting kDefaultSettings[CONTENT_SETTINGS_NUM_TYPES];

  ~HostContentSettingsMap();

  // Sets the fields of |settings| based on the values in |dictionary|.
  void GetSettingsFromDictionary(const DictionaryValue* dictionary,
                                 ContentSettings* settings);

  // Forces the default settings to be explicitly set instead of themselves
  // being CONTENT_SETTING_DEFAULT.
  void ForceDefaultsToBeExplicit();

  // Returns true if |settings| consists entirely of CONTENT_SETTING_DEFAULT.
  bool AllDefault(const ContentSettings& settings) const;

  // Informs observers that content settings have changed. Make sure that
  // |lock_| is not held when calling this, as listeners will usually call one
  // of the GetSettings functions in response, which would then lead to a
  // mutex deadlock.
  void NotifyObservers(const std::string& host);

  // The profile we're associated with.
  Profile* profile_;

  // Copies of the pref data, so that we can read it on the IO thread.
  ContentSettings default_content_settings_;
  HostContentSettings host_content_settings_;

  // Misc global settings.
  bool block_third_party_cookies_;

  // Used around accesses to the settings objects to guarantee thread safety.
  mutable Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(HostContentSettingsMap);
};

#endif  // CHROME_BROWSER_HOST_CONTENT_SETTINGS_MAP_H_
