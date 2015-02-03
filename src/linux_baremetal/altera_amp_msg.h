/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/
#ifndef __ALT_AMP_MSG_H__
#define __ALT_AMP_MSG_H__

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

typedef struct ALT_AMP_MSGHDR_s
{
	int32_t			msgtype;	/* encodes message type */
	uint32_t		length;		/* Size of the total message, in words */
} ALT_AMP_MSGHDR_t;


#define		ALT_AMP_MSG_STATUS_OK			(0)
#define		ALT_AMP_MSG_STATUS_CHECK		(1)
#define		ALT_AMP_MSG_STATUS_FAIL			(-1)
#define		ALT_AMP_MSG_STATUS_CONFIG_OK		(2)
#define		ALT_AMP_MSG_GETCONFIG			(3)
#define		ALT_AMP_MSG_WRITECONFIG			(4)
#define		ALT_AMP_MSG_TO_TERMINAL			(5)

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __ALT_AMP_MSG_H__ */
