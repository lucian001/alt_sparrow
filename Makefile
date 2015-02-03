SHELL := /bin/bash

ifeq ($(VERBOSE),)
SIL=@
endif

AS=$(CROSS_COMPILE)gcc
CC=$(CROSS_COMPILE)gcc
LD=$(CROSS_COMPILE)ld
MKDIR    ?=$(SIL)mkdir -p
OBJ_DUMP ?=$(CROSS_COMPILE)objdump 
OBJ_COPY ?=$(CROSS_COMPILE)objcopy
PYTHON   ?=python
ifdef SYSTEMROOT
	MKIMAGE  ?=mkimage.exe
else
	MKIMAGE  ?=PATH=$(PATH):prebuilt mkimage
endif


# Directories, etc
SRC_DIR  ?= src/sparrow
QCOM_DIR ?= src/qcom
GEN_DIR  ?= gen/$(PROJ)
PROJ_DIR ?= src/$(PROJ)
BIN_DIR  ?= bin/$(PROJ)
LINUX_BIN_DIR := $(BIN_DIR).linux
HWLIB_DIR ?= hwlib
INCLUDE_DIRS = -I $(SRC_DIR) -I $(QCOM_DIR) -I $(HWLIB_DIR)/include -I $(GEN_DIR) $(EXTRA_INCLUDES)
PYSCRIPT ?= $(SRC_DIR)/Sparrow.py
PROJSCRIPT ?= $(PROJ_DIR)/layout.py
GEN_DEPS ?= $(GEN_DIR) $(PY_FILES) $(PYSCRIPT)

# Flags
PROCESSOR=-march=armv7-a -mtune=cortex-a9 -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -marm 
CCFLAGS = $(PROCESSOR) -c $(INCLUDE_DIRS) -DARM -g -fno-builtin -std=c99
ASFLAGS = $(CCFLAGS) $(INCLUDE_DIRS) -DARM -DASM
LDFLAGS = -nostdlib
CCFLAGS += -DALT_INT_PROVISION_VECTOR_SUPPORT=0
LINUX_CFLAGS = -g -O0 -DLINUX 

.PHONY: all
all: baremetal linux_baremetal linux_uCos2 baremetal_dual uCos2_uCos2
#bin/sparrow mage.txt install

objs =  $(BIN_DIR)/startup.o \
	$(BIN_DIR)/reset.o \
	$(BIN_DIR)/resetS.o \
	$(BIN_DIR)/divide.o \
	$(BIN_DIR)/uart.o \
	$(BIN_DIR)/log_buffer.o \
	$(BIN_DIR)/alt_printf.o \
	$(BIN_DIR)/alt_mmu.o \
	$(BIN_DIR)/scu.o \
	$(BIN_DIR)/scuS.o \
	$(BIN_DIR)/mem.o \
	$(BIN_DIR)/mempool.o \
	$(BIN_DIR)/gic.o \
	$(BIN_DIR)/gicS.o \
	$(BIN_DIR)/alt_interrupt.o \
	$(BIN_DIR)/cache.o \
	$(BIN_DIR)/alt_cache.o \
	$(BIN_DIR)/cacheS.o \
	$(BIN_DIR)/trustzone.o \
	$(BIN_DIR)/trustzoneS.o \
	$(BIN_DIR)/Layout.o \
	$(BIN_DIR)/neon.o

##########################################################################################################
#		Generic Rules
##########################################################################################################
#	Note that startup.o MUST be the first one listed!

$(BIN_DIR)/%.o: $(SRC_DIR)/%.S $(GEN_DIR)/Layout.h
	$(SIL)$(AS) $(ASFLAGS) -o $@ $<

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c $(GEN_DIR)/Layout.h
	$(SIL)$(CC) $(CCFLAGS) -o $@ $<

$(BIN_DIR)/%.o: $(GEN_DIR)/%.c $(GEN_DIR)/Layout.h
	$(SIL)$(CC) $(CCFLAGS) -o $@ $<

$(BIN_DIR)/%.o: $(HWLIB_DIR)/src/hwmgr/%.c
	$(SIL)$(CC) $(CCFLAGS) -o $@ $<

$(BIN_DIR)/%.o: $(PROJ_DIR)/%.c $(GEN_DIR)/Layout.h
	$(SIL)$(CC) $(CCFLAGS) -o $@ $< 

$(BIN_DIR)/%.o: $(PROJ_DIR)/%.S $(GEN_DIR)/Layout.h
	$(SIL)$(AS) $(ASFLAGS) -o $@ $<

