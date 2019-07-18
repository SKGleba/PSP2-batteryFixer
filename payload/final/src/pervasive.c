#include <stddef.h>
#include "pervasive.h"
#include "utils.h"

#define PERVASIVE_RESET_BASE_ADDR	0xE3101000
#define PERVASIVE_GATE_BASE_ADDR	0xE3102000
#define PERVASIVE_BASECLK_BASE_ADDR	0xE3103000
#define PERVASIVE_MISC_BASE_ADDR	0xE3100000
#define PERVASIVE2_BASE_ADDR		0xE3110000

#define PERVASIVE_BASECLK_MSIF		((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0xB0))
#define PERVASIVE_BASECLK_DSI_REGS(i)	((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0x180 - (i) * 0x80))
#define PERVASIVE_BASECLK_HDMI_CEC	((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0x1D0))

struct pervasive_dsi_timing_subinfo {
	unsigned int unk00;
	unsigned int unk04;
	unsigned int unk08;
};

struct pervasive_dsi_timing_info {
	unsigned int baseclk_0x24_value;
	unsigned int unk04;
	const struct pervasive_dsi_timing_subinfo *subinfo;
	unsigned int unk0C;
	unsigned int unk10;
	unsigned int unk14;
	unsigned int unk18;
	unsigned int unk1C;
	unsigned int unk20;
	unsigned int unk24;
};

