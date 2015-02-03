/*
Copyright (c) 2013-2014, Altera Corporation.  All rights reserved.

Redistribution and use of this software, in source and binary code forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of Altera Corporation nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

4. This software may only be used to run on Altera products, or to program Altera devices.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE COPYRIGHT HOLDERS OR CONTRIBUTORS ARE ADVISED OF THE POSSIBILITY DAMAGE IS LIKELY TO OCCUR.
 */

    /* standard includes */
#include    <stdint.h>
#include "core.h"
#include "mem.h"

    /* SoCAL layer includes*/
#include    "socal/hps.h"
#include    "socal/socal.h"
#include    "socal/alt_rstmgr.h"
#include    "socal/alt_gpio.h"
#include    "socal/alt_tmr.h"

    /* Hardware Libraries includes */
#include    "alt_clock_manager.h"
#include    "alt_timers.h"
#include    "alt_globaltmr.h"
#include    "alt_watchdog.h"
#include    "alt_mpu_registers.h"


#define SPTMR		0
#define OSCTMR1		1
#define OSCTMR0		2
#define PVTTMR		3

    /* some timer & LWED enables */
#define ENABLE_PVTTMR   true
#define ENABLE_OSC1TMR0 true
#define ENABLE_OSC1TMR1 true
#define ENABLE_SPTMR0   true


/* Some useful definitions */
#define     ALT_AMP_DEMO_MPU1_LED3_PS       0x00

#define     ALT_AMP_DEMO_MPU1_LED0_TMOUT_DEFAULT    0x03211234

#define     ALT_AMP_DEMO_MPU1_LED1_TMOUT_DEFAULT    0x0126F080

        // should be 1/2 second
#define     ALT_AMP_DEMO_MPU1_LED2_TMOUT_DEFAULT    0x02FAF080

        // ARM private timer prescaler
#define     ALT_AMP_DEMO_MPU1_LED3_TMOUT_DEFAULT    0x5F5E100 // One Second


uint32_t ALT_AMP_DEMO_MPU1_LED_TMOUT_DEFAULT[4] = {
                           ALT_AMP_DEMO_MPU1_LED0_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED1_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED2_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED3_TMOUT_DEFAULT};
uint32_t ALT_AMP_DEMO_MPU1_LED_TMOUT[4] = {
                           ALT_AMP_DEMO_MPU1_LED0_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED1_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED2_TMOUT_DEFAULT,
                           ALT_AMP_DEMO_MPU1_LED3_TMOUT_DEFAULT};
uint32_t TimerCounts[4] = {0,0,0,0};




#define     ALT_AMP_DEMO_MPU1_LED0_MSK      0x00008000
        // should be GPIO41 = HPS LED0
#define     ALT_AMP_DEMO_MPU1_LED1_MSK      0x00004000
        // should be GPIO41 = HPS LED1
#define     ALT_AMP_DEMO_MPU1_LED2_MSK      0x00002000
        // should be GPIO41 = HPS LED2
#define     ALT_AMP_DEMO_MPU1_LED3_MSK      0x00001000
        // should be GPIO41 = HPS LED3

#define     alt_toggle_LED0()               alt_xorbits_word(ALT_GPIO1_SWPORTA_DR_ADDR, ALT_AMP_DEMO_MPU1_LED0_MSK)
#define     alt_toggle_LED1()               alt_xorbits_word(ALT_GPIO1_SWPORTA_DR_ADDR, ALT_AMP_DEMO_MPU1_LED1_MSK)
#define     alt_toggle_LED2()               alt_xorbits_word(ALT_GPIO1_SWPORTA_DR_ADDR, ALT_AMP_DEMO_MPU1_LED2_MSK)
#define     alt_toggle_LED3()               alt_xorbits_word(ALT_GPIO1_SWPORTA_DR_ADDR, ALT_AMP_DEMO_MPU1_LED3_MSK)
#define     alt_toggle_all_LEDs()           alt_xorbits_word(ALT_GPIO1_SWPORTA_DR_ADDR, ALT_AMP_DEMO_MPU1_LED0_MSK \
                                            | ALT_AMP_DEMO_MPU1_LED1_MSK | ALT_AMP_DEMO_MPU1_LED2_MSK \
                                            | ALT_AMP_DEMO_MPU1_LED3_MSK)



