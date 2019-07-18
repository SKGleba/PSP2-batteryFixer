#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#define BIT(x)		(1 << (x))
#define ARRAY_SIZE(x)	(sizeof(x) / sizeof((x)[0]))

#define dmb() asm volatile("dmb\n\t")
#define dsb() asm volatile("dsb\n\t")
#define wfe() asm volatile("wfe\n\t")

static inline unsigned int rbit(unsigned int x)
{
	unsigned int xrev;
	asm volatile("rbit %0, %1\n\t" : "=r"(xrev) : "r"(x));
	return xrev;
}

static inline unsigned char readb(volatile void *addr)
{
	return *(unsigned char *)addr;
}

static inline unsigned short readw(volatile void *addr)
{
	return *(unsigned short *)addr;
}

static inline unsigned int readl(volatile void *addr)
{
	return *(unsigned int *)addr;
}

static inline void writeb(unsigned char val, volatile void *addr)
{
	*(unsigned char *)addr = val;
}

static inline void writew(unsigned short val, volatile void *addr)
{
	*(unsigned short *)addr = val;
}

static inline void writel(unsigned int val, volatile void *addr)
{
	*(unsigned int *)addr = val;
}

static inline uint64_t be_uint64_t_load(const void *addr)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return __builtin_bswap64(*(uint64_t *)addr);
#else
	return *(uint64_t *)addr;
#endif
}

static inline void be_uint64_t_store(void *addr, uint64_t val)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	*(uint64_t *)addr = __builtin_bswap64(val);
#else
	*(uint64_t *)addr = val;
#endif
}

static inline unsigned int smc_p(unsigned int cmd, unsigned int arg1,
			       unsigned int arg2, unsigned int arg3,
			       unsigned int arg4)
{
	register unsigned int r0 asm("r0") = arg1;
	register unsigned int r1 asm("r1") = arg1;
	register unsigned int r2 asm("r2") = arg1;
	register unsigned int r3 asm("r3") = arg1;
	register unsigned int r12 asm("r12") = cmd;

	asm(".arch_extension sec\n\t");
	asm volatile(
		"smc #0\n\t"
		: "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3)
		: "r"(r12)
	);

	return r0;
}

void delay(int n);
unsigned int get_cpu_id(void);

#endif
