#!/usr/bin/env python
#
# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Convert Android xml resources to API 14 compatible.

There are two reasons that we cannot just use API attributes,
so we are generating another set of resources by this script.

1. paddingStart attribute can cause a crash on Galaxy Tab 2.
2. There is a bug that paddingStart does not override paddingLeft on
   JB-MR1. This is fixed on JB-MR2.

Therefore, this resource generation script can be removed when
we drop the support for JB-MR1.

Please refer to http://crbug.com/235118 for the details.
"""

import optparse
import os
import re
import sys
import xml.dom.minidom as minidom

from util import build_utils

# Note that we are assuming 'android:' is an alias of
# the namespace 'http://schemas.android.com/apk/res/android'.

GRAVITY_ATTRIBUTES = ('android:gravity', 'android:layout_gravity')

# Almost all the attributes that has "Start" or "End" in
# its name should be mapped.
ATTRIBUTES_TO_MAP = {'paddingStart' : 'paddingLeft',
                     'drawableStart' : 'drawableLeft',
                     'layout_alignStart' : 'layout_alignLeft',
                     'layout_marginStart' : 'layout_marginLeft',
                     'layout_alignParentStart' : 'layout_alignParentLeft',
                     'layout_toStartOf' : 'layout_toLeftOf',
                     'paddingEnd' : 'paddingRight',
                     'drawableEnd' : 'drawableRight',
                     'layout_alignEnd' : 'layout_alignRight',
                     'layout_marginEnd' : 'layout_marginRight',
                     'layout_alignParentEnd' : 'layout_alignParentRight',
                     'layout_toEndOf' : 'layout_toRightOf'}

ATTRIBUTES_TO_MAP = dict(['android:' + k, 'android:' + v] for k, v
                         in ATTRIBUTES_TO_MAP.iteritems())

ATTRIBUTES_TO_MAP_REVERSED = dict([v,k] for k, v
                                  in ATTRIBUTES_TO_MAP.iteritems())


def IterateXmlElements(node):
  """minidom helper function that iterates all the element nodes.
  Iteration order is pre-order depth-first."""
  if node.nodeType == node.ELEMENT_NODE:
    yield node
  for child_node in node.childNodes:
    for child_node_element in IterateXmlElements(child_node):
      yield child_node_element


def WarnDeprecatedAttribute(name, value, filename):
  if name in ATTRIBUTES_TO_MAP_REVERSED:
    print >> sys.stderr, ('warning: ' + filename + ' should use ' +
                          ATTRIBUTES_TO_MAP_REVERSED[name] +
                          ' instead of ' + name)
  elif name in GRAVITY_ATTRIBUTES and ('left' in value or 'right' in value):
    print >> sys.stderr, ('warning: ' + filename +
                          ' should use start/end instead of left/right for ' +
                          name)


def GenerateV14StyleResource(input_filename, output_filename):
  """Convert style resource to API 14 compatible style resource.

  It's mostly a simple replacement, s/Start/Left s/End/Right,
  on the attribute names specified by <item> element.
  If input_filename does not contain style resources, do nothing.
  """
  dom = minidom.parse(input_filename)
  style_elements = dom.getElementsByTagName('style')

  if not style_elements:
    return

  for style_element in style_elements:
    for item_element in style_element.getElementsByTagName('item'):
      name = item_element.attributes['name'].value
      value = item_element.childNodes[0].nodeValue
      if name in ATTRIBUTES_TO_MAP:
        item_element.attributes['name'].value = ATTRIBUTES_TO_MAP[name]
      else:
        WarnDeprecatedAttribute(name, value, input_filename)

  build_utils.MakeDirectory(os.path.dirname(output_filename))
  with open(output_filename, 'w') as f:
    dom.writexml(f, '', '  ', '\n', encoding='utf-8')


def GenerateV14LayoutResource(input_filename, output_filename):
  """Convert layout resource to API 14 compatible layout resource.

  It's mostly a simple replacement, s/Start/Left s/End/Right,
  on the attribute names.
  """
  dom = minidom.parse(input_filename)

  # Iterate all the elements' attributes to find attributes to convert.
  for element in IterateXmlElements(dom):
    for name, value in list(element.attributes.items()):
      # Convert any other API 17 Start/End attributes to Left/Right attributes.
      # For example, from paddingStart="10dp" to paddingLeft="10dp"
      # Note: gravity attributes are not necessary to convert because
      # start/end values are backward-compatible. Explained at
      # https://plus.sandbox.google.com/+RomanNurik/posts/huuJd8iVVXY?e=Showroom
      if name in ATTRIBUTES_TO_MAP:
        element.setAttribute(ATTRIBUTES_TO_MAP[name], value)
        del element.attributes[name]
      else:
        WarnDeprecatedAttribute(name, value, input_filename)

  build_utils.MakeDirectory(os.path.dirname(output_filename))
  with open(output_filename, 'w') as f:
    dom.writexml(f, '', '  ', '\n', encoding='utf-8')


def GenerateV14XmlResourcesInDir(input_dir, output_dir, only_styles=False):
  """Convert resources to API 14 compatible XML resources in the directory."""
  for input_filename in build_utils.FindInDirectory(input_dir, '*.xml'):
    output_filename = os.path.join(output_dir,
                               os.path.relpath(input_filename, input_dir))
    if only_styles:
      GenerateV14StyleResource(input_filename, output_filename)
    else:
      GenerateV14LayoutResource(input_filename, output_filename)


def ParseArgs():
  """Parses command line options.

  Returns:
    An options object as from optparse.OptionsParser.parse_args()
  """
  parser = optparse.OptionParser()
  parser.add_option('--res-dir',
                    help='directory containing resources '
                         'used to generate v14 resources')
  parser.add_option('--res-v14-dir',
                    help='output directory into which '
                         'v14 resources will be generated')
  parser.add_option('--stamp', help='File to touch on success')

  options, args = parser.parse_args()

  if args:
    parser.error('No positional arguments should be given.')

  # Check that required options have been provided.
  required_options = ('res_dir', 'res_v14_dir')
  build_utils.CheckOptions(options, parser, required=required_options)
  return options


def main(argv):
  options = ParseArgs()

  build_utils.DeleteDirectory(options.res_v14_dir)
  build_utils.MakeDirectory(options.res_v14_dir)

  for name in os.listdir(options.res_dir):
    if not os.path.isdir(os.path.join(options.res_dir, name)):
      continue

    dir_pieces = name.split('-')
    resource_type = dir_pieces[0]
    qualifiers = dir_pieces[1:]

    # Android pre-v17 API doesn't support RTL. Skip.
    if 'ldrtl' in qualifiers:
      continue

    input_dir = os.path.join(options.res_dir, name)
    output_dir = os.path.join(options.res_v14_dir, name)

    # We only convert resources under layout*/, xml*/,
    # and style resources under values*/.
    # TODO(kkimlabs): don't process xml directly once all layouts have
    # been moved out of XML directory. see http://crbug.com/238458
    if resource_type in ('layout', 'xml'):
      GenerateV14XmlResourcesInDir(input_dir, output_dir)
    elif resource_type in ('values'):
      GenerateV14XmlResourcesInDir(input_dir, output_dir, only_styles=True)

  if options.stamp:
    build_utils.Touch(options.stamp)

if __name__ == '__main__':
  sys.exit(main(sys.argv))

