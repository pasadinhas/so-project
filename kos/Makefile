OBJECTS=kos.o delay.o buffer.o kos_request.o kos_server.o shard.o files.o

DEFAULT_INCLUDES = -I.   
INCLUDES = -I ../include
DEFS ?= -DHAVE_CONFIG_H #-DDEBUG
# -DC_DEBUG
CC = gcc
CFLAGS ?= -g -O0 -Wall -pthread 

COMPILE = $(CC) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CFLAGS) 

AR = ar
ARFLAGS = cru
RANLIB = ranlib


all: libkos.a 
	

libkos.a: $(OBJECTS) 
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

.c.o:
	$(COMPILE) -c -o $@ $<

clean: 
	rm -f *.o *~ libkos.a

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
