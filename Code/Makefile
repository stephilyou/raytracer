# Compiler and flags
CXX      := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -pedantic
LDFLAGS  := 

# Sources and objects
SRCS := main.cpp camera.cpp bvh.cpp rayIntersection.cpp math.cpp antialiasing.cpp image.cpp shading.cpp texture.cpp config.cpp
OBJS := $(SRCS:.cpp=.o)

# Executable name
TARGET := raytracer

# Default target
all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Housekeeping
clean:
	rm -f $(OBJS) $(TARGET)

# Convenience target: rebuild
rebuild: clean all
