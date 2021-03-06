// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_SYSTEM_CHROMEOS_SETTINGS_TRAY_SETTINGS_H_
#define ASH_SYSTEM_CHROMEOS_SETTINGS_TRAY_SETTINGS_H_

#include "ash/system/tray/system_tray_item.h"
#include "chromeos/power/power_manager_handler.h"

namespace ash {
namespace internal {

namespace tray {
class SettingsDefaultView;
}

class TraySettings : public SystemTrayItem,
                     public chromeos::PowerManagerHandler::Observer {
 public:
  explicit TraySettings(SystemTray* system_tray);
  virtual ~TraySettings();

 private:
  // Overridden from SystemTrayItem
  virtual views::View* CreateTrayView(user::LoginStatus status) OVERRIDE;
  virtual views::View* CreateDefaultView(user::LoginStatus status) OVERRIDE;
  virtual views::View* CreateDetailedView(user::LoginStatus status) OVERRIDE;
  virtual void DestroyTrayView() OVERRIDE;
  virtual void DestroyDefaultView() OVERRIDE;
  virtual void DestroyDetailedView() OVERRIDE;
  virtual void UpdateAfterLoginStatusChange(user::LoginStatus status) OVERRIDE;

  // Overridden from chromeos::PowerManagerHandler::Observer.
  virtual void OnPowerStatusChanged(
      const chromeos::PowerSupplyStatus& status) OVERRIDE;

  tray::SettingsDefaultView* default_view_;

  DISALLOW_COPY_AND_ASSIGN(TraySettings);
};

}  // namespace internal
}  // namespace ash

#endif  // ASH_SYSTEM_CHROMEOS_SETTINGS_TRAY_SETTINGS_H_