static const struct pervasive_dsi_timing_subinfo stru_BD0408 = {0x17,     0xB01, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD0414 = {0x4C074C, 0x904, 0x10180301};
static const struct pervasive_dsi_timing_subinfo stru_BD072C = {0x31,     0xC00, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD04D0 = {0x2014F,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD06EC = {0x20027,  0x203, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0420 = {0x60063,  0xA01, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0620 = {0x20031,  0x202, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0744 = {0x1A,     0xC01, 0x8100301};
static const struct pervasive_dsi_timing_subinfo stru_BD0850 = {0x10052,  0x800, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD0778 = {0x20147,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0738 = {0x20143,  0xA00, 0x10000301};
static const struct pervasive_dsi_timing_subinfo stru_BD0720 = {0x34,     0xA00, 0x8100301};
static const struct pervasive_dsi_timing_subinfo stru_BD0680 = {0x31,     0xA00, 0x4180301};
static const struct pervasive_dsi_timing_subinfo stru_BD06E0 = {0x60063,  0xC01, 0x10000301};

static const struct pervasive_dsi_timing_info stru_BD0458 = {0x10, 0, &stru_BD0408, 0xC0060,  1,     0x10000301, 0,          0x40592EC5, 0,          0x40392EC5};
static const struct pervasive_dsi_timing_info stru_BD0558 = {0,    0, &stru_BD0414, 0xF,      2,     0x4180301,  0,          0x405B0000, 0,          0x403B0000};
static const struct pervasive_dsi_timing_info stru_BD04E0 = {0x10, 1, &stru_BD072C, 0xA014F,  0x200, 0x10000301, 0xA0000000, 0x405F77F1, 0x40000000, 0x40392CC1};
static const struct pervasive_dsi_timing_info stru_BD0508 = {0,    1, &stru_BD0414, 0x27,     0x100, 0x4180301,  0,          0x4070E000, 0,          0x404B0000};
static const struct pervasive_dsi_timing_info stru_BD0690 = {0x10, 0, &stru_BD04D0, 0x140713, 0x100, 0x10000301, 0x40000000, 0x40637B03, 0x40000000, 0x40437B03};
static const struct pervasive_dsi_timing_info stru_BD0860 = {0x10, 0, &stru_BD06EC, 0x2001F,  0x100, 0x4180301,  0,          0x40640000, 0,          0x40440000};
static const struct pervasive_dsi_timing_info stru_BD06F8 = {0x10, 1, &stru_BD0420, 0x80163,  0,     0x10000301, 0x40000000, 0x406859C4, 0x60000000, 0x40437B03};
static const struct pervasive_dsi_timing_info stru_BD05F8 = {0x10, 1, &stru_BD0620, 0x5001F,  0,     0x4180301,  0,          0x40690000, 0,          0x40440000};
static const struct pervasive_dsi_timing_info stru_BD0828 = {0x10, 2, &stru_BD0744, 0x1001F,  0,     0x4180301,  0xA0000000, 0x406C09D8, 0x20000000, 0x4042B13B};
static const struct pervasive_dsi_timing_info stru_BD0630 = {0x10, 2, &stru_BD0850, 0xC005F,  0,     0x4180301,  0xC0000000, 0x406CBB13, 0x80000000, 0x40432762};
static const struct pervasive_dsi_timing_info stru_BD04A8 = {0x10, 2, &stru_BD0778, 0x9004F,  0,     0x8100301,  0,          0x406B0000, 0,          0x40420000};
static const struct pervasive_dsi_timing_info stru_BD0658 = {0x10, 0, &stru_BD06EC, 0x20019,  0,     0x4180301,  0,          0x40704000, 0,          0x40504000};
static const struct pervasive_dsi_timing_info stru_BD05A8 = {0x10, 0, &stru_BD0738, 0xC007B,  0,     0x10000301, 0x80000000, 0x4070957B, 0x80000000, 0x4050957B};
static const struct pervasive_dsi_timing_info stru_BD05D0 = {0x10, 0, &stru_BD06EC, 0xA0063,  0,     0x10000301, 0xE0000000, 0x40710BA2, 0xE0000000, 0x40510BA2};
static const struct pervasive_dsi_timing_info stru_BD0580 = {0x10, 0, &stru_BD072C, 0x6004F,  0,     0x8100301,  0xC0000000, 0x40728B40, 0xC0000000, 0x40528B40};
static const struct pervasive_dsi_timing_info stru_BD0750 = {0,    0, &stru_BD0414, 0x2B,     0,     0x4180301,  0,          0x40729000, 0,          0x40529000};
static const struct pervasive_dsi_timing_info stru_BD0530 = {0x10, 1, &stru_BD0720, 0x30027,  0,     0x4180301,  0,          0x407453A3, 0xC0000000, 0x405042E8};
static const struct pervasive_dsi_timing_info stru_BD0480 = {0x10, 1, &stru_BD0680, 0x80063,  0,     0x10000301, 0xA0000000, 0x40754E8B, 0xE0000000, 0x40510BA2};
static const struct pervasive_dsi_timing_info stru_BD06B8 = {0x10, 1, &stru_BD06E0, 0x31,     0,     0x4180301,  0xE0000000, 0x40772E10, 0xC0000000, 0x40528B40};
static const struct pervasive_dsi_timing_info stru_BD0430 = {0,    1, &stru_BD0414, 0x36,     0,     0x8100301,  0,          0x40773400, 0,          0x40529000};

static const struct {
	unsigned int pixelclock;
	const struct pervasive_dsi_timing_info *timing_info;
} pervasive_dsi_timing_info_lookup[] = {
	{1006301, &stru_BD0458},
	{1080000, &stru_BD0558},
	{1258741, &stru_BD04E0},
	{1350000, &stru_BD0508},
	{1558442, &stru_BD0690},
	{1600000, &stru_BD0860},
	{1948052, &stru_BD06F8},
	{2000000, &stru_BD05F8},
	{2243100, &stru_BD0828},
	{2298462, &stru_BD0630},
	{2356363, &stru_BD04A8},
	{2600000, &stru_BD0658},
	{2653427, &stru_BD05A8},
	{2727273, &stru_BD05D0},
	{2967033, &stru_BD0580},
	{2970000, &stru_BD0750},
	{3252273, &stru_BD0530},
	{3409091, &stru_BD0480},
	{3708791, &stru_BD06B8},
	{3712500, &stru_BD0430},
};

static inline void pervasive_mask_or(unsigned int addr, unsigned int val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"orr %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

static inline void pervasive_mask_and_not(unsigned int addr, unsigned int val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"bic %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

static const struct pervasive_dsi_timing_info *
pervasive_get_dsi_timing_info_for_pixelclock(unsigned int pixelclock)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(pervasive_dsi_timing_info_lookup); i++) {
		if (pervasive_dsi_timing_info_lookup[i].pixelclock == pixelclock)
			return pervasive_dsi_timing_info_lookup[i].timing_info;
	}

	return NULL;
}

unsigned int pervasive_read_misc(unsigned int offset)
{
	return *(unsigned int *)(PERVASIVE_MISC_BASE_ADDR + offset);
}

void pervasive_clock_enable_uart(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_reset_exit_uart(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_clock_enable_gpio(void)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x100, 1);
}

void pervasive_reset_exit_gpio(void)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x100, 1);
}

void pervasive_clock_enable_i2c(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x110 + 4 * bus, 1);
}

void pervasive_reset_exit_i2c(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x110 + 4 * bus, 1);
}

void pervasive_clock_enable_spi(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x104 + 4 * bus, 1);
}

void pervasive_clock_disable_spi(int bus)
{
	pervasive_mask_and_not(PERVASIVE_GATE_BASE_ADDR + 0x104 + 4 * bus, 1);
}

void pervasive_reset_exit_spi(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x104 + 4 * bus, 1);
}

void pervasive_clock_enable_dsi(int bus, int value)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x80 + 4 * bus, value);
}

