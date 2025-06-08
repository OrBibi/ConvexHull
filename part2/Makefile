CXX = g++
CXXFLAGS = -std=c++11 -Wall
INCLUDES = -Iinclude

DEQUE_SRC = src/GeometryUtilsDeque.cpp main/MainDeque.cpp
LIST_SRC  = src/GeometryUtilsList.cpp  main/MainList.cpp

all: deque list

deque:
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o bin/ConvexHullDeque $(DEQUE_SRC)

list:
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o bin/ConvexHullList $(LIST_SRC)

clean:
	rm -rf bin

.PHONY: all clean deque list
