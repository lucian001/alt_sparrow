/*
 * Copyright Altera 2013
 * All Rights Reserved
 * File: hello.c
 *
 */

#include <stdio.h>
#include "alt_clock_manager.h"
#include "alt_fpga_manager.h"
#include "alt_bridge_manager.h"
#include "alt_interrupt.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_fpgamgr.h"

#if 0
ALT_STATUS_CODE test_config_full(void)
{
    // Verify the MSELs are appropriate for the type of image we're using.
    ALT_FPGA_CFG_MODE_t mode = alt_fpga_cfg_mode_get();
    switch (mode)
    {
    case ALT_FPGA_CFG_MODE_PP32_FAST_AESOPT_DC:
    case ALT_FPGA_CFG_MODE_PP32_SLOW_AESOPT_DC:
    case ALT_FPGA_CFG_MODE_PP16_FAST_AESOPT_DC:
    case ALT_FPGA_CFG_MODE_PP16_SLOW_AESOPT_DC:
        printf("INFO: MSEL configured correctly for FPGA image.\n");
        break;
    default:
        printf("ERROR: Incompatible MSEL mode set. Cannot continue with FPGA programming.\n");
        return ALT_E_ERROR;
    }

    // This is the symbol name for the SOF file contents linked in.
    extern char _binary_soc_system_dc_rbf_start;
    extern char _binary_soc_system_dc_rbf_end;

    // Use the above symbols to extract the FPGA image information.
    const char *   fpga_image      = &_binary_soc_system_dc_rbf_start;
    const uint32_t fpga_image_size = &_binary_soc_system_dc_rbf_end - &_binary_soc_system_dc_rbf_start;

    // Trace the FPGA image information.
    printf("INFO: FPGA Image binary at %p.\n", fpga_image);
    printf("INFO: FPGA Image size is %u bytes.\n", (unsigned int)fpga_image_size);

    // Try the full configuration a few times.
    const uint32_t full_config_retry = 5;
    for (uint32_t i = 0; i < full_config_retry; ++i)
    {
        ALT_STATUS_CODE status;
        status = alt_fpga_configure(fpga_image, fpga_image_size);
        if (status == ALT_E_SUCCESS)
        {
            printf("INFO: alt_fpga_configure() successful on the %u of %u retry(s).\n",
                   (unsigned int)(i + 1),
                   (unsigned int)full_config_retry);
            return ALT_E_SUCCESS;
        }
    }

    printf("ERROR: FPGA failed to program after %u attempt(s).\n", (unsigned int)full_config_retry);

    return ALT_E_ERROR;
}

#endif

