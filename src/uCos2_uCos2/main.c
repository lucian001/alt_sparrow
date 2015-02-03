/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/

#include "core.h"
#include "Layout.h"
#include "reset.h"
#include "cache.h"
#include "gic.h"
#include "alt_interrupt.h"




#include "core.h"
#include "trustzone.h"
#include "mempool.h"
#include "gic.h"
#include "cache.h"
#include "mem.h"
#include "alt_interrupt.h"
#include "Layout.h"

#define VERSION "1.0"

void cpu1_main(void);

int cpu0_main(void)
{
  // Were going to set up the L2 cache here before the BM apps are run

  // CPU0 instructions will into WAY0-3
  alt_cache_l2_set_locks(CPU0_INSTRUCTION | CPU0_DATA, WAY0_3);

  // CPU1 instructions will into WAY0-3
  alt_cache_l2_set_locks(CPU1_INSTRUCTION | CPU1_DATA, WAY4_7);

  // enable l2 cache but not the L1
  alt_cache_l2_init();
  alt_cache_l2_prefetch_enable();
  alt_cache_l2_parity_enable();
  alt_cache_l2_enable();

  // Note: Although the l2 cache is enabled, instructions and data will not be cached unless
  // they are in virtual memory with the pages marked as cachable. When the MMU is turned off
  // it will not store any entries to the l2 cache.

  printf("Sparrow v "VERSION"\n");

  SetupTZPeripherals();

  wakeup_CPU(1, cpu1_OS_Start);

  SetupTZMemAccess();

  printf("Starting uCos2 on Core 0....\n\n");

  cpu0_OS_Start();
}

void smc_handler(int Reason)
{
}

