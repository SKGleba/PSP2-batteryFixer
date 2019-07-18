/*
	bicr by SKGleba
	All Rights Reserved
*/

#include <stdio.h>
#include "pervasive.h"
#include "syscon.h"
#include "utils.h"

int main(void *argv)
{
	if (get_cpu_id() != 0) {
		while (1)
			wfe();
	}

	syscon_init();
	sc_call(0, 0x888, 2);
	sc_call(0, 0x989, 1);
	sc_call(0, 0x822, 1);
	smc_p(0x11A, 2, 2, 0, 0);
	
	return 0;
}