ALT_STATUS_CODE test_bridge(void)
{
    // Overall status of the test
    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    
    // Attempt to bring up all bridges
    ALT_STATUS_CODE status1 = alt_bridge_init(ALT_BRIDGE_F2H,   NULL, NULL);
    ALT_STATUS_CODE status2 = alt_bridge_init(ALT_BRIDGE_H2F,   NULL, NULL);
    ALT_STATUS_CODE status3 = alt_bridge_init(ALT_BRIDGE_LWH2F, NULL, NULL);

    if (status1 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_init(ALT_BRIDGE_F2H) successful.\n");
    }
    else
    {
        printf("ERROR: alt_bridge_init(ALT_BRIDGE_F2H) failed; status = %d.\n", (int)status1);
        status = ALT_E_ERROR;
    }

    if (status2 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_init(ALT_BRIDGE_H2F) successful.\n");
    }
    else
    {
        printf("ERROR: alt_bridge_init(ALT_BRIDGE_H2F) failed; status = %d.\n", (int)status2);
        status = ALT_E_ERROR;
    }

    if (status3 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_init(ALT_BRIDGE_LWH2F) successful.\n");

        const uint32_t ALT_LWFPGA_BASE         = 0xFF200000;
        const uint32_t ALT_LWFPGA_SYSID_OFFSET = 0x00010000;
        const uint32_t ALT_LWFPGA_LED_OFFSET   = 0x00010040;

        // Attempt to read the system ID peripheral
        uint32_t sysid = alt_read_word(ALT_LWFPGA_BASE + ALT_LWFPGA_SYSID_OFFSET);
        printf("INFO: LWFPGA Slave => System ID Peripheral value = 0x%x.\n", (unsigned int)sysid);

        // Attempt to toggle the 4 LEDs
        const uint32_t bits = 4;
        printf("INFO: Toggling LEDs ...\n");
        for (uint32_t i = 0; i < (1 << bits); ++i)
        {
            // Use Gray code ... for fun!
            // http://en.wikipedia.org/wiki/Gray_code
            uint32_t gray = (i >> 1) ^ i;

            alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_LED_OFFSET, gray);

            printf("INFO: Gray code(i=0x%x) => 0x%x [", (unsigned int)i, (unsigned int)gray);

            for (uint32_t j = 0; j < bits; ++j)
            {
                printf("%c", (gray & (1 << (bits - j - 1))) ? '1' : '0');
            }

            printf("].\n");
        }

        // Reset the LEDs to all on
        alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_LED_OFFSET, 0);
    }
    else
    {
        printf("ERROR: alt_bridge_init(ALT_BRIDGE_LWH2F) failed; status = %d.\n", (int)status3);
        status = ALT_E_ERROR;
    }

    /*
    ///// [Yuan] : Comment out below code for the following interrupt test

    if (status1 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_uninit(ALT_BRIDGE_F2H) cleanup.\n");
        alt_bridge_uninit(ALT_BRIDGE_F2H, NULL, NULL);
    }
    if (status2 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_uninit(ALT_BRIDGE_H2F) cleanup.\n");
        alt_bridge_uninit(ALT_BRIDGE_H2F, NULL, NULL);
    }
    if (status3 == ALT_E_SUCCESS)
    {
        printf("INFO: alt_bridge_uninit(ALT_BRIDGE_LWH2F) cleanup.\n");
        alt_bridge_uninit(ALT_BRIDGE_LWH2F, NULL, NULL);
    }
	*/

    return status;
}

/********************************************************************/
// [Yuan] : Add interrupt test based on button_pio here
/********************************************************************/
//
// Initializes and enables the interrupt controller.
//
ALT_STATUS_CODE socfpga_int_start(void)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    //
    // Initialize the global and CPU interrupt items
    //

    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_global_init();
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_int_global_init() failed, %d.\n", (int)status);
        }
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_cpu_init();
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_int_cpu_init() failed, %d.\n", (int)status);
        }
    }

    //
    // Enable the CPU and global interrupt
    //

    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_cpu_enable();
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_int_cpu_enable() failed, %d.\n", (int)status);
        }
    }

    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_global_enable();
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_int_global_enable() failed, %d.\n", (int)status);
        }
    }

    return status;
}

void socfpga_int_stop(void)
{
    //
    // Disable the global, CPU, and distributor interrupt
    //

    if (alt_int_global_disable() != ALT_E_SUCCESS)
    {
        printf("WARN: alt_int_global_disable() return non-SUCCESS.");
    }

    if (alt_int_cpu_disable() != ALT_E_SUCCESS)
    {
        printf("WARN: alt_int_cpu_disable() return non-SUCCESS.");
    }

    //
    // Uninitialize the CPU and global data structures.
	//

	if (alt_int_cpu_uninit() != ALT_E_SUCCESS)
    {
        printf("WARN: alt_int_cpu_uninit() return non-SUCCESS.\n");
    }

    if (alt_int_global_uninit() != ALT_E_SUCCESS)
    {
        printf("WARN: alt_int_global_uninit() return non-SUCCESS.\n");
    }
}