$(BIN_DIR) $(GEN_DIR):
	$(SIL)$(MKDIR) $@

# qcom files
$(BIN_DIR)/%.o: $(QCOM_DIR)/%.c
	$(SIL)$(CC) $(CCFLAGS) -c $< -o $@

clean:
	$(SIL)rm -rf bin $(GEN_DIR) src/Sparrow.pyo

.PHONY: projfiles
projfiles: $(BIN_DIR)/sparrow.bin $(BIN_DIR)/sparrow.txt $(BIN_DIR)/sparrow.map $(BIN_DIR)/u-boot.scr

.PHONY: dumpLogScript
dumpLogScript: $(BIN_DIR)/dumpLog.sh

.PHONY: dumpLog
dumpLog: $(LINUX_BIN_DIR)/dumpLog

########################################

CONFIG_FILE = $(PROJ_DIR)/layout.py
$(BIN_DIR)/sparrow.txt: $(BIN_DIR)/sparrow.axf
	$(SIL)$(OBJ_DUMP) -DS $< >$@

$(BIN_DIR)/sparrow.map: $(PROJ_DIR)/layout.py
	$(SIL)$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -p >$@

$(GEN_DIR)/boot.script: $(PROJ_DIR)/layout.py
	$(SIL)$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -boots $@

$(BIN_DIR)/u-boot.scr: $(GEN_DIR)/boot.script
	$(SIL)$(MKIMAGE) -A arm -O linux -T script -C none -a 0 -e 0 -n "bootscript" -d $< $@ > /dev/null

$(GEN_DIR)/Layout.c $(GEN_DIR)/Layout.h: $(GEN_DEPS) $(CONFIG_FILE)
	$(SIL)$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -C $(GEN_DIR)/Layout.c -H $(GEN_DIR)/Layout.h	

$(BIN_DIR)/sparrow.bin: $(BIN_DIR)/sparrow.axf $(CONFIG_FILE)
	$(SIL)$(OBJ_COPY) $< -O binary $(BIN_DIR)/sparrow.raw  && \
	$(MKIMAGE) -A arm -O linux -T kernel -C none \
		-a `$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Address'` \
		-e `$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'` \
		-d $(BIN_DIR)/sparrow.raw $@ > /dev/null && \
	chmod a+x $@
	@echo "*** Created $@ ***"

$(BIN_DIR)/dumpLog.sh: $(BIN_DIR)
	@echo "dumpLog `$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.OS1_s_Log_Buffer.Address'` \
		`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.OS1_s_Log_Buffer.Size'`" > $@
	@chmod u+x $@

$(LINUX_BIN_DIR)/dumpLog: src/qcom/dumpLog.c $(LINUX_BIN_DIR)
	$(SIL)$(CC) $(LINUX_CFLAGS) -o $@ $< \
		-DLOG_ADDRESS=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.OS1_s_Log_Buffer.Address'` \
		-DLOG_SIZE=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.OS1_s_Log_Buffer.Size'`

#########################################       Baremetal App

.PHONY: baremetal
baremetal:
	@make --no-print-directory PROJ=baremetal projfiles VERBOSE=$(VERBOSE)

# baremetal app files
objs_baremetal = \
	$(BIN_DIR)/startup.o \
	$(BIN_DIR)/divide.o \
	$(BIN_DIR)/uart.o \
	$(BIN_DIR)/log_buffer.o \
	$(BIN_DIR)/alt_printf.o \
	$(BIN_DIR)/alt_mmu.o \
	$(BIN_DIR)/mem.o \
	$(BIN_DIR)/mempool.o \
	$(BIN_DIR)/gic.o \
	$(BIN_DIR)/gicS.o \
	$(BIN_DIR)/alt_interrupt.o \
	$(BIN_DIR)/cache.o \
	$(BIN_DIR)/alt_cache.o \
	$(BIN_DIR)/cacheS.o \
	$(BIN_DIR)/Layout.o \
	$(BIN_DIR)/neon.o \
	$(BIN_DIR)/trustzone.o \
	$(BIN_DIR)/main.o \
	$(BIN_DIR)/baremetal_app.o \
	$(BIN_DIR)/alt_clock_manager.o \
	$(BIN_DIR)/alt_timers.o \
	$(BIN_DIR)/alt_globaltmr.o \
	$(BIN_DIR)/alt_watchdog.o

bin/baremetal/sparrow.axf: $(BIN_DIR) $(objs_baremetal)
	$(SIL)$(CC) $(LDFLAGS) -o $@ $(objs_baremetal)  \
		-Wl,-N,--build-id=none,-Ttext=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'` 

