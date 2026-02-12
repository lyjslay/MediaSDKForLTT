#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cvi_osal.h"
#include "cvi_mem.h"
#include "cvi_log.h"
#include "cvi_sys.h"
#include "cvi_vb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MEM_INFO_S{
    uint64_t phy_addr;
    void *vir_addr;
    struct MEM_INFO_S *next;
}MEM_INFO_T;

typedef struct VbPoolMapping {
    void *vir_addr;
    VB_POOL pool_id;
    size_t size;
    uint64_t u64PhyAddr;
    struct VbPoolMapping *next;
} VbPoolMapping_T;

static VbPoolMapping_T *g_vb_info = NULL;
static pthread_mutex_t vb_mutex = PTHREAD_MUTEX_INITIALIZER;
static MEM_INFO_T *g_mem_info = NULL;
static pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER;

static void meminfo_push(uint64_t phy_addr, void *vir_addr){
    MEM_INFO_T *mem = (MEM_INFO_T *)malloc(sizeof(MEM_INFO_T));
    mem->phy_addr = phy_addr;
    mem->vir_addr = vir_addr;
    mem->next = NULL;
    pthread_mutex_lock(&mem_mutex);
    if(g_mem_info == NULL){
        g_mem_info = mem;
    }else{
        MEM_INFO_T *head = g_mem_info;
        while(head->next){
            head = head->next;
        }
        head->next = mem;
    }
    pthread_mutex_unlock(&mem_mutex);
}

static uint64_t meminfo_pop(void *vir_addr){
    pthread_mutex_lock(&mem_mutex);
    uint64_t phy_addr = 0;
    if(g_mem_info != NULL){
        MEM_INFO_T *mem = NULL;
        if(g_mem_info->vir_addr == vir_addr){
            mem = g_mem_info;
            g_mem_info = mem->next;
        }else{
            MEM_INFO_T *tmp = g_mem_info;
            while(tmp->next){
                if(tmp->next->vir_addr == vir_addr){
                    mem = tmp->next;
                    tmp->next = mem->next;
                    break;
                }
                tmp = tmp->next;
            }
        }
        if(mem){
            phy_addr = mem->phy_addr;
            free(mem);
        }
    }

    pthread_mutex_unlock(&mem_mutex);
    return phy_addr;
}

void *CVI_MEM_Allocate(size_t size, const char *name){
    void *vir_addr = NULL;
    CVI_CAR_U64 phy_addr = 0;
    int32_t ret = CVI_SYS_IonAlloc_Cached(&phy_addr, &vir_addr, name, size);
    if (ret == CVI_SUCCESS) {
        meminfo_push(phy_addr, vir_addr);
        return vir_addr;
    }
    return NULL;
}

void CVI_MEM_Free(void *vir_addr){
    if (!vir_addr) {
        return;
    }
    uint64_t phy_addr = meminfo_pop(vir_addr);
    if(phy_addr > 0){
        int32_t ret = CVI_SYS_IonFree(phy_addr, vir_addr);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_SYS_IonFree failed, error [%d]", ret);
        }
    }
}

static void CVI_MEM_AddVbPoolMapping(void *vir_addr, uint64_t u64PhyAddr, VB_POOL pool_id, size_t size)
{
    VbPoolMapping_T *vb_info = (VbPoolMapping_T *)malloc(sizeof(VbPoolMapping_T));
    vb_info->vir_addr = vir_addr;
    vb_info->u64PhyAddr = u64PhyAddr;
    vb_info->size = size;
    vb_info->pool_id = pool_id;
    vb_info->next = NULL;
    pthread_mutex_lock(&vb_mutex);
    if(g_vb_info == NULL){
        g_vb_info = vb_info;
    }else{
        VbPoolMapping_T *head = g_vb_info;
        while(head->next){
            head = head->next;
        }
        head->next = vb_info;
    }
    pthread_mutex_unlock(&vb_mutex);

    return;
}

