GNU = -D GNU
OBJ = main.o cl_util.o file_io.o

all: $(OBJ)
	g++ -o cl $(OBJ) -lOpenCL -std=iso9899:2011

main.o:
	g++ -c $(GNU) OpenCL-Mac/main.c

cl_util.o: OpenCL-Mac/cl_util.h
	g++ -c $(GNU) OpenCL-Mac/cl_util.c

file_io.o: OpenCL-Mac/file_io.h
	g++ -c OpenCL-Mac/file_io.c

clean:
	rm -rf cl
	rm -rf *.o
