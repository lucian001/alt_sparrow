/******************************************************************************
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
******************************************************************************/
#include <core.h>

void _div0(void)
{}

static __inline__ unsigned int intdiv(unsigned int num, unsigned int den, unsigned int *mod)
{
	unsigned int result = 0, shifter=den;
	if(den == 0)
	{
		_div0(); // Error out here;
		return 0; 
	}
	while(((shifter & 0xF0000000) == 0) &&
			shifter < num)
	  shifter <<= 4;
	while(((shifter & 0x80000000) == 0) &&
			shifter < num)
	  shifter <<= 1;
	while(shifter >= den)
	{
		result <<= 1;
		if(num >= shifter)
		{
			num -= shifter;
			result |= 1;
		}
		shifter >>= 1;
	}
	*mod = num;
	return result;
}

unsigned int __aeabi_uidiv(unsigned int num, unsigned int den)
{
	unsigned int mod;
	return intdiv(num, den, &mod);
}

unsigned int __aeabi_uidivmod(unsigned int num, unsigned int den)
{
	unsigned int mod;
	intdiv(num, den, &mod);
	__asm__( "mov r1,%[value]" : : [value] "r" (mod));
	return 0;
}

int __aeabi_idiv(int num, int den)
{
	unsigned int mod, val;
	int sign = 0;
	if(num < 0)
	{
		sign=1;
		num = -num;
	}
	if(den < 0)
	{
		sign^=1;
		den = -den;
	}
	val = intdiv(num & 0x7FFFFFFF, den & 0x7FFFFFFF, &mod);
	if(sign)
		return -val;
	return val;
}

int __aeabi_idivmod(int num, int den)
{
	unsigned int mod;
	int sign = 0;
        if(num < 0)
        {
                sign=1;
                num = -num;
        }
        if(den < 0)
        {
                den = -den;
        }
	intdiv(num, den, &mod);
	if(sign)
		mod = -mod;
	__asm__( "mov r1,%[value]" : : [value] "r" (mod));
	return 0;// return wrong value
}

// 64 bit versions

static uint64 longdiv(uint64 num, uint64 den, uint64 *mod)
{
	unsigned long long result = 0, shifter=den;
	if(den == 0)
	{
		_div0(); // Error out here;
		return 0; 
	}
	while(((shifter & 0xFF00000000000000) == 0) &&
			shifter < num)
	  shifter <<= 8;
	while(((shifter & 0x8000000000000000) == 0) &&
			shifter < num)
	  shifter <<= 1;
	while(shifter >= den)
	{
		result <<= 1;
		if(num >= shifter)
		{
			num -= shifter;
			result |= 1;
		}
		shifter >>= 1;
	}
	*mod = num;
	return result;
}

uint64 __aeabi_uldiv(uint64 num, uint64 den)
{
	uint64 mod;
	return longdiv(num, den, &mod);
}

/* When returning a value it overwrites our r2, 
	so we have to take care of the return manually */
void __aeabi_uldivmod(uint64 num, uint64 den)
{
	uint64 mod, val;
	val = longdiv(num,den, &mod);
	__asm__( "ldrd r0,%[value]" : : [value] "m" (val));
	__asm__( "ldrd r2,%[value]" : : [value] "m" (mod));
}

int64 __aeabi_ldiv(int64 num, int64 den)
{ 
	uint64 mod, val;
	int sign = 0;
	if(num < 0)
	{
		sign=1;
		num = -num;
	}
	if(den < 0)
	{
		sign^=1;
		den = -den;
	}
	val = longdiv(num, den, &mod);
	if(sign)
		return -val;
	return val;
}

void __aeabi_ldivmod(int64 num, int64 den)
{
	uint64 mod, val;
	int signNum = 0, signDen = 0;
	if(num < 0)
	{
		signNum=1;
		num = -num;
	}
	if(den < 0)
	{
		signDen = 1;
		den = -den;
	}
	val = longdiv(num, den, &mod);
	if(signNum)
		mod = -mod;
	if(signNum^signDen)
		val = -val;
	__asm__( "ldrd r0,%[value]" : : [value] "m" (val));
	__asm__( "ldrd r2,%[value]" : : [value] "m" (mod));
}
