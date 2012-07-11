// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_COMMON_GPU_FEATURE_TYPE_H_
#define CONTENT_PUBLIC_COMMON_GPU_FEATURE_TYPE_H_

namespace content {

// Provides flags indicating which gpu features are blacklisted for the system
// on which chrome is currently running.
// If a bit is set to 1, corresponding feature is blacklisted.
enum GpuFeatureType {
  GPU_FEATURE_TYPE_ACCELERATED_2D_CANVAS = 1 << 0,
  GPU_FEATURE_TYPE_ACCELERATED_COMPOSITING = 1 << 1,
  GPU_FEATURE_TYPE_WEBGL = 1 << 2,
  GPU_FEATURE_TYPE_MULTISAMPLING = 1 << 3,
  GPU_FEATURE_TYPE_FLASH3D = 1 << 4,
  GPU_FEATURE_TYPE_FLASH_STAGE3D = 1 << 5,
  GPU_FEATURE_TYPE_ALL = GPU_FEATURE_TYPE_ACCELERATED_2D_CANVAS |
                         GPU_FEATURE_TYPE_ACCELERATED_COMPOSITING |
                         GPU_FEATURE_TYPE_WEBGL |
                         GPU_FEATURE_TYPE_MULTISAMPLING |
                         GPU_FEATURE_TYPE_FLASH3D |
                         GPU_FEATURE_TYPE_FLASH_STAGE3D,
  // All flags initialized to false, i.e., no feature is blacklisted.
  GPU_FEATURE_TYPE_UNKNOWN = 0
};

}  // namespace content

#endif  // CONTENT_PUBLIC_COMMON_GPU_FEATURE_TYPE_H_