void pervasive_reset_exit_dsi(int bus, int value)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x80 + 4 * bus, value);
}

void pervasive_clock_enable_msif(void)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0xB0, 1);
}

void pervasive_clock_disable_msif(void)
{
	pervasive_mask_and_not(PERVASIVE_GATE_BASE_ADDR + 0xB0, 1);
}

void pervasive_reset_exit_msif(void)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0xB0, 1);
}

void pervasive_reset_enter_msif(void)
{
	pervasive_mask_or(PERVASIVE_RESET_BASE_ADDR + 0xB0, 1);
}

void pervasive_dsi_set_pixelclock(int bus, int pixelclock)
{
	volatile unsigned int *baseclk_dsi_regs = PERVASIVE_BASECLK_DSI_REGS(bus);
	const struct pervasive_dsi_timing_info *timing_info =
		pervasive_get_dsi_timing_info_for_pixelclock(pixelclock);

	baseclk_dsi_regs[2] = 1;
	baseclk_dsi_regs[1] = 1;
	baseclk_dsi_regs[1];
	dsb();

	baseclk_dsi_regs[9] = timing_info->baseclk_0x24_value;
	baseclk_dsi_regs[0] = 1;
	baseclk_dsi_regs[0];
	dsb();

	if (timing_info->baseclk_0x24_value & 0x10) {
		const struct pervasive_dsi_timing_subinfo *timing_subinfo =
			timing_info->subinfo;

		baseclk_dsi_regs[0xC] = timing_subinfo->unk00;
		baseclk_dsi_regs[0xD] = timing_subinfo->unk04;
		baseclk_dsi_regs[0xE] = timing_subinfo->unk08;
		baseclk_dsi_regs[0] = 0;
		baseclk_dsi_regs[0];
		dsb();

		delay(1000);
	}

	baseclk_dsi_regs[0x10] = timing_info->unk0C;
	baseclk_dsi_regs[0x11] = timing_info->unk10;
	baseclk_dsi_regs[0x12] = timing_info->unk14;
	baseclk_dsi_regs[8] = timing_info->unk04;
	baseclk_dsi_regs[1] = 0;
	baseclk_dsi_regs[1];
	dsb();

	delay(1000);

	baseclk_dsi_regs[2] = 0;
	baseclk_dsi_regs[2];
	dsb();
}

void pervasive_dsi_misc_unk(int bus)
{
	volatile unsigned int *pervasive_misc_regs = (void *)PERVASIVE_MISC_BASE_ADDR;

	if (bus)
		pervasive_misc_regs[0x52] = 0;
	else
		pervasive_misc_regs[0x50] = 0;

	dmb();
}

void pervasive_hdmi_cec_set_enabled(int enable)
{
	*(volatile unsigned int *)PERVASIVE_BASECLK_HDMI_CEC = enable;
	dmb();
}

int pervasive_msif_get_card_insert_state(void)
{
	return *(volatile unsigned int *)(PERVASIVE2_BASE_ADDR + 0xF40) & 1;
}

unsigned int pervasive_msif_unk(void)
{
	unsigned int val;
	volatile unsigned int *pervasive2_regs = (void *)PERVASIVE2_BASE_ADDR;

	val = pervasive2_regs[0x3D1];
	pervasive2_regs[0x3D1] = val;
	pervasive2_regs[0x3D1];
	dsb();

	return val;
}

void pervasive_msif_set_clock(unsigned int clock)
{
	unsigned int val;
	volatile unsigned int *baseclk_msif_regs = PERVASIVE_BASECLK_MSIF;

	if ((clock & ~(1 << 2)) > 2)
		return;

	if (clock & (1 << 2))
		val = 0x10000;
	else
		val = 0;

	*baseclk_msif_regs = (clock & 0b11) | val;
	dmb();
}
