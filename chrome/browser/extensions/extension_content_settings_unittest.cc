// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"

#include "chrome/browser/extensions/extension_content_settings_helpers.h"

namespace helpers = extension_content_settings_helpers;

TEST(ExtensionContentSettingsHelpersTest, ParseExtensionPattern) {
  const struct {
    const char* extension_pattern;
    const char* content_settings_pattern;
  } kTestPatterns[] = {
    { "<all_urls>", "*" },
    { "*://*.google.com/*", "[*.]google.com" },
    { "http://www.example.com/*", "http://www.example.com" },
    { "*://www.example.com/*", "www.example.com" },
    { "http://www.example.com:8080/*", "http://www.example.com:8080" },
    { "https://*/*", "https://*" },
    { "file:///foo/bar/baz", "file:///foo/bar/baz" },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(kTestPatterns); ++i) {
    std::string error;
    std::string pattern_str = helpers::ParseExtensionPattern(
        kTestPatterns[i].extension_pattern, &error).ToString();
    EXPECT_EQ(kTestPatterns[i].content_settings_pattern, pattern_str)
        << "Unexpected error parsing " << kTestPatterns[i].extension_pattern
        << ": " << error;
  }

  const struct {
    const char* extension_pattern;
    const char* expected_error;
  } kInvalidTestPatterns[] = {
    { "http://www.example.com/path", "Specific paths are not allowed." },
    { "file:///foo/bar/*",
      "Path wildcards in file URL patterns are not allowed." },
  };
  for (size_t i = 0; i < ARRAYSIZE_UNSAFE(kInvalidTestPatterns); ++i) {
    std::string error;
    ContentSettingsPattern pattern = helpers::ParseExtensionPattern(
        kInvalidTestPatterns[i].extension_pattern, &error);
    EXPECT_FALSE(pattern.IsValid());
    EXPECT_EQ(kInvalidTestPatterns[i].expected_error, error)
        << "Unexpected error parsing "
        << kInvalidTestPatterns[i].extension_pattern;
  }

}
