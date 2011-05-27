// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_
#define VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_
#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>

#include <string>
#include <vector>

#include "base/memory/scoped_ptr.h"
#include "base/memory/scoped_vector.h"
#include "base/message_loop.h"
#include "base/win/scoped_comptr.h"
#include "ui/base/win/window_impl.h"
#include "views/focus/focus_manager.h"
#include "views/ime/input_method_delegate.h"
#include "views/layout/layout_manager.h"
#include "views/widget/native_widget.h"

namespace ui {
class ViewProp;
}

namespace gfx {
class CanvasSkia;
class Rect;
}

namespace views {

class DropTargetWin;
class RootView;
class TooltipManagerWin;
class Window;

namespace internal {
class NativeWidgetDelegate;
}

// A Windows message reflected from other windows. This message is sent
// with the following arguments:
// hWnd - Target window
// uMsg - kReflectedMessage
// wParam - Should be 0
// lParam - Pointer to MSG struct containing the original message.
const int kReflectedMessage = WM_APP + 3;

// These two messages aren't defined in winuser.h, but they are sent to windows
// with captions. They appear to paint the window caption and frame.
// Unfortunately if you override the standard non-client rendering as we do
// with CustomFrameWindow, sometimes Windows (not deterministically
// reproducibly but definitely frequently) will send these messages to the
// window and paint the standard caption/title over the top of the custom one.
// So we need to handle these messages in CustomFrameWindow to prevent this
// from happening.
const int WM_NCUAHDRAWCAPTION = 0xAE;
const int WM_NCUAHDRAWFRAME = 0xAF;

///////////////////////////////////////////////////////////////////////////////
//
// NativeWidgetWin
//  A Widget for a views hierarchy used to represent anything that can be
//  contained within an HWND, e.g. a control, a window, etc. Specializations
//  suitable for specific tasks, e.g. top level window, are derived from this.
//
//  This Widget contains a RootView which owns the hierarchy of views within it.
//  As long as views are part of this tree, they will be deleted automatically
//  when the RootView is destroyed. If you remove a view from the tree, you are
//  then responsible for cleaning up after it.
//
///////////////////////////////////////////////////////////////////////////////
class NativeWidgetWin : public ui::WindowImpl,
                        public NativeWidget,
                        public MessageLoopForUI::Observer,
                        public internal::InputMethodDelegate {
 public:
  explicit NativeWidgetWin(internal::NativeWidgetDelegate* delegate);
  virtual ~NativeWidgetWin();

  // Returns true if we are on Windows Vista or greater and composition is
  // enabled.
  static bool IsAeroGlassEnabled();

  // Disable Layered Window updates by setting to false.
  void set_can_update_layered_window(bool can_update_layered_window) {
    can_update_layered_window_ = can_update_layered_window;
  }

  // Obtain the view event with the given MSAA child id.  Used in
  // NativeViewAccessibilityWin::get_accChild to support requests for
  // children of windowless controls.  May return NULL
  // (see ViewHierarchyChanged).
  View* GetAccessibilityViewEventAt(int id);

  // Add a view that has recently fired an accessibility event.  Returns a MSAA
  // child id which is generated by: -(index of view in vector + 1) which
  // guarantees a negative child id.  This distinguishes the view from
  // positive MSAA child id's which are direct leaf children of views that have
  // associated hWnd's (e.g. NativeWidgetWin).
  int AddAccessibilityViewEvent(View* view);

  // Clear a view that has recently been removed on a hierarchy change.
  void ClearAccessibilityViewEvent(View* view);

  BOOL IsWindow() const {
    return ::IsWindow(GetNativeView());
  }

  BOOL ShowWindow(int command) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::ShowWindow(GetNativeView(), command);
  }

  HWND GetParent() const {
    return ::GetParent(GetNativeView());
  }

