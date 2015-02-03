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
#include "socal/hps.h"


#define ALT_MPUSCU_CNTRL_OFFSET           0x0
#define ALT_MPUSCU_CNTRL_ENABLE           0x1
#define ALT_MPUSCU_CNTRL_ADDR_FILTER      0x2
#define ALT_MPUSCU_CNTRL_RAM_PARITY       0x4
#define ALT_MPUSCU_CNTRL_SPEC_LINEFILL    0x8
#define ALT_MPUSCU_CNTRL_PORT0_EN         0x10
#define ALT_MPUSCU_CNTRL_SCU_STANDBY_EN   0x20
#define ALT_MPUSCU_CNTRL_IC_STANDBY_EN    0x40

#define ALT_MPUSCU_INVALIDATE_OFFSET      0xC
#define ALT_MPUSCU_INVALIDATE_ALL         0x0000FFFF
void enable_scu(void)
{
  volatile uint32 *pCntrl = (uint32 *)(ALT_MPUSCU_ADDR + ALT_MPUSCU_CNTRL_OFFSET);
  volatile uint32 *pInvalidate = (uint32 *)(ALT_MPUSCU_ADDR + ALT_MPUSCU_INVALIDATE_OFFSET);

  *pCntrl |= ALT_MPUSCU_CNTRL_SPEC_LINEFILL | ALT_MPUSCU_CNTRL_ENABLE;
  *pInvalidate = ALT_MPUSCU_INVALIDATE_ALL;
  
}
