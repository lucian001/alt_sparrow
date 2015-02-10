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

#####################			 Sample Code for AMP
setDevice("cyclone5")
setPhysicalMemory(1*Gb)

linux = Context("Linux", Secure=False, CreateMap=False, cpuNum=0)
linux2 = Context("Linux2", Secure=False, CreateMap=False, cpuNum=1)

MemoryRegion(
	Name    = "Linux's MR",
	Address = 0,
	MapTo	= [ Map(linux)],
	LoadFiles = [File(0x100, "socfpga_cyclone5.dtb", "dtb"), 
			File(0x7fc0, "uImage", "uimage", linux, 0x40)])

MemoryRegion(
	Name = "Memory for Baremetal App",
	Size = 1*Mb,
	MapTo = [Map(sparrow, AddrMatch=True)],
	LoadFiles = [File(0x0, "sparrow.bin", "sparrow", sparrow, 0x40)],
	MemFor = cpu0)

linux.addComponents(
                privateInts + globalTimer + legacyFIQ + privateTimer + legacyNIRQ + 
		cpu0_parityfail + cpu1_parityfail +
		scu_parityfail + l2_ecc + ddr_ecc_error + 
		fpga + fpga_mngr +
		dma + usb0 + usb1 +
                can0 + can1 +
                sdmmc + nand + qspi + spis + i2cs +
		emac0 + emac1 +
                uart0 + uart1 +
		gpios +
                sptimer0 + sptimer1 + osctimer0 +
		osctimer1 +
                watchDogs +
                clkmgr + mpuwakeup + CoreSight + OCR + acp + rom + sdram + dap + 
		reset + sysmngr + scanmngr + stmslave)

