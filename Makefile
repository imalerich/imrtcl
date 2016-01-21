OCM = OpenCL-Mac
OBJ = main.o cl_util.o file_io.o gl_util.o vector.o
LIB = -lOpenCL -lglfw -lGLEW -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lXi -lglut

all: $(OBJ)
	g++ -o cl $(OBJ) $(LIB)

main.o: gl_util.o cl_util.o vector.o
	g++ -c $(OCM)/main.c

cl_util.o: $(OCM)/cl_util.h file_io.o
	g++ -c $(OCM)/cl_util.c

gl_util.o: $(OCM)/gl_util.h vector.o file_io.o
	g++ -c $(OCM)/gl_util.c

file_io.o: $(OCM)/file_io.h
	g++ -c $(OCM)/file_io.c

vector.o: $(OCM)/vector.h
	g++ -c $(OCM)/vector.c

clean:
	rm -rf cl
	rm -rf *.o
