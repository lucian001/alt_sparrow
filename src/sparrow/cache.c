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
#include "cache.h"
#include "socal/hps.h"

#define ALT_MPUL2_D_LOCKDOWN0_OFST          0x900
#define ALT_MPUL2_I_LOCKDOWN0_OFST          0x904
#define ALT_MPUL2_D_LOCKDOWN1_OFST          0x908
#define ALT_MPUL2_I_LOCKDOWN1_OFST          0x90c
#define ALT_MPUL2_SYNC                      0x730
#define ALT_MPUL2_CLEAN                     0x7bc
#define ALT_MPUL2_INVALIDATE                0x77c
#define ALT_MPUL2_CLEAN_INVALIDATE          0x7fc

#define NUM_LOCK_TYPES 4
struct _cpu_lut
{
// Cpu # + data or instruction. Consts are defined such as CPU0_DATA in .h file
  uint32 cpu_di; 
  volatile uint32 *offset;
} cpus[NUM_LOCK_TYPES] = {   
	{CPU0_DATA, 
          (volatile uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_D_LOCKDOWN0_OFST)},
        {CPU0_INSTRUCTION, 
          (volatile uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_I_LOCKDOWN0_OFST)},
        {CPU1_DATA, 
          (volatile uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_D_LOCKDOWN1_OFST)},
        {CPU1_INSTRUCTION, 
          (volatile uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_I_LOCKDOWN1_OFST)}};

void alt_cache_l2_set_locks( uint32 cpus_di, uint32 ways)
{
  uint32 a;
  for(a = 0; a < NUM_LOCK_TYPES; a++)
  {
    if(cpus[a].cpu_di & cpus_di)
    {
      volatile uint32 *l2_reg = cpus[a].offset;
      *l2_reg = ways;
    }      
  }
}

uint32 *p_CacheSync = (uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_SYNC);
uint32 *p_CacheCleanByWay = (uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_CLEAN);
uint32 *p_CacheCleanInvalidateByWay = (uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_CLEAN_INVALIDATE);
uint32 *p_CacheInvalidateByWay = (uint32 *)(ALT_MPUL2_OFST + ALT_MPUL2_INVALIDATE);

void alt_cache_l2_clean_cache_by_way( uint32 ways)
{
  // Clean the caches
  *p_CacheCleanByWay = ways;
  // Need to wait for completion
  while(*p_CacheCleanByWay & ways)
    ;
//  while(*p_CacheSync & 1)
//    ;
}

void alt_cache_l2_clean_invalidate_cache_by_way( uint32 ways)
{
  // Invalidate the caches
  *p_CacheCleanInvalidateByWay = ways;
  // Need to wait for completion
  while((*p_CacheCleanInvalidateByWay) & ways)
    ;
//  while(*p_CacheSync & 1)
//    ;
}

void alt_cache_l2_invalidate_cache_by_way( uint32 ways)
{
  // Invalidate the caches
  *p_CacheInvalidateByWay = ways;
  // Need to wait for completion
  while(*p_CacheInvalidateByWay & ways)
    ;
//  while(*p_CacheSync & 1)
//    ;
}

