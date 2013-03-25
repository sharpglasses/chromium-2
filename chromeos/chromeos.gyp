# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'variables': {
    'chromium_code': 1,
  },
  'includes': [
    'chromeos_tools.gypi'
  ],
  'targets': [
    {
      'target_name': 'chromeos',
      'type': '<(component)',
      'dependencies': [
        '../base/base.gyp:base',
        '../base/base.gyp:base_prefs',
        '../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../build/linux/system.gyp:dbus',
        '../build/linux/system.gyp:ssl',
        '../dbus/dbus.gyp:dbus',
        '../net/net.gyp:net',
        '../third_party/libxml/libxml.gyp:libxml',
        'power_manager_proto',
        'power_supply_properties_proto',
        'video_activity_update_proto',
      ],
      'defines': [
        'CHROMEOS_IMPLEMENTATION',
      ],
      'sources': [
        'attestation/attestation_flow.cc',
        'attestation/attestation_flow.h',
        'chromeos_export.h',
        'chromeos_switches.cc',
        'chromeos_switches.h',
        'cryptohome/async_method_caller.cc',
        'cryptohome/async_method_caller.h',
        'dbus/blocking_method_caller.cc',
        'dbus/blocking_method_caller.h',
        'dbus/bluetooth_adapter_client.cc',
        'dbus/bluetooth_adapter_client.h',
        'dbus/bluetooth_agent_service_provider.cc',
        'dbus/bluetooth_agent_service_provider.h',
        'dbus/bluetooth_device_client.cc',
        'dbus/bluetooth_device_client.h',
        'dbus/bluetooth_input_client.cc',
        'dbus/bluetooth_input_client.h',
        'dbus/bluetooth_manager_client.cc',
        'dbus/bluetooth_manager_client.h',
        'dbus/bluetooth_node_client.cc',
        'dbus/bluetooth_node_client.h',
        'dbus/bluetooth_out_of_band_client.cc',
        'dbus/bluetooth_out_of_band_client.h',
        'dbus/bluetooth_property.cc',
        'dbus/bluetooth_property.h',
        'dbus/cros_disks_client.cc',
        'dbus/cros_disks_client.h',
        'dbus/cryptohome_client.cc',
        'dbus/cryptohome_client.h',
        'dbus/dbus_client_implementation_type.h',
        'dbus/dbus_method_call_status.h',
        'dbus/dbus_thread_manager.cc',
        'dbus/dbus_thread_manager.h',
        'dbus/debug_daemon_client.cc',
        'dbus/debug_daemon_client.h',
        'dbus/gsm_sms_client.cc',
        'dbus/gsm_sms_client.h',
        'dbus/shill_client_helper.cc',
        'dbus/shill_client_helper.h',
        'dbus/shill_device_client.cc',
        'dbus/shill_device_client.h',
        'dbus/shill_device_client_stub.cc',
        'dbus/shill_device_client_stub.h',
        'dbus/shill_ipconfig_client.cc',
        'dbus/shill_ipconfig_client.h',
        'dbus/shill_ipconfig_client_stub.cc',
        'dbus/shill_ipconfig_client_stub.h',
        'dbus/shill_manager_client.cc',
        'dbus/shill_manager_client.h',
        'dbus/shill_manager_client_stub.cc',
        'dbus/shill_manager_client_stub.h',
        'dbus/shill_profile_client.cc',
        'dbus/shill_profile_client.h',
        'dbus/shill_profile_client_stub.cc',
        'dbus/shill_profile_client_stub.h',
        'dbus/shill_service_client.cc',
        'dbus/shill_service_client.h',
        'dbus/shill_service_client_stub.cc',
        'dbus/shill_service_client_stub.h',
        'dbus/ibus/ibus_client.cc',
        'dbus/ibus/ibus_client.h',
        'dbus/ibus/ibus_component.cc',
        'dbus/ibus/ibus_component.h',
        'dbus/ibus/ibus_config_client.cc',
        'dbus/ibus/ibus_config_client.h',
        'dbus/ibus/ibus_constants.h',
        'dbus/ibus/ibus_engine_service.cc',
        'dbus/ibus/ibus_engine_service.h',
        'dbus/ibus/ibus_engine_factory_service.cc',
        'dbus/ibus/ibus_engine_factory_service.h',
        'dbus/ibus/ibus_lookup_table.cc',
        'dbus/ibus/ibus_lookup_table.h',
        'dbus/ibus/ibus_object.cc',
        'dbus/ibus/ibus_object.h',
        'dbus/ibus/ibus_property.cc',
        'dbus/ibus/ibus_property.h',
        'dbus/ibus/ibus_text.cc',
        'dbus/ibus/ibus_text.h',
        'dbus/ibus/ibus_input_context_client.cc',
        'dbus/ibus/ibus_input_context_client.h',
        'dbus/ibus/ibus_panel_service.cc',
        'dbus/ibus/ibus_panel_service.h',
        'dbus/image_burner_client.cc',
        'dbus/image_burner_client.h',
        'dbus/introspectable_client.cc',
        'dbus/introspectable_client.h',
        'dbus/modem_messaging_client.cc',
        'dbus/modem_messaging_client.h',
        'dbus/permission_broker_client.cc',
        'dbus/permission_broker_client.h',
        'dbus/power_manager_client.cc',
        'dbus/power_manager_client.h',
        'dbus/power_policy_controller.cc',
        'dbus/power_policy_controller.h',
        'dbus/power_supply_status.cc',
        'dbus/power_supply_status.h',
        'dbus/session_manager_client.cc',
        'dbus/session_manager_client.h',
        'dbus/sms_client.cc',
        'dbus/sms_client.h',
        'dbus/system_clock_client.cc',
        'dbus/system_clock_client.h',
        'dbus/update_engine_client.cc',
        'dbus/update_engine_client.h',
        'disks/disk_mount_manager.cc',
        'disks/disk_mount_manager.h',
        'display/output_configurator.cc',
        'display/output_configurator.h',
        'ime/component_extension_ime_manager.cc',
        'ime/component_extension_ime_manager.h',
        'ime/ibus_bridge.cc',
        'ime/ibus_bridge.h',
        'ime/ibus_daemon_controller.cc',
        'ime/ibus_daemon_controller.h',
        'ime/ibus_keymap.cc',
        'ime/ibus_keymap.h',
        'ime/input_method_config.cc',
        'ime/input_method_config.h',
        'ime/input_method_delegate.h',
        'ime/input_method_descriptor.cc',
        'ime/input_method_descriptor.h',
        'ime/input_method_property.cc',
        'ime/input_method_property.h',
        'network/cros_network_functions.cc',
        'network/cros_network_functions.h',
        'network/device_state.cc',
        'network/device_state.h',
        'network/geolocation_handler.cc',
        'network/geolocation_handler.h',
        'network/managed_state.cc',
        'network/managed_state.h',
        'network/network_change_notifier_chromeos.cc',
        'network/network_change_notifier_chromeos.h',
        'network/network_change_notifier_factory_chromeos.cc',
        'network/network_change_notifier_factory_chromeos.h',
        'network/network_configuration_handler.cc',
        'network/network_configuration_handler.h',
        'network/network_event_log.cc',
        'network/network_event_log.h',
        'network/network_handler_callbacks.cc',
        'network/network_handler_callbacks.h',
        'network/network_ip_config.cc',
        'network/network_ip_config.h',
        'network/network_sms_handler.cc',
        'network/network_sms_handler.h',
        'network/network_state.cc',
        'network/network_state.h',
        'network/network_state_handler.cc',
        'network/network_state_handler.h',
        'network/network_state_handler_observer.cc',
        'network/network_state_handler_observer.h',
        'network/network_util.cc',
        'network/network_util.h',
        'network/onc/onc_certificate_importer.cc',
        'network/onc/onc_certificate_importer.h',
        'network/onc/onc_constants.cc',
        'network/onc/onc_constants.h',
        'network/onc/onc_mapper.cc',
        'network/onc/onc_mapper.h',
        'network/onc/onc_merger.cc',
        'network/onc/onc_merger.h',
        'network/onc/onc_normalizer.cc',
        'network/onc/onc_normalizer.h',
        'network/onc/onc_signature.cc',
        'network/onc/onc_signature.h',
        'network/onc/onc_translation_tables.cc',
        'network/onc/onc_translation_tables.h',
        'network/onc/onc_translator.h',
        'network/onc/onc_translator_onc_to_shill.cc',
        'network/onc/onc_translator_shill_to_onc.cc',
        'network/onc/onc_utils.cc',
        'network/onc/onc_utils.h',
        'network/onc/onc_validator.cc',
        'network/onc/onc_validator.h',
        'network/shill_property_handler.cc',
        'network/shill_property_handler.h',
        'network/sms_watcher.cc',
        'network/sms_watcher.h',
        'process_proxy/process_output_watcher.cc',
        'process_proxy/process_output_watcher.h',
        'process_proxy/process_proxy.cc',
        'process_proxy/process_proxy.h',
        'process_proxy/process_proxy_registry.cc',
        'process_proxy/process_proxy_registry.h',
      ],
      'conditions': [
        ['use_x11 == 1', {
          'link_settings': {
            'libraries': [
              '-lXext',
              '-lXrandr',
            ],
          },
        }, {
          # use_x11 == 0
          'sources!': [
            'display/output_configurator.cc',
            'display/output_configurator.h',
          ],
        }],
      ],
    },
    {
      # This target contains mocks that can be used to write unit tests.
      'target_name': 'chromeos_test_support',
      'type': 'static_library',
      'dependencies': [
        '../build/linux/system.gyp:dbus',
        '../testing/gmock.gyp:gmock',
        'chromeos',
        'power_manager_proto',
      ],
      'sources': [
        'attestation/mock_attestation_flow.cc',
        'attestation/mock_attestation_flow.h',
        'chromeos_test_utils.cc',
        'chromeos_test_utils.h',
        'cryptohome/mock_async_method_caller.cc',
        'cryptohome/mock_async_method_caller.h',
        'dbus/fake_session_manager_client.cc',
        'dbus/fake_session_manager_client.h',
        'dbus/mock_bluetooth_adapter_client.cc',
        'dbus/mock_bluetooth_adapter_client.h',
        'dbus/mock_bluetooth_device_client.cc',
        'dbus/mock_bluetooth_device_client.h',
        'dbus/mock_bluetooth_input_client.cc',
        'dbus/mock_bluetooth_input_client.h',
        'dbus/mock_bluetooth_manager_client.cc',
        'dbus/mock_bluetooth_manager_client.h',
        'dbus/mock_bluetooth_node_client.cc',
        'dbus/mock_bluetooth_node_client.h',
        'dbus/mock_bluetooth_out_of_band_client.cc',
        'dbus/mock_bluetooth_out_of_band_client.h',
        'dbus/mock_cros_disks_client.cc',
        'dbus/mock_cros_disks_client.h',
        'dbus/mock_cryptohome_client.cc',
        'dbus/mock_cryptohome_client.h',
        'dbus/mock_dbus_thread_manager.cc',
        'dbus/mock_dbus_thread_manager.h',
        'dbus/mock_debug_daemon_client.cc',
        'dbus/mock_debug_daemon_client.h',
        'dbus/mock_shill_device_client.cc',
        'dbus/mock_shill_device_client.h',
        'dbus/mock_shill_ipconfig_client.cc',
        'dbus/mock_shill_ipconfig_client.h',
        'dbus/mock_shill_manager_client.cc',
        'dbus/mock_shill_manager_client.h',
        'dbus/mock_shill_profile_client.cc',
        'dbus/mock_shill_profile_client.h',
        'dbus/mock_shill_service_client.cc',
        'dbus/mock_shill_service_client.h',
        'dbus/mock_gsm_sms_client.cc',
        'dbus/mock_gsm_sms_client.h',
        'dbus/mock_image_burner_client.cc',
        'dbus/mock_image_burner_client.h',
        'dbus/mock_introspectable_client.cc',
        'dbus/mock_introspectable_client.h',
        'dbus/mock_modem_messaging_client.cc',
        'dbus/mock_modem_messaging_client.h',
        'dbus/mock_permission_broker_client.h',
        'dbus/mock_permission_broker_client.cc',
        'dbus/mock_power_manager_client.cc',
        'dbus/mock_power_manager_client.h',
        'dbus/mock_session_manager_client.cc',
        'dbus/mock_session_manager_client.h',
        'dbus/mock_sms_client.cc',
        'dbus/mock_sms_client.h',
        'dbus/mock_system_clock_client.cc',
        'dbus/mock_system_clock_client.h',
        'dbus/mock_update_engine_client.cc',
        'dbus/mock_update_engine_client.h',
        'disks/mock_disk_mount_manager.cc',
        'disks/mock_disk_mount_manager.h',
        'network/onc/onc_test_utils.cc',
        'network/onc/onc_test_utils.h',
      ],
      'include_dirs': [
        '..',
      ],
    },
    {
      'target_name': 'chromeos_test_support_without_gmock',
      'type': 'static_library',
      'dependencies': [
        '../build/linux/system.gyp:dbus',
        'chromeos',
      ],
      'sources': [
        'dbus/mock_dbus_thread_manager_without_gmock.cc',
        'dbus/mock_dbus_thread_manager_without_gmock.h',
        'dbus/ibus/mock_ibus_client.cc',
        'dbus/ibus/mock_ibus_client.h',
        'dbus/ibus/mock_ibus_config_client.cc',
        'dbus/ibus/mock_ibus_config_client.h',
        'dbus/ibus/mock_ibus_input_context_client.cc',
        'dbus/ibus/mock_ibus_input_context_client.h',
        'dbus/ibus/mock_ibus_engine_factory_service.cc',
        'dbus/ibus/mock_ibus_engine_factory_service.h',
        'dbus/ibus/mock_ibus_engine_service.cc',
        'dbus/ibus/mock_ibus_engine_service.h',
        'dbus/ibus/mock_ibus_panel_service.cc',
        'dbus/ibus/mock_ibus_panel_service.h',
        'ime/mock_ibus_daemon_controller.cc',
        'ime/mock_ibus_daemon_controller.h',
        'ime/mock_input_method_delegate.cc',
        'ime/mock_input_method_delegate.h',
      ],
      'include_dirs': [
        '..',
      ],
    },
    {
      'target_name': 'chromeos_unittests',
      'type': 'executable',
      'dependencies': [
        '../base/base.gyp:run_all_unittests',
        '../base/base.gyp:test_support_base',
        '../build/linux/system.gyp:dbus',
        '../build/linux/system.gyp:ssl',
        '../crypto/crypto.gyp:crypto',
        '../dbus/dbus.gyp:dbus_test_support',
        '../net/net.gyp:net',
        '../testing/gmock.gyp:gmock',
        '../testing/gtest.gyp:gtest',
        'chromeos_test_support',
        'power_manager_proto',
      ],
      'sources': [
        'attestation/attestation_flow_unittest.cc',
        'display/output_configurator_unittest.cc',
        'dbus/blocking_method_caller_unittest.cc',
        'dbus/power_policy_controller_unittest.cc',
        'dbus/shill_client_unittest_base.cc',
        'dbus/shill_client_unittest_base.h',
        'dbus/shill_device_client_unittest.cc',
        'dbus/shill_ipconfig_client_unittest.cc',
        'dbus/shill_manager_client_unittest.cc',
        'dbus/shill_profile_client_unittest.cc',
        'dbus/shill_service_client_unittest.cc',
        'dbus/gsm_sms_client_unittest.cc',
        'dbus/ibus/ibus_client_unittest.cc',
        'dbus/ibus/ibus_config_client_unittest.cc',
        'dbus/ibus/ibus_component_unittest.cc',
        'dbus/ibus/ibus_engine_factory_service_unittest.cc',
        'dbus/ibus/ibus_engine_service_unittest.cc',
        'dbus/ibus/ibus_input_context_client_unittest.cc',
        'dbus/ibus/ibus_lookup_table_unittest.cc',
        'dbus/ibus/ibus_object_unittest.cc',
        'dbus/ibus/ibus_panel_service_unittest.cc',
        'dbus/ibus/ibus_property_unittest.cc',
        'dbus/ibus/ibus_text_unittest.cc',
        'dbus/introspectable_client_unittest.cc',
        'dbus/modem_messaging_client_unittest.cc',
        'disks/disk_mount_manager_unittest.cc',
        'ime/input_method_descriptor_unittest.cc',
        'ime/input_method_property_unittest.cc',
        'network/cros_network_functions_unittest.cc',
        'network/geolocation_handler_unittest.cc',
        'network/network_change_notifier_chromeos_unittest.cc',
        'network/network_configuration_handler_unittest.cc',
        'network/network_event_log_unittest.cc',
        'network/network_sms_handler_unittest.cc',
        'network/network_state_handler_unittest.cc',
        'network/network_util_unittest.cc',
        'network/onc/onc_certificate_importer_unittest.cc',
        'network/onc/onc_merger_unittest.cc',
        'network/onc/onc_normalizer_unittest.cc',
        'network/onc/onc_translator_unittest.cc',
        'network/onc/onc_utils_unittest.cc',
        'network/onc/onc_validator_unittest.cc',
        'network/shill_property_handler_unittest.cc',
        'process_proxy/process_output_watcher_unittest.cc',
        'process_proxy/process_proxy_unittest.cc',
      ],
      'include_dirs': [
        '..',
      ],
      'conditions': [
        [ 'linux_use_tcmalloc==1', {
           'dependencies': [
              '../base/allocator/allocator.gyp:allocator',
            ],
          },
        ],
      ],
    },
    {
      # Protobuf compiler/generator for power-manager related protocol buffers.
      # TODO(derat): Remove other protocol buffers in favor of these.
      'target_name': 'power_manager_proto',
      'type': 'static_library',
      'sources': [
        '../third_party/cros_system_api/dbus/power_manager/input_event.proto',
        '../third_party/cros_system_api/dbus/power_manager/policy.proto',
        '../third_party/cros_system_api/dbus/power_manager/suspend.proto',
      ],
      'variables': {
        'proto_in_dir': '../third_party/cros_system_api/dbus/power_manager',
        'proto_out_dir': 'chromeos/dbus/power_manager',
      },
      'includes': ['../build/protoc.gypi'],
    },
    {
      # Protobuf compiler / generator for the PowerSupplyProperties protocol
      # buffer.
      'target_name': 'power_supply_properties_proto',
      'type': 'static_library',
      'sources': [
        '../third_party/cros_system_api/dbus/power_supply_properties.proto',
      ],
      'variables': {
        'proto_in_dir': '../third_party/cros_system_api/dbus',
        'proto_out_dir': 'chromeos/dbus',
      },
      'includes': ['../build/protoc.gypi'],
    },
    {
      # Protobuf compiler / generator for the VideoActivityUpdate protocol
      # buffer.
      'target_name': 'video_activity_update_proto',
      'type': 'static_library',
      'sources': [
        '../third_party/cros_system_api/dbus/video_activity_update.proto',
      ],
      'variables': {
        'proto_in_dir': '../third_party/cros_system_api/dbus/',
        'proto_out_dir': 'chromeos/dbus',
      },
      'includes': ['../build/protoc.gypi'],
    },
  ],
}
