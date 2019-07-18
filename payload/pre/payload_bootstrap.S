	.align 4
	.text
	.cpu cortex-a9

	.globl _start
@ r0 = payload src paddr, r1 = payload dst paddr, r2 = payload size, r3 = sysroot buffer paddr
_start:
	dsb

	@ Disable interrupts and enter System mode
	cpsid aif, #0x1F

	mov r8, r0
	mov r9, r1
	mov r10, r2
	mov r11, r3

	@ DACR unrestricted
	mov r0, #0xFFFFFFFF
	mcr p15, 0, r0, c3, c0, 0

	ldr r0, =sync_point_1
	bl cpus_sync

	@ Clean and invalidate the entire Dcache
	bl dcache_clean_inv_all

	@ Now we are in an identity-mapped region, let's disable
	@ the MMU, the Dcache and the Icache
	mrc p15, 0, r0, c1, c0, 0
	bic r0, #1 << 0		@ MMU
	bic r0, #1 << 2		@ Dcache
	bic r0, #1 << 12	@ Icache
	mcr p15, 0, r0, c1, c0, 0

	@ Invalidate the entire Dcache
	bl dcache_inv_all

	mov r0, #0
	mcr p15, 0, r0, c7, c5, 6 @ BPIALL (Branch Predictor Invalidate All)
	isb
	mcr p15, 0, r0, c7, c5, 0 @ ICIALLU (Icache Invalidate All to PoU)
	dsb
	mcr p15, 0, r0, c8, c7, 0 @ TLBIALL (Unified TLB Invalidate All)
	isb

	@ Get CPU ID
	mrc p15, 0, r0, c0, c0, 5
	and r0, #0xF
	cmp r0, #0
	bne cpu1_3_cont

	@ Copy the payload to its destination address
	mov r0, r9
	mov r1, r8
	mov r2, r10
	bl memcpy

cpu1_3_cont:
	ldr r0, =sync_point_2
	bl cpus_sync

	@ Jump to the payload!
	mov r0, r11
	mov lr, r9
	bx lr

@ Uses: r0, r1
dcache_clean_inv_all:
	mov r0, #0
1:
	mcr p15, 0, r0, c7, c14, 2 @ DCCISW (Data cache clean and invalidate by set/way)
	adds r0, r0, #0x40000000
	bcc 1b
	adds r0, #0x20
	lsrs r1, r0, #0xD
	beq 1b
	dsb
	bx lr

@ Uses: r0, r1
dcache_inv_all:
	mov r0, #0
1:
	mcr p15, 0, r0, c7, c6, 2 @ DCISW (Data cache invalidate by set/way)
	adds r0, r0, #0x40000000
	bcc 1b
	adds r0, #0x20
	lsrs r1, r0, #0xD
	beq 1b
	dsb
	bx lr


@ r0 = sync point address
@ Uses: r0, r1, r2
cpus_sync:
	mrc p15, 0, r1, c0, c0, 5
	and r1, #0xF
	cmp r1, #0
	streq r1, [r0]
1:
	ldrb r2, [r0]
	cmp r1, r2
	wfene
	bne 1b
	ldrh r2, [r0]
	adds r2, #1
	adds r2, r2, #0x100
	strh r2, [r0]
	dsb
	sev
1:
	ldrb r2, [r0, #1]
	cmp r2, #4
	wfene
	bne 1b
	bx lr

@ r0 = dst, r1 = src, r2 = size
@ Uses: r0, r1, r2, r3
memcpy:
	ldmia r1!, {r3}
	stmia r0!, {r3}
	subs r2, #4
	bne memcpy
	bx lr

	.data
sync_point_1: .word 0
sync_point_2: .word 0
