CC = gcc

COMMIT = $(shell git log -1 --format="%h")

CFLAGS = \
		 -DT2_COMMIT=\"$(COMMIT)\" \
		 -Wall -Iinclude \
		 $(shell pkg-config --cflags glfw3) \
		 $(shell pkg-config --cflags glew)

LIBS = \
	   -framework opencl \
	   $(shell pkg-config --static --libs glfw3) \
	   $(shell pkg-config --static --libs glew)

PROGNAME = t2

OBJS = \
       src/main.o \
       src/info.o \
       src/platform.o \
       src/device.o \
       src/util.o \
	   src/shader_setup.o \
	   src/texture.o \
	   src/samplers.o \
	   src/logging.o \
	   src/args.o

$(PROGNAME): $(OBJS)
	gcc $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

clean:
	rm -f $(PROGNAME) $(OBJS)
