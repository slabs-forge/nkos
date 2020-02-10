#------------------------------------------------------------------------------
# Kernel
#
# Author	: Sebastien LAURENT
# Date		: 17/09/2011
#

SUBDIRS= boot utl mm

.PHONY: clean cleaner all $(SUBDIRS)

vpath %.a $(SUBDIRS)

ROOT=..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

LIBS= -lkrn-utl -lkrn-mm
MODS=

OPTS=
#OPTS=
#OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions
CFLAGS= -nostdinc -fno-builtin -mcmodel=large -std=gnu99 -m64 -g $(OPTS) -I$(ROOT)/inc 
LDFLAGS= -z max-page-size=0x1000

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))
MODULES= $(addsuffix .o,$(addprefix mod/,$(MODS)))

TARGETS= kernel.bin kernel.sym

all: $(SUBDIRS) $(TARGETS)

-include $(DEPENDS)

info:
	@echo $(DEPENDS)
	@echo $(OBJECTS)

$(BASE)/%.bin: $(BASE)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BASE)/%.sym: $(BASE)/%.elf
	$(OBJCOPY) --only-keep-debug $< $@

$(BASE)/kernel.elf: $(BASE)/kernel.ld $(addprefix $(OBJDIR)/,$(OBJECTS)) $(LIBS)
	$(LD) $(LDFLAGS) -T $< $(filter %.o,$+) $(filter %.a,$+) -o $@

$(DEPDIR)/%.d: $(BASE)/%.S | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(DEPDIR)/%.d: $(BASE)/%.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(OBJDIR)/%.o: $(BASE)/%.S $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(BASE)/%.c $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(SUBDIRS):
	make -C $@

$(DEPDIR) $(OBJDIR):
	[ ! -d $@ ] && mkdir $@

clean:
	@for DIR in $(SUBDIRS);			\
	do                               	\
		make --directory=$$DIR clean;   \
	done
	rm -f $(TARGETS)
	rm -f $(OBJDIR)/*.o
	rm -f $(LIB)/*.a

cleaner:
	@for DIR in $(SUBDIRS);			\
	do                               	\
		make --directory=$$DIR cleaner; \
	done
	rm -f .obj/*.o
	rm -f .dep/*.d

