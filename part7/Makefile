CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -Iinclude
SRC = src/GeometryUtils.cpp main/Main.cpp
TARGET = bin/ConvexHullServer

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(SRC)

clean:
	rm -rf bin

.PHONY: all clean
