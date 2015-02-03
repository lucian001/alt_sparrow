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

serverip  = "137.57.188.22"
ethaddr   = "00:11:22:33:02:23"
ipaddr    = "137.57.188.3"
scriptextra = None
scriptbootextra = ""

def set_scriptextra(value):
	global scriptextra
	scriptextra = value

def set_scriptbootextra(value):
	global scriptbootextra
	scriptbootextra = value

import sys
import os

kb = 1024
page = 4*kb
Mb = 1024*kb
Gb = 1024*Mb
Tb = 1024*Gb

allContexts = {}
cpus = {}
stacks = {}
tzstacks = {}
cpu0 = 0
cpu1 = 1
numCPUs = 2
allFiles = []

class OverlapException(Exception):
	def __init__(self, region1, region2):
			self.regions = [region1, region2]
	def __str__(self):
		ret = "Error, Memory region overlap " 
		for item in self.regions:
			ret = ret + str(item) + ", "
		return ret

class MapMismatchException(Exception):
	def __str__(self):
		return "Cannot mix mapped and unmapped memory in a singular context"

class NoMemMapException(Exception):
	def __init__(self, context):
		self.context = context
	def __str__(self):
		return "Cannot create map for context " + self.context.Name + \
			" because it does not have a pool of memory to allocate the tables for it"

class NoContextExcpetion(Exception):
	def __str__(self):
		return "Error, no context defined for Component"

def appendunique(list1, list2):
	for a in list2:
		if a not in list1:
			list1.append(a)

def stripname(name):
	name = name.replace(' ', '_')
	name = name.replace('\'', '_')
	return name

