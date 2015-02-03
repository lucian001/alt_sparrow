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
#include "uart.h"

#include <stdint.h>
#include "socal/hps.h"
#include "socal/alt_uart.h"

ALT_UART_raw_t *gSerial[2]  = { (ALT_UART_raw_t *) ALT_UART0_OFST, 
				(ALT_UART_raw_t *) ALT_UART1_OFST};

int init_uart1()
{
  // UART0 has already set up by uboot
  // UART1 Reset
  gSerial[1]->_u_0x8.fcr =
	// Reset Receive
    ALT_UART_FCR_RFIFOR_SET(ALT_UART_FCR_XFIFOR_E_RST) |
	// Reset Send
    ALT_UART_FCR_XFIFOR_SET(ALT_UART_FCR_XFIFOR_E_RST);
  // Set Data Len  to 8
  gSerial[1]->lcr = ALT_UART_LCR_DLS_E_LEN8;
  gSerial[1]->mcr =
	// Set Data Terminal Ready
    ALT_UART_MCR_DTR_SET(ALT_UART_MCR_DTR_E_LOGIC0) |
	// Set Send Ready
    ALT_UART_MCR_RTS_SET(ALT_UART_MCR_RTS_E_LOGIC0);
}

void uart_putchar(int port, char toprint)
{
  int count;
  // While Ready is 0
  while(ALT_UART_LSR_THRE_GET(gSerial[port]->lsr) == 0)
	;
  gSerial[port]->rbr_thr_dll = ALT_UART_RBR_THR_DLL_VALUE_SET(toprint);

  if(toprint == '\n')
    uart_putchar(port, '\r');
}

int getch(void)
{
  while(ALT_UART_LSR_DR_GET(gSerial[0]->lsr) == ALT_UART_LSR_DR_E_NODATARDY)
	;
  return ALT_UART_RBR_THR_DLL_VALUE_GET(gSerial[0]->rbr_thr_dll);
}
