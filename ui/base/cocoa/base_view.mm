// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/cocoa/base_view.h"

NSString* kViewDidBecomeFirstResponder =
    @"Chromium.kViewDidBecomeFirstResponder";
NSString* kSelectionDirection = @"Chromium.kSelectionDirection";

const int kTrackingOptions = NSTrackingMouseMoved |
                             NSTrackingMouseEnteredAndExited |
                             NSTrackingActiveAlways;

@implementation BaseView

- (void)dealloc {
  if (trackingArea_)
    [self removeTrackingArea:trackingArea_];
  [trackingArea_ release];

  [super dealloc];
}

- (void)mouseEvent:(NSEvent*)theEvent {
  // This method left intentionally blank.
}

- (void)keyEvent:(NSEvent*)theEvent {
  // This method left intentionally blank.
}

- (void)mouseDown:(NSEvent*)theEvent {
  dragging_ = YES;
  [self mouseEvent:theEvent];
}

- (void)rightMouseDown:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseDown:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseUp:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];

  dragging_ = NO;
  if (pendingExitEvent_.get()) {
    NSEvent* exitEvent =
        [NSEvent enterExitEventWithType:NSMouseExited
                               location:[theEvent locationInWindow]
                          modifierFlags:[theEvent modifierFlags]
                              timestamp:[theEvent timestamp]
                           windowNumber:[theEvent windowNumber]
                                context:[theEvent context]
                            eventNumber:[pendingExitEvent_.get() eventNumber]
                         trackingNumber:[pendingExitEvent_.get() trackingNumber]
                               userData:[pendingExitEvent_.get() userData]];
    [self mouseEvent:exitEvent];
    pendingExitEvent_.reset();
  }
}

- (void)rightMouseUp:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseUp:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseMoved:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseDragged:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)rightMouseDragged:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseDragged:(NSEvent*)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseEntered:(NSEvent*)theEvent {
  if (pendingExitEvent_.get()) {
    pendingExitEvent_.reset();
    return;
  }

  [self mouseEvent:theEvent];
}

- (void)mouseExited:(NSEvent*)theEvent {
  // The tracking area will send an exit event even during a drag, which isn't
  // how the event flow for drags should work. This stores the exit event, and
  // sends it when the drag completes instead.
  if (dragging_) {
    pendingExitEvent_.reset([theEvent retain]);
    return;
  }

  [self mouseEvent:theEvent];
}

- (void)keyDown:(NSEvent*)theEvent {
  [self keyEvent:theEvent];
}

- (void)keyUp:(NSEvent*)theEvent {
  [self keyEvent:theEvent];
}

- (void)flagsChanged:(NSEvent*)theEvent {
  [self keyEvent:theEvent];
}

- (gfx::Rect)flipNSRectToRect:(NSRect)rect {
  gfx::Rect new_rect(NSRectToCGRect(rect));
  new_rect.set_y(NSHeight([self bounds]) - new_rect.bottom());
  return new_rect;
}

- (NSRect)flipRectToNSRect:(gfx::Rect)rect {
  NSRect new_rect(NSRectFromCGRect(rect.ToCGRect()));
  new_rect.origin.y = NSHeight([self bounds]) - NSMaxY(new_rect);
  return new_rect;
}

- (void)updateTrackingAreas {
  [super updateTrackingAreas];

  // NSTrackingInVisibleRect doesn't work correctly with Lion's window resizing,
  // http://crbug.com/176725 /  http://openradar.appspot.com/radar?id=2773401 .
  // Tear down old tracking area and create a new one as workaround.
  if (trackingArea_)
    [self removeTrackingArea:trackingArea_];
  [trackingArea_ release];
  trackingArea_ = [[NSTrackingArea alloc] initWithRect:[self frame]
                                               options:kTrackingOptions
                                                 owner:self
                                              userInfo:nil];
  [self addTrackingArea:trackingArea_];
}

@end
