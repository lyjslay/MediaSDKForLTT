#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <thread>
#include <unordered_map>
#include "cvi_demuxer/ffmpeg_demuxer.hpp"
#include "cvi_demuxer/packet.h"
#include "cvi_demuxer/media_info.h"
#include "cvi_player/event/event.h"
#include "cvi_player/frame/frame.h"
#include "cvi_player/stream/audio_stream.hpp"
#include "cvi_player/stream/audio_parameters.h"
#include "cvi_player/stream/video_stream.hpp"
#include "cvi_player/stream/video_parameters.h"
#include "cvi_player/stream/subtitle_stream.hpp"
#include "cvi_player/types.hpp"
#include "cvi_player/service/event_service.hpp"
#include "cvi_player/thread/interval_thread.hpp"
#include "control_context.hpp"
#include "sync_context.hpp"
#include "seek_request.hpp"
#include "play_info.h"

namespace cvi_player {

class Player final : public EventService<CviPlayerEvent> {
public:
    using OutputHandler = AvStream::OutputHandler;

    Player();
    ~Player();

    CVI_ERROR play();
    CVI_ERROR stop();
    bool isRunning() const;
    bool isPaused() const;
    void setDataSource(const std::string &source);
    char *getDataSource() const;
    AVCodecID getVideoCodecId() const;
    void setDefaultSyncType(const AV_SYNC_TYPE &sync_type);
    void togglePause();
    void seekStreamByTime(int64_t time, const int64_t offset = 0);
    void seekPauseStreamByTime(int32_t flage, int64_t time, const int64_t offset = 0);
    void seekStreamByByte(const int64_t pos, const int64_t offset = 0);
    void setMediaOutputHandler(int32_t media_type, OutputHandler handler);
    void setMediaDecodeHandler(int32_t media_type, AvDecodeHandler handler);
    void setSpeed(double speed);
    void setAudioParameters(const CviPlayerAudioParameters& parameter);
    void setVideoParameters(const CviPlayerVideoParameters& parameter);
    CVI_ERROR openDemuxer();
    CVI_ERROR getMediaInfo(CviDemuxerMediaInfo &info);
    CVI_ERROR getPlayInfo(CviPlayerPlayInfo &info) const;
    CVI_ERROR getVideoFrame(CviPlayerFrame *frame) const;
    CVI_ERROR getVideoPacket(CviDemuxerPacket *packet) const;
    CVI_ERROR getVideoExtraPacket(CviDemuxerPacket *packet) const;
    void playerseep(int64_t seeplog, int32_t speed, int32_t backforward);
    void playseep();
    void playseepbakeoff();
    int32_t forwardbackwardstatus();
    CVI_ERROR getMediumVideoExtraPacket(CviDemuxerPacket *packet) const;
    void setPlaySubStreamFlag(bool subflag);

private:
    void prepare();
    CVI_ERROR open();
    CVI_ERROR openDataInput();
    void updateParametersIfNeed(const std::string &extension_name);
    CVI_ERROR seekToStartTime();
    template <typename T> void initStream(T *stream);
    CVI_ERROR openStreams();
    void closeStreams();
    void toggleStreamsPause();
    void startReadThread();
    void startRefreshThread();
    void startProgressThread();
    void startEventThread();
    void stopEventThread();
    bool sendSeekRequest(int64_t pos, int64_t offset, int32_t flags = 0);
    void seekseep(int64_t target, int64_t offset = 0, int32_t flag = 1);
    void updateStreamOutputHandler(int32_t media_type);
    void updateStreamDecodeHandler(int32_t media_type);
    void logStatus();
    void read();
    CVI_ERROR readPacket(AVPacket *packet);
    void putPacketToQueue(AVPacket &packet);
    void handlePause();
    void handleSeekRequest();
    void readToNextIdrPacket();
    CVI_ERROR handleAttachRequest();
    void syncExternalClockSpeed();
    bool reachQueueMaxBytes();
    bool streamsHasEnoughPackets();
    AV_SYNC_TYPE getSuitableSyncTypeByDuration(AV_SYNC_TYPE candidate_type) const;
    void findSuitableSyncContext(AV_SYNC_TYPE candidate_type);

private:
    std::atomic<bool> running{false};
    std::unique_ptr<cvi_demuxer::FFmpegDemuxer> demuxer;
    bool log_status{false};
    bool attach_request{false};
    std::shared_ptr<Clock> extern_clock;
    AV_SYNC_TYPE default_sync_type{AV_SYNC_TYPE::AUDIO_MASTER};
    std::shared_ptr<SyncContext> sync_context;
    CviPlayerAudioParameters audio_parameters{};
    CviPlayerVideoParameters video_parameters{};
    ControlContext control_context{};
    std::unique_ptr<SeekRequest> seek_request;
    std::shared_ptr<std::condition_variable> continue_read_cv;
    std::thread read_thread;
    IntervalThread refresh_thread;
    IntervalThread progress_thread;
    std::unique_ptr<VideoStream> video_stream;
    std::unique_ptr<AudioStream> audio_stream;
    std::unique_ptr<SubtitleStream> subtitle_stream;
    std::unordered_map<int32_t, OutputHandler> output_handler_map;
    std::unordered_map<int32_t, AvDecodeHandler> decode_handler_map;
};

} // namespace cvi_player
