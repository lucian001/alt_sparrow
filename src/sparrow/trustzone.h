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
#include "socal/hps.h"
#include "socal/alt_l3.h"

#define ALT_SDR_CTL_PROTPORT_DENY_NONE        0
#define ALT_SDR_CTL_PROTPORT_DENY_FPGA        0x1F
#define ALT_SDR_CTL_PROTPORT_DENY_CPU_READ    (1<<7)
#define ALT_SDR_CTL_PROTPORT_DENY_CPU_WRITE   (1<<9)
#define ALT_SDR_CTL_PROTPORT_DENY_L3_READ     (1<<6)
#define ALT_SDR_CTL_PROTPORT_DENY_L3_WRITE    (1<<8)
#define ALT_SDR_CTL_PROTPORT_DENY_CPU         ALT_SDR_CTL_PROTPORT_DENY_CPU_READ | ALT_SDR_CTL_PROTPORT_DENY_CPU_WRITE
#define ALT_SDR_CTL_PROTPORT_DENY_L3          ALT_SDR_CTL_PROTPORT_DENY_L3_READ | ALT_SDR_CTL_PROTPORT_DENY_L3_WRITE

#define ALT_SDR_CTL_RULEID_MIN            0x000
#define ALT_SDR_CTL_RULEID_MAX            0xFFF
#define ALT_SDR_CTL_RULEID_L2M0_LO        0b000000000010
#define ALT_SDR_CTL_RULEID_L2M0_HI        0b011111111010
#define ALT_SDR_CTL_RULEID_DMA_LO         0b000000000001
#define ALT_SDR_CTL_RULEID_DMA_HI         0b000001111001
#define ALT_SDR_CTL_RULEID_EMAC0_LO       0b100000000001
#define ALT_SDR_CTL_RULEID_EMAC0_HI       0b100001111001
#define ALT_SDR_CTL_RULEID_EMAC1_LO       0b100000000010
#define ALT_SDR_CTL_RULEID_EMAC1_HI       0b100001111010
#define ALT_SDR_CTL_RULEID_USB0           0b100000000011
#define ALT_SDR_CTL_RULEID_USB1           0b100000000110
#define ALT_SDR_CTL_RULEID_NAND_LO        0b100000000100
#define ALT_SDR_CTL_RULEID_NAND_HI        0b111111111100
#define ALT_SDR_CTL_RULEID_TMC            0b100000000000
#define ALT_SDR_CTL_RULEID_DAP            0b000000000100
#define ALT_SDR_CTL_RULEID_SDMMC          0b100000000101
#define ALT_SDR_CTL_RULEID_FPGA2SOC_LO    0b000000000000
#define ALT_SDR_CTL_RULEID_FPGA2SOC_HI    0b011111111000

#define ALT_SDR_CTL_DATA_ACCESS_SECURE        0
#define ALT_SDR_CTL_DATA_ACCESS_NONSECURE     1
#define ALT_SDR_CTL_DATA_ACCESS_BOTH          2

#define ALT_SDR_CTL_DATA_RULE_PORT_FPGA       0x01F8
#define ALT_SDR_CTL_DATA_RULE_PORT_L3         0x0A00
#define ALT_SDR_CTL_DATA_RULE_ACCESS_CPUS     0x1400
#define ALT_SDR_CTL_DATA_RULE_ALL_PORTS       0x1FF8
#define ALT_SDR_CTL_DATA_DENY_ACCESS          (1<<13)
#define ALT_SDR_CTL_DATA_ALLOW_ACCESS         0
#define ALT_SDR_CTL_PROTRULERDWR_WRITE        (1<<5)
#define MAX_RULE_NUM                          19

