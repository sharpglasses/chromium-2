// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/base/android/media_codec_bridge.h"

#include <jni.h>

#include "base/android/build_info.h"
#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/basictypes.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/safe_numerics.h"
#include "base/stringprintf.h"

#include "jni/MediaCodecBridge_jni.h"
#include "jni/MediaFormat_jni.h"

using base::android::AttachCurrentThread;
using base::android::ConvertUTF8ToJavaString;
using base::android::ScopedJavaLocalRef;

namespace {

class MediaCodecNativeRegisterer {
 public:
  MediaCodecNativeRegisterer() {
    JNIEnv* env = AttachCurrentThread();
    jni_initialized_ =
        media::RegisterNativesImpl(env) &&
        JNI_MediaFormat::RegisterNativesImpl(env);
  }

  bool IsRegistered() {
    return jni_initialized_;
  }

 private:
  bool jni_initialized_;
};

static base::LazyInstance<MediaCodecNativeRegisterer> g_native_registerer =
    LAZY_INSTANCE_INITIALIZER;

}  // namespace

namespace media {

enum { kBufferFlagEndOfStream = 4 };

static const char* AudioCodecToMimeType(const AudioCodec codec) {
  switch (codec) {
    case kCodecMP3:
      return "audio/mpeg";
    case kCodecVorbis:
      return "audio/vorbis";
    case kCodecAAC:
      return "audio/mp4a-latm";
    default:
      return NULL;
  }
}

static const char* VideoCodecToMimeType(const VideoCodec codec) {
  switch (codec) {
    case kCodecH264:
      return "video/avc";
    case kCodecVP8:
      return "video/x-vnd.on2.vp8";
    default:
      return NULL;
  }
}

// static
const base::TimeDelta MediaCodecBridge::kTimeOutInfinity =
    base::TimeDelta::FromMicroseconds(-1);

// static
const base::TimeDelta MediaCodecBridge::kTimeOutNoWait =
    base::TimeDelta::FromMicroseconds(0);

// static
bool MediaCodecBridge::IsAvailable() {
  // MediaCodec is only available on JB and greater.
  return base::android::BuildInfo::GetInstance()->sdk_int() >= 16;
}

MediaCodecBridge::MediaCodecBridge(const char* mime) {
  JNIEnv* env = AttachCurrentThread();
  CHECK(env);
  CHECK(g_native_registerer.Pointer()->IsRegistered());
  DCHECK(mime);

  ScopedJavaLocalRef<jstring> j_type = ConvertUTF8ToJavaString(env, mime);
  j_media_codec_.Reset(Java_MediaCodecBridge_create(
      env, j_type.obj()));
}

MediaCodecBridge::~MediaCodecBridge() {
  JNIEnv* env = AttachCurrentThread();
  CHECK(env);
  Java_MediaCodecBridge_release(env, j_media_codec_.obj());
}

void MediaCodecBridge::StartInternal() {
  JNIEnv* env = AttachCurrentThread();
  Java_MediaCodecBridge_start(env, j_media_codec_.obj());
}

void MediaCodecBridge::Reset() {
  JNIEnv* env = AttachCurrentThread();
  Java_MediaCodecBridge_flush(env, j_media_codec_.obj());
}

void MediaCodecBridge::Stop() {
  JNIEnv* env = AttachCurrentThread();
  Java_MediaCodecBridge_stop(env, j_media_codec_.obj());
}

void MediaCodecBridge::GetOutputFormat(int* width, int* height) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> j_format(
      Java_MediaCodecBridge_getOutputFormat(env, j_media_codec_.obj()));
  if (!j_format.is_null()) {
    ScopedJavaLocalRef<jstring> j_key_width =
        ConvertUTF8ToJavaString(env, "width");
    *width = JNI_MediaFormat::Java_MediaFormat_getInteger(
        env, j_format.obj(), j_key_width.obj());

    ScopedJavaLocalRef<jstring> j_key_height =
        ConvertUTF8ToJavaString(env, "height");
    *height = JNI_MediaFormat::Java_MediaFormat_getInteger(
        env, j_format.obj(), j_key_height.obj());
  }
}

