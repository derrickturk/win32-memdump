CXX=g++
DEBUGOPTS=
OPTOPTS=-O3
CXXOPTS=-Wall -Wextra -Werror -std=c++11 -pedantic $(DEBUGOPTS) $(OPTOPTS)

memdump.exe: memdump.cpp process_memory.o
	$(CXX) $(CXXOPTS) -o memdump memdump.cpp process_memory.o

process_memory.o: process_memory.cpp process_memory.h
	$(CXX) $(CXXOPTS) -c process_memory.cpp
