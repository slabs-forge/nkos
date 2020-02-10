#------------------------------------------------------------------------------
# NK Loader - Modules
#------------------------------------------------------------------------------
# Author	: Sebastien LAURENT
# Date		: 02/09/2011
#------------------------------------------------------------------------------

.PHONY: all clean cleaner
.PRECIOUS: %.o

ROOT=../..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions

CFLAGS= -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(ROOT)/inc 

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

TARGETS = $(OBJECTS)

all: $(TARGETS)

-include $(DEPENDS)

$(DEPDIR)/%.d: $(BASE)/%.S | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@ 

$(DEPDIR)/%.d: $(BASE)/%.c | $(DEPDIR)
	$(CC) $(CFLAGS) -MM $< -MF $@

%.o: $(BASE)/%.S $(DEPDIR)/%.d 
	$(CC) $(CFLAGS) -c $< -o $@

%.o: $(BASE)/%.c $(DEPDIR)/%.d
	$(CC) $(CFLAGS) -c $< -o $@

$(DEPDIR):
	[ ! -d $@ ] && mkdir $@

clean:
	rm -f $(TARGETS)

cleaner: clean
	rm -f $(DEPDIR)/*.d

