#include "cvi_player/cvi_player.h"
#include <algorithm>
#include <type_traits>
#include "cvi_log.h"
#include "cvi_demuxer/utils/check.hpp"
#include "cvi_demuxer/utils/packet.hpp"
#include "cvi_player/decoder/av_decode_handler.hpp"
#include "cvi_player/player/player.hpp"
#include "cvi_player/utils/frame.hpp"

using namespace cvi_player;
int32_t psseekflage = 0;
double seektime = 0;

namespace {

constexpr int32_t MAX_NALU_CHECK_LENGTH = 12;

int32_t setHandler(CVI_PLAYER_HANDLE_T handle, AVMediaType media_type,
        const CVI_PLAYER_OUTPUT_HANDLER &handler)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setMediaOutputHandler(media_type, handler);

    return 0;
}

int32_t setCustomArgHandler(CVI_PLAYER_HANDLE_T handle, AVMediaType media_type,
        const CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER &handler, void *custom_arg)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    auto &&wrapper_handler = [custom_arg, handler] (CviPlayerFrame *frame) {
        handler(custom_arg, frame);
    };
    player->setMediaOutputHandler(media_type, wrapper_handler);

    return 0;
}

template <typename Handler, typename... Args>
std::function<int32_t(AVFrame *av_frame)> createGetFrameFunction(const Handler& handler, Args... args)
{
    return [handler, args...](AVFrame *av_frame) {
        thread_local CviPlayerFrame frame = {};
        utils::shallowCopyFrame(av_frame, &frame);
        int32_t ret = handler(args..., &frame);
        utils::shallowCopyFrame(&frame, av_frame);
        if (0 != ret) {
            ret = AVERROR(EAGAIN);
        }

        return ret;
    };
}

template <typename Handler, typename... Args>
std::function<int32_t(AVPacket *av_packet)> createDecodePacketFunction(const Handler& handler, Args... args)
{
    return [handler, args...](AVPacket *av_packet) {
        thread_local CviDemuxerPacket packet = {};
        cvi_demuxer::utils::shallowCopyPacket(av_packet, &packet);
        int32_t ret = handler(args..., &packet);
        cvi_demuxer::utils::shallowCopyPacket(&packet, av_packet);
        if (-1 == ret) {
            ret = AVERROR_EOF;
        } else if (0 != ret) {
            ret = AVERROR(EAGAIN);
        }

        return ret;
    };
}

int32_t setDecodeHandler(CVI_PLAYER_HANDLE_T handle, AVMediaType media_type,
        const CVI_PLAYER_DECODE_HANDLER_S &handler)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setMediaDecodeHandler(media_type, AvDecodeHandler {
        .get_frame = createGetFrameFunction(handler.get_frame),
        .decode_packet = createDecodePacketFunction(handler.decode_packet)
    });

    return 0;
}

int32_t setCustomArgDecodeHandler(CVI_PLAYER_HANDLE_T handle, AVMediaType media_type,
        CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);

    if (handler.get_frame && handler.decode_packet) {
        player->setMediaDecodeHandler(media_type, AvDecodeHandler {
            .get_frame = createGetFrameFunction(handler.get_frame, custom_arg),
            .decode_packet = createDecodePacketFunction(handler.decode_packet, custom_arg)
        });
    } else {
        player->setMediaDecodeHandler(media_type, AvDecodeHandler {
            .get_frame = NULL,
            .decode_packet = NULL
        });
    }

    // player->setMediaDecodeHandler(media_type, AvDecodeHandler {
    //     .get_frame = createGetFrameFunction(handler.get_frame, custom_arg),
    //     .decode_packet = createDecodePacketFunction(handler.decode_packet, custom_arg)
    // });

    return 0;
}

} // anonymous namespace

int32_t CVI_PLAYER_Init()
{
    avformat_network_init();

    return 0;
}

int32_t CVI_PLAYER_Deinit()
{
    avformat_network_deinit();

    return 0;
}

int32_t CVI_PLAYER_Create(CVI_PLAYER_HANDLE_T *handle)
{
    if (!cvi_demuxer::utils::hasNullptr(*handle)) {
        CVI_LOGE("Player is not null");
        return -1;
    }

    *handle = new Player();

    return 0;
}