class Context:
	def __init__(self, Name, Secure, CreateMap, cpuNum=None):
		self.interrupts = []
		self.Mapped = CreateMap
		self.Name = stripname(Name)
		self.cpuNum = cpuNum
		self.Secure = Secure
		self.MappedRegions = {}
		self.Components = []
		self.Stack = None
		self.MemPool = None
		self.varspace = {}
		allContexts[self.Name] = self
		if cpuNum is not None:
			self.setCPU(cpuNum)
		self.Executable = None
	def __str__(self):
		retval = ""
		if self is System:
			retval += "System.Max_Memory = 0x%08x\n" %self.Max_Memory 
		retval += \
		self.Name + ".interrupts\n" + \
		self.Name + ".Mapped = " + str(self.Mapped) + "\n" + \
		self.Name + ".cpuNum\n" + \
		self.Name + ".Secure = " + str(self.Secure) + "\n" + \
		self.Name + ".Components[" + str(len(self.Components)) + "]\n" + \
		self.Name + ".Stack\n"
		if self.MemPool is not None:
			retval += self.Name + ".MemPool #MR of TLB Memory\n"
		retval += self.Name + ".Executable #MR where the executable binary is loaded and executed from\n"
		for a in self.MappedRegions:
			retval += self.Name + "." + a + "\n"
		return retval
	def setMapped(self, b_mapped):
		self.Mapped = b_mapped 
	def setCPU(self, cpuNum):
		self.cpuNum = cpuNum
		cpus[cpuNum] = self
	def addComponent(self, device):
		self.Components.append(device)
		self.interrupts += device.Interrupts
	def addComponents(self, devices):
		for device in devices:
			self.addComponent(device)
	def addRegion(self, _map):
		region = _map.Region
		#Add to our list
		_map.varname = region.varname
		count = 0
		while _map.varname in self.MappedRegions:
			#print("Error- region '%s' already defined in %s\n" %(region.Name, self.Name))
			count += 1
			_map.varname = region.varname + "_" + str(count)
		self.MappedRegions[_map.varname] = _map
		self.__dict__.update({_map.varname: _map})
		if self.Mapped and \
			not _map.Address is None:
			if not (self.MapOverlap(_map) is None):
				print("virt Addrs: %08x, %08x" %(_map.Address, oldmap.Address))
				raise OverlapException(region, oldmap.Region)
	def MapOverlap(self, _map):
		for name, oldmap in self.MappedRegions.items():
			if oldmap is _map:
				continue
			oldregion = oldmap.Region
			if not (oldmap.Address is None) and \
			   not (oldmap.Region.Size is None) and \
			   oldmap.Address + oldregion.Size > _map.Address and \
			   oldmap.Address < _map.Address + _map.Region.Size:
				return oldmap
		return None
	def allocateRegions(self):
		if self is System:
			# Allocate Regions that should exist, but don't
			# Start with TLBs for the MMU
			for context in allContexts.itervalues():
				if context.Mapped and \
				  context.MemPool is None:
					MemoryRegion(Name  = "TLB for " + context.Name,
						Size    = 1*Mb,
						MapTo   = [Map(context, NoCache=True, \
							AddrMatch=True, Device=True)],
						MemPoolFor = context)
			# Need space in memory for Uboot so we don't accidently overwrite it
			if not "cpuuboot_init" in System.varspace:
				ubootSize = 1*Mb
				MemoryRegion(
					Name = "Memory for Uboot",
					Size = ubootSize,
					MapTo = [Map(uboot)],
					Address = System.Max_Memory - ubootSize)
			# If they didn't define a place for Sparrow
			if sparrow.Executable is None:
				MemoryRegion(
					Name = "Load Address for Sparrow",
					Size = 1*Mb,
					MapTo = [Map(sparrow, AddrMatch=True)],
					LoadFiles = [File(0x0, "sparrow.bin", "sparrow", sparrow, 0x40)],
					MemFor = cpu0)
			# Now allocate stacks for each CPU that will be started
			for a in range(0, numCPUs):
				if not a in stacks and ((a in cpus) or (a == 0)):
					#we have to have a stack for this cpu
					MemoryRegion(
						Name = "Stack for cpu " + str(a),
						Size = 1*Mb,
						MapTo = [Map(sparrow, AddrMatch=True)],
						StackFor = a)
				if a in cpus and not cpus[a].Secure and not a in tzstacks:
					#we need a stack for SMC calls
					MemoryRegion(
						Name = "Stack for cpu0 TrustZone",
						Size = 1*Mb,
						MapTo = [Map(sparrow)],
						TZStackFor = a)
			# Any regions that don't have a defined size get left over space	
			for map in self.MappedRegions.values():
				region = map.Region
				if region.Size is None:
					if region.Address is None:
						print "Error: MemoryRegion must have either Size or Address set"
						raise Exception()
					Limit = System.Max_Memory
					unallocated = 0
					for map2 in self.MappedRegions.values():
						region2 = map2.Region
						if region2.Address < Limit and \
							region2.Address > region.Address:
							Limit = region2.Address
						if region2.Address is None and \
							region2.Size is not None:
							#print("unalloc -> %s" %region2.Name)
							unallocated += region2.Size
					region.Size = Limit - region.Address - unallocated
					if region.Size <= 0:
						raise Exception()
					#print("Allocated 0x%x bytes for %s" %(region.Size, region.Name))
		for name, _map in self.MappedRegions.items():
			if _map.Address is None: 
				if _map.AddrMatch or not self.Mapped:
					_map.Address = _map.Region.Address
				else:
					#Allocate first available space for this
					_map.Address = 0
					while True:
						overlap = self.MapOverlap(_map)
						if overlap is None:
							break;
						_map.Address = overlap.Address + overlap.Region.Size
					if _map.Address + _map.Region.Size > System.Max_Memory:
						print("Error, Ran out of SDRam when allocating %s" %_map.Region.Name)
						raise Exception
					if self is System:
						_map.Region.Address = long(_map.Address)
			_map.Size = _map.Region.Size
		if self is System:
			for fi in allFiles:
				fi.Address = fi.MR.Address + fi.LoadOffset
				fi.Start = fi.Address + fi.ExecOffset
	# allocateRegions must be called before this
	def printself(self):
		print("Map for " + self.Name + ":")
		if self.Mapped:
			print("        Virtual Range      :  Physical")
			for name, map in sorted(self.MappedRegions.items(), key=lambda(k,v): v.Address):
				addr = map.Address
				region = map.Region
				print("  [0x%08x - 0x%08x]: 0x%08x  %s" \
					%(addr, addr + region.Size-1, region.Address, region.Name))
		else:
			print("  Phys Addr : Size")
			for name, map in sorted(self.MappedRegions.items(), key=lambda(k,v): v.Address):
				addr = map.Address
				region = map.Region
				print("  [0x%08x: 0x%08x]  %s" 
					%(region.Address, region.Size, region.Name))
		if len(self.Components) > 0:
			print("  Interrupts:")
			interrupts = {}
			for device in self.Components:
				for intcounter in device.Interrupts:
					interrupts[intcounter] = device.Name
			for intcounter, name in sorted(interrupts.items()):
				print("    %d (0x%x)  %s" %(intcounter, intcounter, name))
		print("==============================\n")

	def createH(self, f):
		for name, _map in self.MappedRegions.items():
			region = _map.Region
			f.write('#define %s_%s_Address\t0x%08x\n' \
				%(self.Name, name, region.Address))
			f.write('#define %s_%s_Size   \t0x%08x\n' \
				%(self.Name, name, region.Size))
			f.write('#define %s_%s_AddrEnd\t0x%08x\n' \
				%(self.Name, name, region.Address + region.Size))
			if self.Mapped and not (self is System):
				f.write('#define %s_%s_MapTo  \t0x%08x\n' \
					%(self.Name, name, _map.Addr()))
		if self.Executable is not None:
			f.write('#define %s_Executable_Address\t(0x%08x + %s_%s_Address)\n' \
				%(self.Name, self.Executable.LoadOffset + self.Executable.ExecOffset,
					self.Name, self.Executable.MR.varname))
		if self.Stack is not None:
			f.write('#define %s_Stack_End	%s_%s_AddrEnd\n' \
				%(self.Name, self.Name, self.Stack.varname))
		for device in self.Components:
			names = [""] + range(1,len(device.Interrupts))
			for count, intcounter in enumerate(device.Interrupts):
				f.write('#define %s_Interrupt%s\t%d // 0x%x\n' %(device.varname, names[count], intcounter, intcounter))
	def writeMap(self, f):
		if self.MemPool is None:
			print("Error - %s is mapped, but no mempool specified" %self.Name)
			return;
		f.write('uint32_t *tb_%s = NULL;\n\n' %self.Name)
		f.write('uint32_t * Map_%s(void)\n{\n' %self.Name)
		f.write('  MemPool pool;\n')
		f.write('  if(tb_%s == NULL)\n' %self.Name)
		f.write('  {\n')
		f.write('    MemPoolInit(&pool, \n')
		f.write('\t%s_%s_Address, //0x%08x\n' \
			%(self.Name, self.MemPool.varname, self.MemPool.Address))
		f.write('\t%s_%s_Size);   //0x%08x\n' \
			%(self.Name, self.MemPool.varname, self.MemPool.Size))
		f.write('    alt_mmu_va_space_create(&tb_%s, %s_regions, sizeof(%s_regions)/sizeof(%s_regions[0]), MemPoolAlloc, &pool);\n' %(self.Name, self.Name, self.Name, self.Name))
		f.write('  }\n')
		f.write('  alt_mmu_DACR_set(g_domain_ap, 16);\n')
		f.write('  alt_mmu_TTBCR_set(true, false, 0);\n')
		f.write('  alt_mmu_TTBR0_set(tb_%s);\n' %self.Name)
		f.write('  alt_mmu_enable();\n')
		f.write('  return tb_%s;\n' %self.Name)
		f.write('}\n\n')
	def mapSystemH(self, filename_h):
		f = open(filename_h, 'w')
		f.write("#ifndef LAYOUT_H\n")
		f.write("#define LAYOUT_H\n")
		f.write("\n#define System_MaxMem	0x%08x\n" %System.Max_Memory)
		self.createH(f)
		for context in allContexts.itervalues():
			context.createH(f)
		for a in stacks:
			f.write("#define cpu%s_Stack_End\t0x%08x\n" %(str(a), stacks[a].Address + stacks[a].Size))
		for a in tzstacks:
			f.write("#define cpu%s_TZ_Stack_End\t0x%08x\n" %(str(a), tzstacks[a].Address + tzstacks[a].Size))
		f.write('\n#ifndef ASM\n')
		f.write('#include "core.h"\n')
		f.write('#include "gic.h"\n')
		f.write('#include "alt_mmu.h"\n')
		f.write("void SetupTZMemAccess();\n");
		f.write("void SetupTZInterrupts();\n");
		f.write("void SetupTZPeripherals();\n\n")
		for num, context in cpus.items():
			f.write("void cpu%d_OS_Start();\n" %num)
		for context in allContexts.itervalues():
			if (context is not System) and context.Mapped:
				f.write('uint32_t * Map_%s(void);\n' %context.Name)
				f.write('static ALT_MMU_MEM_REGION_t %s_regions[] = {\n' %context.Name)
				#	This will only work on 1Mb Entries for now
				for name, map in sorted(context.MappedRegions.items(), key=lambda(k,v): v.Addr()):
					region = map.Region
					mapAddr=map.Addr()
					f.write('\t{(void *)%s_%s_MapTo,   //0x%08x\n' \
						%(context.Name, name, mapAddr))
					f.write('\t (void *)%s_%s_Address, //0x%08x\n'
						%(context.Name, name, region.Address))
					f.write('\t %s_%s_Size,    //0x%08x\n' \
						%(context.Name, name, region.Size))
					f.write('\t ALT_MMU_AP_FULL_ACCESS,\n\t ')
					if map.Device is True:
						f.write("ALT_MMU_ATTR_DEVICE,\n")
					else:
						if map.NoCache is True:
							f.write("ALT_MMU_ATTR_NC_NC,\n")
						else:
							f.write("ALT_MMU_ATTR_WBA_WBA,\n")
					if map.Shared is True:
						f.write('\t ALT_MMU_TTB_S_SHAREABLE,\n')
					else:
						f.write('\t ALT_MMU_TTB_S_NON_SHAREABLE,\n')
					f.write('\t ALT_MMU_TTB_XN_DISABLE,\n')
					if context.Secure and region.hasNonsecure:
						f.write('\t ALT_MMU_TTB_NS_NON_SECURE},\n')
					else:
						f.write('\t ALT_MMU_TTB_NS_SECURE},\n')
				f.write('\t};\n\n')
		#Setup the Trustzone Interrupt Access
		nonsecureints=[]
		secureints=[]
		for context in allContexts.itervalues():
			if context.Secure:
				appendunique(secureints, context.interrupts)
			else:
				appendunique(nonsecureints, context.interrupts)
		secureints.sort()
		nonsecureints.sort();
		if nonsecureints:
			f.write("static int NSInterrupts[] = {");
			for a in nonsecureints:
				f.write("%d, " %a)
			f.write("};\n")
			f.write("#define NSIntCount (sizeof(NSInterrupts)/sizeof(NSInterrupts[0]))\n")
		f.write("static int InterruptTargets[] = {");
		for a in range(0,256):
			if a % 8 is 0:
				f.write("\n/* %03d */  " %a)
			foundOne = False
			for id, context in cpus.items():
				if a in context.interrupts:
					if foundOne:
						f.write(" | ")
					f.write("GIC_CPUID_" + str(id))
					foundOne = True
			if not foundOne:
				f.write("0");
			if a is not 255:
				f.write(", ")
		f.write("};\n");
		f.write("\n")
		f.write('#endif\n\n')
		f.write("#endif //ifndef LAYOUT_H\n")
		f.close()

	def mapSystem(self, filename): # Creates the Layout.c file
		f = open(filename, 'w')
		f.write('#include "core.h"\n');
		f.write('#include "trustzone.h"\n')
		f.write('#include "mempool.h"\n')
		f.write('#include "gic.h"\n')
		f.write('#include "cache.h"\n')
		f.write('#include "mem.h"\n')
		f.write('#include "alt_interrupt.h"\n')
		f.write('#include "Layout.h"\n\n')
		f.write("uint32 cpu_stack_addrs[%d] = { cpu0_Stack_End" %numCPUs)
		for a in range(1, numCPUs):
			if a in stacks:
				f.write(", cpu%d_Stack_End" %a)
			else:
				f.write(", 0")
		f.write("};\n\n")

		# Setup the Trustzone Memory Access
		f.write("void SetupTZMemAccess()\n{\n")
		# Step 1 - Set default rules
		f.write("  /* Set the default rule to permissive until we have written all the rules */\n")
		f.write("  alt_sdr_ctl_set_tz_default(ALT_SDR_CTL_PROTPORT_DENY_NONE);\n")
		self.ruleCount=0
		memRegionsSecure = []
		memRegionsNonsecure = []
		memRegionsBoth = []
		# Step 2 - Figure out which os's are TZ Secure and what memory they can access
		# Step 3 - Figure out which os's are TZ Nonsecure and what memory they can access
		for context in allContexts.itervalues():
			if context.Secure:
				context.consolidateMemRegions(memRegionsSecure)
			else:
				context.consolidateMemRegions(memRegionsNonsecure)
		# Set aside regions that are the same for both Secure and NonSecure
		for l1 in reversed(memRegionsSecure):
			for l2 in reversed(memRegionsNonsecure):
				if l1 == l2:
					memRegionsBoth.append(l1)
					memRegionsSecure.remove(l1)
					memRegionsNonsecure.remove(l1)
		self.ruleCount+=1
		if 0 != len(memRegionsSecure):
			f.write("// Secure Memory\n")
		#for counter in range(len(memRegionsSecure)):
		#	self.writeRegion(f, memRegionsSecure[counter], "ALT_SDR_CTL_DATA_ACCESS_SECURE")
		f.write("  alt_sdr_ctl_set_tz_rule(ALT_SDR_CTL_RULE_NEW, 0, 0x%08x, ALT_SDR_CTL_RULEID_MIN, ALT_SDR_CTL_RULEID_MAX,ALT_SDR_CTL_DATA_ACCESS_SECURE, ALT_SDR_CTL_DATA_RULE_ALL_PORTS, ALT_SDR_CTL_DATA_ALLOW_ACCESS);" %System.Max_Memory)
		if 0 != len(memRegionsBoth):
			f.write("// Shared Memory\n")
		for counter in range(len(memRegionsBoth)):
			self.writeRegion(f, memRegionsBoth[counter], "ALT_SDR_CTL_DATA_ACCESS_BOTH")
		if 0 != len(memRegionsNonsecure):
			f.write("// Nonsecure Memory\n")
		for counter in range(len(memRegionsNonsecure)):
			self.writeRegion(f, memRegionsNonsecure[counter], "ALT_SDR_CTL_DATA_ACCESS_NONSECURE")
		f.write("\n  /* Set the default rule to deny access */\n")
		f.write("  alt_sdr_ctl_set_tz_default(ALT_SDR_CTL_PROTPORT_DENY_CPU); // Deny access to cpus\n");
		f.write("}\n\n");
		f.write("void SetupTZInterrupts()\n{\n");
		f.write("/* All Interrupts are Secure by default */\n");
		f.write("  int a;\n")
		f.write("#if defined(NSIntCount)\n")
		f.write("  for(a=0;a<sizeof(NSInterrupts);a++)\n")
		f.write("  {\n    alt_int_dist_secure_disable(NSInterrupts[a]);\n  }\n")
		f.write("#endif\n")
		f.write("  for(a=0;a<sizeof(InterruptTargets);a++)\n")
		f.write("  {\n    alt_int_dist_target_set(a, InterruptTargets[a]);\n  }\n")
		f.write("}\n\n")
		
		f.write("void SetupTZPeripherals()\n{\n")
		f.write("  alt_l3_secgrp_set_peripheral_access(")
		allNSComponents = []
		for context in allContexts.itervalues():
			if not context.Secure:
				allNSComponents += context.Components
		numPeripherals = 0
		for device in allNSComponents:
			if device.SetNSString is not None:
				if numPeripherals != 0:
					f.write(" | ")
				f.write(device.SetNSString)
				numPeripherals+=1
				if (numPeripherals % 4) == 3:
					f.write("\n    ")
		if numPeripherals == 0:
			f.write("0");
		f.write(", 0);\n")
		#f.write("  *scu_ns_p = scu_ns;\n");
		f.write("}\n\n")
		for num, context in cpus.items():
			StartAddrOffset = 0
			if context.Executable is not None:
				StartAddrOffset = context.Executable.MR.Address + context.Executable.LoadOffset + context.Executable.ExecOffset
				f.write("void cpu%d_OS_Start()\n{\n" %num)
				if not context.Secure:
					f.write("\tstart_nonsecure(%s_Executable_Address, cpu%d_TZ_Stack_End);\n}\n"
						%(context.Name, num))
				else:
					f.write("\tstart_secure(%s_Executable_Address);\n}\n" 
						%(context.Name))
		for context in allContexts.itervalues():
			if context.Mapped is True:
				context.writeMap(f)
		f.close()
	def writeRegion(self, f, memRgn, SecureString):
		if memRgn[0] > System.Max_Memory:
			# This isn't SDRAM
			return
		if memRgn[1] > System.Max_Memory:
			# Part of it is SDRam
			print("Error - %s spans across SDRam/Non-SDRam barrier" %self.Name)
			print("%x -> %x" %(memRgn[0], memRgn[1]))
			raise Exception
		if self.ruleCount == 19: # Max Number of Rules
			print("Error - too many memory rules required for mapping\n")
			raise Exception
		f.write("  alt_sdr_ctl_set_tz_rule(ALT_SDR_CTL_RULE_NEW, 0x%08x, 0x%08x, ALT_SDR_CTL_RULEID_MIN, ALT_SDR_CTL_RULEID_MAX," %(memRgn[0], memRgn[1]))
		f.write(SecureString + ", ALT_SDR_CTL_DATA_RULE_ALL_PORTS, ALT_SDR_CTL_DATA_ALLOW_ACCESS);\n")
		self.ruleCount+=1
	def consolidateMemRegions(self, memRegions):
		for name, newmap in self.MappedRegions.items():
			newmapStart = newmap.Region.Address & ~(Mb-1)
			newmapEnd = (newmap.Region.Address + newmap.Region.Size + Mb-1) & ~(Mb-1)
			#print("nme= %x->%x" %(newmap.Region.Address +  newmap.Region.Size, newmapEnd))
			#Check for appendage
			Found = False
			for counter in range(len(memRegions)):
				if newmap.Region.Address < memRegions[counter][0]:
					if newmapEnd >= memRegions[counter][0]:
						memRegions[counter][0] = newmapStart
					else:
						memRegions.insert(counter, [newmapStart, newmapEnd])
					Found = True
				if newmapStart <= memRegions[counter][1]:
					if memRegions[counter][1] <= newmapEnd:
						memRegions[counter][1] = newmapEnd
					Found = True
				if Found:
					break
			#if no appendage, add to list
			if not Found:
				memRegions.append([newmapStart, newmapEnd])

