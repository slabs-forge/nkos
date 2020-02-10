#
# NK MBR Makefile

.PHONY: all clean cleaner
.PRECIOUS: %.o

ROOT=..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

CFLAGS= -nostdinc -m32 -I$(INCDIR)

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

TARGETS=$(BASE)/boot.bin $(BASE)/boot.sym

all: $(TARGETS)

-include $(DEPENDS)

$(BASE)/%.bin: $(BASE)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BASE)/%.sym: $(BASE)/%.elf
	$(OBJCOPY) --only-keep-debug $< $@

$(BASE)/boot.elf: $(BASE)/boot.ld $(addprefix $(OBJDIR)/,$(OBJECTS))
	$(LD) -T $+ -o $@

$(DEPDIR)/%.d: $(BASE)/%.S | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(OBJDIR)/%.o: $(BASE)/%.S $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR):
	[ ! -d $@ ] && mkdir $@

clean:
	rm -f $(BASE)/*.elf
	rm -f $(TARGETS)
	rm -f $(OBJDIR)/*.o

cleaner: clean
	rm -f .dep/*.d

