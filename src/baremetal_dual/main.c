/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/
#include "mempool.h"
#include "core.h"
#include "scu.h"
#include "reset.h"
#include "cache.h"
#include "neon.h"
#include "Layout.h"

void main(void);
void cpu1_main(void);

uint32 *tb;

int cpu0_main(void)
{
  MemPool pool;

  printf("Baremetal app v 0.6 \n");

  alt_cache_system_enable();

  enable_smp();
  enable_scu();

  // Code copied from generated Layout.c, modified to be used by both cores
  MemPoolInit(&pool,
        sparrow_TLB_for_sparrow_Address,
        sparrow_TLB_for_sparrow_Size);

  alt_mmu_va_space_create(&tb, sparrow_regions, sizeof(sparrow_regions)/sizeof(sparrow_regions[0]), MemPoolAlloc, &pool);



  // This wakeup call should be done before the mmu is enabled because it requires writes not be cached
//  wakeup_CPU(1, cpu1_main);

  alt_mmu_DACR_set(g_domain_ap, 16);
  alt_mmu_TTBCR_set(true, false, 0);
  alt_mmu_TTBR0_set(tb);
  alt_mmu_enable();

  __enable_neon();

#if 1  
	hwlib_main(0, 0);
#endif

  while(1)
    __asm__( "wfi" );
}

void cpu1_main(void)
{
  printf("CPU1 Online \n");

  // Note: Although the cache is enabled, instructions and data will not be cached unless
  // they are in virtual memory with the pages marked as cachable. When the MMU is turned off
  // it will not store any entries to the cache.

  enable_smp();

  alt_mmu_DACR_set(g_domain_ap, 16);
  alt_mmu_TTBCR_set(true, false, 0);
  alt_mmu_TTBR0_set(tb);
  alt_mmu_enable();
  __enable_neon();

#if 1
	while(1);
#else

  main();
#endif  
}

