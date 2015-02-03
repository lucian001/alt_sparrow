/*
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
 */
 
 
#include <stddef.h>
#include <stdint.h>
#ifdef LINUX
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>                /* strcmp() */
#include <stdlib.h>                /* strtoul() */
#include <unistd.h>                /* close() */
#else
#include "mem.h"
#endif
#include "qcom.h"
 
#define QCOM_MAGIC			0xDEADDEA7

#pragma pack(push)
#pragma pack(8)
typedef struct _channel_info
{
  uint32_t magic;
  // Info on my send queue
  uint32_t this_type;
  uint32_t this_size;

  uint32_t this_head;
  uint32_t this_max_count;
  // Only valid if FIXED
  uint32_t this_message_size;
  // In on his queue
  uint32_t that_tail;
#pragma pack()  
} channel_info;
#pragma pack(pop)

/*		Channel Code			*/
typedef struct _channel
{
  int fd;
  char  *w_addr;
  char  *r_addr;
  channel_info *wr_info;
  channel_info *rd_info;
} channel;

int q_reversed = 0;

uint32_t create_channel(channel_handle *ch, uint32_t type, uint32_t msg_size, 
	uint32_t physaddr, uint32_t size)
{
  channel_info *wr_info;
  uint32_t send_recv_size = size/2;
#ifdef LINUX
  channel *chan = (channel *) malloc(sizeof(channel));

  if(!chan || !size)
    return 1;
  chan->fd = 0;
  chan->w_addr = MAP_FAILED;
  chan->r_addr = MAP_FAILED;
  chan->fd = open("/dev/mem", O_RDWR );
  if (chan->fd < 0) {
    printf("File open error, check permissions\n");
    destroy_channel(chan);
    return 1;
  }
  chan->w_addr = mmap(NULL, send_recv_size, 
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, 
		chan->fd, physaddr);
  if (chan->w_addr == MAP_FAILED) {
    printf("Error mapping send queue %d\n", errno);
    destroy_channel(chan);
    return 1;
  }
  chan->r_addr = mmap(NULL, send_recv_size, 
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED | MAP_POPULATE, 
		chan->fd, physaddr + send_recv_size);
  if (chan->r_addr == MAP_FAILED) {
    printf("Error mapping recv queue %d\n", errno);
    destroy_channel(chan);
    return 1;
  }
  if(q_reversed)
  {
    char *temp = chan->r_addr;
    chan->r_addr = chan->w_addr;
    chan->w_addr = temp;
  }
#else
  static channel g_allocated;
  channel *chan = &g_allocated;
  chan->r_addr = (char *)physaddr;
  chan->w_addr = (char *)(physaddr + send_recv_size);
#endif
  chan->wr_info = (channel_info *)(chan->w_addr + send_recv_size - sizeof(channel_info));
  chan->rd_info = (channel_info *)(chan->r_addr + send_recv_size - sizeof(channel_info));

  wr_info = chan->wr_info;
  if(wr_info->magic != QCOM_MAGIC)
  {
    wr_info->that_tail = 0;
    wr_info->this_head = 0;
    wr_info->this_size = (uint32_t)( (char *)wr_info - chan->w_addr);
    // For now, FIXED(256)
    wr_info->this_message_size = msg_size + sizeof(uint32_t); // always save last 4 bytes for size info
    if(type == CHANNEL_TYPE_FIXED)
    {
      wr_info->this_max_count    = wr_info->this_size / wr_info->this_message_size;
      wr_info->this_size = ( wr_info->this_max_count*wr_info->this_message_size);
    }
    else
    {
#ifdef LINUX
      printf("Error, only fixed sized messages are currently supported\n");
#endif
      return 1;
    }
    wr_info->this_type = type;
    // Write MAGIC last
    wr_info->magic     = QCOM_MAGIC;
  }
  else
  {
  // Add some sanity to avoid crashes
    if(wr_info->that_tail > chan->rd_info->this_max_count)
      wr_info->that_tail = 0;
    if(wr_info->this_head > wr_info->this_max_count)
      wr_info->this_head = 0;
  }
  *ch = chan;
  return 0;
}

