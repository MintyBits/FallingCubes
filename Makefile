SOURCES := $(wildcard $(SRC)/*.cpp)
CXXFLAGS := -lSDL2 -lSDL2_ttf

all:
	g++ main.cpp $(CXXFLAGS) -o bin/build