int32_t alt_amp_baremetal_get_pll_grp(ALT_CLK_GRP_t pllgrp, uint32_t * configptr, uint32_t bytecnt)
{
    // for demo purposes, get the values in the clock manager registers
    // and return them

    ALT_CLK_GROUP_RAW_CFG_t     clk_group_raw_cfg;
    ALT_STATUS_CODE             childret;
    int32_t                     ret = -1;

    if ((configptr != NULL) && (sizeof(ALT_CLK_GROUP_RAW_CFG_t) <= bytecnt))
    {
        childret = alt_clk_group_cfg_raw_get(pllgrp, &clk_group_raw_cfg);
        if (childret == ALT_E_SUCCESS)
        {
            if (sizeof(ALT_CLK_GROUP_RAW_CFG_t) < bytecnt)
            {
                bytecnt = sizeof(ALT_CLK_GROUP_RAW_CFG_t);
            }
            if(memcpy(configptr, &clk_group_raw_cfg, bytecnt) == 0)
            {
                ret = bytecnt;
            }
        }
    }
    return ret;
}



int32_t alt_amp_baremetal_get_pll_config(uint32_t * configptr, int32_t bytecnt)
{
    int32_t     ret = -1;


    if (((uint32_t) bytecnt) >= (3 * sizeof(ALT_CLK_GROUP_RAW_CFG_t)))
    {
        ret = alt_amp_baremetal_get_pll_grp(ALT_MAIN_PLL_CLK_GRP, configptr, (uint32_t) bytecnt);
        if ((ret > 0) && ((bytecnt - ret) > 0))
        {
            ret = bytecnt - ret;
            configptr += ret;
            ret = alt_amp_baremetal_get_pll_grp(ALT_PERIPH_PLL_CLK_GRP, configptr, (uint32_t) bytecnt);
            if ((ret > 0) && ((bytecnt - ret) > 0))
            {
                ret = bytecnt - ret;
                configptr += ret;
                ret = alt_amp_baremetal_get_pll_grp(ALT_SDRAM_PLL_CLK_GRP, configptr, (uint32_t) bytecnt);
                if ((ret > 0) && ((bytecnt - ret) >= 0))
                {
                    //ret = 0;
                    ret = 3 * sizeof(ALT_CLK_GROUP_RAW_CFG_t);
                }
            }
        }
    }
    return ret;
}


int32_t alt_amp_baremetal_reset_mpu0(void)
{
    uint32_t tmp, i = 64;

    // put cpu0 into reset mode
    alt_setbits_word(ALT_RSTMGR_MPUMODRST_ADDR, ALT_RSTMGR_MPUMODRST_CPU0_SET_MSK);
    while (i--)
    {
        tmp = alt_read_word(ALT_RSTMGR_MPUMODRST_ADDR);
        // minimum 64 osc1_clk cycles delay is necessary between each
        // change of this field to keep the proper reset/clkoff sequence
    }

    // and release it
    alt_clrbits_word(ALT_RSTMGR_MPUMODRST_ADDR, ALT_RSTMGR_MPUMODRST_CPU0_SET_MSK);
    return 0;

}



int32_t alt_amp_MPU1_pvttmr_setup(void)
{
#if ENABLE_PVTTMR
    // return the local ARM private timer to its reset state programmatically

        // disable timer and its interrupts
    alt_clrbits_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_CTRL_REG_OFFSET,
            CPU_PRIV_TMR_ENABLE | CPU_PRIV_TMR_INT_EN);
        // reset load and counter registers
    alt_write_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_LOAD_REG_OFFSET, 0);
        // clear any pending interrupts
    alt_write_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_INT_STATUS_REG_OFFSET, CPU_PRIV_TMR_INT_STATUS);
        // now write zeros to the control register significant bitfields
    alt_clrbits_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_CTRL_REG_OFFSET,
            (CPU_PRIV_TMR_ENABLE | CPU_PRIV_TMR_AUTO_RELOAD | CPU_PRIV_TMR_INT_EN | CPU_PRIV_TMR_PS_MASK));

        // set counter register
    alt_write_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_LOAD_REG_OFFSET, ALT_AMP_DEMO_MPU1_LED_TMOUT[PVTTMR]);
        // for polled version
    alt_setbits_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_CTRL_REG_OFFSET,
            (CPU_PRIV_TMR_AUTO_RELOAD | (ALT_AMP_DEMO_MPU1_LED3_PS << CPU_PRIV_TMR_PS_SHIFT)));
    alt_setbits_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_CTRL_REG_OFFSET, CPU_PRIV_TMR_ENABLE);

 /*     // for interrupt version
    alt_setbits_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_CTRL_REG_OFFSET, (CPU_PRIV_TMR_AUTO_RELOAD
            | CPU_PRIV_TMR_INT_EN | (ALT_AMP_DEMO_MPU1_LED3_PS << CPU_PRIV_TMR_PS_SHIFT)));
*/
#endif
    return 0;
}



