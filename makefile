# Compiler 
CC = gcc
CXX = g++

# Compiler flags
CFLAGS = -Wall -g
CXXFLAGS = -Wall -g

# Target executable
TARGET = raycast

TARGET_DEL = raycast.exe

# Source files
C_SRCS = v3math.c
CPP_SRCS = raycast.cpp ppm.cpp 

# Object files
C_OBJS = $(C_SRCS:.c=.o)
CPP_OBJS = $(CPP_SRCS:.cpp=.o)
OBJS = $(C_OBJS) $(CPP_OBJS)

# Default rule to build and run the executable
all: $(TARGET)

# Rule to link object files into the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) 

# Rule to compile .c files into .o files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del $(OBJS) $(TARGET_DEL)