/*
 * Simple kplugin loader by xerpi
 */

#include <stdio.h>
#include <taihen.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include "debugScreen.h"

#define printf(...) psvDebugScreenPrintf(__VA_ARGS__)

#define MOD_PATH "ux0:app/SKGB4TF1X/bicr.skprx"

static void wait_key_press();

int main(int argc, char *argv[])
{
	int ret;
	SceUID mod_id;

	psvDebugScreenInit();

	printf("Plugin Loader by xerpi\n");

	tai_module_args_t argg;
	argg.size = sizeof(argg);
	argg.pid = KERNEL_PID;
	argg.args = 0;
	argg.argp = NULL;
	argg.flags = 0;
	mod_id = taiLoadStartKernelModuleForUser(MOD_PATH, &argg);

	if (mod_id < 0)
		printf("Error loading " MOD_PATH ": 0x%08X\n", mod_id);
	else
		printf("Module loaded with ID: 0x%08X\n", mod_id);

	wait_key_press();

	if (mod_id >= 0) {
		tai_module_args_t argg;
		argg.size = sizeof(argg);
		argg.pid = KERNEL_PID;
		argg.args = 0;
		argg.argp = NULL;
		argg.flags = 0;
		ret = taiStopUnloadKernelModuleForUser(mod_id, &argg, NULL, NULL);
		printf("Stop unload module: 0x%08X\n", ret);
	}

	return 0;
}

void wait_key_press()
{
	SceCtrlData pad;

	printf("Press START to continue.\n");

	while (1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & SCE_CTRL_START)
			break;
		sceKernelDelayThread(200 * 1000);
	}
}