int32_t alt_amp_MPU1_osc1tmr0_setup(void)
{
#if  ENABLE_OSC1TMR0
    ALT_STATUS_CODE ret = ALT_E_ERROR;
    uint32_t temp;

    alt_clrbits_word(ALT_RSTMGR_PERMODRST_ADDR, ALT_RSTMGR_PERMODRST_OSC1TMR0_SET_MSK);
    ret = alt_clk_freq_get(ALT_CLK_L3_MP, &temp);
    if (ret == ALT_E_SUCCESS)
    {
        printf("OSC1 Input Frequency = %u\n", (unsigned)temp);
        ret = alt_gpt_int_disable(ALT_GPT_OSC1_TMR0);
        ret = alt_gpt_counter_set(ALT_GPT_OSC1_TMR0, ALT_AMP_DEMO_MPU1_LED_TMOUT[OSCTMR0]);
        ret = alt_gpt_mode_set(ALT_GPT_OSC1_TMR0, ALT_GPT_RESTART_MODE_PERIODIC);
        temp = alt_gpt_time_millisecs_get(ALT_GPT_OSC1_TMR0);
        printf("OSC1 Timer 0 triggers each %u mS\n", (unsigned)temp);
        //ret = alt_gpt_int_enable(ALT_GPT_OSC1_TMR0);
        ret = alt_gpt_tmr_start(ALT_GPT_OSC1_TMR0);
    }
#endif
    return 0;
}


int32_t alt_amp_MPU1_osc1tmr1_setup(void)
{
#if  ENABLE_OSC1TMR1
    ALT_STATUS_CODE ret = ALT_E_ERROR;
    uint32_t temp;

    alt_clrbits_word(ALT_RSTMGR_PERMODRST_ADDR, ALT_RSTMGR_PERMODRST_OSC1TMR1_SET_MSK);
    ret = alt_clk_freq_get(ALT_CLK_L3_MP, &temp);
    if (ret == ALT_E_SUCCESS)
    {
        printf("OSC1 Input Frequency = %u\n", (unsigned)temp);
        ret = alt_gpt_int_disable(ALT_GPT_OSC1_TMR1);
        ret = alt_gpt_counter_set(ALT_GPT_OSC1_TMR1, ALT_AMP_DEMO_MPU1_LED_TMOUT[OSCTMR1]);
        ret = alt_gpt_mode_set(ALT_GPT_OSC1_TMR1, ALT_GPT_RESTART_MODE_PERIODIC);
        temp = alt_gpt_time_millisecs_get(ALT_GPT_OSC1_TMR1);
        printf("OSC1 Timer 1 triggers each %u mS\n", (unsigned)temp);
        //ret = alt_gpt_int_enable(ALT_GPT_OSC1_TMR1);
        ret = alt_gpt_tmr_start(ALT_GPT_OSC1_TMR1);
    }
#endif
    return 0;
}


int32_t alt_amp_MPU1_sptmr0_setup(void)
{
#if  ENABLE_SPTMR0
    ALT_STATUS_CODE ret = ALT_E_ERROR;
    uint32_t temp;

    alt_clrbits_word(ALT_RSTMGR_PERMODRST_ADDR, ALT_RSTMGR_PERMODRST_SPTMR0_SET_MSK);
    ret = alt_clk_freq_get(ALT_CLK_L4_SP, &temp);
    if (ret == ALT_E_SUCCESS)
    {
        printf("SP Timer Input Frequency = %u\n", (unsigned)temp);
        ret = alt_gpt_int_disable(ALT_GPT_SP_TMR0);
        ret = alt_gpt_counter_set(ALT_GPT_SP_TMR0, ALT_AMP_DEMO_MPU1_LED_TMOUT[SPTMR]);
        ret = alt_gpt_mode_set(ALT_GPT_SP_TMR0, ALT_GPT_RESTART_MODE_PERIODIC);
        temp = alt_gpt_time_millisecs_get(ALT_GPT_SP_TMR0);
        printf("SP Timer 0 triggers each %u mS\n", (unsigned)temp);
        //ret = alt_gpt_int_enable(ALT_GPT_OSC1_TMR1);
        ret = alt_gpt_tmr_start(ALT_GPT_SP_TMR0);
    }
#endif
    return 0;
}