int32_t CVI_PLAYER_Destroy(CVI_PLAYER_HANDLE_T *handle)
{
    if (cvi_demuxer::utils::hasNullptr(handle, *handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(*handle);
    delete player;
    *handle = nullptr;

    return 0;
}

int32_t CVI_PLAYER_SetDataSource(CVI_PLAYER_HANDLE_T handle, const char *data_source)
{
    if (cvi_demuxer::utils::hasNullptr(handle, data_source)) {
        CVI_LOGE("Player or source is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setDataSource(data_source);

    return 0;
}

int32_t CVI_PLAYER_GetDataSource(CVI_PLAYER_HANDLE_T handle, char *data_source)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    std::string player_source = player->getDataSource();
    strncpy(data_source, player_source.c_str(), player_source.length());

    return 0;
}

int32_t CVI_PLAYER_LightOpen(CVI_PLAYER_HANDLE_T handle)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    CVI_ERROR ret = player->openDemuxer();
    if (CVI_ERROR::NONE != ret) {
        CVI_LOGE("Player light open failed");
        if (CVI_ERROR::NO_MEMORY == ret) {
            return 1;
        }
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_Play(CVI_PLAYER_HANDLE_T handle)
{
    psseekflage = 0;
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->play()) {
        CVI_LOGE("Player play failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_Stop(CVI_PLAYER_HANDLE_T handle)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->stop()) {
        CVI_LOGE("Player stop failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_Pause(CVI_PLAYER_HANDLE_T handle)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (!player->isPaused()) {
        player->togglePause();
    }

    return 0;
}

int32_t CVI_PLAYER_Resume(CVI_PLAYER_HANDLE_T handle)
{
    psseekflage = 0;
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (player->isPaused()) {
        player->togglePause();
    } else {
        player->seekStreamByTime(0, 0);
    }

    return 0;
}

int32_t CVI_PLAYER_SeekFlage()
{
    return psseekflage;
}

int32_t CVI_PLAYER_SeekTime()
{
    return (seektime / 1000);
}

int32_t CVI_PLAYER_Seek(CVI_PLAYER_HANDLE_T handle, int64_t time_in_ms)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->seekStreamByTime(time_in_ms*1000);

    return 0;
}

int32_t CVI_PLAYER_PlayerSeep(CVI_PLAYER_HANDLE_T handle, int32_t speed, int32_t backforward)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("CVI_PLAYERplayerseep Player is null");
        return -1;
    }
    Player *player = static_cast<Player *>(handle);
    if(backforward == 0 || backforward == 1) {
        player->playerseep(1, speed, backforward);
    } else{
        CVI_LOGE("CVI_PLAYERplayerseep backforward fail");
        return -1;
    }
    return 0;
}

int32_t CVI_PLAYER_GetForWardBackWardStatus(CVI_PLAYER_HANDLE_T handle)
{
    int64_t backstatus;
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("CVI_PLAYERplayerseep Player is null");
        return -1;
    }
    Player *player = static_cast<Player *>(handle);
    backstatus = player->forwardbackwardstatus();
    return backstatus;
}

int32_t CVI_PLAYER_SeekPause(CVI_PLAYER_HANDLE_T handle, int64_t time_in_ms)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    int32_t flage = 1;
    Player *player = static_cast<Player *>(handle);
    player->seekPauseStreamByTime(flage, time_in_ms*1000);
    seektime = (double)time_in_ms;
    psseekflage = 1;

    return 0;
}

int32_t CVI_PLAYER_TPlay(CVI_PLAYER_HANDLE_T handle, double speed)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setSpeed(speed);

    return 0;
}

int32_t CVI_PLAYER_SetAudioParameters(CVI_PLAYER_HANDLE_T handle,
    CVI_PLAYER_AUDIO_PARAMETERS parameters)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setAudioParameters(parameters);

    return 0;
}

int32_t CVI_PLAYER_SetVideoParameters(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_VIDEO_PARAMETERS parameters)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setVideoParameters(parameters);

    return 0;
}

int32_t CVI_PLAYER_SetAOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_OUTPUT_HANDLER handler)
{
    return setHandler(handle, AVMEDIA_TYPE_AUDIO, handler);
}

int32_t CVI_PLAYER_SetCustomArgAOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg)
{
    return setCustomArgHandler(handle, AVMEDIA_TYPE_AUDIO, handler, custom_arg);
}

int32_t CVI_PLAYER_SetVOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_OUTPUT_HANDLER handler)
{
    return setHandler(handle, AVMEDIA_TYPE_VIDEO, handler);
}

int32_t CVI_PLAYER_SetCustomArgVOHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_OUTPUT_HANDLER handler, void *custom_arg)
{
    return setCustomArgHandler(handle, AVMEDIA_TYPE_VIDEO, handler, custom_arg);
}

int32_t CVI_PLAYER_SetEventHandler(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_EVENT_HANDLER handler)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    player->setEventHandler(handler);

    return 0;
}

int32_t CVI_PLAYER_SetCustomArgEventHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_EVENT_HANDLER handler, void *custom_arg)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    auto &&wrapper_handler = [custom_arg, handler] (CVI_PLAYER_EVENT_S *event) {
        handler(custom_arg, event);
    };
    player->setEventHandler(wrapper_handler);

    return 0;
}

