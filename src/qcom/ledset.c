/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "qcom.h"
#include "altera_amp_msg.h"

extern int q_reversed;

#define NUM_TIMERS	2
char *TimerNames[4] = { "Private  Timer",
			"GlobOSC0 Timer",
			"GlobOSC1 Timer",
			"Spec P   Timer" 
};

int main(int argc, char **argv)
{
  channel_handle ch;
  void * mem;
  uint32_t *p_magic;
  unsigned int send, recv;
  unsigned int send_size, recv_size;
  int option = 0;
  int argcount=1, count = 0;
  uint32_t buffer[256/sizeof(uint32_t)];
  uint32_t size = 256;
  uint32_t memrgn_address= PHYS_MEM_ADDR, memrgn_size = MEM_SIZE;

  for(;argcount < argc; argcount++)
  {
    if(!strcmp(argv[argcount], "-r"))
    {
      q_reversed = 1;
      continue;
    }
    if(!strcmp(argv[argcount], "-p"))
    {
      option |= 1;
      continue;
    }
    if(!strcmp(argv[argcount], "-a"))
    {
      if(argc < argcount + 3)
      {
        printf("-a option requires <address> and <size>\n");
        return -1;
      }
      memrgn_address = strtol(argv[argcount+1]);
      memrgn_size = strtol(argv[argcount+2]);
      argcount+=2;
      continue;
    }

    if(!strcmp(argv[argcount], "-s"))
    {
      if(argc < argcount + NUM_TIMERS + 1)
      {
        printf("-s must be followed by %d values\n", NUM_TIMERS);
	return -1;
      }
      for(count=0;count<NUM_TIMERS;count++)
      {
        argcount++;
        buffer[count+1] = strtol(argv[argcount], NULL, 16);
      }
      option |= 2;
      printf("Setting values to:\n");
      for(count=0;count<NUM_TIMERS;count++)
        printf("LED %d (%s) = %x\n", count, TimerNames[count], buffer[count+1]);
    }
  }

  if(create_channel(&ch, CHANNEL_TYPE_FIXED, 252, memrgn_address, memrgn_size))
    return 1;

  // Write our Request
  buffer[0] = ALT_AMP_MSG_STATUS_CHECK;
  if(!option)// Just check for messages
    post_message(ch, (char *)buffer, sizeof(uint32_t));
  buffer[0] = ALT_AMP_MSG_GETCONFIG;
  if(option & 1)// Get Config
    post_message(ch, (char *)buffer, sizeof(uint32_t));
  buffer[0] = ALT_AMP_MSG_WRITECONFIG;
  if(option & 2)// Set Config
    post_message(ch, (char *)buffer, (1+NUM_TIMERS)*sizeof(uint32_t) );

  sleep(1);// give him a second to respond

  // Get our Responses
  size = 256;
  while(0 == poll_message(ch, (char *)buffer, &size))
  {
    switch(buffer[0])
    {
      case ALT_AMP_MSG_STATUS_OK:
        printf("Ok Message Received\n");
        break;
      case ALT_AMP_MSG_STATUS_CONFIG_OK:
        printf("Clock Data Received:\n");
        for(count=0;count<NUM_TIMERS;count++)
          printf("LED %d (%s) = 0x%x (%d hits)\n", count, TimerNames[count], 
			buffer[count+1], buffer[count+1+NUM_TIMERS]);
        break;
      default:
        printf("Unexpected Message Received:%x\n", buffer[0]);
    }
    size = 256;
  }
  destroy_channel(ch);
  return 0;
}

