/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/
#include "Layout.h"

.align 5
.globl _start
_start: @ 40
@ Need to set SP_MON and LR_MON
	adr	r5, u_boot_params
	stmia	r5, {r0,r1,r2,r3,r12,lr}
	ldr	sp, =(cpu0_Stack_End)
	# Call memset(__bss_start__,0,__bss_end__-__bss_start__)
	ldr	r0, =__bss_start__
	ldr	r2, =__bss_end__
	mov	r1, #0
	sub	r2,r2,r0
	bl	memset

	bl	gic_init_dist
	bl	gic_init_cpu
	b	cpu0_main

.globl u_boot_params
u_boot_params:
	.word	0,0,0,0,0,0,0,0


@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@	_interrupts - int vector
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.align 5
.globl _interrupts
_interrupts:
	ldr	pc, [pc, #24]	@ reset
	ldr	pc, [pc, #24]	@ undefined instruction
	ldr	pc, [pc, #24]	@ swi
	ldr	pc, [pc, #24]	@ prefetch abort

	ldr	pc, [pc, #24]	@ Data Abort
	.word	0		@ Unused
	ldr	pc, [pc, #24]	@ IRQ
	ldr	pc, [pc, #24]	@ Fast IRQ
	
	.word	_iloop
	.word	_iloop
	.word	_iloop
	.word	_iloop

	.word	_iloop
	.word	_iloop
	.word	_iloop
	.word	_iloop
_iloop:
	nop
	nop
	nop
	b	_iloop

.globl _interrupts_end
_interrupts_end:

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@	set_vbar
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.globl set_vbar
set_vbar:
	@ Set the VBar
	mcr 	p15, 0, r0, c12, c0, 0
	bx	lr


# Space set aside for storing command line options from uboot
.globl start_secure	@(cpu%d_StartAddr)
start_secure:
	mov	pc,r0