class MemoryRegion:
	def __init__(self, Name, Size=None, MapTo=[], Address=None, LoadFiles=[],
			MemPoolFor=None, StackFor=None, MemFor=None, TZStackFor=None):
		if not Address is None:
			Address = long(Address)
		if Size is not None:
			Size = long(Size)
		self.StackFor = StackFor
		self.varname = stripname(Name)
		self.Name = Name
		self.Address = Address
		self.Size = Size
		self.MappedTo = MapTo
		self.MemPoolFor = MemPoolFor
		self.hasNonsecure = False
		self.LoadFiles = LoadFiles
		for fi in LoadFiles:
			fi.MR = self
		if Address is not None and \
			Size is not None and \
			Address < System.Max_Memory & Address + Size > System.Max_Memory:
			print("Error, Memory Region %s spans the sdram/non-sdram barrier" %Name)
			raise Exception
		newmap = Map(System, Address=self.Address)
		newmap.Region = self
		System.addRegion(newmap)		
		for maps in MapTo:
			maps.Region = self
			maps.Context.addRegion(maps) 
			if maps.Context.Secure == False:
				self.hasNonsecure = True
		if MemPoolFor is not None:
			MemPoolFor.MemPool = self
		if TZStackFor is not None:
			if not isinstance(TZStackFor, (int,long)):
				print("Error, TZStackFor paramter should be a CPU number")
				raise Exception
			tzstacks[TZStackFor] = self
		if StackFor is not None:
			if not isinstance(StackFor, (int,long)):
				print("Error, StackFor paramter should be a CPU number")
				raise Exception
			stacks[StackFor] = self
		if MemFor is not None:
			System.varspace["cpu" + str(MemFor) + "_init"] = self
	def __str__(self):
		str = ""
		if self.Name is not None:
			str = self.Name
		return str + "[0x%08x:0x%08x]" %(self.Address, self.Size)
