CC = g++
CCFLAGS = -std=c++14
INCLUDES = -lglew
INCLUDES += -lglfw
OBJ = main
FRAMEWORK = OpenGL

build: main.cpp
	$(CC) -Wall $(CCFLAGS) -O0 -g -o $(OBJ) $(INCLUDES) -framework $(FRAMEWORK) main.cpp

clean:
	rm $(OBJ)
	rm -rf main.dSYM