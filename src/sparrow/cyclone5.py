#/******************************************************************************
#Copyright (c) 2013, Altera Corporation
#All rights reserved.
#
#Redistribution and use in source and binary forms, with or without
#modification, are permitted provided that the following conditions are met:
#
#Redistributions of source code must retain the above copyright notice, this
#list of conditions and the following disclaimer.
#
#Redistributions in binary form must reproduce the above copyright notice, this
#list of conditions and the following disclaimer in the documentation and/or
#other materials provided with the distribution.
#
#Neither the name of the Altera Corporation nor the names of its contributors
#may be used to endorse or promote products derived from this software without
#specific prior written permission.
#
#THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#******************************************************************************/

privateInts 	= [Component("privateInts", Interrupts = range(0,16))]
globalTimer 	= [Component("globalTimer", Interrupts = [27],
			SetNSString="ALT_L3_SEC_PERIPHERAL_GLOBALTIMER")]
legacyFIQ   	= [Component("legacyFIQ", Interrupts = [28])]
privateTimer 	= [Component("privateTimer", Interrupts = [29],
			SetNSString="ALT_L3_SEC_PERIPHERAL_PRIVATETIMER")]
legacyNIRQ  	= [Component("legacyNIRQ", Interrupts = [31])]
cpu0_parityfail = [Component("cpu0_parityfail", Interrupts = range(33,48))]
cpu1_parityfail = [Component("cpu1_parityfail", Interrupts = range(48,64))]
scu_parityfail 	= [Component("scu_parityfail", Interrupts = [64,65,66])]
l2_ecc	 	= [Component("l2_ecc", Interrupts = range(67,71))]
ddr_ecc_error 	= [Component("ddr_ecc_error", Interrupts = [71])]
fpga 		= [Component("fpga", Interrupts = range(72, 136), 
			SetNSString="ALT_L3_SEC_PERIPHERAL_FPGA")]
fpga_mngr 	= [Component("fpga manager", Interrupts = [207],
			SetNSString="ALT_L3_SEC_PERIPHERAL_FPGAMNGR")]
dma 		= [Component("dma", Interrupts = range(136,147),
			SetNSString="ALT_L3_SEC_PERIPHERAL_DMA")]
usb0 		= [Component("usb0", 
			Interrupts = [157,158,159], 
			SetNSString="ALT_L3_SEC_PERIPHERAL_USB0")] 
usb1 		= [Component("usb1", 
			Interrupts = [160,161,162], 
			SetNSString="ALT_L3_SEC_PERIPHERAL_USB1")]
can0 		= [Component("can0", Interrupts = [163,164,165,166],
			SetNSString="ALT_L3_SEC_PERIPHERAL_CAN0")]
can1 		= [Component("can1", Interrupts = [167,168,169,170],
			SetNSString="ALT_L3_SEC_PERIPHERAL_CAN1")]
sdmmc 		= [Component("sdmmc", Interrupts = range(171,176),
			SetNSString="ALT_L3_SEC_PERIPHERAL_SDMMC")]
nand 		= [Component("nand", Interrupts = range(176,183), 
			SetNSString="ALT_L3_SEC_PERIPHERAL_NAND")]
qspi 		= [Component("qspi", Interrupts = [183,184,185], 
			SetNSString="ALT_L3_SEC_PERIPHERAL_QSPI")]
