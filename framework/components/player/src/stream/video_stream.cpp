#include "cvi_player/stream/video_stream.hpp"
#include "cvi_player/decoder/video_decoder.hpp"
#include "cvi_log.h"

extern "C"
{
    #include <libavutil/time.h>
    #include <libavutil/intreadwrite.h>
}

namespace cvi_player {

constexpr double AV_SYNC_THRESHOLD_MIN = 0.04;
constexpr double AV_SYNC_THRESHOLD_MAX = 0.5;
constexpr double AV_SYNC_FRAMEDUP_THRESHOLD = 0.5;
constexpr uint8_t PACKET_START_CODE[] = { 0, 0, 0, 1 };
constexpr uint8_t START_CODE_SIZE = sizeof(PACKET_START_CODE);
extern double MediaRate;

namespace {

bool isFrameValid(const CviPlayerFrame &frame) {
    return (frame.width != 0) && (frame.height != 0) && (frame.data != nullptr);
}

} // anonymous namespace

VideoStream::VideoStream(int32_t index, AVStream *stream)
: AvStream(index, stream)
{
    AVCodecContext *avctx = createCodecContext();
    decoder = std::make_unique<VideoDecoder>(avctx);
    clock->setQueueSharedSerial(decoder->getPacketQueue().getSharedSerial());
    dynamic_cast<VideoDecoder*>(decoder.get())->setCodecParameters(stream->codecpar);
    dynamic_cast<VideoDecoder*>(decoder.get())->setTimebase(stream->time_base);
    dynamic_cast<VideoDecoder*>(decoder.get())->setFramerate(stream->r_frame_rate);
}

VideoStream::~VideoStream()
{
    if (extra_packet_buffer != nullptr) {
        free(extra_packet_buffer);
    }
}

bool VideoStream::isValidPacket(const AVPacket& packet) const
{
    return BaseStream::isValidPacket(packet) && (packet.size <= max_packet_size);
}

void VideoStream::setMaxFrameDuration(double duration)
{
    max_frame_duration = duration;
}

void VideoStream::setParameters(const CviPlayerVideoParameters &parameter)
{
    if (parameter.max_packet_size != 0) {
        max_packet_size = parameter.max_packet_size;
    }

    if (decoder) {
        dynamic_cast<VideoDecoder *>(decoder.get())->setOutputSize(
            parameter.output_width, parameter.output_height);
    }
}

/** @def getExtraPacket
 *  @brief get parsed extra data, include sps and pps
 *
 * aligned(8) class AVCDecoderConfigurationRecord {
 *   uint32_t(8) configurationVersion = 1;
 *   uint32_t(8) AVCProfileIndication;
 *   uint32_t(8) profile_compatibility;
 *   uint32_t(8) AVCLevelIndication;
 *   bit(6) reserved = ‘111111’b;
 *   uint32_t(2) lengthSizeMinusOne;
 *   bit(3) reserved = ‘111’b;
 *   uint32_t(5) numOfSequenceParameterSets;
 *   for (i = 0; i < numOfSequenceParameterSets; i++) {
 *       uint32_t(16) sequenceParameterSetLength;
 *       bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
 *   }
 *   uint32_t(8) numOfPictureParameterSets;
 *   for (i = 0; i < numOfPictureParameterSets; i++) {
 *       uint32_t(16) pictureParameterSetLength;
 *       bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
 *   }
 * }
 */
CviDemuxerPacket *VideoStream::getExtraPacket()
{
    if ((!decoder) || (extra_packet)) {
        return extra_packet.get();
    }

    uint64_t total_size = 0;
    int32_t extra_data_size = 0;
    int32_t current_pos = 0;
    uint8_t *extradata = dynamic_cast<VideoDecoder *>(decoder.get())->getExtraData(extra_data_size);

    if (extra_packet_buffer != nullptr) {
        free(extra_packet_buffer);
        extra_packet_buffer = nullptr;
    }
    // start parse extra data
    current_pos += 4;
    if (current_pos > extra_data_size) {
        return extra_packet.get();
    }
    int32_t length_size = (*(extradata + current_pos) & 0x3) + 1;
    CVI_LOGI("Extra packet length size:%d", length_size);
    ++current_pos;
    // parse sps and pps
    constexpr int32_t number_of_unit = 2;
    for (int32_t unit_index = 0; unit_index < number_of_unit; ++unit_index) {
        int32_t number_of_part_unit = *(extradata + current_pos);
        if (unit_index == 0) {
            // first unit is sps, and it save number in 5 bits
            number_of_part_unit &= 0x1f;
        } else {
            // secode unit is pps, and save number in 8 bits
            number_of_part_unit &= 0xff;
        }
        ++current_pos;
        for (int32_t part_unit_index = 0; part_unit_index < number_of_part_unit; ++part_unit_index) {
            uint16_t part_unit_size = AV_RB16(extradata + current_pos);
            if ((current_pos + part_unit_size) > extra_data_size) {
                break;
            }
            uint64_t buffer_unit_size = part_unit_size + START_CODE_SIZE;
            current_pos += 2;
            if (av_reallocp(&extra_packet_buffer, total_size + buffer_unit_size) < 0) {
                CVI_LOGE("Can't alloc buffer");
                break;
            }
            memcpy(extra_packet_buffer + total_size, PACKET_START_CODE, START_CODE_SIZE);
            memcpy(extra_packet_buffer + total_size + START_CODE_SIZE, extradata + current_pos, part_unit_size);
            total_size += buffer_unit_size;
            current_pos += part_unit_size;
        }
    }

    extra_packet = std::make_unique<CviDemuxerPacket>();
    extra_packet->data = extra_packet_buffer;
    extra_packet->size = total_size;

    return extra_packet.get();
}

void VideoStream::refresh(double &remaining_time)
{
    FrameQueue &frame_queue = decoder->getFrameQueue();
    if (frame_queue.getSize() == 0) {
        // nothing to do, no frame in the queue
    } else {
        double last_duration, delay, time;
        Frame *frame, *last_frame;

        last_frame = frame_queue.peekLast();
        frame = frame_queue.peek();
        if (frame->serial != getPacketQueue().getSerial()) {
            frame_queue.next();
            remaining_time = 0;
            return;
        }

        if (last_frame->serial != frame->serial) {
            last_refresh_time = av_gettime_relative() / 1000000.0;
        }

        // compute last duration and delay
        last_duration = getFrameDuration(*last_frame, *frame);
        delay = computeTargetDelay(last_duration);
        time = av_gettime_relative()/1000000.0;
        if (last_frame->serial == frame->serial) {
            if (delay > 0.050000) {
                frame_queue.next();
                write();
                if (!isnan(frame->pts)) {
                    clock->setClock(frame->pts, frame->serial);
                }
                return;
            }
        }

        if (time < (last_refresh_time + delay)) {
            // frame display not finish yet
            remaining_time = FFMIN(last_refresh_time + delay - time, remaining_time);
            write();
            return;
        } else if (time > (last_refresh_time + delay)) {
            remaining_time = 0;
        } else {
            remaining_time = MediaRate;
        }

        last_refresh_time += delay;
        if ((delay > 0) && ((time - last_refresh_time) > MediaRate)) {
            last_refresh_time = time;
        }

        // update clock
        if (!isnan(frame->pts)) {
            clock->setClock(frame->pts, frame->serial);
        }

        // drop late frame
        if (frame_queue.getSize() > 1) {
            SyncContext *sync_context = dynamic_cast<AvDecoder*>(decoder.get())->getSyncContext();
            if (sync_context->getType() != AV_SYNC_TYPE::VIDEO_MASTER) {
                Frame *next_frame = frame_queue.peekNext();
                double duration = getFrameDuration(*frame, *next_frame);
                if (time > (last_refresh_time + duration)) {
                    frame_queue.next();
                    remaining_time = 0;
                    return;
                }
            }
        }

        frame_queue.next();
        write();
    }
}

void VideoStream::write()
{
    if (output_handler) {
        thread_local CviPlayerFrame frame = {};
        if (CVI_ERROR::NONE == getFrame(&frame) && isFrameValid(frame)) {
            output_handler(&frame);
        }
    }
}

double VideoStream::getFrameDuration(const Frame &frame, const Frame &next_frame)
{
    double duration = 0.0;
    if (frame.serial == next_frame.serial) {
        duration = next_frame.pts - frame.pts;
        if (isnan(duration) || duration <= 0 ||
            (duration > this->max_frame_duration)) {
            duration = frame.duration;
        }
    }

    return duration;
}

double VideoStream::computeTargetDelay(double delay)
{
    if (!decoder) {
        return 0;
    }

    SyncContext *sync_context = dynamic_cast<AvDecoder*>(decoder.get())->getSyncContext();
    double sync_threshold, diff = 0;
    /* update delay to follow master synchronisation source */
    if (sync_context->getType() != AV_SYNC_TYPE::VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = getClockTime() - sync_context->getClockTime();
        /* skip or repeat frame. We take into account the
           delay to compute the threshold. I still don't know
           if it is the best guess */
        sync_threshold = FFMAX(MediaRate, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < max_frame_duration) {
            if (diff <= -sync_threshold) {
                delay = FFMAX(0, delay + diff);
            } else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD) {
                delay = delay + diff;
            } else if (diff >= sync_threshold) {
                delay = 2 * delay;
            }
        }
    }