int32_t CVI_PLAYER_SaveImage(CVI_PLAYER_HANDLE_T handle, const char *file_path)
{
    if (cvi_demuxer::utils::hasNullptr(handle, file_path)) {
        CVI_LOGE("Player or file path is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    CVI_PLAYER_FRAME_S video_frame;
    if (CVI_ERROR::NONE != player->getVideoFrame(&video_frame)) {
        CVI_LOGE("Get video frame failed");
        return -1;
    }

    FILE *image_file;
    image_file = fopen(file_path, "wb");
    if (cvi_demuxer::utils::hasNullptr(image_file)) {
        CVI_LOGE("Open %s failed", file_path);
        return -1;
    }

    uint32_t pitch_y = video_frame.linesize[0];
    uint32_t pitch_u = video_frame.linesize[1];
    uint32_t pitch_v = video_frame.linesize[2];

    uint8_t *av_y = video_frame.data[0];
    uint8_t *av_u = video_frame.data[1];
    uint8_t *av_v = video_frame.data[2];

    for (int32_t i = 0; i < video_frame.height; ++i) {
        fwrite(av_y, video_frame.width, 1, image_file);
        av_y += pitch_y;
    }

    for (int32_t i = 0; i < video_frame.height/2; ++i) {
        fwrite(av_u, video_frame.width/2, 1, image_file);
        av_u += pitch_u;
    }

    for (int32_t i = 0; i < video_frame.height/2; ++i) {
        fwrite(av_v, video_frame.width/2, 1, image_file);
        av_v += pitch_v;
    }

    fclose(image_file);

    return 0;
}

int32_t CVI_PLAYER_GetMediaInfo(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_MEDIA_INFO_S *info)
{
    if (cvi_demuxer::utils::hasNullptr(handle, info)) {
        CVI_LOGE("Player or info is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getMediaInfo(*info)) {
        CVI_LOGE("Get media info failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_GetPlayInfo(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PLAY_INFO *info)
{
    if (cvi_demuxer::utils::hasNullptr(handle, info)) {
        CVI_LOGE("Player or info is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getPlayInfo(*info)) {
        CVI_LOGE("Get play info failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_GetVideoFrame(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_FRAME_S *frame)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getVideoFrame(frame)) {
        CVI_LOGE("Get video frame failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_GetVideoPacket(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getVideoPacket(packet)) {
        CVI_LOGE("Get video packet failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_GetVideoExtraPacket(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getVideoExtraPacket(packet)) {
        CVI_LOGE("Get video extra packet failed");
        return -1;
    }

    return 0;
}

int32_t CVI_PLAYER_SetVideoDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_DECODE_HANDLER_S handler)
{
    return setDecodeHandler(handle, AVMEDIA_TYPE_VIDEO, handler);
}

int32_t CVI_PLAYER_SetVideoCustomArgDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg)
{
    return setCustomArgDecodeHandler(handle, AVMEDIA_TYPE_VIDEO, handler, custom_arg);
}

int32_t CVI_PLAYER_SetAudioDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_DECODE_HANDLER_S handler)
{
    return setDecodeHandler(handle, AVMEDIA_TYPE_AUDIO, handler);
}

int32_t CVI_PLAYER_SetAudioCustomArgDecodeHandler(CVI_PLAYER_HANDLE_T handle,
        CVI_PLAYER_CUSTOM_ARG_DECODE_HANDLER_S handler, void *custom_arg)
{
    return setCustomArgDecodeHandler(handle, AVMEDIA_TYPE_AUDIO, handler, custom_arg);
}

bool CVI_PLAYER_PacketContainSps(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet)
{
    if (cvi_demuxer::utils::hasNullptr(handle, packet)) {
        CVI_LOGE("Player or packet is null");
        return false;
    }

    Player *player = static_cast<Player *>(handle);
    AVCodecID codec_id = player->getVideoCodecId();
    return cvi_demuxer::utils::hasNalUnit(cvi_demuxer::utils::getSpsNalType(codec_id), codec_id,
        packet->data, std::min(packet->size, MAX_NALU_CHECK_LENGTH));
}

int32_t CVI_PLAYER_GetMediumVideoExtraPacket(CVI_PLAYER_HANDLE_T handle, CVI_PLAYER_PACKET_S *packet)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return -1;
    }

    Player *player = static_cast<Player *>(handle);
    if (CVI_ERROR::NONE != player->getMediumVideoExtraPacket(packet)) {
        CVI_LOGE("Get video extra packet failed");
        return -1;
    }

    return 0;
}

void CVI_PLAYER_SetPlaySubStreamFlag(CVI_PLAYER_HANDLE_T handle, bool subflag)
{
    if (cvi_demuxer::utils::hasNullptr(handle)) {
        CVI_LOGE("Player is null");
        return;
    }

    Player *player = static_cast<Player *>(handle);
    player->setPlaySubStreamFlag(subflag);
}