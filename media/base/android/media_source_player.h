// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_BASE_ANDROID_MEDIA_SOURCE_PLAYER_H_
#define MEDIA_BASE_ANDROID_MEDIA_SOURCE_PLAYER_H_

#include <jni.h>
#include <map>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread.h"
#include "base/time.h"
#include "media/base/android/demuxer_stream_player_params.h"
#include "media/base/android/media_codec_bridge.h"
#include "media/base/android/media_player_android.h"
#include "media/base/media_export.h"

namespace base {
class MessageLoopProxy;
}

namespace media {

class VideoDecoderJob;
class AudioDecoderJob;

// Class for managing all the decoding tasks. Each decoding task will be posted
// onto the same thread. The thread will be stopped once Stop() is called.
class MediaDecoderJob {
 public:
  virtual ~MediaDecoderJob();

  // Callback when a decoder job finishes its work. Args: presentation time,
  // timestamp when the data is rendered, whether decoder is reaching EOS.
  typedef base::Callback<void(const base::TimeDelta&, const base::Time&, bool)>
      DecoderCallback;

  // Called by MediaSourcePlayer to decode some data.
  void Decode(
      const MediaPlayerHostMsg_ReadFromDemuxerAck_Params::AccessUnit& unit,
      const base::Time& start_wallclock_time,
      const base::TimeDelta& start_presentation_timestamp,
      const MediaDecoderJob::DecoderCallback& callback);

  // Flush the decoder.
  void Flush();

  struct Deleter {
      inline void operator()(MediaDecoderJob* ptr) const { ptr->Release(); }
  };

  // Causes this instance to be deleted on the thread it is bound to.
  void Release();

 protected:
  MediaDecoderJob(
      bool is_audio, const scoped_refptr<base::MessageLoopProxy>& message_loop);

  // Release the output buffer and render it.
  void ReleaseOutputBuffer(
      int outputBufferIndex, size_t size,
      const base::TimeDelta& presentation_timestamp,
      bool end_of_stream, const MediaDecoderJob::DecoderCallback& callback);

  // Helper function to decoder data on |thread_|. |unit| contains all the data
  // to be decoded. |start_wallclock_time| and |start_presentation_timestamp|
  // represent the system time and the presentation timestamp when the first
  // frame is rendered. We use these information to estimate when the current
  // frame should be rendered. If |needs_flush| is true, codec needs to be
  // flushed at the beginning of this call.
  void DecodeInternal(
      const MediaPlayerHostMsg_ReadFromDemuxerAck_Params::AccessUnit& unit,
      const base::Time& start_wallclock_time,
      const base::TimeDelta& start_presentation_timestamp,
      bool needs_flush,
      const MediaDecoderJob::DecoderCallback& callback);

  // The media codec bridge used for decoding.
  scoped_ptr<MediaCodecBridge> media_codec_bridge_;

  // The message loop where callbacks should be dispatched.
  scoped_refptr<base::MessageLoopProxy> message_loop_;

  // Thread the decode task runs on.
  scoped_ptr<base::Thread> thread_;

  // Whether the decoder needs to be flushed.
  bool needs_flush_;

  // Whether this is an audio decoder.
  bool is_audio_;

  // Weak pointer passed to media decoder jobs for callbacks.
  base::WeakPtrFactory<MediaDecoderJob> weak_this_;
};

typedef scoped_ptr<MediaDecoderJob, MediaDecoderJob::Deleter>
    ScopedMediaDecoderJob;

// This class handles media source extensions on Android. It uses Android
// MediaCodec to decode audio and video streams in two separate threads.
// IPC is being used to send data from the render process to this object.
// TODO(qinmin): use shared memory to send data between processes.
class MEDIA_EXPORT MediaSourcePlayer : public MediaPlayerAndroid {
 public:
  // Construct a MediaSourcePlayer object with all the needed media player
  // callbacks.
  MediaSourcePlayer(int player_id,
                    MediaPlayerManager* manager);
  virtual ~MediaSourcePlayer();