size_t MediaCodecBridge::QueueInputBuffer(
    int index, const uint8* data, int size,
    const base::TimeDelta& presentation_time) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> j_buffer(
      Java_MediaCodecBridge_getInputBuffer(env, j_media_codec_.obj(), index));

  uint8* direct_buffer =
      static_cast<uint8*>(env->GetDirectBufferAddress(j_buffer.obj()));
  int64 buffer_capacity = env->GetDirectBufferCapacity(j_buffer.obj());

  size_t size_to_copy = (buffer_capacity < size) ? buffer_capacity : size;

  if (size_to_copy > 0)
    memcpy(direct_buffer, data, size_to_copy);
  Java_MediaCodecBridge_queueInputBuffer(
      env, j_media_codec_.obj(),
      index, 0, size_to_copy, presentation_time.InMicroseconds(), 0);
  return size_to_copy;
}

void MediaCodecBridge::QueueEOS(int input_buffer_index) {
  JNIEnv* env = AttachCurrentThread();
  Java_MediaCodecBridge_queueInputBuffer(
      env, j_media_codec_.obj(),
      input_buffer_index, 0, 0, 0, kBufferFlagEndOfStream);
}

int MediaCodecBridge::DequeueInputBuffer(base::TimeDelta timeout) {
  JNIEnv* env = AttachCurrentThread();
  return Java_MediaCodecBridge_dequeueInputBuffer(
      env, j_media_codec_.obj(), timeout.InMicroseconds());
}

int MediaCodecBridge::DequeueOutputBuffer(
    base::TimeDelta timeout, size_t* offset, size_t* size,
    base::TimeDelta* presentation_time, bool* end_of_stream) {
  JNIEnv* env = AttachCurrentThread();

  ScopedJavaLocalRef<jobject> result =
      Java_MediaCodecBridge_dequeueOutputBuffer(env, j_media_codec_.obj(),
                                                timeout.InMicroseconds());

  int j_buffer = Java_DequeueOutputResult_index(env, result.obj());
  if (j_buffer >= 0) {
    int64 presentation_time_us =
        Java_DequeueOutputResult_presentationTimeMicroseconds(
            env, result.obj());
    int flags = Java_DequeueOutputResult_flags(env, result.obj());
    *offset = base::checked_numeric_cast<size_t>(
        Java_DequeueOutputResult_offset(env, result.obj()));
    *size = base::checked_numeric_cast<size_t>(
        Java_DequeueOutputResult_numBytes(env, result.obj()));
    *presentation_time =
        base::TimeDelta::FromMicroseconds(presentation_time_us);
    *end_of_stream = flags & kBufferFlagEndOfStream;
  }
  return j_buffer;
}

void MediaCodecBridge::ReleaseOutputBuffer(int index, bool render) {
  JNIEnv* env = AttachCurrentThread();
  CHECK(env);

  Java_MediaCodecBridge_releaseOutputBuffer(
      env, j_media_codec_.obj(), index, render);
}

void MediaCodecBridge::GetOutputBuffers() {
  JNIEnv* env = AttachCurrentThread();
  Java_MediaCodecBridge_getOutputBuffers(env, j_media_codec_.obj());
}

AudioCodecBridge::AudioCodecBridge(const AudioCodec codec)
    : MediaCodecBridge(AudioCodecToMimeType(codec)) {
}

