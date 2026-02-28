#include "cvi_player/player/player.hpp"
#include <chrono>
#include <cmath>
#include <type_traits>
#include <mutex>
#include "cvi_log.h"
#include "cvi_demuxer/utils/file.hpp"
#include "cvi_demuxer/utils/packet.hpp"

extern "C"
{
    #include <libavutil/time.h>
    #include <libavutil/bprint.h>
}

namespace cvi_player {

using std::condition_variable;
using std::thread;
using std::string;
#define true (1)
#define false (0)
#define QUEUEMAX (50)

constexpr int32_t EXTERNAL_CLOCK_MIN_FRAMES = 2;
constexpr int32_t EXTERNAL_CLOCK_MAX_FRAMES = 10;
constexpr double EXTERNAL_CLOCK_SPEED_MIN = 0.900;
constexpr double EXTERNAL_CLOCK_SPEED_MAX = 1.010;
constexpr double EXTERNAL_CLOCK_SPEED_STEP = 0.001;
constexpr int32_t REFRESH_INTERVAL_MS = 100;
constexpr int32_t PLAY_PROGRESS_INTERVAL_MS = 100;
int32_t seekflage = false;
int32_t readflage = false;
int32_t timeflage = false;
int32_t seekpause = false;
int32_t seektimetime = 0;
double lasttimepts = 0;
int64_t g_seep = false;
double timetime = 0;
double timetimebake = 0;
int32_t seepeed = 0;
int32_t seepeedbake = 0;
int32_t seep = 1;
int32_t backforwards = 2;
double lastetime = 0;

Player::Player()
{
    continue_read_cv = std::make_shared<condition_variable>();
    demuxer = std::make_unique<cvi_demuxer::FFmpegDemuxer>();
    extern_clock = std::make_shared<Clock>();
    sync_context = std::make_shared<SyncContext>();
}

Player::~Player()
{
    stop();
}

CVI_ERROR Player::play()
{
    seekflage = false;
    readflage = false;
    timeflage = false;
    lasttimepts = 0;
    timetime = 0;
    timetimebake = 0;
    seep = 1;
    seepeed = 0;
    seepeedbake = 0;
    backforwards = 2;
    if (running) {
        CVI_LOGE("Player already running");
        return CVI_ERROR::FAILURE;
    }

    running = true;
    prepare();
    if (CVI_ERROR::NONE != open()) {
        CVI_LOGE("Player open failed");
        stop();
        return CVI_ERROR::FAILURE;
    }

    startReadThread();
    startRefreshThread();
    startProgressThread();

    publishEvent(CviPlayerEvent {
        .type = CVI_PLAYER_EVENT_PLAY
    });

    return CVI_ERROR::NONE;
}

void Player::prepare()
{
    extern_clock.reset(new Clock());
    extern_clock->setQueueSharedSerial(extern_clock->getSharedSerial());
}

CVI_ERROR Player::open()
{
    CVI_ERROR error = openDataInput();
    if (CVI_ERROR::NONE != error) {
        CVI_LOGE("Open data input error:%d", static_cast<int32_t>(error));
        return error;
    }

    if (log_status) {
        demuxer->dumpInputInfo();
    }

    seekToStartTime();
    error = openStreams();
    if (CVI_ERROR::NONE != error) {
        CVI_LOGE("Open streams error:%d", static_cast<int32_t>(error));
        return error;
    }

    return CVI_ERROR::NONE;
}

CVI_ERROR Player::openDataInput()
{
    if ((!demuxer->isOpened()) && (openDemuxer() != CVI_ERROR::NONE)) {
        return CVI_ERROR::FAILURE;
    }

    updateParametersIfNeed(cvi_demuxer::utils::getFileExtensionName(demuxer->getInput()));
    control_context.max_frame_duration = demuxer->getMaxFrameDuration();
    control_context.realtime = demuxer->isRealTime();

    return CVI_ERROR::NONE;
}

void Player::updateParametersIfNeed(const string &extension_name)
{
    if ("raw" == extension_name) {
        if (audio_parameters.sample_rate == 0) {
            audio_parameters.sample_rate = 16000;
        }
    }
}

CVI_ERROR Player::seekToStartTime()
{
    if (control_context.start_time != AV_NOPTS_VALUE) {
        int64_t start_time = control_context.start_time;
        if (AV_NOPTS_VALUE != demuxer->getStartTime()) {
            start_time += demuxer->getStartTime();
        }

        if (0 != demuxer->seek(start_time)) {
            return CVI_ERROR::FAILURE;
        }
    }

    return CVI_ERROR::NONE;
}

template <typename T>
void Player::initStream(T *stream)
{
    if (std::is_base_of<BaseStream, T>::value) {
        stream->setEmptyCV(continue_read_cv);
    }
    if (std::is_base_of<AvStream, T>::value) {
        dynamic_cast<AvStream *>(stream)->setSyncContext(sync_context);
    }
    if (std::is_same<VideoStream, T>::value) {
        dynamic_cast<VideoStream *>(stream)->setParameters(video_parameters);
        dynamic_cast<VideoStream *>(stream)->setMaxFrameDuration(control_context.max_frame_duration);
        updateStreamOutputHandler(AVMEDIA_TYPE_VIDEO);
        updateStreamDecodeHandler(AVMEDIA_TYPE_VIDEO);
    }
    if (std::is_same<AudioStream, T>::value) {
        dynamic_cast<AudioStream *>(stream)->setParameters(audio_parameters);
        updateStreamOutputHandler(AVMEDIA_TYPE_AUDIO);
        updateStreamDecodeHandler(AVMEDIA_TYPE_AUDIO);
    }
}

CVI_ERROR Player::openStreams()
{
    /* init each streams */
    int32_t stream_index = demuxer->getStreamIndex(AVMEDIA_TYPE_VIDEO);
    AVStream *av_stream = demuxer->getAvStream(stream_index);
    if (av_stream != nullptr) {
        video_stream = std::make_unique<VideoStream>(stream_index, av_stream);
        initStream(video_stream.get());
    }

    stream_index = demuxer->getStreamIndex(AVMEDIA_TYPE_AUDIO);
    av_stream = demuxer->getAvStream(stream_index);
    if (av_stream != nullptr) {
        audio_stream = std::make_unique<AudioStream>(stream_index, av_stream);
        initStream(audio_stream.get());
    }

    /// TODO: Support subtitle stream
    /*
    stream_index = demuxer->getStreamIndex(AVMEDIA_TYPE_SUBTITLE);
    av_stream = demuxer->getAvStream(stream_index);
    if (av_stream != nullptr) {
        subtitle_stream = std::make_unique<SubtitleStream>(stream_index, av_stream);
        initStream(subtitle_stream.get());
    }
    */

    if (!video_stream && !audio_stream) {
        CVI_LOGE("Failed to open file '%s' or configure filtergraph", demuxer->getInput());
        return CVI_ERROR::FAILURE;
    }

    findSuitableSyncContext(default_sync_type);
    if (video_stream) {
        video_stream->open();
    }
    if (audio_stream) {
        audio_stream->open();
    }

    if (0 != (demuxer->getfilenameextern())) {
        if (subtitle_stream) {
            subtitle_stream->open();
        }
    }

    return CVI_ERROR::NONE;
}

CVI_ERROR Player::stop()
{
    running = false;
    continue_read_cv->notify_all();

    if (read_thread.joinable()) {
        read_thread.join();
    }
    refresh_thread.stop();
    progress_thread.stop();

    closeStreams();
    demuxer->close();

    attach_request = false;
    audio_parameters = {};
    control_context = {};
    seek_request.reset();

    return CVI_ERROR::NONE;
}

void Player::closeStreams()
{
    if (video_stream) {
        if (video_stream.get()) {
            video_stream.reset();
        }
    }
    if (audio_stream) {
        if (audio_stream.get()) {
            audio_stream.reset();
        }
    }

    if (0 != (demuxer->getfilenameextern())) {
        if (subtitle_stream) {
            subtitle_stream.reset();
        }
    }
}

bool Player::isRunning() const
{
    return running;
}

bool Player::isPaused() const
{
    return control_context.paused;
}

void Player::setDataSource(const string &source)
{
    demuxer->setInput(source);
}

char *Player::getDataSource() const
{
    return demuxer->getInput();
}

AVCodecID Player::getVideoCodecId() const
{
    return demuxer->getVideoCodecId();
}

void Player::setDefaultSyncType(const AV_SYNC_TYPE &sync_type)
{
    default_sync_type = sync_type;
}

void Player::togglePause()
{
    g_seep = false;
    seepeedbake = 0;
    seep = 1;
    seepeed = 0;
    toggleStreamsPause();

    CviPlayerEventType type = (isPaused()) ? CVI_PLAYER_EVENT_PAUSE : CVI_PLAYER_EVENT_RESUME;
    if (true == seekpause) {
        if (type == CVI_PLAYER_EVENT_PAUSE) {
            readflage = true;
        } else {
            readflage = false;
        }
    }
    publishEvent(CviPlayerEvent {.type = type});
}

void Player::toggleStreamsPause()
{
    if (isPaused() && video_stream) {
        Clock &video_clock = video_stream->getClock();
        control_context.frame_timer += av_gettime_relative() / 1000000.0 - video_clock.getLastUpdated();
        video_clock.syncTime();
    }
    extern_clock->syncTime();
    control_context.paused = !control_context.paused;
    extern_clock->setPaused(control_context.paused);
    if (audio_stream) {
        audio_stream->setPaused(control_context.paused);
    }
    if (video_stream) {
        video_stream->setPaused(control_context.paused);
    }
}

void Player::startReadThread()
{
    read_thread = thread(&Player::read, this);
}

void Player::startRefreshThread()
{
    refresh_thread.start([this]() {
        if (!isPaused()) {
            if ((sync_context->getType() == AV_SYNC_TYPE::EXTERNAL_CLOCK) &&
                (control_context.realtime)) {
                syncExternalClockSpeed();
            }

            if (log_status) {
                logStatus();
            }
        }
    }, std::chrono::milliseconds(REFRESH_INTERVAL_MS));
}

void Player::startProgressThread()
{
    progress_thread.start([this]() {
            if (!control_context.finished) {
                double clock_time = sync_context->getClockPts();
                double difference_time = 0;
                if (clock_time < lasttimepts) {
                    difference_time = (lasttimepts - clock_time);
                } else{
                    difference_time = (clock_time - lasttimepts);
                }
                if (difference_time >= 1) {
                    publishEvent(CviPlayerEvent {
                        .type = CVI_PLAYER_EVENT_PLAY_PROGRESS
                    });
                    lasttimepts = clock_time;
                }
            }
    },std::chrono::milliseconds(PLAY_PROGRESS_INTERVAL_MS));
}

void Player::seekStreamByTime(int64_t time, const int64_t offset)
{
    seekpause = false;
    CviDemuxerMediaInfo info;
    if (!isRunning()) {
        return;
    }

    getMediaInfo(info);
    int64_t durationtime = info.duration_sec * 1000000;
    if (time >= durationtime - 1000000) {
        time = (durationtime - 1000000);
    }

    if (false == sendSeekRequest(time, offset)) {
        CVI_LOGE("sendSeekRequest make_unique is a null pointer");
        return;
    }
}

void Player::seekPauseStreamByTime(int32_t flage, int64_t time, const int64_t offset)
{
    seektimetime = time;
    seekpause = flage;
    CviDemuxerMediaInfo info;
    if (!isRunning()) {
        return;
    }

    getMediaInfo(info);
    int64_t durationtime = info.duration_sec * 1000000;
    if (time >= (durationtime - 1000000)) {
        time = (durationtime - 1000000);
    }

    if (false == sendSeekRequest(time, offset)) {
        CVI_LOGE("sendSeekRequest make_unique is a null pointer");
        return;
    }
}

void Player::seekStreamByByte(const int64_t pos, const int64_t offset)
{
    if (!isRunning()) {
        return;
    }

    sendSeekRequest(pos, offset, AVSEEK_FLAG_BYTE);
}

bool Player::sendSeekRequest(int64_t pos, int64_t offset, int32_t flags)
{
    bool result = true;
    for (int32_t i = 0; i < 3; i++) {
        seek_request = std::make_unique<SeekRequest>();
        if (nullptr != seek_request) {
            result = true;
            break;
        }
        result = false;
        CVI_LOGE("seek request Is a null pointer, reassignment required");
    }

    if (false == result) {
        return result;
    }

    seek_request->pos = pos;
    seek_request->offset = offset;
    seek_request->flags = flags;
    continue_read_cv->notify_one();
    return result;
}

void Player::setMediaOutputHandler(int32_t media_type, OutputHandler handler)
{
    output_handler_map[media_type] = std::move(handler);
    updateStreamOutputHandler(media_type);
}

void Player::updateStreamOutputHandler(int32_t media_type)
{
    if (output_handler_map.end() == output_handler_map.find(media_type)) {
        return;
    }

    if ((media_type == AVMEDIA_TYPE_AUDIO) && audio_stream) {
        audio_stream->setOutputHandler(output_handler_map[media_type]);
    } else if ((media_type == AVMEDIA_TYPE_VIDEO) && video_stream) {
        video_stream->setOutputHandler(output_handler_map[media_type]);
    }
}

void Player::setMediaDecodeHandler(int32_t media_type, AvDecodeHandler handler)
{
    decode_handler_map[media_type] = std::move(handler);
    updateStreamDecodeHandler(media_type);
}

void Player::updateStreamDecodeHandler(int32_t media_type)
{
    if (decode_handler_map.end() == decode_handler_map.find(media_type)) {
        return;
    }

    if ((media_type == AVMEDIA_TYPE_AUDIO) && audio_stream) {
        audio_stream->setDecodeHandler(decode_handler_map[media_type]);
    } else if ((media_type == AVMEDIA_TYPE_VIDEO) && video_stream) {
        video_stream->setDecodeHandler(decode_handler_map[media_type]);
    }
}

void Player::setSpeed(double speed)
{
    if (video_stream) {
        video_stream->setClockSpeed(speed);
    }
    if (audio_stream) {
        audio_stream->setClockSpeed(speed);
    }
    extern_clock->setSpeed(speed);
}

void Player::setAudioParameters(const CviPlayerAudioParameters& parameter)
{
    audio_parameters = parameter;
}

void Player::setVideoParameters(const CviPlayerVideoParameters& parameter)
{
    video_parameters = parameter;
}

CVI_ERROR Player::openDemuxer()
{
    int32_t ret = 0;
    ret = demuxer->openTs();
    if (ret != 0) {
        CVI_LOGE("Demuxer open failed");
        publishEvent(CviPlayerEvent {
            .type = CVI_PLAYER_EVENT_OPEN_FAILED
        });

        if (1 == ret) {
            return CVI_ERROR::NO_MEMORY;
        }
        return CVI_ERROR::FAILURE;
    }

    return CVI_ERROR::NONE;
}

CVI_ERROR Player::getMediaInfo(CviDemuxerMediaInfo &info)
{
    CVI_ERROR error = CVI_ERROR::NONE;
    // open demuxer if not opened, and need close latter
    bool need_close_input = false;
    if (!demuxer->isOpened()) {
        if (openDemuxer() != CVI_ERROR::NONE) {
            return CVI_ERROR::FAILURE;
        }
        need_close_input = true;
    }

    if (demuxer->getMediaInfo(info) != 0) {
        CVI_LOGE("Demuxer get media info failed");
        error = CVI_ERROR::FAILURE;
    }

    if (need_close_input) {
        demuxer->close();
    }

    return error;
}

CVI_ERROR Player::getPlayInfo(CviPlayerPlayInfo &info) const
{
    double clock_time = sync_context->getClockPts();
    // if sync clock is abnormal, use video clock instead
    if (((clock_time == 0) || std::isnan(clock_time)) &&
        video_stream) {
        clock_time = video_stream->getClockPts();
    }
    clock_time = std::isnan(clock_time) ? 0 : clock_time;
    info.duration_sec = clock_time;

    return CVI_ERROR::NONE;
}

void Player::logStatus()
{
    thread_local int64_t last_time = 0;

    int64_t cur_time = av_gettime_relative();
    if ((last_time == 0) || (cur_time - last_time) >= 30000) {
        int32_t audio_queue_bytes = 0, video_queue_bytes = 0, subtitle_queue_bytes = 0;
        if (audio_stream) {
            audio_queue_bytes = audio_stream->getPacketQueue().getTotalBytes();
        }
        if (video_stream) {
            video_queue_bytes = video_stream->getPacketQueue().getTotalBytes();
        }

        if (0 != (demuxer->getfilenameextern())) {
            if (subtitle_stream) {
                subtitle_queue_bytes = subtitle_stream->getPacketQueue().getTotalBytes();
            }
        }

        double av_diff = 0;
        if (audio_stream && video_stream) {
            av_diff = audio_stream->getClockTime() - video_stream->getClockTime();
        } else if (video_stream) {
            av_diff = sync_context->getClockTime() - video_stream->getClockTime();
        } else if (audio_stream) {
            av_diff = sync_context->getClockTime() - audio_stream->getClockTime();
        }

        AVBPrint buf;
        av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
        if (0 != (demuxer->getfilenameextern())) {
            av_bprintf(&buf,
                    "%7.2f %s:%7.3f aq=%5dKB vq=%5dKB sq=%5dB\r",
                    sync_context->getClockTime(),
                    (audio_stream && video_stream) ? "A-V" :
                    (video_stream ? "M-V" : (audio_stream ? "M-A" : "   ")),
                    av_diff,
                    audio_queue_bytes / 1024,
                    video_queue_bytes / 1024,
                    subtitle_queue_bytes);
        } else {
            av_bprintf(&buf,
                    "%7.2f %s:%7.3f aq=%5dKB vq=%5dKB\r",
                    sync_context->getClockTime(),
                    (audio_stream && video_stream) ? "A-V" :
                    (video_stream ? "M-V" : (audio_stream ? "M-A" : "   ")),
                    av_diff,
                    audio_queue_bytes / 1024,
                    video_queue_bytes / 1024);
        }

        CVI_LOGI("%s", buf.str);
        av_bprint_finalize(&buf, NULL);

        last_time = cur_time;
    }
}

void Player::playseep()
{
    double seek = (double)(1.5 * seep);
    double Intervaltime = video_stream->getRatetime();
    double jumptime = seek * Intervaltime;
    CviDemuxerMediaInfo minfo;
    getMediaInfo(minfo);

    double clock_time = sync_context->getClockPts();

    if (((clock_time == 0) || std::isnan(clock_time)) &&
        video_stream) {
        clock_time = video_stream->getClockPts();
    }

    if (lastetime != clock_time) {
        if (seepeed == 0) {
            if (std::isnan(clock_time)) {
                clock_time = 0;
            }
            timetime = clock_time;
            seepeed++;
        }
        if ((clock_time + jumptime) <= minfo.duration_sec) {
            timetime = (timetime + jumptime);
            if (timetime <= minfo.duration_sec) {
                seekseep((int64_t)((timetime) * 1000000));
            }
        }
        lastetime = clock_time;
    }
}

void Player::playseepbakeoff()
{
    double seek = (double)(1.5 * seep);
    double Intervaltime = video_stream->getRatetime();
    double jumptime = seek * Intervaltime;
    CviDemuxerMediaInfo minfo;
    getMediaInfo(minfo);

    double clock_time = sync_context->getClockPts();

    if (((clock_time == 0) || std::isnan(clock_time)) &&
        video_stream) {
        clock_time = video_stream->getClockPts();
    }

    if (lastetime != clock_time) {
        if (seepeedbake == 0) {
            if (std::isnan(clock_time)) {
                clock_time = 0;
            }
            timetimebake = clock_time;
            seepeedbake++;
        }
        if ((timetimebake - jumptime) >= 0) {
            timetimebake = (timetimebake - jumptime);
        } else {
            timetimebake = 0;
            seekseep((int64_t)((timetimebake) * 1000000));

            publishEvent(CviPlayerEvent {
                .type = CVI_PLAYER_EVENT_PLAY_PROGRESS
            });

/*             if (clock_time == 0) { */
            if (!isPaused()) {
                togglePause();
            }
            progress_thread.pause();
            return;
/*             } */
        }
        if (timetimebake >= 0) {
            seekseep((int64_t)((timetimebake) * 1000000));
        }
        lastetime = clock_time;
    }
}

int32_t Player::forwardbackwardstatus()
{
    return seep;
}

void Player::read()
{
    int32_t AUflage = 0;
    std::mutex wait_mutex;
    int32_t reamin_loop_times = control_context.play_loop_times;
    thread_local AVPacket packet = {};
    lastetime = 0;

    auto needLoop = [](int32_t remain_loop_times) -> bool {
        /* zero for infinite loop */
        if (remain_loop_times == 0) {
            return true;
        }

        return (remain_loop_times > 1);
    };

    while (running) {
        if (isPaused() != control_context.last_paused) {
            handlePause();
        }

        if (seek_request) {
            handleSeekRequest();
            if (true == seekpause) {
                seekflage = true;
                readflage = true;
            }
        }
        if (g_seep == true) {
            if (backforwards == 0) {
                playseepbakeoff();
            } else if(backforwards == 1) {
                playseep();
            }
        }

        if ((attach_request) &&
            (CVI_ERROR::NONE != handleAttachRequest())) {
            CVI_LOGE("handle attach request failed");
            break;
        }

        /* if the queue are full, no need to read more */
        if ((!control_context.realtime) && streamsHasEnoughPackets()) {
        /* wait a while */
            std::unique_lock<std::mutex> lock(wait_mutex);
            continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
            lock.unlock();
            continue;
        }

        if ((!isPaused()) && (control_context.eof) && (!control_context.finished) &&
            (!audio_stream || (audio_stream && audio_stream->isEmpty())) &&
            (!video_stream || (video_stream && video_stream->isEmpty()))) {
            if (needLoop(reamin_loop_times)) {
                seekStreamByTime(control_context.start_time != AV_NOPTS_VALUE ? control_context.start_time : 0);
                if (reamin_loop_times > 1) {
                    reamin_loop_times--;
                }
            } else {
                    publishEvent(CviPlayerEvent {
                        .type = CVI_PLAYER_EVENT_PLAY_PROGRESS
                    });
                    publishEvent(CviPlayerEvent {
                        .type = CVI_PLAYER_EVENT_PLAY_FINISHED
                    });
                control_context.finished = true;
            }
        }

        // read packet from demuxer
        if (true == seekpause) {
            if (readflage == true) {
                if (((audio_stream->getClockPts()) >= (seektimetime/(static_cast<double>(AV_TIME_BASE))))
                    && ((video_stream->getClockPts() >= (seektimetime/(static_cast<double>(AV_TIME_BASE)))))) {
                    if (AUflage == 0) {
                        seekpause = false;
                        seekflage = false;
                        readflage = false;
                        timeflage = false;
                        if (!isPaused()) {
                            togglePause();
                        }
                        progress_thread.pause();
                        AUflage++;
                    }
                    continue;
                } else {
                    if (video_stream->getPacketQueue().getSize() < QUEUEMAX) {
                        if (readPacket(&packet) == CVI_ERROR::NONE) {
                            putPacketToQueue(packet);
                        } else {
                            // if read failed, wait a while
                            std::unique_lock<std::mutex> lock(wait_mutex);
                            continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                        }
                    } else {
                        // if read failed, wait a while
                        std::unique_lock<std::mutex> lock(wait_mutex);
                        continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                    }
                }
            } else {
                if (video_stream->getPacketQueue().getSize() < QUEUEMAX) {
                    if (readPacket(&packet) == CVI_ERROR::NONE) {
                        putPacketToQueue(packet);
                    } else {
                        // if read failed, wait a while
                        std::unique_lock<std::mutex> lock(wait_mutex);
                        continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                    }
                } else {
                    // if read failed, wait a while
                    std::unique_lock<std::mutex> lock(wait_mutex);
                    continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                }
            }
        } else {
            if (video_stream) {

                if (video_stream->getPacketQueue().getSize() < QUEUEMAX) {
                    if (readPacket(&packet) == CVI_ERROR::NONE) {

                        putPacketToQueue(packet);
                    } else {
                        // if read failed, wait a while
                        std::unique_lock<std::mutex> lock(wait_mutex);
                        continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                    }
                } else {
                    // if read failed, wait a while
                    std::unique_lock<std::mutex> lock(wait_mutex);
                    continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                }
            } else {
                if (readPacket(&packet) == CVI_ERROR::NONE) {
                    putPacketToQueue(packet);
                } else {
                    // if read failed, wait a while
                    std::unique_lock<std::mutex> lock(wait_mutex);
                    continue_read_cv->wait_for(lock, std::chrono::milliseconds(10));
                }
            }
        }
    }
}

void Player::playerseep(int64_t seeplog, int32_t speed, int32_t backforward)
{
    seep = speed;
    if (seep == 1) {
        g_seep = false;
        backforwards = 2;
        seepeed = 0;
        seepeedbake = 0;
    } else {
        g_seep = seeplog;
        backforwards = backforward;
    }
}

CVI_ERROR Player::readPacket(AVPacket *packet)
{
    int32_t ret = demuxer->read(packet);
    if (ret < 0) {
        av_packet_unref(packet);
        // put null packet when read finish
        if ((ret == AVERROR_EOF) && (!control_context.eof)) {
            if (video_stream) {
                video_stream->putNullPacket();
            }
            if (audio_stream) {
                audio_stream->putNullPacket();
            }

            if (0 != (demuxer->getfilenameextern())) {
                if (subtitle_stream) {
                    subtitle_stream->putNullPacket();
                }
            }

            control_context.eof = true;
        }

        return CVI_ERROR::FAILURE;
    }

    control_context.eof = false;
    control_context.finished = false;

    return CVI_ERROR::NONE;
}

void Player::putPacketToQueue(AVPacket &packet)
{
    if (0 != (demuxer->getfilenameextern())) {
        if (subtitle_stream && (subtitle_stream->isValidPacket(packet))) {
            subtitle_stream->getPacketQueue().put(&packet);
        }
    }

    if (audio_stream && (audio_stream->isValidPacket(packet))) {
        audio_stream->getPacketQueue().put(&packet);
    } else if (video_stream && (video_stream->isValidPacket(packet)) &&
              (!video_stream->hasAttachedPicture())) {
        video_stream->getPacketQueue().put(&packet);
    } else {
        av_packet_unref(&packet);
    }
}

void Player::handlePause()
{
    control_context.last_paused = control_context.paused;
    if (isPaused()) {
        if (true == seekpause) {
            readflage = true;
            timeflage = true;
        }
        demuxer->pause();
        progress_thread.pause();
    } else {
        if (true == seekpause){
            readflage = false;
            timeflage = false;
        }
        demuxer->resume();
        progress_thread.resume();
    }
}

void Player::seekseep(int64_t target, int64_t offset, int32_t flag)
{
    bool result = true;
    int ret = 0;
    for (int32_t i = 0; i < 3; i++) {
        seek_request = std::make_unique<SeekRequest>();
        if (nullptr != seek_request) {
            result = true;
            break;
        }
        result = false;
        CVI_LOGE("seek request Is a null pointer, reassignment required");
    }

    if (false == result) {
        return;
    }

    seek_request->pos = target;
    seek_request->offset = offset;
    seek_request->flags = flag;

    printf("### seekseep: target:%lld, offset:%lld, flag:%d\n", target, offset, flag);
    ret = demuxer->seek(target, offset, flag);
    if (ret < 0) {
        CVI_LOGE("error while seeking, ret: %d", ret);
    } else {
        if (video_stream) {
            video_stream->packetflush();
        }
        if (audio_stream) {
            audio_stream->flush();
        }

        if (0 != (demuxer->getfilenameextern())) {
            if (subtitle_stream) {
                subtitle_stream->flush();
            }
        }

        if ((seek_request->flags & AVSEEK_FLAG_BYTE) != 0) {
            extern_clock->setClock(NAN, 0);
        } else {
            extern_clock->setClock(seek_request->pos/(static_cast<double>(AV_TIME_BASE)), 0);
        }

        readToNextIdrPacket();
    }

    seek_request.reset();
    attach_request = true;
    control_context.eof = false;
    control_context.finished = false;
    if (isPaused()) {
        toggleStreamsPause();
    }
}
void Player::handleSeekRequest()
{
    int ret = 0;
    printf("### handleSeekRequest: target:%lld, offset:%lld, flag:%d\n", seek_request->pos, seek_request->offset, seek_request->flags);
    ret = demuxer->seek(seek_request->pos, seek_request->offset, seek_request->flags);
    if (ret < 0) {
        CVI_LOGE("error while seeking, ret: %d", ret);
    } else {
        if (video_stream) {
            video_stream->packetflush();
        }
        if (audio_stream) {
            audio_stream->flush();
        }

        if (0 != (demuxer->getfilenameextern())) {
            if (subtitle_stream) {
                subtitle_stream->flush();
            }
        }

        if ((seek_request->flags & AVSEEK_FLAG_BYTE) != 0) {
            extern_clock->setClock(NAN, 0);
        } else {
            extern_clock->setClock(seek_request->pos/(static_cast<double>(AV_TIME_BASE)), 0);
        }

        readToNextIdrPacket();
    }

    seek_request.reset();
    attach_request = true;
    control_context.eof = false;
    control_context.finished = false;
    if (isPaused()) {
        toggleStreamsPause();
    }
}

void Player::readToNextIdrPacket()
{
    if (!video_stream) {
        return;
    }

    constexpr int32_t max_read_size = 5120, max_read_count = 100;
    thread_local AVPacket packet = {};
    // keep read video packet until find idr packet
    for (int32_t read_count = 0; read_count < max_read_count;) {
        if (readPacket(&packet) != CVI_ERROR::NONE) {
            break;
        }
        // check is video packet or not
        if (video_stream->isValidPacket(packet)) {
            bool is_idr = cvi_demuxer::utils::hasNalUnit(cvi_demuxer::utils::getIdrNalType(getVideoCodecId()),
                getVideoCodecId(), packet.data, std::min(packet.size, max_read_size));
            if (is_idr) {
                putPacketToQueue(packet);
                break;
            } else {
                av_packet_unref(&packet);
            }
            read_count++;
        } else {
            av_packet_unref(&packet);
        }
    }
}

CVI_ERROR Player::handleAttachRequest()
{
    if ((video_stream) && (video_stream->hasAttachedPicture())) {
        AVPacket packet;
        if (av_packet_ref(&packet, &video_stream->getAttachedPacket()) < 0) {
            return CVI_ERROR::FAILURE;
        }
        video_stream->getPacketQueue().put(&packet);
        video_stream->putNullPacket();
    }
    attach_request = false;

    return CVI_ERROR::NONE;
}

void Player::syncExternalClockSpeed()
{
    if ((video_stream && (video_stream->getPacketQueue().getSize() <= EXTERNAL_CLOCK_MIN_FRAMES)) ||
        (audio_stream && (audio_stream->getPacketQueue().getSize() <= EXTERNAL_CLOCK_MIN_FRAMES))) {
        extern_clock->setSpeed(FFMAX(EXTERNAL_CLOCK_SPEED_MIN, extern_clock->getSpeed() - EXTERNAL_CLOCK_SPEED_STEP));
    } else if ((video_stream || (video_stream->getPacketQueue().getSize() > EXTERNAL_CLOCK_MAX_FRAMES)) &&
               (audio_stream || (audio_stream->getPacketQueue().getSize() > EXTERNAL_CLOCK_MAX_FRAMES))) {
        extern_clock->setSpeed(FFMIN(EXTERNAL_CLOCK_SPEED_MAX, extern_clock->getSpeed() + EXTERNAL_CLOCK_SPEED_STEP));
    } else {
        double speed = extern_clock->getSpeed();
        if (speed != 1.0) {
           extern_clock->setSpeed(speed + EXTERNAL_CLOCK_SPEED_STEP * (1.0 - speed) / fabs(1.0 - speed));
        }
    }
}

bool Player::streamsHasEnoughPackets()
{
    bool enough = true;
    if (audio_stream) {
        enough &= audio_stream->hasEnoughPackets();
    }
    if (video_stream) {
        enough &= video_stream->hasEnoughPackets();
    }

    if (0 != (demuxer->getfilenameextern())) {
        if (subtitle_stream) {
            enough &= subtitle_stream->hasEnoughPackets();
        }
    }

    return enough;
}

AV_SYNC_TYPE Player::getSuitableSyncTypeByDuration(AV_SYNC_TYPE candidate_type) const
{
    // only type audio and video need check duration
    if (((candidate_type != AV_SYNC_TYPE::AUDIO_MASTER) &&
         (candidate_type != AV_SYNC_TYPE::VIDEO_MASTER)) ||
        (!video_stream || !audio_stream)) {
        return candidate_type;
    }

    constexpr double diff_threshold = 1.0;
    double diff = video_stream->getDuration() - audio_stream->getDuration();
    if (abs(diff) > diff_threshold) {
        // if duration diff large than threshold, choose longer stream type
        if (diff > 0) {
            return AV_SYNC_TYPE::VIDEO_MASTER;
        } else {
            return AV_SYNC_TYPE::AUDIO_MASTER;
        }
    }

    return candidate_type;
}

void Player::findSuitableSyncContext(AV_SYNC_TYPE candidate_type)
{
    candidate_type = getSuitableSyncTypeByDuration(candidate_type);
    if (candidate_type == AV_SYNC_TYPE::VIDEO_MASTER) {
        if (video_stream) {
            sync_context->setType(AV_SYNC_TYPE::VIDEO_MASTER);
            sync_context->setSharedClock(video_stream->getSharedClock());
        } else if (audio_stream) {
            sync_context->setType(AV_SYNC_TYPE::AUDIO_MASTER);
            sync_context->setSharedClock(audio_stream->getSharedClock());
        } else {
            sync_context->setType(AV_SYNC_TYPE::EXTERNAL_CLOCK);
            sync_context->setSharedClock(extern_clock);
        }
    } else if (candidate_type == AV_SYNC_TYPE::AUDIO_MASTER) {
        if (audio_stream) {
            sync_context->setType(AV_SYNC_TYPE::AUDIO_MASTER);
            sync_context->setSharedClock(audio_stream->getSharedClock());
        } else if (video_stream) {
            sync_context->setType(AV_SYNC_TYPE::VIDEO_MASTER);
            sync_context->setSharedClock(video_stream->getSharedClock());
        } else {
            sync_context->setType(AV_SYNC_TYPE::EXTERNAL_CLOCK);
            sync_context->setSharedClock(extern_clock);
        }
    } else {
        sync_context->setType(AV_SYNC_TYPE::EXTERNAL_CLOCK);
        sync_context->setSharedClock(extern_clock);
    }
}

CVI_ERROR Player::getVideoFrame(CviPlayerFrame *frame) const
{
    return video_stream->getFrame(frame);
}

CVI_ERROR Player::getVideoPacket(CviDemuxerPacket *packet) const
{
    if ((packet == nullptr) || (!video_stream)) {
        return CVI_ERROR::NULL_PTR;
    }

    AVPacketNode *node = video_stream->getPacketQueue().getLastPacketNode();
    if (node == nullptr) {
        return CVI_ERROR::NULL_PTR;
    }

    cvi_demuxer::utils::shallowCopyPacket(&node->pkt, packet);

    return CVI_ERROR::NONE;
}


CVI_ERROR Player::getVideoExtraPacket(CviDemuxerPacket *packet) const
{
    if ((packet == nullptr) || (!video_stream)) {
        return CVI_ERROR::NULL_PTR;
    }

    CviDemuxerPacket *extra_packet = video_stream->getExtraPacket();
    if (extra_packet == nullptr) {
        return CVI_ERROR::NULL_PTR;
    }

    cvi_demuxer::utils::shallowCopyPacket(extra_packet, packet);

    return CVI_ERROR::NONE;
}

CVI_ERROR Player::getMediumVideoExtraPacket(CviDemuxerPacket *packet) const
{
    if ((packet == nullptr) || (!video_stream)) {
        return CVI_ERROR::NULL_PTR;
    }

    // AVStream *stream = demuxer->getAvStream(demuxer->getStreamIndex(AVMEDIA_TYPE_VIDEO));
    // AVCodecParameters *codec_parm = stream->codecpar;

    // CviDemuxerPacket *extra_packet = video_stream->getmediumExtraPacket(codec_parm->extradata, codec_parm->extradata_size);
    CviDemuxerPacket *extra_packet = video_stream->getmediumExtraPacket();
    if (extra_packet == nullptr) {
        return CVI_ERROR::NULL_PTR;
    }

    cvi_demuxer::utils::shallowCopyPacket(extra_packet, packet);

    return CVI_ERROR::NONE;
}

void Player::setPlaySubStreamFlag(bool subflag)
{
    demuxer->setSubStreamFlag(subflag);
}

} // namespace cvi_player