int32_t alt_amp_MPU1_gpio_setup(void)
{
    alt_setbits_word(ALT_GPIO1_SWPORTA_DR_ADDR,
            (ALT_AMP_DEMO_MPU1_LED0_MSK | ALT_AMP_DEMO_MPU1_LED1_MSK |
            ALT_AMP_DEMO_MPU1_LED2_MSK  | ALT_AMP_DEMO_MPU1_LED3_MSK));
            // which turns LEDs off
    alt_setbits_word(ALT_GPIO1_SWPORTA_DDR_ADDR,
            (ALT_AMP_DEMO_MPU1_LED0_MSK | ALT_AMP_DEMO_MPU1_LED1_MSK |
            ALT_AMP_DEMO_MPU1_LED2_MSK  | ALT_AMP_DEMO_MPU1_LED3_MSK));
    return 0;
}

bool alt_amp_prvtmr_update(void)
{
    bool ret = 0;
#if ENABLE_PVTTMR
    uint32_t    temp;

    temp = alt_read_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_INT_STATUS_REG_OFFSET);
    ret = temp & CPU_PRIV_TMR_INT_STATUS;

    if (ret)
    {
	TimerCounts[PVTTMR]++;
        alt_toggle_LED3();

        // clear any pending notifications
        alt_write_word(CPU_PRIVATE_TMR_BASE + CPU_PRIV_TMR_INT_STATUS_REG_OFFSET, CPU_PRIV_TMR_INT_STATUS);
    }
#endif
    return ret;
}

bool alt_amp_osc1tmr0_update(void)
{
    bool ret = 0;
#if  ENABLE_OSC1TMR0
    ALT_STATUS_CODE rtn;
    ret = alt_read_word(ALT_OSC1TMR0_TMRSRAWINTSTAT_ADDR) & ALT_TMR_TMRSINTSTAT_TMRSINTSTAT_SET_MSK;

    if (ret)
    {
	TimerCounts[OSCTMR0]++;
        alt_toggle_LED2();
        rtn = alt_gpt_int_clear_pending(ALT_GPT_OSC1_TMR0);
    }
#endif
    return ret;
}

bool alt_amp_osc1tmr1_update(void)
{
    bool ret = 0;
#if  ENABLE_OSC1TMR1
    ALT_STATUS_CODE rtn;

    ret = alt_read_word(ALT_OSC1TMR1_TMRSRAWINTSTAT_ADDR) & ALT_TMR_TMRSINTSTAT_TMRSINTSTAT_SET_MSK;

    if (ret)
    {
	TimerCounts[OSCTMR1]++;
        alt_toggle_LED1();
        rtn = alt_gpt_int_clear_pending(ALT_GPT_OSC1_TMR1);
    }
#endif
    return ret;
}


bool alt_amp_sptmr0_update(void)
{
    bool ret = 0;
#if  ENABLE_SPTMR0
    ALT_STATUS_CODE rtn;

    ret = alt_read_word(ALT_SPTMR0_TMRSRAWINTSTAT_ADDR) & ALT_TMR_TMRSINTSTAT_TMRSINTSTAT_SET_MSK;

    if (ret)
    {
	TimerCounts[SPTMR]++;
        alt_toggle_LED0();
        rtn = alt_gpt_int_clear_pending(ALT_GPT_SP_TMR0);
    }
#endif
    return ret;
}


void main_loop()
{
    uint32_t    exit = 0;

    while (exit == 0)
    {
        // check for any timeout from global timer
        alt_amp_osc1tmr0_update();
        // check for any timeout from cpu1 private timer
        alt_amp_prvtmr_update();
        alt_amp_osc1tmr1_update();
        alt_amp_sptmr0_update();
    }
}

void main(void)
{
    // initialize ports necessary
    // set up interrupts for reading messages & sensing inputs from FPGA
    // set up timer to trigger @ 1 sec - use to toggle LED
    TimerCounts[0] = TimerCounts[1] = TimerCounts[2] = TimerCounts[3] = 0;

    // setup gpio to toggle LED
    alt_amp_MPU1_gpio_setup();
    alt_gpt_int_disable(ALT_GPT_OSC1_TMR0);
    alt_gpt_tmr_stop(ALT_GPT_OSC1_TMR0);

    // start the private timer
    alt_amp_MPU1_pvttmr_setup();
    alt_amp_MPU1_osc1tmr0_setup();
    alt_amp_MPU1_osc1tmr1_setup();
    alt_amp_MPU1_sptmr0_setup();

    main_loop();
}

