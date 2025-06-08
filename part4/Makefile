CXX = g++
CXXFLAGS = -std=c++11 -Wall
SRC = src/GeometryUtils.cpp main/Main.cpp
TARGET = bin/ConvexHull

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $(SRC)

clean:
	rm -rf bin

.PHONY: all clean
