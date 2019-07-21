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
#include "sysroot.h"
#include "config.h"
	
#define LOG(...) \
	do { \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		logg(buffer, strlen(buffer), "ur0:temp/batteryFixer.log", 2); \
} while (0)
	
#define LOG_START(...) \
	do { \
		char buffer[256]; \
		snprintf(buffer, sizeof(buffer), ##__VA_ARGS__); \
		logg(buffer, strlen(buffer), "ur0:temp/batteryFixer.log", 1); \
} while (0)
	
#define WRAP(TEXT, ...) \
	do { \
		LOG((TEXT), __VA_ARGS__); \
		blit_stringf(10, 30+20*entries++, (TEXT), __VA_ARGS__); \
} while (0)
	
#define ALIGN(x, a)	(((x) + ((a) - 1)) & ~((a) - 1))

void _start() __attribute__ ((weak, alias ("module_start")));

static int (* get_soc_rev)() = NULL;
static int *(* sc_call)() = NULL;

static int logg(void *buffer, int length, const char* logloc, int create)
{
	int fd;
	if (create == 0) {
		fd = ksceIoOpen(logloc, SCE_O_WRONLY | SCE_O_APPEND, 6);
	} else if (create == 1) {
		fd = ksceIoOpen(logloc, SCE_O_WRONLY | SCE_O_TRUNC | SCE_O_CREAT, 6);
	} else if (create == 2) {
		fd = ksceIoOpen(logloc, SCE_O_WRONLY | SCE_O_APPEND | SCE_O_CREAT, 6);
	}
	if (fd < 0)
		return 0;

	ksceIoWrite(fd, buffer, length);
	ksceIoClose(fd);
	return 1;
}

typedef struct SceSysconResumeContext {
	unsigned int size;
	unsigned int unk;
	unsigned int buff_vaddr;
	unsigned int resume_func_vaddr;
	unsigned int SCTLR;
	unsigned int ACTLR;
	unsigned int CPACR;
	unsigned int TTBR0;
	unsigned int TTBR1;
	unsigned int TTBCR;
	unsigned int DACR;
	unsigned int PRRR;
	unsigned int NMRR;
	unsigned int VBAR;
	unsigned int CONTEXTIDR;
	unsigned int TPIDRURW;
	unsigned int TPIDRURO;
	unsigned int TPIDRPRW;
	unsigned int unk2[6];
	unsigned long long time;
} SceSysconResumeContext;

extern void resume_function(void);

static unsigned int *get_lvl1_page_table_va(void);
static unsigned int resume_ctx_buff[32];

static int find_paddr(uint32_t paddr, const void *vaddr_start, unsigned int range, void **found_vaddr);
static int alloc_phycont(unsigned int size, unsigned int alignment,  SceUID *uid, void **addr);
static int load_file_phycont(const char *path, SceUID *uid, void **addr, unsigned int *size);

static tai_hook_ref_t SceSyscon_ksceSysconResetDevice_ref;
static SceUID SceSyscon_ksceSysconResetDevice_hook_uid = -1;
static tai_hook_ref_t SceSyscon_ksceSysconSendCommand_ref;
static SceUID SceSyscon_ksceSysconSendCommand_hook_uid = -1;
static SceSysconResumeContext resume_ctx;
static uintptr_t resume_ctx_paddr;

uintptr_t payload_load_paddr;
uintptr_t sysroot_buffer_paddr;

unsigned int payload_size;

void *lvl1_pt_va;

static void setup_payload(void)
{
	memset(&resume_ctx, 0, sizeof(resume_ctx));
	resume_ctx.size = sizeof(resume_ctx);
	resume_ctx.buff_vaddr = (unsigned int )resume_ctx_buff;
	resume_ctx.resume_func_vaddr = (unsigned int)&resume_function;
	asm volatile("mrc p15, 0, %0, c1, c0, 0\n\t" : "=r"(resume_ctx.SCTLR));
	asm volatile("mrc p15, 0, %0, c1, c0, 1\n\t" : "=r"(resume_ctx.ACTLR));
	asm volatile("mrc p15, 0, %0, c1, c0, 2\n\t" : "=r"(resume_ctx.CPACR));
	asm volatile("mrc p15, 0, %0, c2, c0, 0\n\t" : "=r"(resume_ctx.TTBR0));
	asm volatile("mrc p15, 0, %0, c2, c0, 1\n\t" : "=r"(resume_ctx.TTBR1));
	asm volatile("mrc p15, 0, %0, c2, c0, 2\n\t" : "=r"(resume_ctx.TTBCR));
	asm volatile("mrc p15, 0, %0, c3, c0, 0\n\t" : "=r"(resume_ctx.DACR));
	asm volatile("mrc p15, 0, %0, c10, c2, 0\n\t" : "=r"(resume_ctx.PRRR));
	asm volatile("mrc p15, 0, %0, c10, c2, 1\n\t" : "=r"(resume_ctx.NMRR));
	asm volatile("mrc p15, 0, %0, c12, c0, 0\n\t" : "=r"(resume_ctx.VBAR));
	asm volatile("mrc p15, 0, %0, c13, c0, 1\n\t" : "=r"(resume_ctx.CONTEXTIDR));
	asm volatile("mrc p15, 0, %0, c13, c0, 2\n\t" : "=r"(resume_ctx.TPIDRURW));
	asm volatile("mrc p15, 0, %0, c13, c0, 3\n\t" : "=r"(resume_ctx.TPIDRURO));
	asm volatile("mrc p15, 0, %0, c13, c0, 4\n\t" : "=r"(resume_ctx.TPIDRPRW));
	resume_ctx.time = ksceKernelGetSystemTimeWide();
	ksceKernelCpuDcacheAndL2WritebackRange(&resume_ctx, sizeof(resume_ctx));
	lvl1_pt_va = get_lvl1_page_table_va();
}

static int ksceSysconResetDevice_hook_func(int type, int mode)
{
	if (type == SCE_SYSCON_RESET_TYPE_POWEROFF) {
		setup_payload();
		type = SCE_SYSCON_RESET_TYPE_SOFT_RESET;
	}
	return TAI_CONTINUE(int, SceSyscon_ksceSysconResetDevice_ref, type, mode);
}

static int ksceSysconSendCommand_hook_func(int cmd, void *buffer, unsigned int size)
{
	if (cmd == SCE_SYSCON_CMD_RESET_DEVICE && size == 4)
		buffer = &resume_ctx_paddr;
	return TAI_CONTINUE(int, SceSyscon_ksceSysconSendCommand_ref, cmd, buffer, size);
}

// ty xerpi
int r_nos()
{
	LOG("baremetal loader by xerpi\n");
	int ret;
	SceUID payload_uid;
	SceUID sysroot_buffer_uid;
	void *payload_vaddr;
	void *sysroot_buffer_vaddr;
	struct sysroot_buffer *sysroot;
	ret = load_file_phycont(PAYLOAD_PATH, &payload_uid, &payload_vaddr, &payload_size);
	if (ret < 0) {
		LOG("load_file_phycont failed\n");
		return SCE_KERNEL_START_FAILED;
	}
	ksceKernelGetPaddr(payload_vaddr, &payload_load_paddr);
	sysroot = ksceKernelGetSysrootBuffer();
	ret = alloc_phycont(sysroot->size, 4096, &sysroot_buffer_uid, &sysroot_buffer_vaddr);
	if (ret < 0) {
		LOG("alloc_phycont kbl_param failed\n");
		ksceKernelFreeMemBlock(payload_uid);
		return SCE_KERNEL_START_FAILED;
	}
	ksceKernelCpuUnrestrictedMemcpy(sysroot_buffer_vaddr, sysroot, sysroot->size);
	ksceKernelGetPaddr(sysroot_buffer_vaddr, &sysroot_buffer_paddr);
	SceSyscon_ksceSysconResetDevice_hook_uid = taiHookFunctionExportForKernel(KERNEL_PID,
		&SceSyscon_ksceSysconResetDevice_ref, "SceSyscon", 0x60A35F64,
		0x8A95D35C, ksceSysconResetDevice_hook_func);
	SceSyscon_ksceSysconSendCommand_hook_uid = taiHookFunctionExportForKernel(KERNEL_PID,
		&SceSyscon_ksceSysconSendCommand_ref, "SceSyscon", 0x60A35F64,
		0xE26488B9, ksceSysconSendCommand_hook_func);
	ksceKernelGetPaddr(&resume_ctx, &resume_ctx_paddr);
	LOG("initial patches done, req standby\n");
	kscePowerRequestStandby();
	return SCE_KERNEL_START_SUCCESS;
}

unsigned int *get_lvl1_page_table_va(void)
{
	uint32_t ttbcr;
	uint32_t ttbr0;
	uint32_t ttbcr_n;
	uint32_t lvl1_pt_pa;
	void *lvl1_pt_va;
	asm volatile(
		"mrc p15, 0, %0, c2, c0, 2\n\t"
		"mrc p15, 0, %1, c2, c0, 0\n\t"
		: "=r"(ttbcr), "=r"(ttbr0));
	ttbcr_n = ttbcr & 7;
	lvl1_pt_pa = ttbr0 & ~((1 << (14 - ttbcr_n)) - 1);
	if (!find_paddr(lvl1_pt_pa, (void *)0, 0xFFFFFFFF, &lvl1_pt_va))
		return NULL;
	return lvl1_pt_va;
}

int find_paddr(uint32_t paddr, const void *vaddr_start, unsigned int range, void **found_vaddr)
{
	const unsigned int step = 0x1000;
	void *vaddr = (void *)vaddr_start;
	const void *vaddr_end = vaddr_start + range;
	for (; vaddr < vaddr_end; vaddr += step) {
		uintptr_t cur_paddr;
		if (ksceKernelGetPaddr(vaddr, &cur_paddr) < 0)
			continue;
		if ((cur_paddr & ~(step - 1)) == (paddr & ~(step - 1))) {
			if (found_vaddr)
				*found_vaddr = vaddr;
			return 1;
		}
	}
	return 0;
}

int alloc_phycont(unsigned int size, unsigned int alignment, SceUID *uid, void **addr)
{
	int ret;
	SceUID mem_uid;
	void *mem_addr;
	SceKernelAllocMemBlockKernelOpt opt;
	memset(&opt, 0, sizeof(opt));
	opt.size = sizeof(opt);
	opt.attr = SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_PHYCONT | SCE_KERNEL_ALLOC_MEMBLOCK_ATTR_HAS_ALIGNMENT;
	opt.alignment = ALIGN(alignment, 0x1000);
	mem_uid = ksceKernelAllocMemBlock("phycont", 0x30808006, ALIGN(size, 0x1000), &opt);
	if (mem_uid < 0)
		return mem_uid;
	ret = ksceKernelGetMemBlockBase(mem_uid, &mem_addr);
	if (ret < 0) {
		ksceKernelFreeMemBlock(mem_uid);
		return ret;
	}
	if (uid)
		*uid = mem_uid;
	if (addr)
		*addr = mem_addr;
	return 0;
}

int load_file_phycont(const char *path, SceUID *uid, void **addr, unsigned int *size)
{
	int ret;
	SceUID fd;
	SceUID mem_uid;
	void *mem_addr;
	unsigned int file_size;
	unsigned int aligned_size;
	fd = ksceIoOpen(path, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	file_size = ksceIoLseek(fd, 0, SCE_SEEK_END);
	aligned_size = ALIGN(file_size, 4096);
	ret = alloc_phycont(aligned_size, 4096, &mem_uid, &mem_addr);
	if (ret < 0) {
		ksceIoClose(fd);
		return ret;
	}
	ksceIoLseek(fd, 0, SCE_SEEK_SET);
	ksceIoRead(fd, mem_addr, file_size);
	ksceKernelCpuDcacheAndL2WritebackRange(mem_addr, aligned_size);
	ksceIoClose(fd);
	if (uid)
		*uid = mem_uid;
	if (addr)
		*addr = mem_addr;
	if (size)
		*size = file_size;
	return 0;
}

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

static int nfoshow(void) {
	SceKernelAllocMemBlockKernelOpt optp;
	SceDisplayFrameBuf fb;
	void *fb_addr = NULL;
	int uid;
	optp.size = 0x58;
	optp.attr = 2;
	optp.paddr = 0x1C000000;
	fb.size        = sizeof(fb);		
	fb.pitch       = 960;
	fb.pixelformat = 0;
	fb.width       = 960;
	fb.height      = 544;
	uid = ksceKernelAllocMemBlock("SceDisplay", 0x6020D006, 0x200000, &optp);
	ksceKernelGetMemBlockBase(uid, (void**)&fb_addr);
	ksceKernelCpuDcacheAndL2WritebackInvalidateRange(fb_addr, 0x1FE000);
	memset(fb_addr, 0, 0x1FE000);
	fb.base = fb_addr;
	blit_set_frame_buf(&fb);
	int entries = 0;
	int sysroot = ksceKernelGetSysbase();
	int kbl_param = *(unsigned int *)(sysroot + 0x6c);
	char cur_fw[8], min_fw[8];
	firmware_string(cur_fw, *(uint32_t *)(*(int *)(sysroot + 0x6c) + 4));
	firmware_string(min_fw, *(uint32_t *)(kbl_param + 8));
	unsigned int fwinfo, dfinfo;
	unsigned long long int hwinfo;
	ksceSysconGetBatteryVersion(&hwinfo, &fwinfo, &dfinfo);
	int cold = (ksceKernelSysrootGetShellPid() < 0) ? 1 : 0;
	blit_set_color(0x00FFFF00, 0xFF000000);
	WRAP("\n %s info:", "Console");
	blit_set_color(0x00FFFFFF, 0xFF000000);
	if (get_soc_rev != NULL)
		WRAP("\n  SoC rev: %X.%01X", (get_soc_rev() << 0xf) >> 0x13, get_soc_rev() & 0xf);
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
	if ((unsigned int)hwinfo < 0xFF00) {
		blit_set_color(0x0000FF00, 0xFF000000);
		WRAP("\n CHKOK: %s", "Resetting the battery chip in 5s...");
		WRAP("\n CHKOK: %s", "After the vita shuts down wait 10 seconds");
		WRAP("\n CHKOK: %s", "and then turn it back on.");
	} else {
		blit_set_color(0x000000FF, 0xFF000000);
		WRAP("\n CHKERR: %s", "Unsupported battery controller rev.");
		WRAP("\n CHKERR: %s", "Hold ( PWR + PS + LT + SELECT + START )");
		WRAP("\n CHKERR: %s", "for 20s to force syscon hard reset.");
		WRAP("\n CHKERR: %s", "It may (or not) help.");
		WRAP("\n CHKERR: %s", "You can do it at any time.");
		WRAP("\n CHKERR: %s", (cold == 1) ? "Exiting in 5s..." : "Press [START] to exit.");
	}
	ksceDisplaySetFrameBuf(&fb, 1);
	ksceKernelDelayThread(500);
	int i = 16;
	while (i > 0) {
		ksceKernelCpuDcacheAndL2WritebackInvalidateRange(fb_addr, 0x1FE000);
		ksceDisplayWaitVblankStart();
		ksceDisplayWaitVblankStart();
		ksceKernelDelayThread(10);
		ksceDisplayWaitVblankStart();
		i--;
	}
	if (cold == 1 || (unsigned int)hwinfo < 0xFF00) ksceKernelDelayThread(5*1000*1000);
	if ((unsigned int)hwinfo < 0xFF00) { // Chip probably supports the reset cmd
		LOG("\nChip probably supports the reset cmd\n");
		if (cold == 1) { // Before shell, try chip hard reset via syscon
			LOG("Before shell, try chip hard reset via syscon\n");
			sc_call(0, 0x888, 2);
			sc_call(0, 0x989, 1);
		} else { // After shell, resume to no-os env and force chip kill
			LOG("After shell, resume to no-os env and force chip kill\n");
			r_nos();
		}
	} else {
		LOG("\nChip probably does not support the reset cmd\n");
	}
	ksceKernelFreeMemBlock(uid);
	return 1;
}

int module_start(SceSize argc, const void *args){
	LOG_START("batteryFixer started!\n");
	tai_module_info_t info;
	if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSyscon", &info) >= 0)
		module_get_offset(KERNEL_PID, info.modid, 0, 0x30C9, (uintptr_t *)&sc_call);
	if (taiGetModuleInfoForKernel(KERNEL_PID, "SceLowio", &info) >= 0)
		module_get_offset(KERNEL_PID, info.modid, 0, 0x3A5, (uintptr_t *)&get_soc_rev);
	
	LOG("starting main thread\n");
	
	nfoshow();
	
	LOG("all done\n");

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args){
	return SCE_KERNEL_STOP_CANCEL;
}
