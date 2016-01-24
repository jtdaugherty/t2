CC = gcc
CFLAGS = -Wall
LIBS = -framework opencl
PROGNAME = t2

OBJS = src/main.o

$(PROGNAME): $(OBJS)
	gcc $(CFLAGS) -o $(PROGNAME) src/main.o $(LIBS)

clean:
	rm -f $(PROGNAME) $(OBJS)
