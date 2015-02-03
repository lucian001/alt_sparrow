/*
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
*/
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "qcom.h"


int main(int argc, char **argv)
{
  channel_handle ch;
  void * mem;
  uint32_t *p_magic;
  unsigned int send, recv;
  unsigned int send_size, recv_size;
  int num_messages = 1;
  int count = 1;
  char *message = "Hello";
  char buffer[256];
  uint32_t size = 256;
  uint32_t memrgn_address= PHYS_MEM_ADDR, memrgn_size = MEM_SIZE;

  for(;count < argc; count++)
  {
    if(!strcmp(argv[count], "-r"))
    {
      q_reversed = 1;
      continue;
    }
    if(!strcmp(argv[count], "-n"))
    {
      count++;
      num_messages = atoi(argv[count]);
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
    message = argv[count];
  }

  if(create_channel(&ch, CHANNEL_TYPE_FIXED, 252, memrgn_address, memrgn_size))
    return 1;

  for(count = 0; count < num_messages; count++)
  {
    post_message(ch, message, 1 + strlen(message));
  }
  while(1)
  {
    size = 256;
    if(0 == poll_message(ch, buffer, &size))
      printf("Msg recvd (%d bytes): %s\n", size, buffer);fflush(stdout);
  }
  destroy_channel(ch);
  return 0;
}