  // MediaPlayerAndroid implementation.
  virtual void SetVideoSurface(jobject surface) OVERRIDE;
  virtual void Start() OVERRIDE;
  virtual void Pause() OVERRIDE;
  virtual void SeekTo(base::TimeDelta timestamp) OVERRIDE;
  virtual void Release() OVERRIDE;
  virtual void SetVolume(float leftVolume, float rightVolume) OVERRIDE;
  virtual int GetVideoWidth() OVERRIDE;
  virtual int GetVideoHeight() OVERRIDE;
  virtual base::TimeDelta GetCurrentTime() OVERRIDE;
  virtual base::TimeDelta GetDuration() OVERRIDE;
  virtual bool IsPlaying() OVERRIDE;
  virtual bool CanPause() OVERRIDE;
  virtual bool CanSeekForward() OVERRIDE;
  virtual bool CanSeekBackward() OVERRIDE;
  virtual bool IsPlayerReady() OVERRIDE;

  // Called when the demuxer is ready.
  virtual void DemuxerReady(
      const MediaPlayerHostMsg_DemuxerReady_Params& params) OVERRIDE;

  // Called when the requested data is received from the demuxer.
  virtual void ReadFromDemuxerAck(
      const MediaPlayerHostMsg_ReadFromDemuxerAck_Params& params) OVERRIDE;

 private:
  // Update the timestamps for A/V sync scheduling. |kickoff_time| keeps
  // track of when the job is started. We need this to check if a seek is
  // performed during decoding.
  void UpdateTimestamps(
      const base::Time& kickoff_time,
      const base::TimeDelta& presentation_timestamp,
      const base::Time& wallclock_time);

  // Helper function for starting media playback.
  void StartInternal();

  // Playback is completed for one channel.
  void PlaybackCompleted(bool is_audio);

  // Called when the decoder finishes its task.
  void MediaDecoderCallback(
        bool is_audio, const base::Time& kickoff_time,
        const base::TimeDelta& presentation_timestamp,
        const base::Time& wallclock_time, bool end_of_stream);

  // Called to decoder more data.
  void DecodeMoreAudio();
  void DecodeMoreVideo();

  // Functions check whether audio/video is present.
  bool HasVideo();
  bool HasAudio();

  // Pending play event while player is preparing.
  bool pending_play_;

  // Stats about the media.
  base::TimeDelta duration_;
  int width_;
  int height_;
  AudioCodec audio_codec_;
  VideoCodec video_codec_;
  int num_channels_;
  int sampling_rate_;
  bool seekable_;
  base::TimeDelta last_presentation_timestamp_;
  std::vector<uint8> audio_extra_data_;
  bool audio_finished_;
  bool video_finished_;
  bool playing_;

  // Timestamps for providing simple A/V sync. When start decoding an audio
  // chunk, we record its presentation timestamp and the current system time.
  // Then we use this information to estimate when the next audio/video frame
  // should be rendered.
  // TODO(qinmin): Need to fix the problem if audio/video lagged too far behind
  // due to network or decoding problem.
  base::Time start_wallclock_time_;
  base::TimeDelta start_presentation_timestamp_;
  base::Time last_seek_time_;

  // Decoder jobs
  ScopedMediaDecoderJob audio_decoder_job_;
  ScopedMediaDecoderJob video_decoder_job_;

  // These variables keep track of the current decoding data.
  // TODO(qinmin): remove these variables when we no longer relies on IPC for
  // data passing.
  size_t audio_access_unit_index_;
  size_t video_access_unit_index_;
  bool waiting_for_audio_data_;
  bool waiting_for_video_data_;
  MediaPlayerHostMsg_ReadFromDemuxerAck_Params received_audio_;
  MediaPlayerHostMsg_ReadFromDemuxerAck_Params received_video_;

  // Weak pointer passed to media decoder jobs for callbacks.
  base::WeakPtrFactory<MediaSourcePlayer> weak_this_;

  DISALLOW_COPY_AND_ASSIGN(MediaSourcePlayer);
};

}  // namespace media

#endif  // MEDIA_BASE_ANDROID_MEDIA_SOURCE_PLAYER_H_
