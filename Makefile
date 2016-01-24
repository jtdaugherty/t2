CC = gcc
CFLAGS = -Wall -Iinclude
LIBS = -framework opencl
PROGNAME = t2

OBJS = \
       src/main.o \
       src/info.o \
       src/platform.o \
       src/device.o \
       src/util.o

$(PROGNAME): $(OBJS)
	gcc $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

clean:
	rm -f $(PROGNAME) $(OBJS)
