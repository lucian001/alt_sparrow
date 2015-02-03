/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/

.globl l2_cache_preload_instructions
alt_cache_l2_preload_instructions:
	@ r0 = memory location
	@ r1 = size
loop:
        cmp	r1, #0
	ble	done
	sub	r1, #4
	pli	[r0, r1]
	b	loop
done:
	mov	pc, lr

.globl alt_cache_l2_load2ways
alt_cache_l2_load2ways:
@ r0 address
@ r1 size
@ r2 way
# clear this memory from cache
#        push	{r3, r4}
#	mov	r3, #(0xFFFEF7F0)
# disable the l1 d cache
#	mrc	p15, 0, r5, c1, c0, 0
#	bic	r4, r5, #1	@save r5 for end of function
#	mcr	p15, 0, r4, c1, c0, 0
	add	r1,r1,r0	@ r1 = addr end
# set the cache for data only
	ldr	r3, =(0xFFFEF90c)
	ldr	r4, =(0xFFFF)
	str	r4, [r3]	@ 0xFFFEF90c = CPU1 I
	sub	r3,r3,#4
	str	r4, [r3]	@ 0xFFFEF908 = CPU1 D
	sub	r3,r3,#4
	str	r4, [r3]	@ 0xFFFEF904 = CPU0 I
	sub	r3,r3,#4
	eor	r2,r2,r4
	str	r2, [r3]	@ 0xFFFEF900 = CPU0 D
	dmb
	isb
# load each value
load_cache_loop:
	ldr	r2, [r0]	@ Load memory to cache
#	str	r2, [r0]	@ Store it back to get past l1
	add	r0, #4
	cmp	r0,r1
	bne	load_cache_loop

#	mcr	p15, 0, r5, c1, c0, 0
	str	r4, [r3]	@ 0xFFFEF900 = CPU0 D 
	bx	lr