#########################################       Baremetal App

objs_baremetal_dual = $(objs_baremetal) \
	$(BIN_DIR)/scu.o \
	$(BIN_DIR)/scuS.o \
	$(BIN_DIR)/reset.o \
	$(BIN_DIR)/resetS.o 

.PHONY: baremetal_udal
baremetal_dual:
	@make --no-print-directory PROJ=baremetal_dual projfiles VERBOSE=$(VERBOSE)

bin/baremetal_dual/sparrow.axf: $(BIN_DIR) $(objs_baremetal_dual)
	$(SIL)$(CC) $(LDFLAGS) -o $@ $(objs_baremetal_dual)  \
		-Wl,-N,--build-id=none,-Ttext=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'` 

#########################################	AMP Linux/Baremetal App

$(LINUX_BIN_DIR):
	$(SIL)$(MKDIR) $@

.PHONY: ledset 
ledset: $(LINUX_BIN_DIR)/ledset

.PHONY: linux_baremetal
linux_baremetal:
	@make --no-print-directory PROJ=linux_baremetal VERBOSE=$(VERBOSE) projfiles dumpLogScript

# baremetal app files
objs_linux_baremetal = $(objs) \
		$(BIN_DIR)/qcom.o \
		$(BIN_DIR)/main.o \
		$(BIN_DIR)/baremetal_app.o \
		$(BIN_DIR)/alt_clock_manager.o \
		$(BIN_DIR)/alt_timers.o \
		$(BIN_DIR)/alt_globaltmr.o \
		$(BIN_DIR)/alt_watchdog.o

# baremetal side of qcome
$(BIN_DIR)/qcom.o: $(QCOM_DIR)/qcom.c
	$(SIL)$(CC) $(CCFLAGS) -o $@ $< \
		-DPHYS_MEM_ADDR=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
					-var 'sparrow.Shared_Comm_Channel.Address'` \
		-DMEM_SIZE=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
					-var 'sparrow.Shared_Comm_Channel.Size'`

# Linux app to talk with baremetal
$(LINUX_BIN_DIR)/qcom.o: $(QCOM_DIR)/qcom.c $(LINUX_BIN_DIR)
	$(SIL)$(CC) -c $(LINUX_CFLAGS) -o $@ $<

$(LINUX_BIN_DIR)/ledset.o: src/qcom/ledset.c $(LINUX_BIN_DIR)
	$(SIL)$(CC) -c $(LINUX_CFLAGS) -o $@ $< -Isrc/linux_baremetal \
                -DPHYS_MEM_ADDR=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.Shared_Comm_Channel.Address'` \
                -DMEM_SIZE=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) \
                                        -var 'Linux.Shared_Comm_Channel.Size'`

$(LINUX_BIN_DIR)/ledset: $(LINUX_BIN_DIR)/qcom.o $(LINUX_BIN_DIR)/ledset.o
	$(SIL)$(CC) $(CFLAGS) -o $@ $^

bin/linux_baremetal/sparrow.axf: $(BIN_DIR) $(objs_linux_baremetal)
	$(SIL)$(CC) $(LDFLAGS) -o $@ $(objs_linux_baremetal)  \
		-Wl,-N,--build-id=none,-Ttext=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'` 

#########################################	linux-uCos2

.PHONY: linux_uCos2
linux_uCos2:
	@make --no-print-directory PROJ=linux_uCos2 VERBOSE=$(VERBOSE) projfiles dumpLogScript

objs_linux_ucos2 = $(objs) \
		$(BIN_DIR)/main.o

bin/linux_uCos2/sparrow.axf: $(BIN_DIR) $(objs_linux_ucos2)
	$(SIL)$(CC) $(LDFLAGS) -o $@ $(objs_linux_ucos2)  \
		-Wl,-N,--build-id=none,-Ttext=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'` 

#########################################       linux-uCos2

.PHONY: uCos2_uCos2
uCos2_uCos2:
	@make --no-print-directory PROJ=uCos2_uCos2 VERBOSE=$(VERBOSE) projfiles

objs_ucos2_ucos2 = $(objs) \
	$(BIN_DIR)/main.o

bin/uCos2_uCos2/sparrow.axf: $(BIN_DIR) $(objs_ucos2_ucos2)
	$(SIL)$(CC) $(LDFLAGS) -o $@ $(objs_ucos2_ucos2)  \
		-Wl,-N,--build-id=none,-Ttext=`$(PYTHON) $(PYSCRIPT) $(PROJSCRIPT) -var 'sparrow.Executable.Start'`