class Component:
	def __init__(self, Name, Interrupts=[], SetNSString=None):
		self.varname = stripname(Name)
		self.Name = Name
		self.Interrupts = Interrupts
		self.SetNSString=SetNSString
	def __str__(self):
		str = ""
		if self.Name is not None:
			str = self.Name
		return str

class Map:
	def __init__(self, Context, Address=None, Shared=False, NoCache=False,\
			 Device=False, AddrMatch=False):
		self.Context = Context
		self.Address = Address
		self.Shared = Shared
		self.NoCache = NoCache
		self.Device = Device
		self.AddrMatch = AddrMatch
		self.Size = 0
	def __str__(self):
		retval  = '.Address = 0x%08x\n' %self.Address
		retval += '.Size = 0x%08x\n' %self.Size
		retval += '.Context = ' + self.Context.Name + "\n"
		retval += '.Shared = ' + str(self.Shared) + "\n"
		retval += '.NoCache = ' + str(self.NoCache) + "\n"
		retval += '.Device = ' + str(self.Device) + "\n"
		retval += '.Region = ' + str(self.Region) + "\n"
		retval += '.AddrMatch = ' + str(self.AddrMatch) + "\n"
		return retval
	def Addr(self):
		if self.Address is None:
			return self.Region.Address
		return self.Address

