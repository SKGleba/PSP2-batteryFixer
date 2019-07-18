#include "utils.h"

void delay(int n)
{
	volatile int i, j;

	for (i = 0; i < n; i++)
		for (j = 0; j < 200; j++)
			;
}

unsigned int get_cpu_id(void)
{
	unsigned int mpidr;
	asm volatile("mrc p15, 0, %0, c0, c0, 5\n\t" : "=r"(mpidr));
	return mpidr & 0xF;
}

