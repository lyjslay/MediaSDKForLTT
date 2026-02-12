#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "cvi_upgrade.h"

#ifdef SOFT_VER
#define Soft_ver SOFT_VER
#else
#define Soft_ver "0.0.0.0"
#endif

#ifdef MODEL_NAME
#define Model_name MODEL_NAME
#else
#define Model_name "juntaida"
#endif

void print_help(void)
{
	printf("Please specific PKG path!\n");
}

void eventCb(CVI_UPGRADE_EVENT_S *event)
{
	if (event->eventID == CVI_UPGRADE_EVENT_PROGRESS) {
		CVI_U32 percentage = (CVI_U32)(intptr_t) event->argv;

		printf("Percentage=%d%%\n", percentage);
	}
}

int main(int argc, const char *argv[])
{
	CVI_UPGRADE_DEV_INFO_S tDev;
	int ret = 1;

	// Read from define, or change to read from file-system
	snprintf(tDev.szSoftVersion, CVI_COMM_STR_LEN, "%s", Soft_ver);
	snprintf(tDev.szModel, CVI_COMM_STR_LEN, "%s", Model_name);

	if (argc < 2) {
		print_help();
		return 1;
	}

	CVI_UPGRADE_Init();
	CVI_UPGRADE_RegisterEvent(eventCb);

	if (CVI_UPGRADE_CheckPkg(argv[1], &tDev, false) == CVI_SUCCESS) {
		printf("There is a new package!!\n");
		ret = CVI_UPGRADE_DoUpgrade(argv[1]);
	}

	CVI_UPGRADE_Deinit();
	return ret;
}