  LONG GetWindowLong(int index) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::GetWindowLong(GetNativeView(), index);
  }

  BOOL GetWindowRect(RECT* rect) const {
    return ::GetWindowRect(GetNativeView(), rect);
  }

  LONG SetWindowLong(int index, LONG new_long) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::SetWindowLong(GetNativeView(), index, new_long);
  }

  BOOL SetWindowPos(HWND hwnd_after, int x, int y, int cx, int cy, UINT flags) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::SetWindowPos(GetNativeView(), hwnd_after, x, y, cx, cy, flags);
  }

  BOOL IsZoomed() const {
    DCHECK(::IsWindow(GetNativeView()));
    return ::IsZoomed(GetNativeView());
  }

  BOOL MoveWindow(int x, int y, int width, int height) {
    return MoveWindow(x, y, width, height, TRUE);
  }

  BOOL MoveWindow(int x, int y, int width, int height, BOOL repaint) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::MoveWindow(GetNativeView(), x, y, width, height, repaint);
  }

  int SetWindowRgn(HRGN region, BOOL redraw) {
    DCHECK(::IsWindow(GetNativeView()));
    return ::SetWindowRgn(GetNativeView(), region, redraw);
  }

  BOOL GetClientRect(RECT* rect) const {
    DCHECK(::IsWindow(GetNativeView()));
    return ::GetClientRect(GetNativeView(), rect);
  }

  // Overridden from NativeWidget:
  virtual void InitNativeWidget(const Widget::InitParams& params) OVERRIDE;
  virtual Widget* GetWidget() OVERRIDE;
  virtual const Widget* GetWidget() const OVERRIDE;
  virtual gfx::NativeView GetNativeView() const OVERRIDE;
  virtual gfx::NativeWindow GetNativeWindow() const OVERRIDE;
  virtual Window* GetContainingWindow() OVERRIDE;
  virtual const Window* GetContainingWindow() const OVERRIDE;
  virtual void ViewRemoved(View* view) OVERRIDE;
  virtual void SetNativeWindowProperty(const char* name, void* value) OVERRIDE;
  virtual void* GetNativeWindowProperty(const char* name) OVERRIDE;
  virtual TooltipManager* GetTooltipManager() const OVERRIDE;
  virtual bool IsScreenReaderActive() const OVERRIDE;
  virtual void SendNativeAccessibilityEvent(
      View* view,
      ui::AccessibilityTypes::Event event_type) OVERRIDE;
  virtual void SetMouseCapture() OVERRIDE;
  virtual void ReleaseMouseCapture() OVERRIDE;
  virtual bool HasMouseCapture() const OVERRIDE;
  virtual bool IsMouseButtonDown() const OVERRIDE;
  virtual InputMethod* GetInputMethodNative() OVERRIDE;
  virtual void ReplaceInputMethod(InputMethod* input_method) OVERRIDE;
  virtual gfx::Rect GetWindowScreenBounds() const OVERRIDE;
  virtual gfx::Rect GetClientAreaScreenBounds() const OVERRIDE;
  virtual void SetBounds(const gfx::Rect& bounds) OVERRIDE;
  virtual void SetSize(const gfx::Size& size) OVERRIDE;
  virtual void SetBoundsConstrained(const gfx::Rect& bounds,
                                    Widget* other_widget) OVERRIDE;
  virtual void MoveAbove(gfx::NativeView native_view) OVERRIDE;
  virtual void SetShape(gfx::NativeRegion shape) OVERRIDE;
  virtual void Close() OVERRIDE;
  virtual void CloseNow() OVERRIDE;
  virtual void Show() OVERRIDE;
  virtual void Hide() OVERRIDE;
  virtual bool IsVisible() const OVERRIDE;
  virtual void Activate() OVERRIDE;
  virtual void Deactivate() OVERRIDE;
  virtual bool IsActive() const OVERRIDE;
  virtual void SetAlwaysOnTop(bool always_on_top) OVERRIDE;
  virtual void Maximize() OVERRIDE;
  virtual void Minimize() OVERRIDE;
  virtual bool IsMaximized() const OVERRIDE;
  virtual bool IsMinimized() const OVERRIDE;
  virtual void Restore() OVERRIDE;
  virtual void SetOpacity(unsigned char opacity) OVERRIDE;
  virtual bool IsAccessibleWidget() const OVERRIDE;
  virtual bool ContainsNativeView(gfx::NativeView native_view) const OVERRIDE;
  virtual void RunShellDrag(View* view,
                            const ui::OSExchangeData& data,
                            int operation) OVERRIDE;
  virtual void SchedulePaintInRect(const gfx::Rect& rect) OVERRIDE;
  virtual void SetCursor(gfx::NativeCursor cursor) OVERRIDE;

 protected:
  // Overridden from MessageLoop::Observer:
  void WillProcessMessage(const MSG& msg) OVERRIDE;
  virtual void DidProcessMessage(const MSG& msg) OVERRIDE;

  // Overridden from WindowImpl:
  virtual HICON GetDefaultWindowIcon() const OVERRIDE;
  virtual LRESULT OnWndProc(UINT message,
                            WPARAM w_param,
                            LPARAM l_param) OVERRIDE;

  // Message Handlers ----------------------------------------------------------

  BEGIN_MSG_MAP_EX(NativeWidgetWin)
    // Range handlers must go first!
    MESSAGE_RANGE_HANDLER_EX(WM_MOUSEFIRST, WM_MOUSELAST, OnMouseRange)
    MESSAGE_RANGE_HANDLER_EX(WM_NCMOUSEMOVE, WM_NCXBUTTONDBLCLK, OnMouseRange)

    // Reflected message handler
    MESSAGE_HANDLER_EX(kReflectedMessage, OnReflectedMessage)

    // CustomFrameWindow hacks
    MESSAGE_HANDLER_EX(WM_NCUAHDRAWCAPTION, OnNCUAHDrawCaption)
    MESSAGE_HANDLER_EX(WM_NCUAHDRAWFRAME, OnNCUAHDrawFrame)

    // Vista and newer
    MESSAGE_HANDLER_EX(WM_DWMCOMPOSITIONCHANGED, OnDwmCompositionChanged)

    // Non-atlcrack.h handlers
    MESSAGE_HANDLER_EX(WM_GETOBJECT, OnGetObject)

    // Mouse events.
    MESSAGE_HANDLER_EX(WM_MOUSEACTIVATE, OnMouseActivate)
    MESSAGE_HANDLER_EX(WM_MOUSELEAVE, OnMouseRange)
    MESSAGE_HANDLER_EX(WM_NCMOUSELEAVE, OnMouseRange)
    MESSAGE_HANDLER_EX(WM_SETCURSOR, OnSetCursor);

    // Key events.
    MESSAGE_HANDLER_EX(WM_KEYDOWN, OnKeyEvent)
    MESSAGE_HANDLER_EX(WM_KEYUP, OnKeyEvent)
    MESSAGE_HANDLER_EX(WM_SYSKEYDOWN, OnKeyEvent)
    MESSAGE_HANDLER_EX(WM_SYSKEYUP, OnKeyEvent)

    // IME Events.
    MESSAGE_HANDLER_EX(WM_IME_SETCONTEXT, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_IME_COMPOSITION, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_IME_ENDCOMPOSITION, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_CHAR, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_SYSCHAR, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_DEADCHAR, OnImeMessages)
    MESSAGE_HANDLER_EX(WM_SYSDEADCHAR, OnImeMessages)

    // This list is in _ALPHABETICAL_ order! OR I WILL HURT YOU.
    MSG_WM_ACTIVATE(OnActivate)
    MSG_WM_ACTIVATEAPP(OnActivateApp)
    MSG_WM_APPCOMMAND(OnAppCommand)
    MSG_WM_CANCELMODE(OnCancelMode)
    MSG_WM_CAPTURECHANGED(OnCaptureChanged)
    MSG_WM_CLOSE(OnClose)
    MSG_WM_COMMAND(OnCommand)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_DISPLAYCHANGE(OnDisplayChange)
    MSG_WM_ERASEBKGND(OnEraseBkgnd)
    MSG_WM_ENDSESSION(OnEndSession)
    MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
    MSG_WM_EXITMENULOOP(OnExitMenuLoop)
    MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
    MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
    MSG_WM_HSCROLL(OnHScroll)
    MSG_WM_INITMENU(OnInitMenu)
    MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
    MSG_WM_INPUTLANGCHANGE(OnInputLangChange)
    MSG_WM_KILLFOCUS(OnKillFocus)
    MSG_WM_MOVE(OnMove)
    MSG_WM_MOVING(OnMoving)
    MSG_WM_NCACTIVATE(OnNCActivate)
    MSG_WM_NCCALCSIZE(OnNCCalcSize)
    MSG_WM_NCHITTEST(OnNCHitTest)
    MSG_WM_NCPAINT(OnNCPaint)
    MSG_WM_NOTIFY(OnNotify)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_POWERBROADCAST(OnPowerBroadcast)
    MSG_WM_SETFOCUS(OnSetFocus)
    MSG_WM_SETICON(OnSetIcon)
    MSG_WM_SETTEXT(OnSetText)
    MSG_WM_SETTINGCHANGE(OnSettingChange)
    MSG_WM_SIZE(OnSize)
    MSG_WM_SYSCOMMAND(OnSysCommand)
    MSG_WM_THEMECHANGED(OnThemeChanged)
    MSG_WM_VSCROLL(OnVScroll)
    MSG_WM_WINDOWPOSCHANGING(OnWindowPosChanging)
    MSG_WM_WINDOWPOSCHANGED(OnWindowPosChanged)
  END_MSG_MAP()

  // These are all virtual so that specialized Widgets can modify or augment
  // processing.
  // This list is in _ALPHABETICAL_ order!
  // Note: in the base class these functions must do nothing but convert point
  //       coordinates to client coordinates (if necessary) and forward the
  //       handling to the appropriate Process* function. This is so that
  //       subclasses can easily override these methods to do different things
  //       and have a convenient function to call to get the default behavior.
  virtual void OnActivate(UINT action, BOOL minimized, HWND window);
  virtual void OnActivateApp(BOOL active, DWORD thread_id);
  virtual LRESULT OnAppCommand(HWND window, short app_command, WORD device,
                               int keystate);
  virtual void OnCancelMode();
  virtual void OnCaptureChanged(HWND hwnd);
  virtual void OnClose();
  virtual void OnCommand(UINT notification_code, int command_id, HWND window);
  virtual LRESULT OnCreate(CREATESTRUCT* create_struct);
  // WARNING: If you override this be sure and invoke super, otherwise we'll
  // leak a few things.
  virtual void OnDestroy();
  virtual void OnDisplayChange(UINT bits_per_pixel, CSize screen_size);
  virtual LRESULT OnDwmCompositionChanged(UINT msg,
                                          WPARAM w_param,
                                          LPARAM l_param);
  virtual void OnEndSession(BOOL ending, UINT logoff);
  virtual void OnEnterSizeMove();
  virtual LRESULT OnEraseBkgnd(HDC dc);
  virtual void OnExitMenuLoop(BOOL is_track_popup_menu);
  virtual void OnExitSizeMove();
  virtual LRESULT OnGetObject(UINT uMsg, WPARAM w_param, LPARAM l_param);
  virtual void OnGetMinMaxInfo(MINMAXINFO* minmax_info);
  virtual void OnHScroll(int scroll_type, short position, HWND scrollbar);
  virtual LRESULT OnImeMessages(UINT message, WPARAM w_param, LPARAM l_param);
  virtual void OnInitMenu(HMENU menu);
  virtual void OnInitMenuPopup(HMENU menu, UINT position, BOOL is_system_menu);
  virtual void OnInputLangChange(DWORD character_set, HKL input_language_id);
  virtual LRESULT OnKeyEvent(UINT message, WPARAM w_param, LPARAM l_param);
  virtual void OnKillFocus(HWND focused_window);
  virtual LRESULT OnMouseActivate(UINT message, WPARAM w_param, LPARAM l_param);
  virtual LRESULT OnMouseRange(UINT message, WPARAM w_param, LPARAM l_param);
  virtual void OnMove(const CPoint& point);
  virtual void OnMoving(UINT param, LPRECT new_bounds);
  virtual LRESULT OnNCActivate(BOOL active);
  virtual LRESULT OnNCCalcSize(BOOL w_param, LPARAM l_param);
  virtual LRESULT OnNCHitTest(const CPoint& pt);
  virtual void OnNCPaint(HRGN rgn);
  virtual LRESULT OnNCUAHDrawCaption(UINT msg,
                                     WPARAM w_param,
                                     LPARAM l_param);
  virtual LRESULT OnNCUAHDrawFrame(UINT msg, WPARAM w_param, LPARAM l_param);
  virtual LRESULT OnNotify(int w_param, NMHDR* l_param);
  virtual void OnPaint(HDC dc);
  virtual LRESULT OnPowerBroadcast(DWORD power_event, DWORD data);
  virtual LRESULT OnReflectedMessage(UINT msg, WPARAM w_param, LPARAM l_param);
  virtual LRESULT OnSetCursor(UINT message, WPARAM w_param, LPARAM l_param);
  virtual void OnSetFocus(HWND focused_window);
  virtual LRESULT OnSetIcon(UINT size_type, HICON new_icon);
  virtual LRESULT OnSetText(const wchar_t* text);
  virtual void OnSettingChange(UINT flags, const wchar_t* section);
  virtual void OnSize(UINT param, const CSize& size);
  virtual void OnSysCommand(UINT notification_code, CPoint click);
  virtual void OnThemeChanged();
  virtual void OnVScroll(int scroll_type, short position, HWND scrollbar);
  virtual void OnWindowPosChanging(WINDOWPOS* window_pos);
  virtual void OnWindowPosChanged(WINDOWPOS* window_pos);

  // Deletes this window as it is destroyed, override to provide different
  // behavior.
  virtual void OnFinalMessage(HWND window);

  // Start tracking all mouse events so that this window gets sent mouse leave
  // messages too.
  void TrackMouseEvents(DWORD mouse_tracking_flags);

  // Called when a MSAA screen reader client is detected.
  virtual void OnScreenReaderDetected();

  // Sets-up the focus manager with the view that should have focus when the
  // window is shown the first time.  If NULL is returned, the focus goes to the
  // button if there is one, otherwise the to the Cancel button.
  virtual void SetInitialFocus();

  // Executes the specified SC_command.
  void ExecuteSystemMenuCommand(int command);

  // The TooltipManager.
  // WARNING: RootView's destructor calls into the TooltipManager. As such, this
  // must be destroyed AFTER root_view_.
  scoped_ptr<TooltipManagerWin> tooltip_manager_;

  scoped_refptr<DropTargetWin> drop_target_;

  // Are a subclass of NativeWindowWin?
  bool is_window_;

  const gfx::Rect& invalid_rect() const { return invalid_rect_; }

 private:
  typedef ScopedVector<ui::ViewProp> ViewProps;

  // Implementation of GetWindow. Ascends the parents of |hwnd| returning the
  // first ancestor that is a Window.
  static Window* GetWindowImpl(HWND hwnd);

  // Called after the WM_ACTIVATE message has been processed by the default
  // windows procedure.
  static void PostProcessActivateMessage(NativeWidgetWin* widget,
                                         int activation_state);

  void SetInitParams(const Widget::InitParams& params);

  // Synchronously paints the invalid contents of the Widget.
  void RedrawInvalidRect();

  // Synchronously updates the invalid contents of the Widget. Valid for
  // layered windows only.
  void RedrawLayeredWindowContents();

  // Responds to the client area changing size, either at window creation time
  // or subsequently.
  void ClientAreaSizeChanged();

  // Overridden from NativeWidget.
  virtual gfx::AcceleratedWidget GetAcceleratedWidget() OVERRIDE;

  // Overridden from internal::InputMethodDelegate
  virtual void DispatchKeyEventPostIME(const KeyEvent& key) OVERRIDE;

  // A delegate implementation that handles events received here.
  // See class documentation for Widget in widget.h for a note about ownership.
  internal::NativeWidgetDelegate* delegate_;

  // The following factory is used for calls to close the NativeWidgetWin
  // instance.
  ScopedRunnableMethodFactory<NativeWidgetWin> close_widget_factory_;

  // The flags currently being used with TrackMouseEvent to track mouse
  // messages. 0 if there is no active tracking. The value of this member is
  // used when tracking is canceled.
  DWORD active_mouse_tracking_flags_;

  // Should we keep an off-screen buffer? This is false by default, set to true
  // when WS_EX_LAYERED is specified before the native window is created.
  //
  // NOTE: this is intended to be used with a layered window (a window with an
  // extended window style of WS_EX_LAYERED). If you are using a layered window
  // and NOT changing the layered alpha or anything else, then leave this value
  // alone. OTOH if you are invoking SetLayeredWindowAttributes then you'll
  // most likely want to set this to false, or after changing the alpha toggle
  // the extended style bit to false than back to true. See MSDN for more
  // details.
  bool use_layered_buffer_;

  // The default alpha to be applied to the layered window.
  BYTE layered_alpha_;

  // A canvas that contains the window contents in the case of a layered
  // window.
  scoped_ptr<gfx::CanvasSkia> layered_window_contents_;

  // We must track the invalid rect ourselves, for two reasons:
  // For layered windows, Windows will not do this properly with
  // InvalidateRect()/GetUpdateRect(). (In fact, it'll return misleading
  // information from GetUpdateRect()).
  // We also need to keep track of the invalid rectangle for the RootView should
  // we need to paint the non-client area. The data supplied to WM_NCPAINT seems
  // to be insufficient.
  gfx::Rect invalid_rect_;

  // A factory that allows us to schedule a redraw for layered windows.
  ScopedRunnableMethodFactory<NativeWidgetWin> paint_layered_window_factory_;

  // See class documentation for Widget in widget.h for a note about ownership.
  Widget::InitParams::Ownership ownership_;

  // True if we are allowed to update the layered window from the DIB backing
  // store if necessary.
  bool can_update_layered_window_;

  // Whether the focus should be restored next time we get enabled.  Needed to
  // restore focus correctly when Windows modal dialogs are displayed.
  bool restore_focus_when_enabled_;

  // Instance of accessibility information and handling for MSAA root
  base::win::ScopedComPtr<IAccessible> accessibility_root_;

  // Value determines whether the Widget is customized for accessibility.
  static bool screen_reader_active_;

  // The maximum number of view events in our vector below.
  static const int kMaxAccessibilityViewEvents = 20;

  // A vector used to access views for which we have sent notifications to
  // accessibility clients.  It is used as a circular queue.
  std::vector<View*> accessibility_view_events_;

  // The current position of the view events vector.  When incrementing,
  // we always mod this value with the max view events above .
  int accessibility_view_events_index_;

  // The last cursor that was active before the current one was selected. Saved
  // so that we can restore it.
  gfx::NativeCursor previous_cursor_;

  ViewProps props_;

  scoped_ptr<InputMethod> input_method_;

  // Indicates if the |input_method_| is an InputMethodWin instance.
  bool is_input_method_win_;

  DISALLOW_COPY_AND_ASSIGN(NativeWidgetWin);
};

}  // namespace views

#endif  // VIEWS_WIDGET_NATIVE_WIDGET_WIN_H_
