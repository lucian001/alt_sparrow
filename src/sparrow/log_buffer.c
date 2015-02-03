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
#include "log_buffer.h"

#define LOG_MAGIC	0x12348765

struct bufferInfo_t
{
  char *buffer;
  uint32 size;
  uint32 *p_offset;
};

static struct bufferInfo_t bufferInfo[NUM_LOGS];

uint32 init_log_buffer(FILE *f_logid, void *location, uint32 size)
{
  uint32 logid;
  if(f_logid > LAST_SINK || f_logid < LOG0)
    return 1;
  logid = (uint32) f_logid;
  bufferInfo[logid].buffer = location;
  bufferInfo[logid].size = size;
  bufferInfo[logid].p_offset = (uint32 *)(location + size - 2*sizeof(uint32));
  bufferInfo[logid].p_offset[0] = 0;
  bufferInfo[logid].p_offset[1] = LOG_MAGIC;
}

void buffer_putchar(int port, char toprint)
{
  /*if(logid > LAST_LOG || bufferInfo[logid].size == 0)
    return;// Invalid LOG ID	Unnecessary - will be done by caller */
  if(*bufferInfo[port].p_offset >= bufferInfo[port].size)
    return;//Full
  bufferInfo[port].buffer[(*bufferInfo[port].p_offset)++] = toprint;
}