class File:
	def __init__(self, LoadOffset, Filename, Name, ExecutableFor=None, ExecutableOffset=0):
		self.LoadOffset = LoadOffset
		self.Filename = Filename
		self.Name = Name
		self.ExecutableFor = ExecutableFor
		if ExecutableFor is not None:
			ExecutableFor.Executable = self
		self.ExecOffset = ExecutableOffset
		allFiles.append(self)


System = Context("System", True, CreateMap=True)
del allContexts["System"]
System.Max_Memory = 4*Gb
sparrow = Context("sparrow", Secure=True, CreateMap=False)
uboot = Context("uboot", Secure=False, CreateMap=False)

def setPhysicalMemory(size):
	global System
	System.Max_Memory = size

deviceFiles = {	"cyclone5":"cyclone5.py",
		"arria5":"cyclone5.py"}

def setDevice(devName):
	scriptdir = os.path.dirname(os.path.realpath(sys.argv[0]))
	if not devName in deviceFiles:
		print("Error, no match for device: " + devName)
		return
	lfile = open(scriptdir + "/" + deviceFiles[devName])
	f = lfile.read()
	exec(f, globals())
	lfile.close()

# -C {Filename} = Create C and H files for a context
# -H {Filename} = Create just H file for a context
# -p <Contexts> 		  = Print System Memory Map followed by each Context
# -var {VarName}= Prints out the Value of {context}.{VarName}

