all: t2

t2: src/main.c t2.cl
	gcc -o t2 src/main.c -framework opencl

clean:
	rm -f t2
