#------------------------------------------------------------------------------
# NK Loader Makefile
#
# Author	: Sebastien LAURENT
# Date		: 03/09/2011
#

SUBDIRS=utl krn bs mod
#SUBDIRS=utl krn bs

.PHONY: clean cleaner all $(SUBDIRS)

vpath %.a $(SUBDIRS)

ROOT=..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

LIBS= -lnkl-krn -lnkl-utl -lnkl-bs
MODS = mod_ext2 mod_extp

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions
CFLAGS= -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(ROOT)/inc 

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))
MODULES= $(addsuffix .o,$(addprefix mod/,$(MODS)))

TARGETS= $(BASE)/nkloader.bin $(BASE)/nkloader.sym

all: $(SUBDIRS) $(TARGETS)

-include $(DEPENDS)

info:
	@echo $(DEPENDS)
	@echo $(OBJECTS)

$(BASE)/%.bin: $(BASE)/%.elf
	$(OBJCOPY) -O binary $< $@

$(BASE)/%.sym: $(BASE)/%.elf
	$(OBJCOPY) --only-keep-debug $< $@

$(BASE)/nkloader.elf: $(BASE)/nkloader.lds $(addprefix $(OBJDIR)/,$(OBJECTS)) $(LIBS) $(MODULES)
	$(LD) $(LDFLAGS) -T $< $(filter %.o,$+) --start-group $(filter %.a,$+) --end-group -o $@

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
	rm -f $(OBJDIR)/*.o
	rm -f $(LIB)/*.a

cleaner:
	@for DIR in $(SUBDIRS);			\
	do                               	\
		make --directory=$$DIR cleaner; \
	done
	rm -f $(BASE)/nkloader.sym
	rm -f $(BASE)/nkloader.elf
	rm -f $(BASE)/nkloader.img
	rm -f .obj/*.o
	rm -f .dep/*.d

