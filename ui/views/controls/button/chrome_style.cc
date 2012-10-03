// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/controls/button/chrome_style.h"

#include <algorithm>

#include "ui/gfx/canvas.h"
#include "ui/gfx/color_utils.h"
#include "ui/views/painter.h"
#include "ui/views/background.h"

namespace views {
namespace {
const int kMinWidth = 72;
const int kMinHeight = 27;

// Fractional position between top and bottom of button where the
// gradient starts.
const SkScalar kGradientStartLocation = SkFloatToScalar(0.38f);
const SkColor kNormalBackgroundTopColor = SkColorSetRGB(0xf0, 0xf0, 0xf0);
const SkColor kNormalBackgroundBottomColor = SkColorSetRGB(0xe0, 0xe0, 0xe0);
const SkColor kHotBackgroundTopColor = SkColorSetRGB(0xf4, 0xf4, 0xf4);
const SkColor kHotBackgroundBottomColor = SkColorSetRGB(0xe4, 0xe4, 0xe4);
const SkColor kPushedBackgroundTopColor = SkColorSetRGB(0xeb, 0xeb, 0xeb);
const SkColor kPushedBackgroundBottomColor = SkColorSetRGB(0xdb, 0xdb, 0xdb);
const SkColor kDisabledBackgroundTopColor = SkColorSetRGB(0xed, 0xed, 0xed);
const SkColor kDisabledBackgroundBottomColor = SkColorSetRGB(0xde, 0xde, 0xde);

const SkColor kEnabledTextColor = SkColorSetRGB(0x33, 0x33, 0x33);
const SkColor kDisabledTextColor = SkColorSetRGB(0xaa, 0xaa, 0xaa);
const SkColor kHoverTextColor = SkColorSetRGB(0x0, 0x0, 0x0);

const SkColor kTextShadowColor = SkColorSetRGB(0xf0, 0xf0, 0xf0);
const int kTextShadowOffsetX = 0;
const int kTextShadowOffsetY = 1;

const int kBorderWidth = 1;
const int kBorderRadius = 2;
const SkColor kBorderNormalColor = SkColorSetARGB(0x3f, 0x0, 0x0, 0x0);
const SkColor kBorderActiveColor = SkColorSetARGB(0x4b, 0x0, 0x0, 0x0);
const SkColor kBorderDisabledColor = SkColorSetARGB(0x1d, 0x0, 0x0, 0x0);

const int kFocusRingWidth = 2;
const int kFocusRingRadius = 2;
const SkColor kFocusRingColor = SkColorSetARGB(0x7f, 0xe5, 0x97, 0x00);

// Returns the uniform inset of the button from its local bounds.
int GetButtonInset() {
  return std::max(kBorderWidth, kFocusRingWidth);
}

class ChromeStyleTextButtonBackground : public Background {
 public:
  ChromeStyleTextButtonBackground() {
  }

  virtual ~ChromeStyleTextButtonBackground() {
  }

  // Overriden from Background
  virtual void Paint(gfx::Canvas* canvas, View* view) const {
    if (painter_.get())
    {
      gfx::Rect bounds = view->GetLocalBounds();
      // Inset to the actual button region.
      int inset = GetButtonInset();
      bounds.Inset(inset, inset, inset, inset);
      Painter::PaintPainterAt(canvas, painter_.get(), bounds);
    }
  }

  void SetColors(SkColor top, SkColor bottom) {
    static const int count = 3;
    SkColor colors[count] = { top, top, bottom };
    SkScalar pos[count] = {
      SkFloatToScalar(0.0f), kGradientStartLocation, SkFloatToScalar(1.0f) };

    painter_.reset(
        Painter::CreateVerticalMultiColorGradient(colors, pos, count));
    SetNativeControlColor(
        color_utils::AlphaBlend(colors[0], colors[count - 1], 128));
  }

 private:
  scoped_ptr<Painter> painter_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStyleTextButtonBackground);
};

class ChromeStyleTextButtonBorderPainter : public views::Painter {
 public:
  ChromeStyleTextButtonBorderPainter()
      // This value should be updated prior to rendering; it's set to a
      // well-defined value here defensively.
      : color_(SkColorSetRGB(0x0, 0x0, 0x0)) {
  }

  // Overriden from Painter
  virtual void Paint(gfx::Canvas* canvas, const gfx::Size& size) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setColor(color_);
    paint.setStrokeWidth(kBorderWidth);

    // Inset by 1/2 pixel to align the stroke with pixel centers.
    SkScalar inset = SkFloatToScalar(0.5f);
    SkRect rect = SkRect::MakeLTRB(inset, inset,
                                   SkIntToScalar(size.width()) - inset,
                                   SkIntToScalar(size.height()) - inset);

    canvas->sk_canvas()->drawRoundRect(
        rect, kBorderRadius, kBorderRadius, paint);
  }

  void set_color(SkColor color) {
    color_ = color;
  }

 private:
  SkColor color_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStyleTextButtonBorderPainter);
};


class ChromeStyleTextButtonBorder : public views::Border {
 public:
  ChromeStyleTextButtonBorder()
      : painter_(new ChromeStyleTextButtonBorderPainter) {
  }

  // Overriden from Border
  virtual void Paint(const View& view, gfx::Canvas* canvas) const {
    gfx::Rect bounds = view.GetLocalBounds();
    int border_inset = GetButtonInset() - kBorderWidth;
    bounds.Inset(border_inset, border_inset, border_inset, border_inset);
    Painter::PaintPainterAt(canvas, painter_, bounds);
  }
  virtual void GetInsets(gfx::Insets* insets) const {
    DCHECK(insets);
    int inset = GetButtonInset();
    insets->Set(inset, inset, inset, inset);
  }

