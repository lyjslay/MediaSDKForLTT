#include "thumbnail_extractor.hpp"
#include "cvi_demuxer/ffmpeg_demuxer.hpp"
#include "cvi_demuxer/utils/file.hpp"
#include "cvi_demuxer/utils/packet.hpp"
#include "cvi_log.h"

extern "C"
{
    #include <libavcodec/avcodec.h>
}

namespace cvi_thumbnail_extractor {

using namespace cvi_demuxer;

int32_t ThumbnailExtractor::getThumbnail(const std::string& input, CviDemuxerPacket& thumbnail)
{
    int32_t ret = -1;
    if (utils::isJpegFile(input)) {
        ret = getThumbnailFromJpegFile(input, thumbnail);
    } else {
        ret = getThumbnailFromDemuxer(input, thumbnail);
    }

    return ret;
}

long ThumbnailExtractor::getTick(char *str_time)
{
	struct tm stm;
	int32_t iY, iM, iD, iH, iMin, iS;
	memset(&stm,0,sizeof(stm));

	iY = atoi(str_time);
	iM = atoi(str_time+5);
	iD = atoi(str_time+8);
	iH = atoi(str_time+11);
	iMin = atoi(str_time+14);
	iS = atoi(str_time+17);

	stm.tm_year=iY-1900;
	stm.tm_mon=iM-1;
	stm.tm_mday=iD;
	stm.tm_hour=iH;
	stm.tm_min=iMin;
	stm.tm_sec=iS;

	return mktime(&stm);
}

int32_t ThumbnailExtractor::ReplaceStr(char *sSrc, char const *sMatchStr, char const *sReplaceStr)
{
    int32_t  StringLen;
    char caNewString[128];
    char *FindPos = strstr(sSrc, sMatchStr);

    if((!FindPos) || (!sMatchStr)) {
        return -1;
    }

    while(FindPos) {
        memset(caNewString, 0, sizeof(caNewString));
        StringLen = FindPos - sSrc;
        strncpy(caNewString, sSrc, StringLen);
        strcat(caNewString, sReplaceStr);
        strcat(caNewString, FindPos + strlen(sMatchStr));
        strcpy(sSrc, caNewString);
        FindPos = strstr(sSrc, sMatchStr);
    }

    return 0;
}

int32_t ThumbnailExtractor::getThumbnailFromDemuxer(const std::string& input,
        CviDemuxerPacket& thumbnail)
{
    int32_t ret = 0;
    for(int32_t i = 0; i < 4; i++) {
        thumbnail.errorccode[i] = 0;
    }

    static FFmpegDemuxer demuxer;
    if (demuxer.isOpened()) {
        demuxer.close();
    }
    demuxer.setInput(input);
    ret = demuxer.open();
    if (ret != 0) {
        CVI_LOGE("Demuxer open %s failed", input.c_str());
        for(int32_t i = 0; i < 4; i++) {
           thumbnail.errorccode[i] = -1;
        }
        if (ret == 1) {
            return 1;
        } else {
            return -1;
        }
    }
    // thumbnail is in data stream
    int32_t data_stream_index = demuxer.getStreamIndex(AVMEDIA_TYPE_DATA);
    if (data_stream_index < 0) {
        CVI_LOGE("Can't get media data stream");
        for(int32_t i = 0; i < 4; i++) {
           thumbnail.errorccode[i] = -1;
        }
        return -1;
    }
    // read until get thumbnail packet
    bool got_thumbnail = false;
    AVPacket packet = {};
    while (demuxer.read(&packet) == 0) {
        if (packet.stream_index == data_stream_index) {
            got_thumbnail = true;
            break;
        }
        av_packet_unref(&packet);
    }

    if (got_thumbnail) {
        utils::deepCopyPacket(&packet, &thumbnail);
        av_packet_unref(&packet);
    }

    if (thumbnail.data == nullptr) {
        thumbnail.errorccode[1] = -1;
    }

    if(NULL != demuxer.getcreationtime()) {
        char timevalue[64] = {0};
        memcpy(timevalue, demuxer.getcreationtime(), (strrchr(demuxer.getcreationtime(), '.') - demuxer.getcreationtime()));
        ReplaceStr(timevalue, "T", " ");
        thumbnail.creationtime = getTick(timevalue);
    } else{
        thumbnail.errorccode[2] = -1;
    }

    thumbnail.duration = demuxer.getfileinfotime();
    if (0 == thumbnail.duration) {
        thumbnail.errorccode[3] = -1;
    }

    demuxer.close();

    return got_thumbnail ? 0 : -1;
}

int32_t ThumbnailExtractor::getThumbnailFromJpegFile(const std::string& input,
        CviDemuxerPacket& thumbnail)
{
    FILE *fp = fopen(input.c_str(), "rb");
    if (fp == nullptr) {
        CVI_LOGF("fail to open %s", input.c_str());
        return -1;
    }

    char header[16] = {0};
    // https://en.wikipedia.org/wiki/JPEG_File_Interchange_Format
    unsigned char marker[] = {0xFF, 0xD8, 0xFF, 0xE0}; // SOI & APP0 marker
    unsigned char JFXX_identifier[] = {
        0x4A, 0x46,0x58,0x58, 0x00, // Identifier,
        0x10 // Thumbnail format, 1 means jpeg
    };

    // SOI(2 bytes), marker(2 bytes), size(2 bytes), identifier(5 bytes), format(1 byte)
    fread(header, 12, 1, fp);

    // SOI & APP0 marker check
    if (0 != strncmp(header,(char *)marker, sizeof(marker))) {
        CVI_LOGE("parse jpeg thumbnail fail, no SOI or APP0 marker");
        fclose(fp);
        return -1;
    }

    // identifier check
    if (0 != strncmp(header+6, (char *)JFXX_identifier, sizeof(JFXX_identifier))) {
        CVI_LOGE("parse jpeg thumbnail fail, wrong identifier");
        fclose(fp);
        return -1;
    }

    thumbnail.size = header[5] & 0xFF;
    thumbnail.size |= (header[4] & 0xFF) << 8;
    thumbnail.size -= 8;
    thumbnail.data = new uint8_t[thumbnail.size];

    int32_t size = thumbnail.size;
    int32_t limit = size < 1024 ? size: 1024;
    int32_t offset = 0;
    while (size) {
        size -= limit;
        char buf[1024] = {0};
        fread(buf, limit, 1, fp);
        memcpy(thumbnail.data + offset, buf, limit);
        offset += limit;
        limit = size < limit ? size: 1024;
    }

    fclose(fp);

    return 0;
}

} // namespace cvi_thumbnail_extractor
