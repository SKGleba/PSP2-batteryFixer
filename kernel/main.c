/*

	bicr by SKGleba
	All Rights Reserved
	
*/

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/stat.h>
#include <psp2kern/display.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <taihen.h>
#include "blit.h"

#define ERNIE_SHUTDOWN_REBOOT 1

#define printf ksceDebugPrintf
	
#define WRAP(TEXT, ...) \
	do { \
		printf((TEXT), __VA_ARGS__); \
		blit_stringf(10, 30+20*entries++, (TEXT), __VA_ARGS__); \
} while (0)
	
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

void _start() __attribute__ ((weak, alias ("module_start")));

// ty flow
void firmware_string(char string[8], unsigned int version) {
  char a = (version >> 24) & 0xf;
  char b = (version >> 20) & 0xf;
  char c = (version >> 16) & 0xf;
  char d = (version >> 12) & 0xf;
  memset(string, 0, 8);
  string[0] = '0' + a;
  string[1] = '.';
  string[2] = '0' + b;
  string[3] = '0' + c;
  string[4] = '\0';
  if (d) {
    string[4] = '0' + d;
    string[5] = '\0';
  }
}

int nfoshow(void) {
	SceKernelAllocMemBlockKernelOpt optp;
	SceDisplayFrameBuf fb;
	void *fb_addr = NULL;
	optp.size = 0x58;
	optp.attr = 2;
	optp.paddr = 0x1C000000;
	fb.size        = sizeof(fb);		
	fb.pitch       = 960;
	fb.pixelformat = 0;
	fb.width       = 960;
	fb.height      = 544;
	ksceKernelGetMemBlockBase(ksceKernelAllocMemBlock("SceDisplay", 0x6020D006, 0x200000, &optp), (void**)&fb_addr);
	if (!fb_addr)
		return -1;
	ksceKernelCpuDcacheAndL2WritebackInvalidateRange(fb_addr, 0x1FE000);
	memset(fb_addr, 0, 0x1FE000);
	fb.base = fb_addr;
	blit_set_frame_buf(&fb);
	int entries = 0;
	int sysroot = ksceSysrootGetSysroot();
	int kbl_param = *(unsigned int *)(sysroot + 0x6c);
	char cur_fw[8], min_fw[8];
	firmware_string(cur_fw, *(uint32_t *)(*(int *)(sysroot + 0x6c) + 4));
	firmware_string(min_fw, *(uint32_t *)(kbl_param + 8));
	unsigned int fwinfo, dfinfo;
	unsigned long long int hwinfo;
	ksceSysconGetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
	blit_set_color(0x00FFFF00, 0xFF000000);
	WRAP("\n %s info:", "Console");
	blit_set_color(0x00FFFFFF, 0xFF000000);
	WRAP("\n  SoC rev: %X.%01X", (kscePervasiveGetHardwareConfiguration() << 0xf) >> 0x13, kscePervasiveGetHardwareConfiguration() & 0xf);
	WRAP("\n  Cur firmware: %s", cur_fw);
	WRAP("\n  Min firmware: %s", min_fw);
	WRAP("\n  PS TV emulation: %s", (ksceKernelCheckDipsw(152) != 0) ? "On" : "Off");
	WRAP("\n  Secure state bit: 0x%X", ((*(unsigned int *)(sysroot + 0x28) ^ 1) & 1));
	WRAP("\n  Manufacturing mode: %s", ((*(uint32_t *)(kbl_param + 0x6C) & 0x4) != 0) ? "Yes" : "No");
	WRAP("\n  Use QA (blank) PSID: %s", (*(uint32_t *)(kbl_param + 0x4C) == 0) ? "Yes" : "No");
	blit_set_color(0x00FFFF00, 0xFF000000);
	WRAP("\n %s info:", "Battery Controller");
	blit_set_color(0x00FFFFFF, 0xFF000000);
	WRAP("\n  Name: %s", (hwinfo > 7) ? "Abby" : "Bert");
	WRAP("\n  HW: 0x%x", (unsigned int)hwinfo);
	WRAP("\n  FW: 0x%x", fwinfo);
	WRAP("\n  DF: 0x%x", dfinfo);
	WRAP("\n  BOOT: 0x%llx", hwinfo);
	WRAP("\n  ---- %s ---- ", "<>");
	blit_set_color(0x0000FF00, 0xFF000000);
	WRAP("\n READY: %s", "Resetting the battery chip in 10s...");
	WRAP("\n READY: %s", "After the vita shuts down wait 10 seconds");
	WRAP("\n READY: %s", "and then turn it back on.");
	WRAP("\n READY: %s", "It should ask you to set the current time");
	ksceDisplaySetFrameBuf(&fb, 1);
	ksceKernelDelayThread(500);
	int i = 16;
	while (i > 0) { // cleanup artifacts
		ksceKernelCpuDcacheAndL2WritebackInvalidateRange(fb_addr, 0x1FE000);
		ksceDisplayWaitVblankStart();
		ksceDisplayWaitVblankStart();
		ksceKernelDelayThread(10);
		ksceDisplayWaitVblankStart();
		i--;
	}
	ksceKernelDelayThread(9*1000*1000);
	return 0;
}

int abby_reset_nocalib(void) {
	int status = 0;
	
	// end all pending transactions
	for (int i = 0; i < 4; i -= -1) {
		ksceSysconAbbySync(&status);
		if (status << 0x19 < 0)
			break;
	}

	// Fake the post-update reset
	ksceSysconBatterySWReset();

	// Reboot via ernie, it should shut down instead if abby was reset
	// NOTE: race, we need to do that before abby requests calibration data from ernie
	kscePowerRequestErnieShutdown(ERNIE_SHUTDOWN_REBOOT);

	while(1){};
}

int module_start(SceSize argc, const void *args){
	printf("batteryFixer started!\n");

	printf("showing info - legacy leftover\n");
	nfoshow();
	
	printf("resetting abby without calibration\n");
	abby_reset_nocalib();
	
	printf("all done, you shouldnt see this\n");

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args){
	return SCE_KERNEL_STOP_CANCEL;
}