bool AudioCodecBridge::Start(
    const AudioCodec codec, int sample_rate, int channel_count,
    const uint8* extra_data, size_t extra_data_size, bool play_audio) {
  JNIEnv* env = AttachCurrentThread();
  DCHECK(AudioCodecToMimeType(codec));

  ScopedJavaLocalRef<jstring> j_mime =
      ConvertUTF8ToJavaString(env, AudioCodecToMimeType(codec));
  ScopedJavaLocalRef<jobject> j_format(
      JNI_MediaFormat::Java_MediaFormat_createAudioFormat(
          env, j_mime.obj(), sample_rate, channel_count));
  DCHECK(!j_format.is_null());

  if (extra_data_size > 0) {
    DCHECK_EQ(kCodecVorbis, codec);
    if (extra_data[0] != 2) {
      LOG(ERROR) << "Invalid number of headers before the codec header: "
                 << extra_data[0];
      return false;
    }

    size_t header_length[2];
    // |total_length| keeps track of the total number of bytes before the last
    // header.
    size_t total_length = 1;
    const uint8* current_pos = extra_data;
    // Calculate the length of the first 2 headers.
    for (int i = 0; i < 2; ++i) {
      header_length[i] = 0;
      while (total_length < extra_data_size) {
        size_t size = *(++current_pos);
        total_length += 1 + size;
        if (total_length > 0x80000000) {
          LOG(ERROR) << "Header size too large";
          return false;
        }
        header_length[i] += size;
        if (size < 0xFF)
          break;
      }
      if (total_length >= extra_data_size) {
        LOG(ERROR) << "Invalid header size in the extra data";
        return false;
      }
    }
    current_pos++;
    // The first header is identification header.
    jobject identification_header = env->NewDirectByteBuffer(
        const_cast<uint8*>(current_pos), header_length[0]);
    ScopedJavaLocalRef<jstring> j_csd_0 = ConvertUTF8ToJavaString(env, "csd-0");
    JNI_MediaFormat::Java_MediaFormat_setByteBuffer(
        env, j_format.obj(), j_csd_0.obj(), identification_header);
    // The last header is codec header.
    jobject codec_header = env->NewDirectByteBuffer(
        const_cast<uint8*>(extra_data + total_length),
        extra_data_size - total_length);
    ScopedJavaLocalRef<jstring> j_csd_1 = ConvertUTF8ToJavaString(env, "csd-1");
    JNI_MediaFormat::Java_MediaFormat_setByteBuffer(
        env, j_format.obj(), j_csd_1.obj(), codec_header);
    env->DeleteLocalRef(codec_header);
    env->DeleteLocalRef(identification_header);
  }

  Java_MediaCodecBridge_configureAudio(
      env, media_codec(), j_format.obj(), NULL, 0, play_audio);
  StartInternal();
  return true;
}

void AudioCodecBridge::PlayOutputBuffer(int index, size_t size) {
  DCHECK_LE(0, index);
  int numBytes = base::checked_numeric_cast<int>(size);
  JNIEnv* env = AttachCurrentThread();
  ScopedJavaLocalRef<jobject> buf =
      Java_MediaCodecBridge_getOutputBuffer(env, media_codec(), index);
  uint8* buffer = static_cast<uint8*>(env->GetDirectBufferAddress(buf.obj()));

  ScopedJavaLocalRef<jbyteArray> byte_array =
      base::android::ToJavaByteArray(env, buffer, numBytes);
  Java_MediaCodecBridge_playOutputBuffer(
      env, media_codec(), byte_array.obj());
}

VideoCodecBridge::VideoCodecBridge(const VideoCodec codec)
    : MediaCodecBridge(VideoCodecToMimeType(codec)) {
}

bool VideoCodecBridge::Start(
    const VideoCodec codec, const gfx::Size& size, jobject surface) {
  JNIEnv* env = AttachCurrentThread();
  DCHECK(VideoCodecToMimeType(codec));

  ScopedJavaLocalRef<jstring> j_mime =
      ConvertUTF8ToJavaString(env, VideoCodecToMimeType(codec));
  ScopedJavaLocalRef<jobject> j_format(
      JNI_MediaFormat::Java_MediaFormat_createVideoFormat(
          env, j_mime.obj(), size.width(), size.height()));
  DCHECK(!j_format.is_null());
  Java_MediaCodecBridge_configureVideo(
      env, media_codec(), j_format.obj(), surface, NULL, 0);
  StartInternal();
  return true;
}

}  // namespace media

