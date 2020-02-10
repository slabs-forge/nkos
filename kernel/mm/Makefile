#------------------------------------------------------------------------------
# Kernel- Memory Manager related functions
#------------------------------------------------------------------------------
# Author	: Sebastien LAURENT
# Date		: 28/09/2011
#------------------------------------------------------------------------------

.PHONY: all clean cleaner
.PRECIOUS: %.o

ROOT=../..
BASE=.

-include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions

CFLAGS= -nostdinc -fno-builtin -mcmodel=large -m64 -std=gnu99 -g $(OPTS) -I$(INCDIR) 

TARGETS=libkrn-mm.a

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

all: $(TARGETS)

info:
	@echo $(OBJECTS)
	@echo $(DEPENDS)

-include $(DEPENDS)

libkrn-mm.a: libkrn-mm.a($(OBJECTS))
	$(RANLIB) $@

libkrn-mm.a(%.o): $(OBJDIR)/%.o
	$(AR) cr $@ $(OBJDIR)/$%

$(DEPDIR)/%.d: $(BASE)/%.S | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(DEPDIR)/%.d: $(BASE)/%.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ -MT '$(OBJDIR)/$*.o'

$(OBJDIR)/%.o: $(BASE)/%.S $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(BASE)/%.c $(DEPDIR)/%.d | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(DEPDIR):
	[ ! -d $@ ] && mkdir $@

clean:
	rm -f $(TARGETS)
	rm -f $(OBJDIR)/*.o

cleaner: clean
	rm -f $(DEPDIR)/*.d

