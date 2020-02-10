#------------------------------------------------------------------------------
# NK Loader - Utility functions library makefile
#------------------------------------------------------------------------------
# Author	: Sebastien LAURENT
# Date		: 02/09/2011
#------------------------------------------------------------------------------

.PHONY: all clean cleaner
.PRECIOUS: %.o

ROOT=../../..
BASE=.

-include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions

CFLAGS= -nostdinc -fno-builtin -std=gnu99 -m32 -g $(OPTS) -I$(INCDIR) 

TARGETS=libks-utl.a

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

all: $(TARGETS)

info:
	@echo $(OBJECTS)
	@echo $(DEPENDS)

-include $(DEPENDS)

libks-utl.a: libks-utl.a($(OBJECTS))
	$(RANLIB) $@

libks-utl.a(%.o): $(OBJDIR)/%.o
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

