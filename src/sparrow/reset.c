/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/

#include <stdint.h>
#include "core.h"
#include "reset.h"
#include "socal/hps.h"
#include "socal/alt_rstmgr.h"

int g_cpuX_state = 0;
reset_function *g_cpuX_main_func = 0;
uint32 g_cpuX_Stack_End = 0;


extern uint32 resetvector;
extern uint32 cpu_stack_addrs[];
volatile uint32 * const reset_mpumodrst = (uint32 *) (ALT_RSTMGR_OFST + ALT_RSTMGR_MPUMODRST_OFST);

#define MAX_CPUS 2
#define RESET_FUNCTION_SIZE 2
uint32 interrupt_save_buffer[RESET_FUNCTION_SIZE];

void unblock_CPU0(void)
{
  // Unblock CPU 0
  g_cpuX_state = 1;
}

void wakeup_CPU(uint32 cpuNum, reset_function *call_function)
{
  uint32 resetbits = 1<<cpuNum;

  if(cpuNum >= MAX_CPUS)
    return;

  // Step 1 - reset our jump vector
  uint32 c;
  uint32 *interrupt_table_loc = (uint32 *) 0; // It is literally at address 0
  uint32 *interrupt_table_from = &resetvector;

  // Setup global variables
  g_cpuX_state = 0; 
  g_cpuX_main_func = call_function;
  g_cpuX_Stack_End = cpu_stack_addrs[cpuNum];

  // Saving whatever was at address 0 in case it was important
  for(c=0 ; c < RESET_FUNCTION_SIZE; c++)
  {
    interrupt_save_buffer[c] = interrupt_table_loc[c];
    interrupt_table_loc[c] = interrupt_table_from[c];
  }

  // Step 2 - reset CPU 1
  *reset_mpumodrst &= ~resetbits;

  // Step 3 - Wait for CPU 1 to wake up before overwriting address 0
  while(g_cpuX_state == 0)
    ;

  // Step 4 - Restoring whatever was at address 0
  for(c=0 ; c < RESET_FUNCTION_SIZE; c++)
  {
    interrupt_table_loc[c] = interrupt_save_buffer[c];
  }
}

void reset_CPU(uint32 cpuNum)
{
  uint32 resetbits = 1<<cpuNum;
  *reset_mpumodrst |= resetbits;
}
