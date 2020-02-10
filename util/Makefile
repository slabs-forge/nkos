#
# NK Util Makefile
#

.PHONY: all clean cleaner
.PRECIOUS: %.o

ROOT=..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

CFLAGS= -I$(INCDIR)

OBJECTS=$(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS=$(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

TARGETS=$(BASE)/mbr-install $(BASE)/nkloader-install $(BASE)/mkimage

all: $(TARGETS)

-include $(DEPENDS)

$(BASE)/mbr-install: $(OBJDIR)/mbr-install.o
	$(CC) $< -o $@

$(BASE)/nkloader-install: $(OBJDIR)/nkloader-install.o
	$(CC) $< -o $@

$(BASE)/mkimage: $(OBJDIR)/mkimage.o
	$(CC) $< -o $@

$(DEPDIR)/%.d: $(BASE)/%.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(OBJDIR)/%.o: $(BASE)/%.c $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR):
	[ ! -d $@ ] && mkdir $@

clean:
	rm -f $(TARGETS)
	rm -f $(OBJDIR)/*.o

cleaner: clean
	rm -f $(DEP)/*.d

