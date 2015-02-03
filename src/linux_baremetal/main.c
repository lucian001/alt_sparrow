/******************************************************************************

Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/
#include <core.h>
#include <reset.h>
#include <cache.h>
#include <neon.h>
#include <scu.h>
#include <log_buffer.h>
#include "Layout.h"

void cpu1_main(void);

int cpu0_main(void)
{
  uint32 clrmem;

  // Initialize the Shared Memory
  clrmem = System_Shared_Comm_Channel_Address;
  while(clrmem < System_Shared_Comm_Channel_Address + 0x1000)
                //System_Shared_Comm_Channel_AddrEnd)
  {
    *((uint32 *)clrmem) = 0;
    clrmem += 4;
  }

  // disable l2 cache to CPU0 initially. Linux will need to turn it on when it is ready
  alt_cache_l2_set_locks(CPU0_INSTRUCTION | CPU0_DATA, WAY0_7);

  // CPU1 instructions will go into WAY0 or WAY1
  alt_cache_l2_set_locks(CPU1_INSTRUCTION, WAY2|WAY3|WAY4_7);

  // Don't let CPU1's data affect the instructions cached
  alt_cache_l2_set_locks(CPU1_DATA, WAY0);

  // enable l2 cache but not the L1
  alt_cache_l2_init();
  alt_cache_l2_prefetch_enable();
  alt_cache_l2_parity_enable();
  alt_cache_l2_enable();

  enable_scu();

  // Note: Although the l2 cache is enabled, instructions and data will not be cached unless
  // they are in virtual memory with the pages marked as cachable. When the MMU is turned off
  // it will not store any entries to the l2 cache.

  printf("Baremetal merged AMP app v 0.6 \n");

  SetupTZPeripherals();

  // Must be done before SetupTZMemAccess because address 0 needs to be used
  wakeup_CPU(1, cpu1_main);

  SetupTZMemAccess();

  printf("Starting Linux on Core 0....\n\n");

  cpu0_OS_Start();
}

void main(void);

void cpu1_main(void)
{
/*	Special code for Sparrow integration		*/
    __enable_neon();

    Map_sparrow();

    alt_cache_l1_enable_all();

    init_log_buffer(LOG0, (void *) sparrow_OS1_s_Log_Buffer_Address, sparrow_OS1_s_Log_Buffer_Size);

    fprintf(LOG0, "Linux_Baremetal App\n");

    main();
}

//      These should match the driver(s) in Linux
#define TRIGGER_INT             1
#define DISABLE_L2_CACHE_CPU0   2
#define ENABLE_L2_CACHE_CPU0    3

void smc_handler(int Reason)
{
  switch(Reason)
  {
    case TRIGGER_INT:
      break;
    case DISABLE_L2_CACHE_CPU0:
      // diable CPU0's access to all ways
      alt_cache_l2_set_locks(CPU0_INSTRUCTION | CPU0_DATA, WAY0_7);
      break;
    case ENABLE_L2_CACHE_CPU0:
      // enable CPU0's access to L2 ways 2-7, leaving way 0-1 to CPU 1
      alt_cache_l2_set_locks(CPU0_INSTRUCTION | CPU0_DATA, WAY0 | WAY1);
      break;
  }
}