#define ALT_L3_SEC_PERIPHERAL_DMA         (1ull<<0)
#define ALT_L3_SEC_PERIPHERAL_USB0        (1ull<<1)
#define ALT_L3_SEC_PERIPHERAL_USB1        (1ull<<2)
#define ALT_L3_SEC_PERIPHERAL_CAN0        (1ull<<3)
#define ALT_L3_SEC_PERIPHERAL_CAN1        (1ull<<4)
#define ALT_L3_SEC_PERIPHERAL_SDMMC       (1ull<<5)
#define ALT_L3_SEC_PERIPHERAL_NAND        (1ull<<6)
#define ALT_L3_SEC_PERIPHERAL_QSPI        (1ull<<7)
#define ALT_L3_SEC_PERIPHERAL_SPI0_SLAVE  (1ull<<8)
#define ALT_L3_SEC_PERIPHERAL_SPI1_SLAVE  (1ull<<9)
#define ALT_L3_SEC_PERIPHERAL_SPI0_MASTER (1ull<<10)
#define ALT_L3_SEC_PERIPHERAL_SPI1_MASTER (1ull<<11)
#define ALT_L3_SEC_PERIPHERAL_I2C0        (1ull<<12)
#define ALT_L3_SEC_PERIPHERAL_I2C1        (1ull<<13)
#define ALT_L3_SEC_PERIPHERAL_I2C2        (1ull<<14)
#define ALT_L3_SEC_PERIPHERAL_I2C3        (1ull<<15)
#define ALT_L3_SEC_PERIPHERAL_EMAC0       (1ull<<16)
#define ALT_L3_SEC_PERIPHERAL_EMAC1       (1ull<<17)
#define ALT_L3_SEC_PERIPHERAL_UART0       (1ull<<18)
#define ALT_L3_SEC_PERIPHERAL_UART1       (1ull<<19)
#define ALT_L3_SEC_PERIPHERAL_GPIO0       (1ull<<20)
#define ALT_L3_SEC_PERIPHERAL_GPIO1       (1ull<<21)
#define ALT_L3_SEC_PERIPHERAL_GPIO2       (1ull<<22)
#define ALT_L3_SEC_PERIPHERAL_SPTIMER0    (1ull<<23)
#define ALT_L3_SEC_PERIPHERAL_SPTIMER1    (1ull<<24)
#define ALT_L3_SEC_PERIPHERAL_OSCTIMER0   (1ull<<25)
#define ALT_L3_SEC_PERIPHERAL_OSCTIMER1   (1ull<<26)
#define ALT_L3_SEC_PERIPHERAL_WATCHDOG0   (1ull<<27)
#define ALT_L3_SEC_PERIPHERAL_WATCHDOG1   (1ull<<28)
#define ALT_L3_SEC_PERIPHERAL_CLKMGR      (1ull<<29)
#define ALT_L3_SEC_PERIPHERAL_OCR         (1ull<<30)
#define ALT_L3_SEC_PERIPHERAL_ACP         (1ull<<31)
#define ALT_L3_SEC_PERIPHERAL_ROM         (1ull<<32)
#define ALT_L3_SEC_PERIPHERAL_SDRAMCTRL   (1ull<<33)
#define ALT_L3_SEC_PERIPHERAL_DAP         (1ull<<34)
#define ALT_L3_SEC_PERIPHERAL_RESET       (1ull<<35)
#define ALT_L3_SEC_PERIPHERAL_SYSMGR      (1ull<<36)
#define ALT_L3_SEC_PERIPHERAL_SCANMGR     (1ull<<37)
#define ALT_L3_SEC_PERIPHERAL_STMSLAVE    (1ull<<38)
#define ALT_L3_SEC_PERIPHERAL_FPGA        (1ull<<39)
#define ALT_L3_SEC_PERIPHERAL_FPGAMNGR    (1ull<<40)
#define ALT_L3_SEC_PERIPHERAL_GLOBALTIMER (1ull<<39)
#define ALT_L3_SEC_PERIPHERAL_PRIVATETIMER (1ull<<40)

void start_nonsecure(uint32 where, uint32 StackEnd);
void start_secure(uint32 where);

#define ALT_SDR_CTL_RULE_NEW			0xFFFFFFFFu
#define ALT_SDR_CTL_RULE_INVALID		0xFFFFFFFEu

uint32  alt_sdr_ctl_set_tz_rule(uint32 RuleNum, uint32 LowAddr, uint32 HighAddr, uint32 RuleMin, uint32 RuleMax,
             uint32 SecurityType, uint32 Ports, uint32 AccessPermission);
void    alt_sdr_ctl_delete_rule(uint32 RuleNum);
uint32  alt_sdr_ctl_set_tz_default(uint32 defaultRule);

void alt_l3_secgrp_set_peripheral_access(uint64 NonSecure, uint64 SecureOnly);