def parseOptions(commands):
	scriptfile = None
	valid = 0
	opts = {'-C':{}, '-H':{}, '-p':{}, '-var':{}, '-boots':{}}
	current = None
	for opt in commands:
		if opt in opts:
			current = opt
			opts[current][valid] = True
		else:
			if current is None:
				scriptfile = opt
			if current is not None:
				opts[current][len(opts[current])] = opt
	if scriptfile is None:
		print("Error, no script specified");
		exit()
	lfile = open(scriptfile,'r')
	f = lfile.read()
	exec(f)
	lfile.close()
	System.allocateRegions()
	for cont in allContexts.itervalues():
		cont.allocateRegions()
	if 1 in opts['-C']:
		System.mapSystem(opts['-C'][1])
	if 1 in opts['-H']:
		System.mapSystemH(opts['-H'][1])
	if 1 in opts['-boots']:
		createBootScript(opts['-boots'][1]);
	if valid in opts['-p']:
		System.printself()
		if len(opts['-p']) == 1:
			for a in allContexts:
				if a != System:
					allContexts[a].printself()
		for count in range(1, len(opts['-p'])):
			allContexts[opts['-p'][count]].printself()
	if valid in opts['-var']:
		if not 1 in opts['-var']:
			#No variable specified, print the list of available vars
			for c_name, context in [("System", System)] + allContexts.items():
				print(c_name)
		nameSpace = allContexts.copy()
		nameSpace["System"] = System
		for count in range(1,len(opts['-var'])):
			try:
				value = eval(opts['-var'][count], nameSpace)
			except:
				print("Error: could not find " + opts['-var'][count])
				exit()
			if isinstance(value, bool):
				print value
			elif isinstance(value, (int,long)):
				print("0x%08x" %value)
			elif isinstance(value, list):
				for a in value:
					print a
			else:
				print(value)

