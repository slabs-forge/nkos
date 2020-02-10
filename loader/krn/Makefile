#------------------------------------------------------------------------------
# NK Loader - Loader kernel services and managers Makefile
#------------------------------------------------------------------------------
# Author	: Sebastien LAURENT
# Date		: 02/09/2011
#------------------------------------------------------------------------------

.PHONY: all clean cleaner

ROOT=../..
BASE=.

include $(ROOT)/Makefile.inc

INCDIR=$(ROOT)/inc
LIBDIR=$(BASE)

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions

#CFLAGS= -D__DEBUG__ -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(ROOT)/inc 
CFLAGS= -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(INCDIR) 

OBJECTS= $(patsubst %.c,%.o,$(wildcard *.c)) $(patsubst %.S,%.o,$(wildcard *.S))
DEPENDS= $(addprefix $(DEPDIR)/,$(OBJECTS:.o=.d))

TARGETS=libnkl-krn.a

all: $(TARGETS)

-include $(DEPENDS)

libnkl-krn.a: libnkl-krn.a($(OBJECTS))
	$(RANLIB) $@

libnkl-krn.a(%.o): $(OBJDIR)/%.o
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
	rm -f $(OBJ)/*.d