static VbPoolMapping_T CVI_MEM_FindVbPoolByMapp(void *vir_addr)
{
    VbPoolMapping_T VbPoolMapping = {0};
    if(g_vb_info != NULL){
        VbPoolMapping_T *vb_info = NULL;
        if(g_vb_info->vir_addr == vir_addr){
            vb_info = g_vb_info;
            g_vb_info = vb_info->next;
        }else{
            VbPoolMapping_T *tmp = g_vb_info;
            while(tmp->next){
                if(tmp->next->vir_addr == vir_addr){
                    vb_info = tmp->next;
                    tmp->next = vb_info->next;
                    break;
                }
                tmp = tmp->next;
            }
        }
        if(vb_info){
            memcpy(&VbPoolMapping, vb_info, sizeof(VbPoolMapping_T));
            CVI_SYS_Munmap(vir_addr, VbPoolMapping.size);
            free(vb_info);
        }
    }

    return VbPoolMapping;
}

void *CVI_MEM_AllocateVb(size_t size)
{
    VB_POOL_CONFIG_S stVbPoolCfg = {0};
    void *vir_addr = NULL;

    stVbPoolCfg.u32BlkSize  = size;
    stVbPoolCfg.u32BlkCnt   = 1;
    stVbPoolCfg.enRemapMode = VB_REMAP_MODE_CACHED;
    VB_BLK vb_blk;
    VB_POOL CreatVbHand = VB_INVALID_POOLID;

    CreatVbHand = CVI_VB_CreatePool(&stVbPoolCfg);

    if (CreatVbHand == VB_INVALID_POOLID) {
        CVI_LOGE("CVI_VB_CreatePool Fail");
        return NULL;
    }

    vb_blk = CVI_VB_GetBlock(CreatVbHand, stVbPoolCfg.u32BlkSize);
    if (vb_blk == VB_INVALID_HANDLE) {
        CVI_LOGE("CVI_VB_GetBlock Fail");
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    uint64_t u64PhyAddr = CVI_VB_Handle2PhysAddr(vb_blk);
    if (u64PhyAddr == 0) {
        CVI_LOGE("CVI_VB_Handle2PhysAddr Fail");
        CVI_VB_ReleaseBlock(vb_blk);
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    vir_addr = CVI_SYS_MmapCache(u64PhyAddr, stVbPoolCfg.u32BlkSize);
    if (vir_addr == NULL) {
        CVI_LOGE("CVI_SYS_Mmap Fail");
        CVI_VB_ReleaseBlock(vb_blk);
        CVI_VB_DestroyPool(CreatVbHand);
        return NULL;
    }

    CVI_MEM_AddVbPoolMapping(vir_addr, u64PhyAddr, CreatVbHand, size);

    return vir_addr;
}

void CVI_MEM_VbFree(void *vir_addr)
{
    if (!vir_addr) {
        return;
    }
    VB_BLK vb_blk;
    CVI_S32 ret = 0;
    pthread_mutex_lock(&vb_mutex);
    VbPoolMapping_T V_PooldMapp = CVI_MEM_FindVbPoolByMapp(vir_addr);
    pthread_mutex_unlock(&vb_mutex);

    if (V_PooldMapp.u64PhyAddr > 0) {
        vb_blk = CVI_VB_PhysAddr2Handle(V_PooldMapp.u64PhyAddr);
        if (vb_blk == VB_INVALID_HANDLE) {
		    CVI_LOGE("CVI_VB_PhysAddr2Handle fail");
            return;
	    }

        ret = CVI_VB_ReleaseBlock(vb_blk);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_VB_ReleaseBlock fail");
            return;
        }

        ret = CVI_VB_DestroyPool(V_PooldMapp.pool_id);
        if (ret != CVI_SUCCESS) {
            CVI_LOGE("CVI_VB_DestroyPool fail");
            return;
        }
    } else {
        CVI_LOGE("Invalid virtual address\n");
    }

    return;
}

#ifdef __cplusplus
}
#endif