try:
	lfile = open("config.py")
	f = lfile.read()
	exec(f, globals())
	lfile.close()
except IOError:
	pass

setenvcmd = 'setenv bootargs console=ttyS0,115200 root=${mmcroot} rw rootwait'

def createBootScript(filename):
	f = open(filename, 'w')
	if serverip is not None:
		f.write('set serverip "%s"\n' %serverip)
	if ethaddr is not None:
		f.write('set ethaddr "%s"\n' %ethaddr)
	if ipaddr is not None:
		f.write('set ipaddr "%s"\n' %ipaddr)
	if scriptextra is not None:
		f.write(scriptextra + '\n')
	f.write('\n')

	for fi in allFiles:
		f.write('set %saddr "0x%x"\n' %(fi.Name, fi.Address))
		if fi.ExecutableFor is not None:
			f.write('set %sstart "0x%x"\n' %(fi.Name, fi.Address + fi.ExecOffset))
		f.write("set tf%s 'tftp ${%saddr} %s'\n" 
			%(fi.Name, fi.Name, fi.Filename))
		f.write("set mc%s '${mmcloadcmd} mmc 0:${mmcloadpart} ${%saddr} %s'\n" 
			%(fi.Name, fi.Name, fi.Filename))
		f.write('\n')
	f.write("set mmcload 'mmc rescan;")

	for fi in allFiles:
		f.write("run mc%s;" %fi.Name)
	f.write("'\n")

	f.write("set tfload '")
	for fi in allFiles:
		f.write("run tf%s;" %fi.Name)
	f.write("'\n\n")

	hasdtb = False
	for fi in allFiles:
		if fi.Name == "dtb":
			hasdtb = True
	zimage = False
	linuxSize = 0
	for fi in allFiles:
		if fi.ExecutableFor is not None and \
		   fi.ExecutableFor.Name is "Linux":
			linuxSize = fi.MR.Size
			if fi.Filename[0] is 'z':
				zimage = True
			break
	if hasdtb:
		if zimage:
			f.write("set mmcboot '%s;%s mem=0x%x; bootz ${sparrowaddr} - ${dtbaddr}'\n" 
				%(scriptbootextra, setenvcmd, linuxSize))
		else:
			f.write("set mmcboot '%s;%s mem=0x%x; bootm ${sparrowaddr} - ${dtbaddr}'\n" 
				%(scriptbootextra, setenvcmd, linuxSize))
		
	else:
		f.write("set mmcboot '%s;go ${sparrowstart}'\n" %(scriptbootextra))
	f.close()

parseOptions(sys.argv)	