spi0_slave 	= [Component("spi0", Interrupts = [186], 
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPI0_SLAVE")]
spi1_slave 	= [Component("spi1", Interrupts = [187],
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPI1_SLAVE")]
spi0_master 	= [Component("spi2", Interrupts = [188],
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPI0_MASTER")]
spi1_master 	= [Component("spi3", Interrupts = [189],
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPI1_MASTER")]
spis 	= spi0_slave + spi1_slave + spi0_master + spi1_master

i2c0 		= [Component("i2c0", Interrupts = [190],
			SetNSString="ALT_L3_SEC_PERIPHERAL_I2C0")]
i2c1 		= [Component("i2c1", Interrupts = [191],
			SetNSString="ALT_L3_SEC_PERIPHERAL_I2C1")]
i2c2 		= [Component("i2c2 emac0", Interrupts = [192],
			SetNSString="ALT_L3_SEC_PERIPHERAL_I2C2")]
i2c3 		= [Component("i2c3 emac1", Interrupts = [193],
			SetNSString="ALT_L3_SEC_PERIPHERAL_I2C3")]
i2cs 	= i2c0 + i2c1 + i2c2 + i2c3

emac0 		= [Component("emac0", Interrupts = range(147,152),
			SetNSString="ALT_L3_SEC_PERIPHERAL_EMAC0")]
emac1 		= [Component("emac1", Interrupts = range(152,157),
			SetNSString="ALT_L3_SEC_PERIPHERAL_EMAC1")]

uart0 		= [Component("uart0", Interrupts = [194],
			SetNSString="ALT_L3_SEC_PERIPHERAL_UART0")]
uart1 		= [Component("uart1", Interrupts = [195],
			SetNSString="ALT_L3_SEC_PERIPHERAL_UART1")]
gpio0 		= [Component("gpio0", Interrupts = [196],
			SetNSString="ALT_L3_SEC_PERIPHERAL_GPIO0")]
gpio1 		= [Component("gpio1", Interrupts = [197],
			SetNSString="ALT_L3_SEC_PERIPHERAL_GPIO1")]
gpio2 		= [Component("gpio2", Interrupts = [198],
			SetNSString="ALT_L3_SEC_PERIPHERAL_GPIO2")]
gpios 	= gpio0 + gpio1 + gpio2

sptimer0 	= [Component("sptimer0", Interrupts = [199],
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPTIMER0")]
sptimer1 	= [Component("sptimer1", Interrupts = [200],
			SetNSString="ALT_L3_SEC_PERIPHERAL_SPTIMER1")]
osctimer0 	= [Component("osctimer0", Interrupts = [201],
			SetNSString="ALT_L3_SEC_PERIPHERAL_OSCTIMER0")]
osctimer1 	= [Component("osctimer1", Interrupts = [202],
			SetNSString="ALT_L3_SEC_PERIPHERAL_OSCTIMER1")]
timers 		= sptimer0 + sptimer1 + osctimer0 + osctimer1

watchDog 	= [Component("watchDog", Interrupts = [30])]
watchdog0 	= [Component("watchdog0", Interrupts = [203],
			SetNSString="ALT_L3_SEC_PERIPHERAL_WATCHDOG0")]
watchdog1 	= [Component("watchdog1", Interrupts = [204],
			SetNSString="ALT_L3_SEC_PERIPHERAL_WATCHDOG1")]
watchDogs 	= watchDog + watchdog0 + watchdog1

clkmgr 		= [Component("clkmgr", Interrupts = [205],
			SetNSString="ALT_L3_SEC_PERIPHERAL_CLKMGR")]
mpuwakeup 	= [Component("mpuwakeup", Interrupts = [206])]
CoreSight 	= [Component("CoreSight", Interrupts = [208,209])]
OCR 		= [Component("OCR_ecc", Interrupts = [210,211],
			SetNSString="ALT_L3_SEC_PERIPHERAL_OCR")]
acp 		= [Component("mpu acp", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_ACP")]
rom 		= [Component("rom", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_ROM")]
sdram 		= [Component("SDRam Registers", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_SDRAMCTRL")]
dap 		= [Component("Dap", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_DAP")]
reset 		= [Component("Reset Manager", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_RESET")]
sysmngr 	= [Component("System Manager", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_SYSMGR")]
scanmngr 	= [Component("Scan Manager", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_SCANMGR")]
stmslave 	= [Component("STM Peripheral Slave", 
			SetNSString="ALT_L3_SEC_PERIPHERAL_STMSLAVE")]

