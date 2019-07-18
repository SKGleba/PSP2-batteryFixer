#include "gpio.h"
#include "utils.h"

#define GPIO0_BASE_ADDR			0xE20A0000
#define GPIO1_BASE_ADDR			0xE0100000

#define GPIO_REGS(i)			((void *)((i) == 0 ? GPIO0_BASE_ADDR : GPIO1_BASE_ADDR))

void gpio_set_port_mode(int bus, int port, int mode)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[0] = (gpio_regs[0] & ~(1 << port)) | (mode << port);

	dmb();
}

int gpio_port_read(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	return (gpio_regs[1] >> port) & 1;
}

void gpio_port_set(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[2] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_port_clear(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	gpio_regs[3] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_set_intr_mode(int bus, int port, int mode)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);
	unsigned int reg = 5 + port / 15;
	unsigned int off = 2 * (port % 15);

	gpio_regs[reg] |= (gpio_regs[reg] & ~(3 << off)) | (mode << off);

	dmb();
}

int gpio_query_intr(int bus, int port)
{
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	return (1 << port) & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
			      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
			      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
			      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
			      (gpio_regs[0x12] & ~gpio_regs[0x0B]));
}

int gpio_acquire_intr(int bus, int port)
{
	unsigned int ret;
	unsigned int mask = 1 << port;
	volatile unsigned int *gpio_regs = GPIO_REGS(bus);

	ret = mask & ((gpio_regs[0x0E] & ~gpio_regs[0x07]) |
		      (gpio_regs[0x0F] & ~gpio_regs[0x08]) |
		      (gpio_regs[0x10] & ~gpio_regs[0x09]) |
		      (gpio_regs[0x11] & ~gpio_regs[0x0A]) |
		      (gpio_regs[0x12] & ~gpio_regs[0x0B]));

	gpio_regs[0x0E] = mask;
	gpio_regs[0x0F] = mask;
	gpio_regs[0x10] = mask;
	gpio_regs[0x11] = mask;
	gpio_regs[0x12] = mask;
	dsb();

	return ret;
}
