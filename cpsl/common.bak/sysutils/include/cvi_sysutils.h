#ifndef CVI_INSMOD_H_
#define CVI_INSMOD_H_
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include <stdbool.h>
#include <fcntl.h>
#define CVI_MAX_PATH_LEN (64)

int32_t cvi_insmod(const char *pszPath, const char *pszOptions);
int32_t cvi_rmmod(const char *pszPath);
int32_t cvi_PathIsDirectory(const char *pszPath);
int32_t cvi_rmdir(const char *pszPath);
int32_t cvi_mkdir(const char *pszPath, mode_t mode);
int32_t cvi_system(const char *pszCmd);
int32_t cvi_usleep(uint32_t usec);
int32_t cvi_du(const char *pszPath, uint64_t *pu64Size_KB);
int32_t cvi_async(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CVI_DTCF_H_ */