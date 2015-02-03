/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/

#if !defined(CACHE_H)
#define CACHE_H
#include "alt_cache.h"

// L2 cache 
#define WAY0  1
#define WAY1  2
#define WAY2  4
#define WAY3  8
#define WAY4  0x10
#define WAY5  0x20
#define WAY6  0x40
#define WAY7  0x80
#define WAY0_3 0x0F
#define WAY4_7 0xF0
#define WAY0_7 0xFF

#define CPU_NONE          0

#define CPU0_DATA         1
#define CPU0_INSTRUCTION  2
#define CPU0              3

#define CPU1_DATA         4
#define CPU1_INSTRUCTION  8
#define CPU1              12

void alt_cache_l2_set_locks(uint32 cpus_di, uint32 ways);
// This function will assume that the instructions are not already in the cache
void alt_cache_l2_preload_instructions(void *where, uint32 size);

void alt_cache_l2_load2ways(void *address, uint32 size, uint32 way);

// This is a blocking call until the ways are clean
void alt_cache_l2_clean_cache_by_way( uint32 ways);
void alt_cache_l2_clean_invalidate_cache_by_way( uint32 ways);
void alt_cache_l2_invalidate_cache_by_way( uint32 ways);

#endif
