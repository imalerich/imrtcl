OCM = OpenCL-Mac
OBJ = main.o cl_util.o file_io.o gl_util.o vector.o camera.o material.o surface.o
LIB = -lOpenCL -lglfw -lGLEW -lGL -lGLU -lX11 -lXxf86vm -lXrandr -lXi -lglut

all: $(OBJ)
	g++ -o cl $(OBJ) $(LIB)

main.o: $(OCM)/main.c gl_util.o cl_util.o vector.o camera.o
	g++ -c $(OCM)/main.c

cl_util.o: $(OCM)/cl_util.c $(OCM)/cl_util.h file_io.o
	g++ -c $(OCM)/cl_util.c

gl_util.o: $(OCM)/gl_util.c $(OCM)/gl_util.h vector.o file_io.o
	g++ -c $(OCM)/gl_util.c

file_io.o: $(OCM)/file_io.c $(OCM)/file_io.h
	g++ -c $(OCM)/file_io.c

vector.o: $(OCM)/vector.c $(OCM)/vector.h
	g++ -c $(OCM)/vector.c

camera.o: $(OCM)/camera.c $(OCM)/camera.h vector.o
	g++ -c $(OCM)/camera.c

material.o: $(OCM)/material.c $(OCM)/material.h vector.o
	g++ -c $(INC) $(OCM)/material.c

surface.o: $(OCM)/surface.c $(OCM)/surface.h vector.o

clean:
	rm -rf cl
	rm -rf *.o