static void alt_fpga_pio_button_int_callback(uint32_t icciar, void * context)
{
    const uint32_t ALT_LWFPGA_BASE         	= 0xFF200000;
    const uint32_t ALT_LWFPGA_LED_OFFSET   = 0x00010040;
    const uint32_t ALT_LWFPGA_BUTTON_OFFSET_EDGECAPTURE	= 0x000100CC;

    //Only port 0 and port 1 are available on Altera Cyclye V GSRD board
    uint32_t pio_edgecapture = alt_read_word(ALT_LWFPGA_BASE + ALT_LWFPGA_BUTTON_OFFSET_EDGECAPTURE) & 0x3;

	printf("INFO: alt_fpga_pio_button_int_callback()\n");

    switch (pio_edgecapture)
    {
    	//"Enable bit-clearing for edge capture register" is turn on in Qsys
        //Clear the bit in edgecapture register firstly to prevent the continuous interrupt generation
    	case 0x01 :
    		alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_BUTTON_OFFSET_EDGECAPTURE, 0x01);

            // Reset the LEDs to all on
            alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_LED_OFFSET, 0);

    		break;

    	case 0x2 :
    		alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_BUTTON_OFFSET_EDGECAPTURE, 0x2);

            // Reset the LEDs to all off
            alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_LED_OFFSET, 0xF);

    		break;

    	case 0x3 :
    		alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_BUTTON_OFFSET_EDGECAPTURE, 0x3);
    		break;
    }
}

ALT_STATUS_CODE alt_fpga_pio_button_init(ALT_INT_INTERRUPT_t int_id, uint32_t int_target)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    // Enable interrupts for the button PIO ports 0 and 1.
    if (status == ALT_E_SUCCESS)
    {
        const uint32_t ALT_LWFPGA_BASE         	= 0xFF200000;
        const uint32_t ALT_LWFPGA_BUTTON_OFFSET_INTERRUPTMASK	= 0x000100C8;

    	alt_write_word(ALT_LWFPGA_BASE + ALT_LWFPGA_BUTTON_OFFSET_INTERRUPTMASK, 0x3);
    }

    // Register our ISR
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_isr_register(int_id, alt_fpga_pio_button_int_callback, NULL);
    }

    if (   (status == ALT_E_SUCCESS)  && (int_id >= 32)) // Ignore target_set() for non-SPI interrupts.
    {
        if (int_target == 0)
        {
            int_target = (1 << alt_int_util_cpu_count()) - 1;
        }
        status = alt_int_dist_target_set(int_id, int_target);
    }

    // Enable interrupt
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_dist_enable(int_id);
    }

    return status;
}

ALT_STATUS_CODE test_interrupts(void)
{
    // Overall status of the test
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    //
    // Start the interrupt system
    //
    if (status == ALT_E_SUCCESS)
    {
        status = socfpga_int_start();
    }

#define CORE_0      (0)
#define CORE_1      (1)

    if (status == ALT_E_SUCCESS)
    {
        status = alt_fpga_pio_button_init(ALT_INT_INTERRUPT_F2S_FPGA_IRQ1, CORE_0);
    }

    if (status == ALT_E_SUCCESS)
    {
    	printf("INFO : The interrupt initialization for FPGA Button PIO finished.\n");
    }
    else
    {
        printf("ERROR : The interrupt initialization for FPGA Button PIO failed!\n");
    }

    return status;
}

int hwlib_main(int argc, char** argv)
{
    // Verify power is on
    if (alt_fpga_state_get() == ALT_FPGA_STATE_POWER_OFF)
    {
        printf("ERROR: FPGA Monitor reports FPGA is powered off.\n");
        return 1;
    }

    /////

    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    printf("INFO: alt_fpga_control_enable().\n");
    status = alt_fpga_control_enable();

    if (status == ALT_E_SUCCESS)
    {
//        status = test_config_full();
    }

    if (status == ALT_E_SUCCESS)
    {
        status = test_bridge();
    }

    if (status == ALT_E_SUCCESS)
    {
        status = test_interrupts();
    }


    for (;;);

    printf("INFO: alt_fpga_control_disable().\n");
    alt_fpga_control_disable();

    if (status == ALT_E_SUCCESS)
    {
        printf("RESULT: All tests successful.\n");
        return 0;
    }
    else
    {
        printf("RESULT: Some failures detected.\n.");
        return 1;
    }
}
