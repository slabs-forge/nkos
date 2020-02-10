#------------------------------------------------------------------------------
# NK Loader - Loader BIOS services
#------------------------------------------------------------------------------
# Author	: Sebastien LAURENT
# Date		: 02/09/2011
#------------------------------------------------------------------------------

.PHONY: clean cleaner

ROOT=../..
BASE=.

INC=$(ROOT)/inc
OBJ=$(BASE)/.obj
DEP=$(BASE)/.dep
OBJ=$(BASE)/.obj
LIB=$(BASE)

OPTS=
#OPTS=
OPTS= -Os
#OPTS= -O1
#OPTS= -O2
#OPTS= -O3 -fno-inline-functions

#CFLAGS= -D__DEBUG__ -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(ROOT)/inc 
CFLAGS= -nostdinc -fno-builtin -m32 -g $(OPTS) -I$(ROOT)/inc 
LDFLAGS= -L$(LIB)

DEPENDENCIES=$(addprefix $(DEP)/,$(patsubst %.S,%.d,$(wildcard *.S)))
DEPENDENCIES+=$(addprefix $(DEP)/,$(patsubst %.c,%.d,$(wildcard *.c)))

OBJECTS = 	$(OBJ)/bs13_get_drive_parms.o \
		$(OBJ)/bs13_read_sect.o

all: $(LIB)/libnkl-bs.a

-include $(DEPENDENCIES)

$(LIB)/libnkl-bs.a: $(OBJECTS)
	ar rcsu $@ $+
	ranlib $@
	touch $@

$(DEP)/%.d: $(BASE)/%.S
	gcc $(CFLAGS) -MM -MD -E $< -MT '$(OBJ)/$*.o' -o $@

$(DEP)/%.d: $(BASE)/%.c
	gcc $(CFLAGS) -MM -MD -E $< -MT '$(OBJ)/$*.o' -o $@

$(OBJ)/%.o: $(BASE)/%.S $(DEP)/%.d
	gcc $(CFLAGS) -c $< -o $@

$(OBJ)/%.o: $(BASE)/%.c $(DEP)/%.d
	gcc $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)/*.o
	rm -f $(LIB)/*.a

cleaner:
	rm -f $(LIB)/*.a
	rm -f $(OBJ)/*.o
	rm -f $(OBJ)/*.d

