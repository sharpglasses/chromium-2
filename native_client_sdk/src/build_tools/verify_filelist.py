#!/usr/bin/env python
# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import optparse
import os
import re
import sys

from build_paths import SDK_SRC_DIR

# Add SDK make tools scripts to the python path.
sys.path.append(os.path.join(SDK_SRC_DIR, 'tools'))

import getos

VALID_PLATFORMS = ['linux', 'mac', 'win']

class ParseException(Exception):
  def __init__(self, filename, line, message):
    Exception.__init__(self)
    self.filename = filename
    self.line = line
    self.message = message

  def __str__(self):
    return '%s:%d: %s' % (self.filename, self.line, self.message)


class VerifyException(Exception):
  pass

class Rules(object):
  def __init__(self, platform, filename, contents=None):
    self.glob_prefixes = []
    self.exact_filenames = set()
    self.filename = filename
    self.platform = platform

    if platform not in VALID_PLATFORMS:
      raise ParseException(self.filename, 1, 'Unknown platform %s' % platform)

    if not contents:
      with open(filename) as f:
        contents = f.read()

    for line_no, rule in enumerate(contents.split('\n')):
      rule = rule.strip()
      if rule:
        self.ParsePattern(line_no + 1, rule)

  def ParsePattern(self, line_no, pattern):
    match = re.match(r'^\[([^\]]*)\](.*)$', pattern)
    if match:
      # platform-specific pattern
      platforms = match.group(1).split(',')
      unknown_platforms = set(platforms) - set(VALID_PLATFORMS)
      if unknown_platforms:
        msg = 'Unknown platform(s) %s.' % (
            ', '.join('"%s"' % platform for platform in unknown_platforms))
        raise ParseException(self.filename, line_no, msg)
      if self.platform not in platforms:
        return

      # If this platform is included, strip the [...] part.
      pattern = match.group(2)

    pattern = pattern.replace('${PLATFORM}', self.platform)

    if '*' in pattern:
      # glob pattern
      # We only support * at the end.
      if pattern.find('*') != len(pattern) - 1:
        msg = '* is only allowed at the end of the line.'
        raise ParseException(self.filename, line_no, msg)

      # Remove the *
      pattern = pattern[:-1]
      self.glob_prefixes.append(pattern)
    else:
      self.exact_filenames.add(pattern)

  def VerifyDirectoryList(self, directory_list):
    exact_filenames_used = set()
    glob_prefixes_used = set()
    expected_globs = set()
    expected_filenames = set()
    unexpected_filenames = set()

    for filename in directory_list:
      if os.path.sep != '/':
        filename = filename.replace(os.path.sep, '/')
      if filename in self.exact_filenames:
        exact_filenames_used.add(filename)
        continue

      # glob pattern
      found_prefix = False
      for prefix in self.glob_prefixes:
        if filename.startswith(prefix):
          glob_prefixes_used.add(prefix)
          found_prefix = True
          break

      if not found_prefix:
        unexpected_filenames.add(filename)

    if len(exact_filenames_used) != len(self.exact_filenames):
      # We looped through the directory list, so if the lengths are unequal, it
      # must be that we expected something that isn't there.
      expected_filenames = self.exact_filenames - exact_filenames_used

    if len(glob_prefixes_used) != self.glob_prefixes:
      expected_globs = set(self.glob_prefixes) - glob_prefixes_used

    if expected_filenames or unexpected_filenames or expected_globs:
      msg = ''
      if unexpected_filenames:
        msg += '>>> Unexpected filenames: <<<\n%s\n' % (
            '\n'.join(sorted(unexpected_filenames)))
      if expected_filenames:
        msg += '>>> Expected filenames: <<<\n%s\n' % (
            '\n'.join(sorted(expected_filenames)))
      if expected_globs:
        msg += '>>> Expected 1+ files in these directories: <<< \n%s\n' % (
            '\n'.join(sorted(expected_globs)))
      raise VerifyException(msg)


def GetDirectoryList(directory_path):
  result = []
  for root, _, files in os.walk(directory_path):
    rel_root = os.path.relpath(root, directory_path)
    if rel_root == '.':
      rel_root = ''
    for base_name in files:
      result.append(os.path.join(rel_root, base_name))
  return result


def Verify(platform, rule_path, directory_path):
  rules = Rules(platform, rule_path)
  directory_list = GetDirectoryList(directory_path)
  rules.VerifyDirectoryList(directory_list)


def main(args):
  parser = optparse.OptionParser(usage='%prog <rule file> <directory>')
  parser.add_option('-p', '--platform',
      help='Test with this platform, instead of the system\'s platform')
  options, args = parser.parse_args(args)
  if len(args) != 2:
    parser.error('Expected rule file and directory.')

  rule_path, directory_path = args
  if options.platform:
    if options.platform not in VALID_PLATFORMS:
      parser.error('Unknown platform: %s' % options.platform)
    platform = options.platform
  else:
    platform = getos.GetPlatform()

  try:
    return Verify(platform, rule_path, directory_path)
  except ParseException, e:
    print >> sys.stderr, 'Error parsing rules:\n', e
    return 1
  except VerifyException, e:
    print >> sys.stderr, 'Error verifying file list:\n', e
    return 1
  return 0


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))