  void SetColor(SkColor color) {
    painter_->set_color(color);
  }

 private:
  ChromeStyleTextButtonBorderPainter* painter_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStyleTextButtonBorder);
};

class ChromeStyleFocusBorder : public views::FocusBorder {
 public:
  ChromeStyleFocusBorder() {}

  // Overriden from Border
  virtual void Paint(const View& view, gfx::Canvas* canvas) const {
    gfx::Rect rect(view.GetLocalBounds());
    SkScalar inset = SkFloatToScalar(GetButtonInset() - kFocusRingWidth / 2.0f);
    rect.Inset(inset, inset);
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(kFocusRingWidth));
    paint.setColor(kFocusRingColor);
    canvas->DrawRoundRect(rect, SkIntToScalar(kFocusRingRadius), paint);
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ChromeStyleFocusBorder);
};

class ChromeStyleStateChangedUpdater
    : public CustomButtonStateChangedDelegate {
 public:
  // The background and border may be NULL.
  ChromeStyleStateChangedUpdater(TextButton* button,
                                 ChromeStyleTextButtonBackground* background,
                                 ChromeStyleTextButtonBorder* border)
      : button_(button),
        background_(background),
        border_(border),
        prior_state_(button->state()) {
    SetBackgroundForState(button->state());
    SetShadowForState(button->state());
    SetBorderColorForState(button->state());
  }

  virtual void StateChanged(CustomButton::ButtonState state) {
    SetBackgroundForState(state);

    // Update text shadow when transitioning to/from pushed state.
    if (state == CustomButton::BS_PUSHED ||
        prior_state_ == CustomButton::BS_PUSHED) {
      SetShadowForState(state);
    }

    // Update border color.  We need to change it in all cases except hot
    // followed by pushed.
    if (!(state == CustomButton::BS_PUSHED &&
          prior_state_ == CustomButton::BS_HOT)) {
      SetBorderColorForState(state);
    }

    prior_state_ = state;
  }

 private:
  void SetBackgroundForState(CustomButton::ButtonState state) {
    if (!background_)
      return;

    SkColor top;
    SkColor bottom;

    switch (state) {
      case CustomButton::BS_NORMAL:
        top = kNormalBackgroundTopColor;
        bottom = kNormalBackgroundBottomColor;
        break;

      case CustomButton::BS_HOT:
        top = kHotBackgroundTopColor;
        bottom = kHotBackgroundBottomColor;
        break;

      case CustomButton::BS_PUSHED:
        top = kPushedBackgroundTopColor;
        bottom = kPushedBackgroundBottomColor;
        break;

      case CustomButton::BS_DISABLED:
        top = kDisabledBackgroundTopColor;
        bottom = kDisabledBackgroundBottomColor;
        break;

      default:
        NOTREACHED();
        return;
        break;
    }

    background_->SetColors(top, bottom);
  }

  void SetShadowForState(CustomButton::ButtonState state) {
    if (state == CustomButton::BS_PUSHED) {
      // Turn off text shadow.
      button_->ClearEmbellishing();
    } else {
      button_->SetTextShadowColors(kTextShadowColor, kTextShadowColor);
      button_->SetTextShadowOffset(kTextShadowOffsetX, kTextShadowOffsetY);
    }
  }

  void SetBorderColorForState(CustomButton::ButtonState state) {
    if (!border_)
      return;

    SkColor border_color;

    switch (state) {
      case CustomButton::BS_NORMAL:
        border_color = kBorderNormalColor;
        break;

      case CustomButton::BS_HOT:
      case CustomButton::BS_PUSHED:
        border_color = kBorderActiveColor;
        break;

      case CustomButton::BS_DISABLED:
        border_color = kBorderDisabledColor;
        break;

      default:
        NOTREACHED();
        return;
        break;
    }

    border_->SetColor(border_color);
  }

  // Weak pointer to the associated button.
  TextButton* button_;

  // Weak pointers to background and border owned by the CustomButton.
  ChromeStyleTextButtonBackground* background_;
  ChromeStyleTextButtonBorder* border_;

  CustomButton::ButtonState prior_state_;

  DISALLOW_COPY_AND_ASSIGN(ChromeStyleStateChangedUpdater);
};
}  // namespace


void ApplyChromeStyle(TextButton* button) {
  button->set_focusable(true);
  button->set_request_focus_on_press(false);

  button->set_alignment(TextButton::ALIGN_CENTER);
  button->set_min_width(kMinWidth);
  button->set_min_height(kMinHeight);

  button->SetEnabledColor(kEnabledTextColor);
  button->SetDisabledColor(kDisabledTextColor);
  button->SetHoverColor(kHoverTextColor);

  ChromeStyleTextButtonBackground* background =
      new ChromeStyleTextButtonBackground;
  button->set_background(background);

  ChromeStyleTextButtonBorder* border = new ChromeStyleTextButtonBorder;
  button->set_border(border);

  button->set_focus_border(new ChromeStyleFocusBorder);

  ChromeStyleStateChangedUpdater* state_changed_updater =
      new ChromeStyleStateChangedUpdater(button, background, border);

  button->set_state_changed_delegate(state_changed_updater);
}

}  // namespace views
