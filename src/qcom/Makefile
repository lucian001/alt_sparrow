CFLAGS = -g -O0 -DLINUX -I../BareMtl_app/
BINDIR ?= bin
QCOM ?= .

.PHONY: all
all: qcom ledset

$(BINDIR):
	mkdir $(BINDIR)

qcom: $(BINDIR)/qcom.o $(BINDIR)/main.o
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^

$(BINDIR)/main.o: main.c $(BINDIR)
	 $(CROSS_COMPILE)gcc -c $(CFLAGS) -o $@ $<

ledset: $(BINDIR)/qcom.o $(BINDIR)/ledset.o
	$(CROSS_COMPILE)gcc $(CFLAGS) -o $@ $^

$(BINDIR)/ledset.o: ledset.c $(BINDIR)
	 $(CROSS_COMPILE)gcc -c $(CFLAGS) -o $@ $< -I../linux_baremetal

$(BINDIR)/qcom.o: $(QCOM)/qcom.c $(BINDIR)
	 $(CROSS_COMPILE)gcc -c $(CFLAGS) -o $@ $< -DPHYS_MEM_ADDR=0x3de00000 -DMEM_SIZE=0x2000000

clean:
	rm -f $(BIN) $(BINDIR)
