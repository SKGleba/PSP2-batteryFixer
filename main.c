#include <stdio.h>
#include <string.h>
#include <taihen.h>
#include <psp2kern/kernel/modulemgr.h>

static int *(* sc_ctrl_power)() = NULL;
static int *(* sc_kill_bic)(void) = NULL;

static int get_sc_f(void) {
	tai_module_info_t info;
	info.size = sizeof(tai_module_info_t);
	if (taiGetModuleInfoForKernel(KERNEL_PID, "SceSyscon", &info) < 0)
		return -1;
	module_get_offset(KERNEL_PID, info.modid, 0, 0x30C9, (uintptr_t *)&sc_ctrl_power);
	module_get_offset(KERNEL_PID, info.modid, 0, 0x68A5, (uintptr_t *)&sc_kill_bic);
	return 0;
}

static void ledtest(no, cnt, ts) // cuz y not?
{
	while (cnt > 0) {
		sc_ctrl_power(no | 0x80, 0x891, 2);
		if (ts > 0) ksceKernelDelayThread(ts*1000*1000);
		sc_ctrl_power(no, 0x891, 2);
		cnt--;
	}
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args)
{
	if (get_sc_f() < 0)
		return SCE_KERNEL_START_NO_RESIDENT;
	ledtest(0x40, 2, 0.25);
	if (ksceIoRemove("ur0:tai/bicr.skprx") != 0 && ksceIoRemove("ux0:tai/bicr.skprx") != 0)
		return SCE_KERNEL_START_NO_RESIDENT;
	ledtest(0x40, 2, 0.25);
	sc_ctrl_power(1, 0x888, 2);
	ledtest(0x40, 3, 0.14);
	sc_kill_bic();
	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args)
{
	return SCE_KERNEL_STOP_SUCCESS;
}
