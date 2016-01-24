all: t2

t2: src/main.c
	gcc -Wall -o t2 src/main.c -framework opencl

clean:
	rm -f t2
