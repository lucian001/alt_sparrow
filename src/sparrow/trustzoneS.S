/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/


.globl start_nonsecure	@(cpu%d_StartAddr, Stack Addr)
start_nonsecure:
	push	{r0,r1}
	bl      set_mvbar
	bl      _set_nsacr
	
	@ Call Monitor Mode
	pop	{r0,r1}
	SMC     #1

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@       _set_nsacr
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
_set_nsacr:
        @ Allow NS-Access to Coprocessors (NSACR)
        ldr     r1, =(0x30c00)  @ NSACR
        MCR     p15, 0, r1, c1, c1, 2
        bx      lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@	set_mvbar
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.globl set_mvbar
set_mvbar:
	@ Set the MVBar
	adr 	r0, _mvbar_init
really_set_mvbar:
	MCR 	p15, 0, r0, c12, c0, 1 @ Write Rt to MVBar
	bx 	lr

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
@	_mvbar - monitor vector
@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
.align 5
_mvbar:
	.word	0			@ Unused
	.word	0			@ Unused
	adr	pc, _smc_os		@ Call _smc_os
	adr	pc, _smc_loop		@ Prefetch Abort

	adr	pc, _smc_loop		@ Data Abort
	.word	0			@ Unused
	adr	pc, _smc_loop		@ IRQ
	adr	pc, _smc_loop		@ Fast IRQ

# This is the mvbar the first time it is called (transition to non-secure mode)
_mvbar_init:
	.word	0			@ Unused
	.word	0			@ Unused
	adr	pc, _smc_startup	@ smc
	adr	pc, _smc_loop		@ Prefetch Abort

	adr	pc, _smc_loop		@ Data Abort
	.word	0			@ Unused
	adr	pc, _smc_loop		@ IRQ
	adr	pc, _smc_loop		@ Fast IRQ

_smc_loop:	
	nop
	b	_smc_loop

# This is the smc handler which is called by a non-secure OS
_smc_os:
	#	save state
	stmfd	sp!,{r0-r4,lr}

	bl	smc_handler

	ldmfd	sp!,{r0-r4,pc}^

# Param 0 - Jump to address
# Param 1 - Stack End
_smc_startup:
	mov	r5, r0
	@ set up the SMC stack to param #1
	mov	sp, r1

	@ step 1 - replace this SMC with a new one in case it's called from non-secure mode
	adr	r0, _mvbar
	bl	really_set_mvbar

	@ Change to run-secure mode
	mov     r1, #0x31
	MCR     p15, 0, r1, c1, c1, 0 @ Write Rt to SCR

@ Setup return cpsr
	mrs	r0, cpsr
	@ Remove the 'monitor' mode
	bic	r0, r0, #0x1f
	@ Add 'svc' mode
	orr	r0, r0, #19
	push 	{r0}
	
	ldr	r6, =(u_boot_params)
	ldmia	r6, {r0,r1,r2,r3,r12,lr}

	@ Set Address of Address of next instruction
	push	{r5}
	RFEIA   sp!

