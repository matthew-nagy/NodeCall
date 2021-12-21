COMPILER = clang++
COMPILER_FLAGS = -std=c++17
OPTIMISATION_FLAGS = -O3
DEBUG_FLAGS = -ggdb -g3 -O0 -pedantic -Wall

NC_SOURCE = $(wildcard include/*cpp)

default:
	$(COMPILER) $(COMPILER_FLAGS) $(NC_SOURCE) $(DEBUG_FLAGS) main.cpp -o Main
	./Main