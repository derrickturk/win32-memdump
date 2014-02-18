CXX=g++
CXXOPTS=-Wall -Wextra -Werror -std=c++11 -pedantic

process_memory.o: process_memory.cpp process_memory.h
	$(CXX) $(CXXOPTS) -c process_memory.cpp