uint32_t destroy_channel(channel_handle ch)
{
#ifdef LINUX
  channel *chan = (channel *)ch;
  if(chan->w_addr != MAP_FAILED)
    munmap(chan->w_addr, chan->wr_info->this_size + sizeof(channel_info));
  if(chan->w_addr != MAP_FAILED)
    munmap(chan->r_addr, chan->rd_info->this_size + sizeof(channel_info));
  if(chan->fd)
    close(chan->fd);
  free(chan);
#endif
  return 0;
}

static int32_t next_head(channel *chan)
{
  uint32_t other_tail = 0;
  uint32_t next_head = (chan->wr_info->this_head + 1) % chan->wr_info->this_max_count;
  if(chan->rd_info->magic == QCOM_MAGIC)
    other_tail = chan->rd_info->that_tail;
  if(next_head == other_tail )
    return -1;// Queue is full
  return next_head;
}

uint32_t zero_getbuff(channel_handle ch, char **buff, uint32_t *size)
{
  channel *chan = (channel *)ch;
  if(next_head(chan) < 0 || buff == NULL)
    return 1;
  *buff = chan->w_addr + 
      (chan->wr_info->this_head * chan->wr_info->this_message_size);
  return 0;  
}

uint32_t zero_postbuff(channel_handle *ch)
{
  int32_t n_head;
  channel *chan = (channel *)ch;
  n_head = next_head(chan);
  if(n_head < 0)
    return 1;
  chan->wr_info->this_head = n_head; 
  return 0;
}

uint32_t post_message(channel_handle *ch, char *buff, uint32_t size)
{
  channel *chan = (channel *)ch;
  char *nextbuff;
  if(zero_getbuff(ch, &nextbuff, &size))
    return 1;
  // Copy
  if(size > chan->wr_info->this_message_size - sizeof(uint32_t))
    return 1; // Message too big
  memcpy(nextbuff, buff, size);
  *((uint32_t *)(nextbuff +  chan->wr_info->this_message_size - sizeof(uint32_t))) = size;
  // Post
  zero_postbuff(ch);
  return 0;
}

uint32_t recv_empty(channel *chan)
{
  uint32_t head = 0;
  if(chan->rd_info->magic == QCOM_MAGIC)
    head = chan->rd_info->this_head;
  if(chan->wr_info->that_tail == head)
    return 1;
  return 0;
}

uint32_t zero_pollbuff(channel_handle *ch, char **buff, uint32_t *size)
{
  channel *chan = (channel *)ch;
  if(!buff || recv_empty(chan))
    return 1;
  if(chan->rd_info->this_type == CHANNEL_TYPE_FIXED)
  {
    *buff = chan->r_addr + chan->wr_info->that_tail*chan->rd_info->this_message_size;
    *size = *((uint32_t *)(*buff +  chan->rd_info->this_message_size - sizeof(uint32_t)));
  }
  else
    return 1;
  return 0;
}

uint32_t zero_buffdone(channel_handle *ch)
{
  channel *chan = (channel *)ch;
  uint32_t head = 0;
  if(recv_empty(chan))
    return 1;
  if(chan->rd_info->this_type == CHANNEL_TYPE_FIXED)
  {
    chan->wr_info->that_tail = (chan->wr_info->that_tail + 1) % chan->rd_info->this_max_count;
  }
  return 0;
}

uint32_t poll_message(channel_handle *ch, char *buff, uint32_t *size)
{
  channel *chan = (channel *)ch;
  char *nbuff;
  uint32_t message_size;
  // Get Message
  if(zero_pollbuff(ch, &nbuff, &message_size))
    return 1;
  // Copy
  if(!size)
    size = &message_size;
  if(*size > message_size)
    *size = message_size;
  memcpy(buff, nbuff, *size);
  // Done
  return zero_buffdone(ch);
}

