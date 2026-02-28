#include "file_recover.hpp"
#include "container_factory.hpp"
#include "cvi_demuxer/utils/file.hpp"
#include "cvi_log.h"
#include <stdio.h>
#include <unistd.h>
#include <cstring>

namespace cvi_file_recover {

using std::string;
using std::thread;
#define TYPE_MP4 (1)
#define TYPE_MOV (0)
static bool SetPreallocFlage = true;
static int32_t file_type = TYPE_MOV;

static void syncFile(const string &file_path) {
    FILE* file = fopen(file_path.c_str(),"rb+");

    if (file) {
        int32_t fn = fileno(file);
        if(fn != -1) {
            fsync(fn);
        }

        fclose(file);
    }
}

FileRecover::~FileRecover()
{
    join();
    close();
}

int32_t FileRecover::open(const string &input_file_path)
{
    file_path = input_file_path;
    string file_extension = cvi_demuxer::utils::getFileExtensionName(input_file_path);
    file = ContainerFactory::createContainer(file_extension);
    if (!file) {
        CVI_LOGE("Unsupport file format");
        return -1;
    }

    char s_file[64];
    strcpy(s_file, file_path.c_str());
    FILE *fp = fopen(s_file, "rb");
    if (NULL == fp) {
        CVI_LOGE("file open fail, file is (%s)", s_file);
        return 1;
    }

    unsigned char read_temp_buf[48] = {0};
    fseek(fp, 0, SEEK_SET);
    fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
    fclose(fp);
    if ((read_temp_buf[32] == 0X6D) && (read_temp_buf[33] == 0X64) && (read_temp_buf[34] == 0X61) && (read_temp_buf[35] == 0X74)) {
        file_type = TYPE_MOV;
    } else if ((read_temp_buf[44] == 0X6D) && (read_temp_buf[45] == 0X64) && (read_temp_buf[46] == 0X61) && (read_temp_buf[47] == 0X74)) {
        file_type = TYPE_MP4;
    } else {
        CVI_LOGE("File no mdat");
        publishEvent(CviFileRecoverEvent {
            .type = CVI_FILE_RECOVER_EVENT_OPEN_FAILED
        });

        file.reset();
        return -1;
    }

    if (file->open(input_file_path) != 0) {
        CVI_LOGE("File open failed");
        publishEvent(CviFileRecoverEvent {
            .type = CVI_FILE_RECOVER_EVENT_OPEN_FAILED
        });
        file.reset();
        return -1;
    }/*  else {
        char s_file[64];
        strcpy(s_file, file_path.c_str());
        FILE *fp = fopen(s_file, "rb");
        if (NULL == fp) {
            CVI_LOGE("file open fail, file is (%s)", s_file);
            return 1;
        }

        unsigned char read_temp_buf[48] = {0};
        fseek(fp, 0, SEEK_SET);
        fread(read_temp_buf, sizeof(read_temp_buf), 1, fp);
        fclose(fp);
        if ((read_temp_buf[32] == 0X6D) && (read_temp_buf[33] == 0X64) && (read_temp_buf[34] == 0X61) && (read_temp_buf[35] == 0X74)) {
            file_type = TYPE_MOV;
        } else if ((read_temp_buf[44] == 0X6D) && (read_temp_buf[45] == 0X64) && (read_temp_buf[46] == 0X61) && (read_temp_buf[47] == 0X74)) {
            file_type = TYPE_MP4;
        } else {
            CVI_LOGE("File no mdat");
            publishEvent(CviFileRecoverEvent {
                .type = CVI_FILE_RECOVER_EVENT_OPEN_FAILED
            });

            file->close();
            file.reset();
            return -1;
        }
    } */
    // set event handler to handle file container's event
    file->setEventHandler([this](const CviFileRecoverEvent& event) {
        publishEvent(event);
    });

    return 0;
}

void FileRecover::join()
{
    if (recover_thread.joinable()) {
        recover_thread.join();
    }
}

void FileRecover::close()
{
    if (file) {
        file->close();
        syncFile(file_path);
    }
}

int32_t FileRecover::check() const
{
    if (!file) {
        CVI_LOGE("File is not opened");
        return -1;
    }

    return file->check();
}

int32_t FileRecover::dump() const
{
    if (!file) {
        CVI_LOGE("File is not opened");
        return -1;
    }

    file->dump();

    return 0;
}

void FileRecover::preallocatestate(bool PreallocFlage)
{
    SetPreallocFlage = PreallocFlage;
}

int32_t FileRecover::recover(const string &output_file_path, const string &device_model, bool has_create_time)
{
    if (!file) {
        CVI_LOGE("File is not opened");
        return -1;
    }
    if (is_recovering) {
        CVI_LOGE("Recover is under progress");
        return -1;
    }
    // start file recover
    is_recovering = true;
    publishEvent(CviFileRecoverEvent {
        .type = CVI_FILE_RECOVER_EVENT_RECOVER_START
    });
    CVI_LOGI("Recover input file %s...", file->getFilePath().c_str());
    if (file->recover(device_model, has_create_time, SetPreallocFlage, file_type) != 0) {
        publishEvent(CviFileRecoverEvent {
            .type = CVI_FILE_RECOVER_EVENT_RECOVER_FAILED
        });
        is_recovering = false;
        return -1;
    }
    file->save(output_file_path);
    CVI_LOGI("Recover done, save output file %s", output_file_path.c_str());
    publishEvent(CviFileRecoverEvent {
        .type = CVI_FILE_RECOVER_EVENT_RECOVER_FINISHED
    });
    is_recovering = false;

    return 0;
}

int32_t FileRecover::recoverAsync(const std::string &output_file_path, const std::string &device_model, bool has_create_time)
{
    if (is_recovering) {
        CVI_LOGE("Recover is under progress");
        return -1;
    }

    recover_thread = thread([output_file_path, device_model, has_create_time, this]() {
        recover(output_file_path, device_model, has_create_time);
    });

    return 0;
}

} // namespace cvi_file_recover
