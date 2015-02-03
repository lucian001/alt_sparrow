/*
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
*/
#ifndef QCOM_H
#define QCOM_H

typedef void *channel_handle;


#define CHANNEL_TYPE_FIXED              0
#define CHANNEL_TYPE_VARIABLE           1
uint32_t create_channel(channel_handle *ch, uint32_t type, uint32_t msg_size, 
	uint32_t physaddr, uint32_t size);
uint32_t destroy_channel(channel_handle ch);

// 0 copy interface - only use if speed is crucial!
uint32_t zero_getbuff(channel_handle ch, char **buff, uint32_t *size);
uint32_t zero_postbuff(channel_handle *ch);
uint32_t zero_pollbuff(channel_handle *ch, char **buff, uint32_t *size);
uint32_t zero_buffdone(channel_handle *ch);

// Simple message interface
uint32_t post_message(channel_handle *chan, char *buff, uint32_t size);
uint32_t poll_message(channel_handle *chan, char *buff, uint32_t *size);

extern int q_reversed;
#endif

