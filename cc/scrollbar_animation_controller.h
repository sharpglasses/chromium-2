// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_SCROLLBAR_ANIMATION_CONTROLLER_H_
#define CC_SCROLLBAR_ANIMATION_CONTROLLER_H_

#include "base/time.h"
#include "cc/base/cc_export.h"
#include "ui/gfx/vector2d_f.h"

namespace cc {

// This abstract class represents the compositor-side analogy of ScrollbarAnimator.
// Individual platforms should subclass it to provide specialized implementation.
class CC_EXPORT ScrollbarAnimationController {
public:
    virtual ~ScrollbarAnimationController() {}

    virtual bool isScrollGestureInProgress() const = 0;
    virtual bool isAnimating() const = 0;
    virtual base::TimeDelta delayBeforeStart(base::TimeTicks now) const = 0;

    virtual bool animate(base::TimeTicks) = 0;
    virtual void didScrollGestureBegin() = 0;
    virtual void didScrollGestureEnd(base::TimeTicks now) = 0;
    virtual void didProgrammaticallyUpdateScroll(base::TimeTicks now) = 0;
};

} // namespace cc

#endif  // CC_SCROLLBAR_ANIMATION_CONTROLLER_H_
