CC = gcc

COMMIT = $(shell git log -1 --format="%h")

CFLAGS = \
		 -O3 \
		 -DT2_COMMIT=\"$(COMMIT)\" \
		 -Wall -Iinclude \
		 $(shell pkg-config --cflags glfw3) \
		 $(shell pkg-config --cflags freetype2) \
		 $(shell pkg-config --cflags glew)

LIBS = \
	   -framework opencl -framework OpenGL \
	   $(shell pkg-config --static --libs glfw3) \
	   $(shell pkg-config --libs freetype2) \
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
	   src/args.o \
	   src/text.o \
	   src/overlay.o

$(PROGNAME): $(OBJS)
	gcc $(CFLAGS) -o $(PROGNAME) $(OBJS) $(LIBS)

clean:
	rm -f $(PROGNAME) $(OBJS)