    CVI_LOGD("video: delay=%0.3f A-V=%f", delay, -diff);

    return delay;
}

CviDemuxerPacket *VideoStream:: getmediumExtraPacket()
{
    if ((!decoder) || (extra_packet)) {
        return extra_packet.get();
    }

    uint64_t total_size = 0;
    int extra_data_size = 0;
    int current_pos = 0;

    uint8_t *extradata = dynamic_cast<VideoDecoder *>(decoder.get())->getVideoExtraData(extra_data_size);

    if (extra_packet_buffer != nullptr) {
        av_freep(&extra_packet_buffer);
        extra_packet_buffer = nullptr;
    }
    // start parse extra data
    current_pos += 4;
    if (current_pos > extra_data_size) {
        return extra_packet.get();
    }
    int length_size = (*(extradata + current_pos) & 0x3) + 1;
    CVI_LOGI("Extra packet length size:%d", length_size);
    ++current_pos;
    // parse sps and pps
    constexpr int number_of_unit = 2;
    for (int unit_index = 0; unit_index < number_of_unit; ++unit_index) {
        int number_of_part_unit = *(extradata + current_pos);
        if (unit_index == 0) {
            // first unit is sps, and it save number in 5 bits
            number_of_part_unit &= 0x1f;
        } else {
            // secode unit is pps, and save number in 8 bits
            number_of_part_unit &= 0xff;
        }
        ++current_pos;
        for (int part_unit_index = 0; part_unit_index < number_of_part_unit; ++part_unit_index) {
            uint16_t part_unit_size = AV_RB16(extradata + current_pos);
            if ((current_pos + part_unit_size) > extra_data_size) {
                break;
            }
            uint64_t buffer_unit_size = part_unit_size + START_CODE_SIZE;
            current_pos += 2;
            if (av_reallocp(&extra_packet_buffer, total_size + buffer_unit_size) < 0) {
                CVI_LOGE("Can't alloc buffer");
                break;
            }
            memcpy(extra_packet_buffer + total_size, PACKET_START_CODE, START_CODE_SIZE);
            memcpy(extra_packet_buffer + total_size + START_CODE_SIZE, extradata + current_pos, part_unit_size);
            total_size += buffer_unit_size;
            current_pos += part_unit_size;
        }
    }

    extra_packet = std::make_unique<CviDemuxerPacket>();
    extra_packet->data = extra_packet_buffer;
    extra_packet->size = total_size;

    return extra_packet.get();
}

} // namespace cvi_player
