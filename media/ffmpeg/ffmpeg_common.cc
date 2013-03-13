// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "media/ffmpeg/ffmpeg_common.h"

#include "base/basictypes.h"
#include "base/logging.h"
#include "media/base/decoder_buffer.h"
#include "media/base/video_frame.h"
#include "media/base/video_util.h"

namespace media {

// Why FF_INPUT_BUFFER_PADDING_SIZE? FFmpeg assumes all input buffers are
// padded. Check here to ensure FFmpeg only receives data padded to its
// specifications.
COMPILE_ASSERT(DecoderBuffer::kPaddingSize >= FF_INPUT_BUFFER_PADDING_SIZE,
               decoder_buffer_padding_size_does_not_fit_ffmpeg_requirement);

// Alignment requirement by FFmpeg for input and output buffers. This need to
// be updated to match FFmpeg when it changes.
#if defined(ARCH_CPU_ARM_FAMILY)
static const int kFFmpegBufferAddressAlignment = 16;
#else
static const int kFFmpegBufferAddressAlignment = 32;
#endif

// Check here to ensure FFmpeg only receives data aligned to its specifications.
COMPILE_ASSERT(
    DecoderBuffer::kAlignmentSize >= kFFmpegBufferAddressAlignment &&
    DecoderBuffer::kAlignmentSize % kFFmpegBufferAddressAlignment == 0,
    decoder_buffer_alignment_size_does_not_fit_ffmpeg_requirement);

// Allows faster SIMD YUV convert. Also, FFmpeg overreads/-writes occasionally.
// See video_get_buffer() in libavcodec/utils.c.
static const int kFFmpegOutputBufferPaddingSize = 16;

COMPILE_ASSERT(VideoFrame::kFrameSizePadding >= kFFmpegOutputBufferPaddingSize,
               video_frame_padding_size_does_not_fit_ffmpeg_requirement);

COMPILE_ASSERT(
    VideoFrame::kFrameAddressAlignment >= kFFmpegBufferAddressAlignment &&
    VideoFrame::kFrameAddressAlignment % kFFmpegBufferAddressAlignment == 0,
    video_frame_address_alignment_does_not_fit_ffmpeg_requirement);

static const AVRational kMicrosBase = { 1, base::Time::kMicrosecondsPerSecond };

base::TimeDelta ConvertFromTimeBase(const AVRational& time_base,
                                    int64 timestamp) {
  int64 microseconds = av_rescale_q(timestamp, time_base, kMicrosBase);
  return base::TimeDelta::FromMicroseconds(microseconds);
}

int64 ConvertToTimeBase(const AVRational& time_base,
                        const base::TimeDelta& timestamp) {
  return av_rescale_q(timestamp.InMicroseconds(), kMicrosBase, time_base);
}

AudioCodec CodecIDToAudioCodec(CodecID codec_id) {
  switch (codec_id) {
    case CODEC_ID_AAC:
      return kCodecAAC;
    case CODEC_ID_MP3:
      return kCodecMP3;
    case CODEC_ID_VORBIS:
      return kCodecVorbis;
    case CODEC_ID_PCM_U8:
    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S24LE:
    case CODEC_ID_PCM_F32LE:
      return kCodecPCM;
    case CODEC_ID_PCM_S16BE:
      return kCodecPCM_S16BE;
    case CODEC_ID_PCM_S24BE:
      return kCodecPCM_S24BE;
    case CODEC_ID_FLAC:
      return kCodecFLAC;
    case CODEC_ID_AMR_NB:
      return kCodecAMR_NB;
    case CODEC_ID_AMR_WB:
      return kCodecAMR_WB;
    case CODEC_ID_GSM_MS:
      return kCodecGSM_MS;
    case CODEC_ID_PCM_MULAW:
      return kCodecPCM_MULAW;
#ifndef CHROMIUM_OMIT_CODEC_ID_OPUS
    case CODEC_ID_OPUS:
      return kCodecOpus;
#endif
    default:
      DVLOG(1) << "Unknown audio CodecID: " << codec_id;
  }
  return kUnknownAudioCodec;
}

static CodecID AudioCodecToCodecID(AudioCodec audio_codec,
                                   SampleFormat sample_format) {
  switch (audio_codec) {
    case kCodecAAC:
      return CODEC_ID_AAC;
    case kCodecMP3:
      return CODEC_ID_MP3;
    case kCodecPCM:
      switch (sample_format) {
        case kSampleFormatU8:
          return CODEC_ID_PCM_U8;
        case kSampleFormatS16:
          return CODEC_ID_PCM_S16LE;
        case kSampleFormatS32:
          return CODEC_ID_PCM_S24LE;
        case kSampleFormatF32:
          return CODEC_ID_PCM_F32LE;
        default:
          DVLOG(1) << "Unsupported sample format: " << sample_format;
      }
      break;
    case kCodecPCM_S16BE:
      return CODEC_ID_PCM_S16BE;
    case kCodecPCM_S24BE:
      return CODEC_ID_PCM_S24BE;
    case kCodecVorbis:
      return CODEC_ID_VORBIS;
    case kCodecFLAC:
      return CODEC_ID_FLAC;
    case kCodecAMR_NB:
      return CODEC_ID_AMR_NB;
    case kCodecAMR_WB:
      return CODEC_ID_AMR_WB;
    case kCodecGSM_MS:
      return CODEC_ID_GSM_MS;
    case kCodecPCM_MULAW:
      return CODEC_ID_PCM_MULAW;
#ifndef CHROMIUM_OMIT_CODEC_ID_OPUS
    case kCodecOpus:
      return CODEC_ID_OPUS;
#endif
    default:
      DVLOG(1) << "Unknown AudioCodec: " << audio_codec;
  }
  return CODEC_ID_NONE;
}

VideoCodec CodecIDToVideoCodec(CodecID codec_id) {
  switch (codec_id) {
    case CODEC_ID_H264:
      return kCodecH264;
    case CODEC_ID_THEORA:
      return kCodecTheora;
    case CODEC_ID_MPEG4:
      return kCodecMPEG4;
    case CODEC_ID_VP8:
      return kCodecVP8;
#ifndef CHROMIUM_OMIT_AV_CODEC_ID_VP9
    case AV_CODEC_ID_VP9:
      return kCodecVP9;
#endif
    default:
      DVLOG(1) << "Unknown video CodecID: " << codec_id;
  }
  return kUnknownVideoCodec;
}

static CodecID VideoCodecToCodecID(VideoCodec video_codec) {
  switch (video_codec) {
    case kCodecH264:
      return CODEC_ID_H264;
    case kCodecTheora:
      return CODEC_ID_THEORA;
    case kCodecMPEG4:
      return CODEC_ID_MPEG4;
    case kCodecVP8:
      return CODEC_ID_VP8;
#ifndef CHROMIUM_OMIT_AV_CODEC_ID_VP9
    case kCodecVP9:
      return AV_CODEC_ID_VP9;
#endif
    default:
      DVLOG(1) << "Unknown VideoCodec: " << video_codec;
  }
  return CODEC_ID_NONE;
}

static VideoCodecProfile ProfileIDToVideoCodecProfile(int profile) {
  // Clear out the CONSTRAINED & INTRA flags which are strict subsets of the
  // corresponding profiles with which they're used.
  profile &= ~FF_PROFILE_H264_CONSTRAINED;
  profile &= ~FF_PROFILE_H264_INTRA;
  switch (profile) {
    case FF_PROFILE_H264_BASELINE:
      return H264PROFILE_BASELINE;
    case FF_PROFILE_H264_MAIN:
      return H264PROFILE_MAIN;
    case FF_PROFILE_H264_EXTENDED:
      return H264PROFILE_EXTENDED;
    case FF_PROFILE_H264_HIGH:
      return H264PROFILE_HIGH;
    case FF_PROFILE_H264_HIGH_10:
      return H264PROFILE_HIGH10PROFILE;
    case FF_PROFILE_H264_HIGH_422:
      return H264PROFILE_HIGH422PROFILE;
    case FF_PROFILE_H264_HIGH_444_PREDICTIVE:
      return H264PROFILE_HIGH444PREDICTIVEPROFILE;
    default:
      DVLOG(1) << "Unknown profile id: " << profile;
  }
  return VIDEO_CODEC_PROFILE_UNKNOWN;
}

static int VideoCodecProfileToProfileID(VideoCodecProfile profile) {
  switch (profile) {
    case H264PROFILE_BASELINE:
      return FF_PROFILE_H264_BASELINE;
    case H264PROFILE_MAIN:
      return FF_PROFILE_H264_MAIN;
    case H264PROFILE_EXTENDED:
      return FF_PROFILE_H264_EXTENDED;
    case H264PROFILE_HIGH:
      return FF_PROFILE_H264_HIGH;
    case H264PROFILE_HIGH10PROFILE:
      return FF_PROFILE_H264_HIGH_10;
    case H264PROFILE_HIGH422PROFILE:
      return FF_PROFILE_H264_HIGH_422;
    case H264PROFILE_HIGH444PREDICTIVEPROFILE:
      return FF_PROFILE_H264_HIGH_444_PREDICTIVE;
    default:
      DVLOG(1) << "Unknown VideoCodecProfile: " << profile;
  }
  return FF_PROFILE_UNKNOWN;
}

static SampleFormat AVSampleFormatToSampleFormat(AVSampleFormat sample_format) {
  switch (sample_format) {
    case AV_SAMPLE_FMT_U8:
      return kSampleFormatU8;
    case AV_SAMPLE_FMT_S16:
      return kSampleFormatS16;
    case AV_SAMPLE_FMT_S32:
      return kSampleFormatS32;
    case AV_SAMPLE_FMT_FLT:
      return kSampleFormatF32;
    case AV_SAMPLE_FMT_S16P:
      return kSampleFormatPlanarS16;
    case AV_SAMPLE_FMT_FLTP:
      return kSampleFormatPlanarF32;
    default:
      DVLOG(1) << "Unknown AVSampleFormat: " << sample_format;
  }
  return kUnknownSampleFormat;
}

static AVSampleFormat SampleFormatToAVSampleFormat(SampleFormat sample_format) {
  switch (sample_format) {
    case kSampleFormatU8:
      return AV_SAMPLE_FMT_U8;
    case kSampleFormatS16:
      return AV_SAMPLE_FMT_S16;
    case kSampleFormatS32:
      return AV_SAMPLE_FMT_S32;
    case kSampleFormatF32:
      return AV_SAMPLE_FMT_FLT;
    case kSampleFormatPlanarS16:
      return AV_SAMPLE_FMT_S16P;
    case kSampleFormatPlanarF32:
      return AV_SAMPLE_FMT_FLTP;
    default:
      DVLOG(1) << "Unknown SampleFormat: " << sample_format;
  }
  return AV_SAMPLE_FMT_NONE;
}

void AVCodecContextToAudioDecoderConfig(
    const AVCodecContext* codec_context,
    AudioDecoderConfig* config) {
  DCHECK_EQ(codec_context->codec_type, AVMEDIA_TYPE_AUDIO);

  AudioCodec codec = CodecIDToAudioCodec(codec_context->codec_id);

  SampleFormat sample_format =
      AVSampleFormatToSampleFormat(codec_context->sample_fmt);

  ChannelLayout channel_layout = ChannelLayoutToChromeChannelLayout(
      codec_context->channel_layout, codec_context->channels);

  if (codec == kCodecOpus) {
    // |codec_context->sample_fmt| is not set by FFmpeg because Opus decoding is
    // not enabled in FFmpeg, so we need to manually set the sample format.
    sample_format = kSampleFormatS16;
  }

  config->Initialize(codec,
                     sample_format,
                     channel_layout,
                     codec_context->sample_rate,
                     codec_context->extradata,
                     codec_context->extradata_size,
                     false,  // Not encrypted.
                     true);
  if (codec != kCodecOpus) {
    DCHECK_EQ(av_get_bytes_per_sample(codec_context->sample_fmt) * 8,
              config->bits_per_channel());
  }
}

void AudioDecoderConfigToAVCodecContext(const AudioDecoderConfig& config,
                                        AVCodecContext* codec_context) {
  codec_context->codec_type = AVMEDIA_TYPE_AUDIO;
  codec_context->codec_id = AudioCodecToCodecID(config.codec(),
                                                config.sample_format());
  codec_context->sample_fmt = SampleFormatToAVSampleFormat(
      config.sample_format());

  // TODO(scherkus): should we set |channel_layout|? I'm not sure if FFmpeg uses
  // said information to decode.
  codec_context->channels =
      ChannelLayoutToChannelCount(config.channel_layout());
  codec_context->sample_rate = config.samples_per_second();

  if (config.extra_data()) {
    codec_context->extradata_size = config.extra_data_size();
    codec_context->extradata = reinterpret_cast<uint8_t*>(
        av_malloc(config.extra_data_size() + FF_INPUT_BUFFER_PADDING_SIZE));
    memcpy(codec_context->extradata, config.extra_data(),
           config.extra_data_size());
    memset(codec_context->extradata + config.extra_data_size(), '\0',
           FF_INPUT_BUFFER_PADDING_SIZE);
  } else {
    codec_context->extradata = NULL;
    codec_context->extradata_size = 0;
  }
}

void AVStreamToVideoDecoderConfig(
    const AVStream* stream,
    VideoDecoderConfig* config) {
  gfx::Size coded_size(stream->codec->coded_width, stream->codec->coded_height);

  // TODO(vrk): This assumes decoded frame data starts at (0, 0), which is true
  // for now, but may not always be true forever. Fix this in the future.
  gfx::Rect visible_rect(stream->codec->width, stream->codec->height);

  AVRational aspect_ratio = { 1, 1 };
  if (stream->sample_aspect_ratio.num)
    aspect_ratio = stream->sample_aspect_ratio;
  else if (stream->codec->sample_aspect_ratio.num)
    aspect_ratio = stream->codec->sample_aspect_ratio;

  VideoCodec codec = CodecIDToVideoCodec(stream->codec->codec_id);

  VideoCodecProfile profile = VIDEO_CODEC_PROFILE_UNKNOWN;
  if (codec == kCodecVP8)
    profile = VP8PROFILE_MAIN;
  else if (codec == kCodecVP9)
    profile = VP9PROFILE_MAIN;
  else
    profile = ProfileIDToVideoCodecProfile(stream->codec->profile);

  gfx::Size natural_size = GetNaturalSize(
      visible_rect.size(), aspect_ratio.num, aspect_ratio.den);

  VideoFrame::Format format = PixelFormatToVideoFormat(stream->codec->pix_fmt);
  if (codec == kCodecVP9) {
    // TODO(tomfinegan): libavcodec doesn't know about VP9.
    format = VideoFrame::YV12;
    coded_size = natural_size;
  }

  config->Initialize(codec,
                     profile,
                     format,
                     coded_size, visible_rect, natural_size,
                     stream->codec->extradata, stream->codec->extradata_size,
                     false,  // Not encrypted.
                     true);
}

void VideoDecoderConfigToAVCodecContext(
    const VideoDecoderConfig& config,
    AVCodecContext* codec_context) {
  codec_context->codec_type = AVMEDIA_TYPE_VIDEO;
  codec_context->codec_id = VideoCodecToCodecID(config.codec());
  codec_context->profile = VideoCodecProfileToProfileID(config.profile());
  codec_context->coded_width = config.coded_size().width();
  codec_context->coded_height = config.coded_size().height();
  codec_context->pix_fmt = VideoFormatToPixelFormat(config.format());

  if (config.extra_data()) {
    codec_context->extradata_size = config.extra_data_size();
    codec_context->extradata = reinterpret_cast<uint8_t*>(
        av_malloc(config.extra_data_size() + FF_INPUT_BUFFER_PADDING_SIZE));
    memcpy(codec_context->extradata, config.extra_data(),
           config.extra_data_size());
    memset(codec_context->extradata + config.extra_data_size(), '\0',
           FF_INPUT_BUFFER_PADDING_SIZE);
  } else {
    codec_context->extradata = NULL;
    codec_context->extradata_size = 0;
  }
}

ChannelLayout ChannelLayoutToChromeChannelLayout(int64_t layout, int channels) {
  switch (layout) {
    case AV_CH_LAYOUT_MONO:
      return CHANNEL_LAYOUT_MONO;
    case AV_CH_LAYOUT_STEREO:
      return CHANNEL_LAYOUT_STEREO;
    case AV_CH_LAYOUT_2_1:
      return CHANNEL_LAYOUT_2_1;
    case AV_CH_LAYOUT_SURROUND:
      return CHANNEL_LAYOUT_SURROUND;
    case AV_CH_LAYOUT_4POINT0:
      return CHANNEL_LAYOUT_4_0;
    case AV_CH_LAYOUT_2_2:
      return CHANNEL_LAYOUT_2_2;
    case AV_CH_LAYOUT_QUAD:
      return CHANNEL_LAYOUT_QUAD;
    case AV_CH_LAYOUT_5POINT0:
      return CHANNEL_LAYOUT_5_0;
    case AV_CH_LAYOUT_5POINT1:
      return CHANNEL_LAYOUT_5_1;
    case AV_CH_LAYOUT_5POINT0_BACK:
      return CHANNEL_LAYOUT_5_0_BACK;
    case AV_CH_LAYOUT_5POINT1_BACK:
      return CHANNEL_LAYOUT_5_1_BACK;
    case AV_CH_LAYOUT_7POINT0:
      return CHANNEL_LAYOUT_7_0;
    case AV_CH_LAYOUT_7POINT1:
      return CHANNEL_LAYOUT_7_1;
    case AV_CH_LAYOUT_7POINT1_WIDE:
      return CHANNEL_LAYOUT_7_1_WIDE;
    case AV_CH_LAYOUT_STEREO_DOWNMIX:
      return CHANNEL_LAYOUT_STEREO_DOWNMIX;
    case AV_CH_LAYOUT_2POINT1:
      return CHANNEL_LAYOUT_2POINT1;
    case AV_CH_LAYOUT_3POINT1:
      return CHANNEL_LAYOUT_3_1;
    case AV_CH_LAYOUT_4POINT1:
      return CHANNEL_LAYOUT_4_1;
    case AV_CH_LAYOUT_6POINT0:
      return CHANNEL_LAYOUT_6_0;
    case AV_CH_LAYOUT_6POINT0_FRONT:
      return CHANNEL_LAYOUT_6_0_FRONT;
    case AV_CH_LAYOUT_HEXAGONAL:
      return CHANNEL_LAYOUT_HEXAGONAL;
    case AV_CH_LAYOUT_6POINT1:
      return CHANNEL_LAYOUT_6_1;
    case AV_CH_LAYOUT_6POINT1_BACK:
      return CHANNEL_LAYOUT_6_1_BACK;
    case AV_CH_LAYOUT_6POINT1_FRONT:
      return CHANNEL_LAYOUT_6_1_FRONT;
    case AV_CH_LAYOUT_7POINT0_FRONT:
      return CHANNEL_LAYOUT_7_0_FRONT;
#ifdef AV_CH_LAYOUT_7POINT1_WIDE_BACK
    case AV_CH_LAYOUT_7POINT1_WIDE_BACK:
      return CHANNEL_LAYOUT_7_1_WIDE_BACK;
#endif
    case AV_CH_LAYOUT_OCTAGONAL:
      return CHANNEL_LAYOUT_OCTAGONAL;
    default:
      // FFmpeg channel_layout is 0 for .wav and .mp3.  Attempt to guess layout
      // based on the channel count.
      return GuessChannelLayout(channels);
  }
}

VideoFrame::Format PixelFormatToVideoFormat(PixelFormat pixel_format) {
  switch (pixel_format) {
    case PIX_FMT_YUV422P:
      return VideoFrame::YV16;
    // TODO(scherkus): We should be paying attention to the color range of each
    // format and scaling as appropriate when rendering. Regular YUV has a range
    // of 16-239 where as YUVJ has a range of 0-255.
    case PIX_FMT_YUV420P:
    case PIX_FMT_YUVJ420P:
      return VideoFrame::YV12;
    default:
      DVLOG(1) << "Unsupported PixelFormat: " << pixel_format;
  }
  return VideoFrame::INVALID;
}

PixelFormat VideoFormatToPixelFormat(VideoFrame::Format video_format) {
  switch (video_format) {
    case VideoFrame::YV16:
      return PIX_FMT_YUV422P;
    case VideoFrame::YV12:
      return PIX_FMT_YUV420P;
    default:
      DVLOG(1) << "Unsupported VideoFrame::Format: " << video_format;
  }
  return PIX_FMT_NONE;
}

}  // namespace media
