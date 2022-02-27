CMP = clang++
COMPILER_FLAGS = -Wall -std=c++17 -pthread
OPTIMISATION_FLAGS = -O3
DEBUG_FLAGS = -ggdb -g3 -O0 -pedantic -fsanitize=address
LIBRARY_NAME = NodeCall2.a
DEBUG_LIBRARY_NAME = NodeCall2.d.a

BUILD_DIR = build

default: clean object
	$(CMP) main.cpp $(wildcard build/*.o) $(COMPILER_FLAGS) $(OPTIMISATION_FLAGS) -o Main
	./Main

debug: clean debug_object
	$(CMP) main.cpp $(wildcard build/*.o) $(COMPILER_FLAGS) $(DEBUG_FLAGS) -o Main

clean:
	cd build && rm -f *.o

object: clean
	cd $(BUILD_DIR) && $(CMP) -c $(addprefix ../, $(wildcard include/*cpp)) $(COMPILER_FLAGS) $(OPTIMISATION_FLAGS)

debug_object: clean
	cd $(BUILD_DIR) && $(CMP) -c $(addprefix ../, $(wildcard include/*cpp)) $(COMPILER_FLAGS) $(DEBUG_FLAGS)

library: clean object
	ar cr $(LIBRARY_NAME) $(wildcard build/*.o)
	mv $(LIBRARY_NAME) lib

debug_library: clean debug_object
	ar cr $(DEBUG_LIBRARY_NAME) $(wildcard build/*.o)
	mv $(DEBUG_LIBRARY_NAME) lib
