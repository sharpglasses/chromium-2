# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'dependencies': [
    '../jingle/jingle.gyp:jingle_glue',
    '../net/net.gyp:net',
    '../skia/skia.gyp:skia',
    '../third_party/hyphen/hyphen.gyp:hyphen',
    '../third_party/icu/icu.gyp:icuuc',
    '../third_party/icu/icu.gyp:icui18n',
    '../third_party/libjingle/libjingle.gyp:libjingle',
    '../third_party/npapi/npapi.gyp:npapi',
    '<(webkit_src_dir)/Source/WebKit/chromium/WebKit.gyp:webkit',
    '../ui/native_theme/native_theme.gyp:native_theme',
    '../ui/surface/surface.gyp:surface',
    '../v8/tools/gyp/v8.gyp:v8',
    '../webkit/compositor_bindings/compositor_bindings.gyp:webkit_compositor_bindings',
    '../webkit/support/webkit_support.gyp:glue',
    '../webkit/support/webkit_support.gyp:webkit_media',
    '../webkit/support/webkit_support.gyp:webkit_gpu',
  ],
  'include_dirs': [
    '..',
  ],
  'sources': [
    'public/renderer/android_content_detection_prefixes.cc',
    'public/renderer/android_content_detection_prefixes.h',
    'public/renderer/content_renderer_client.cc',
    'public/renderer/content_renderer_client.h',
    'public/renderer/context_menu_client.h',
    'public/renderer/document_state.cc',
    'public/renderer/document_state.h',
    'public/renderer/navigation_state.cc',
    'public/renderer/navigation_state.h',
    'public/renderer/password_form_conversion_utils.h',
    'public/renderer/renderer_ppapi_host.h',
    'public/renderer/render_process_observer.cc',
    'public/renderer/render_process_observer.h',
    'public/renderer/render_thread.cc',
    'public/renderer/render_thread.h',
    'public/renderer/render_view.h',
    'public/renderer/render_view_observer.cc',
    'public/renderer/render_view_observer.h',
    'public/renderer/render_view_observer_tracker.h',
    'public/renderer/render_view_visitor.h',
    'public/renderer/v8_value_converter.h',
    'renderer/accessibility_node_serializer.cc',
    'renderer/accessibility_node_serializer.h',
    'renderer/active_notification_tracker.cc',
    'renderer/active_notification_tracker.h',
    'renderer/all_rendering_benchmarks.cc',
    'renderer/all_rendering_benchmarks.h',
    'renderer/android/address_detector.cc',
    'renderer/android/address_detector.h',
    'renderer/android/content_detector.cc',
    'renderer/android/content_detector.h',
    'renderer/android/email_detector.cc',
    'renderer/android/email_detector.h',
    'renderer/android/phone_number_detector.cc',
    'renderer/android/phone_number_detector.h',
    'renderer/device_orientation_dispatcher.cc',
    'renderer/device_orientation_dispatcher.h',
    'renderer/devtools/devtools_agent.cc',
    'renderer/devtools/devtools_agent.h',
    'renderer/devtools/devtools_agent_filter.cc',
    'renderer/devtools/devtools_agent_filter.h',
    'renderer/devtools/devtools_client.cc',
    'renderer/devtools/devtools_client.h',
    'renderer/disambiguation_popup_helper.cc',
    'renderer/disambiguation_popup_helper.h',
    'renderer/do_not_track_bindings.cc',
    'renderer/do_not_track_bindings.h',
    'renderer/dom_automation_controller.cc',
    'renderer/dom_automation_controller.h',
    'renderer/dom_storage/dom_storage_dispatcher.cc',
    'renderer/dom_storage/dom_storage_dispatcher.h',
    'renderer/dom_storage/webstoragearea_impl.cc',
    'renderer/dom_storage/webstoragearea_impl.h',
    'renderer/dom_storage/webstoragenamespace_impl.cc',
    'renderer/dom_storage/webstoragenamespace_impl.h',
    'renderer/external_popup_menu.cc',
    'renderer/external_popup_menu.h',
    'renderer/favicon_helper.cc',
    'renderer/favicon_helper.h',
    'renderer/gamepad_shared_memory_reader.cc',
    'renderer/gamepad_shared_memory_reader.h',
    'renderer/geolocation_dispatcher.cc',
    'renderer/geolocation_dispatcher.h',
    'renderer/gpu/compositor_output_surface.cc',
    'renderer/gpu/compositor_output_surface.h',
    'renderer/gpu/compositor_software_output_device_gl_adapter.cc',
    'renderer/gpu/compositor_software_output_device_gl_adapter.h',
    'renderer/gpu/compositor_thread.cc',
    'renderer/gpu/compositor_thread.h',
    'renderer/gpu/input_event_filter.cc',
    'renderer/gpu/input_event_filter.h',
    'renderer/gpu/gpu_benchmarking_extension.cc',
    'renderer/gpu/gpu_benchmarking_extension.h',
    'renderer/gpu/stream_texture_host_android.cc',
    'renderer/gpu/stream_texture_host_android.h',
    'renderer/hyphenator/hyphenator.cc',
    'renderer/hyphenator/hyphenator.h',
    'renderer/idle_user_detector.cc',
    'renderer/idle_user_detector.h',
    'renderer/input_tag_speech_dispatcher.cc',
    'renderer/input_tag_speech_dispatcher.h',
    'renderer/java/java_bridge_channel.cc',
    'renderer/java/java_bridge_channel.h',
    'renderer/java/java_bridge_dispatcher.cc',
    'renderer/java/java_bridge_dispatcher.h',
    'renderer/load_progress_tracker.cc',
    'renderer/load_progress_tracker.h',
    'renderer/media/audio_device_factory.cc',
    'renderer/media/audio_device_factory.h',
    'renderer/media/audio_hardware.cc',
    'renderer/media/audio_hardware.h',
    'renderer/media/audio_input_message_filter.cc',
    'renderer/media/audio_input_message_filter.h',
    'renderer/media/audio_message_filter.cc',
    'renderer/media/audio_message_filter.h',
    'renderer/media/audio_renderer_mixer_manager.cc',
    'renderer/media/audio_renderer_mixer_manager.h',
    'renderer/media/local_video_capture.cc',
    'renderer/media/local_video_capture.h',
    'renderer/media/media_stream_center.h',
    'renderer/media/media_stream_dependency_factory.h',
    'renderer/media/media_stream_dispatcher.h',
    'renderer/media/media_stream_dispatcher_eventhandler.h',
    'renderer/media/media_stream_impl.h',
    'renderer/media/pepper_platform_video_decoder_impl.cc',
    'renderer/media/pepper_platform_video_decoder_impl.h',
    'renderer/media/render_media_log.cc',
    'renderer/media/render_media_log.h',
    'renderer/media/renderer_audio_output_device.cc',
    'renderer/media/renderer_audio_output_device.h',
    'renderer/media/renderer_gpu_video_decoder_factories.cc',
    'renderer/media/renderer_gpu_video_decoder_factories.h',
    'renderer/media/renderer_webaudiodevice_impl.cc',
    'renderer/media/renderer_webaudiodevice_impl.h',
    'renderer/media/rtc_video_decoder.cc',
    'renderer/media/rtc_video_decoder.h',
    'renderer/media/rtc_video_renderer.cc',
    'renderer/media/rtc_video_renderer.h',
    'renderer/media/stream_texture_factory_impl_android.cc',
    'renderer/media/stream_texture_factory_impl_android.h',
    'renderer/media/video_capture_impl.cc',
    'renderer/media/video_capture_impl.h',
    'renderer/media/video_capture_impl_manager.cc',
    'renderer/media/video_capture_impl_manager.h',
    'renderer/media/video_capture_message_filter.cc',
    'renderer/media/video_capture_message_filter.h',
    'renderer/media/webmediaplayer_proxy_impl_android.cc',
    'renderer/media/webmediaplayer_proxy_impl_android.h',
    'renderer/mhtml_generator.cc',
    'renderer/mhtml_generator.h',
    'renderer/mouse_lock_dispatcher.cc',
    'renderer/mouse_lock_dispatcher.h',
    'renderer/notification_provider.cc',
    'renderer/notification_provider.h',
    'renderer/paint_aggregator.cc',
    'renderer/paint_aggregator.h',
    'renderer/password_form_conversion_utils.cc',
    'renderer/pepper/content_renderer_pepper_host_factory.cc',
    'renderer/pepper/content_renderer_pepper_host_factory.h',
    'renderer/pepper/pepper_audio_input_host.cc',
    'renderer/pepper/pepper_audio_input_host.h',
    'renderer/pepper/pepper_broker_impl.cc',
    'renderer/pepper/pepper_broker_impl.h',
    'renderer/pepper/pepper_device_enumeration_event_handler.cc',
    'renderer/pepper/pepper_device_enumeration_event_handler.h',
    'renderer/pepper/pepper_device_enumeration_host_helper.cc',
    'renderer/pepper/pepper_device_enumeration_host_helper.h',
    'renderer/pepper/pepper_file_chooser_host.cc',
    'renderer/pepper/pepper_file_chooser_host.h',
    'renderer/pepper/pepper_file_io_host.cc',
    'renderer/pepper/pepper_file_io_host.h',
    'renderer/pepper/pepper_flash_clipboard_host.cc',
    'renderer/pepper/pepper_flash_clipboard_host.h',
    'renderer/pepper/pepper_graphics_2d_host.cc',
    'renderer/pepper/pepper_graphics_2d_host.h',
    'renderer/pepper/pepper_hung_plugin_filter.cc',
    'renderer/pepper/pepper_hung_plugin_filter.h',
    'renderer/pepper/pepper_in_process_resource_creation.cc',
    'renderer/pepper/pepper_in_process_resource_creation.h',
    'renderer/pepper/pepper_in_process_router.cc',
    'renderer/pepper/pepper_in_process_router.h',
    'renderer/pepper/pepper_parent_context_provider.cc',
    'renderer/pepper/pepper_parent_context_provider.h',
    'renderer/pepper/pepper_platform_audio_input_impl.cc',
    'renderer/pepper/pepper_platform_audio_input_impl.h',
    'renderer/pepper/pepper_platform_audio_output_impl.cc',
    'renderer/pepper/pepper_platform_audio_output_impl.h',
    'renderer/pepper/pepper_platform_context_3d_impl.cc',
    'renderer/pepper/pepper_platform_context_3d_impl.h',
    'renderer/pepper/pepper_platform_image_2d_impl.cc',
    'renderer/pepper/pepper_platform_image_2d_impl.h',
    'renderer/pepper/pepper_platform_video_capture_impl.cc',
    'renderer/pepper/pepper_platform_video_capture_impl.h',
    'renderer/pepper/pepper_plugin_delegate_impl.cc',
    'renderer/pepper/pepper_plugin_delegate_impl.h',
    'renderer/pepper/pepper_proxy_channel_delegate_impl.cc',
    'renderer/pepper/pepper_proxy_channel_delegate_impl.h',
    'renderer/pepper/pepper_video_capture_host.cc',
    'renderer/pepper/pepper_video_capture_host.h',
    'renderer/pepper/pepper_websocket_host.cc',
    'renderer/pepper/pepper_websocket_host.h',
    'renderer/pepper/renderer_ppapi_host_impl.cc',
    'renderer/pepper/renderer_ppapi_host_impl.h',
    'renderer/plugin_channel_host.cc',
    'renderer/plugin_channel_host.h',
    'renderer/browser_plugin/browser_plugin.cc',
    'renderer/browser_plugin/browser_plugin.h',
    'renderer/browser_plugin/browser_plugin_backing_store.h',
    'renderer/browser_plugin/browser_plugin_backing_store.cc',
    'renderer/browser_plugin/browser_plugin_bindings.h',
    'renderer/browser_plugin/browser_plugin_bindings.cc',
    'renderer/browser_plugin/browser_plugin_manager.h',
    'renderer/browser_plugin/browser_plugin_manager.cc',
    'renderer/browser_plugin/browser_plugin_manager_factory.h',
    'renderer/browser_plugin/browser_plugin_manager_impl.h',
    'renderer/browser_plugin/browser_plugin_manager_impl.cc',
    'renderer/browser_plugin/browser_plugin_compositing_helper.h',
    'renderer/browser_plugin/browser_plugin_compositing_helper.cc',
    'renderer/render_process.h',
    'renderer/render_process_impl.cc',
    'renderer/render_process_impl.h',
    'renderer/render_thread_impl.cc',
    'renderer/render_thread_impl.h',
    'renderer/render_view_impl.cc',
    'renderer/render_view_impl.h',
    'renderer/render_view_impl_android.cc',
    'renderer/render_view_impl_params.cc',
    'renderer/render_view_impl_params.h',
    'renderer/render_view_linux.cc',
    'renderer/render_view_mouse_lock_dispatcher.cc',
    'renderer/render_view_mouse_lock_dispatcher.h',
    'renderer/render_view_pepper_helper.cc',
    'renderer/render_view_pepper_helper.h',
    'renderer/render_widget.cc',
    'renderer/render_widget.h',
    'renderer/render_widget_fullscreen.cc',
    'renderer/render_widget_fullscreen.h',
    'renderer/render_widget_fullscreen_pepper.cc',
    'renderer/render_widget_fullscreen_pepper.h',
    'renderer/renderer_accessibility.cc',
    'renderer/renderer_accessibility.h',
    'renderer/renderer_accessibility_complete.cc',
    'renderer/renderer_accessibility_complete.h',
    'renderer/renderer_accessibility_focus_only.cc',
    'renderer/renderer_accessibility_focus_only.h',
    'renderer/renderer_clipboard_client.cc',
    'renderer/renderer_clipboard_client.h',
    'renderer/renderer_date_time_picker.cc',
    'renderer/renderer_date_time_picker.h',
    'renderer/renderer_main.cc',
    'renderer/renderer_main_platform_delegate.h',
    'renderer/renderer_main_platform_delegate_android.cc',
    'renderer/renderer_main_platform_delegate_linux.cc',
    'renderer/renderer_main_platform_delegate_mac.mm',
    'renderer/renderer_main_platform_delegate_win.cc',
    'renderer/renderer_webapplicationcachehost_impl.cc',
    'renderer/renderer_webapplicationcachehost_impl.h',
    'renderer/renderer_webcookiejar_impl.cc',
    'renderer/renderer_webcookiejar_impl.h',
    'renderer/renderer_webcolorchooser_impl.cc',
    'renderer/renderer_webcolorchooser_impl.h',
    'renderer/renderer_webkitplatformsupport_impl.cc',
    'renderer/renderer_webkitplatformsupport_impl.h',
    'renderer/rendering_benchmark.cc',
    'renderer/rendering_benchmark.h',
    'renderer/speech_recognition_dispatcher.cc',
    'renderer/speech_recognition_dispatcher.h',
    'renderer/text_input_client_observer.cc',
    'renderer/text_input_client_observer.h',
    'renderer/v8_value_converter_impl.cc',
    'renderer/v8_value_converter_impl.h',
    'renderer/web_intents_host.cc',
    'renderer/web_intents_host.h',
    'renderer/web_ui_extension.cc',
    'renderer/web_ui_extension.h',
    'renderer/web_ui_extension_data.cc',
    'renderer/web_ui_extension_data.h',
    'renderer/webplugin_delegate_proxy.cc',
    'renderer/webplugin_delegate_proxy.h',
    'renderer/websharedworker_proxy.cc',
    'renderer/websharedworker_proxy.h',
    'renderer/websharedworkerrepository_impl.cc',
    'renderer/websharedworkerrepository_impl.h',
  ],
  'conditions': [
    ['notifications==0', {
      'sources!': [
        'renderer/notification_provider.cc',
        'renderer/active_notification_tracker.cc',
      ],
    }],
    ['input_speech==0', {
      'sources!': [
        'renderer/input_tag_speech_dispatcher.cc',
        'renderer/input_tag_speech_dispatcher.h',
        'renderer/speech_recognition_dispatcher.cc',
        'renderer/speech_recognition_dispatcher.h',
      ]
    }],
    ['toolkit_uses_gtk == 1', {
      'dependencies': [
        '../build/linux/system.gyp:gtk',
      ],
    }],
    ['OS=="mac"', {
      'sources!': [
        'common/process_watcher_posix.cc',
      ],
    }],
    ['OS=="win" and win_use_allocator_shim==1', {
      'dependencies': [
          '../base/allocator/allocator.gyp:allocator',
      ],
    }],
    ['OS=="android"', {
      'dependencies': [
        '../third_party/libphonenumber/libphonenumber.gyp:libphonenumber',
      ],
    }],
    # TODO(jrg): remove the OS=="android" section?
    # http://crbug.com/113172
    # Understand better how media_stream_ is tied into Chromium.
    ['enable_webrtc==0 and OS=="android"', {
      'sources/': [
        ['exclude', '^renderer/media/media_stream_'],
      ],
    }],
    ['enable_webrtc==1', {
      'dependencies': [
        '../third_party/libjingle/libjingle.gyp:libjingle_p2p',
        '../third_party/libjingle/libjingle.gyp:libjingle_peerconnection',
        '../third_party/webrtc/system_wrappers/source/system_wrappers.gyp:system_wrappers',
        '../third_party/webrtc/modules/modules.gyp:audio_device',
        '../third_party/webrtc/video_engine/video_engine.gyp:video_engine_core',
        '../third_party/webrtc/voice_engine/voice_engine.gyp:voice_engine_core',
        '<(DEPTH)/crypto/crypto.gyp:crypto',
      ],
      'sources': [
        'renderer/media/media_stream_center.cc',
        'renderer/media/media_stream_dependency_factory.cc',
        'renderer/media/media_stream_dispatcher.cc',
        'renderer/media/media_stream_impl.cc',
        'renderer/media/peer_connection_handler_base.cc',
        'renderer/media/peer_connection_handler_base.h',
        'renderer/media/peer_connection_tracker.cc',
        'renderer/media/peer_connection_tracker.h',
        'renderer/media/rtc_data_channel_handler.cc',
        'renderer/media/rtc_data_channel_handler.h',
        'renderer/media/rtc_media_constraints.cc',
        'renderer/media/rtc_media_constraints.h',
        'renderer/media/rtc_peer_connection_handler.cc',
        'renderer/media/rtc_peer_connection_handler.h',
        'renderer/media/rtc_video_capture_delegate.cc',
        'renderer/media/rtc_video_capture_delegate.h',
        'renderer/media/rtc_video_capturer.cc',
        'renderer/media/rtc_video_capturer.h',
        'renderer/media/webaudio_capturer_source.cc',
        'renderer/media/webaudio_capturer_source.h',
        'renderer/media/webrtc_audio_capturer.cc',
        'renderer/media/webrtc_audio_capturer.h',
        'renderer/media/webrtc_audio_device_impl.cc',
        'renderer/media/webrtc_audio_device_impl.h',
        'renderer/media/webrtc_audio_renderer.cc',
        'renderer/media/webrtc_audio_renderer.h',
        'renderer/media/webrtc_local_audio_renderer.cc',
        'renderer/media/webrtc_local_audio_renderer.h',
        'renderer/p2p/host_address_request.cc',
        'renderer/p2p/host_address_request.h',
        'renderer/p2p/ipc_network_manager.cc',
        'renderer/p2p/ipc_network_manager.h',
        'renderer/p2p/ipc_socket_factory.cc',
        'renderer/p2p/ipc_socket_factory.h',
        'renderer/p2p/port_allocator.cc',
        'renderer/p2p/port_allocator.h',
        'renderer/p2p/socket_client.cc',
        'renderer/p2p/socket_client.h',
        'renderer/p2p/socket_dispatcher.cc',
        'renderer/p2p/socket_dispatcher.h',
      ],
    }],
    ['enable_plugins==1', {
      'dependencies': [
        '../ppapi/ppapi_internal.gyp:ppapi_host',
        '../ppapi/ppapi_internal.gyp:ppapi_proxy',
        '../ppapi/ppapi_internal.gyp:ppapi_shared',
      ],
    }, {  # enable_plugins==0
      'sources/': [
        ['exclude', '^renderer/pepper/'],
      ],
      'sources!': [
        'renderer/render_widget_fullscreen_pepper.cc',
        'renderer/render_widget_fullscreen_pepper.h',
      ],
    }],
    ['java_bridge==1', {
      'defines': [
        'ENABLE_JAVA_BRIDGE',
      ],
    }, {
      'sources!': [
        'renderer/java/java_bridge_channel.cc',
        'renderer/java/java_bridge_channel.h',
        'renderer/java/java_bridge_dispatcher.cc',
        'renderer/java/java_bridge_dispatcher.h',
      ],
    }],
  ],
  'target_conditions': [
    ['OS=="android"', {
      'sources/': [
        ['include', '^renderer/render_view_linux\\.cc$'],
      ],
    }],
  ],
}
