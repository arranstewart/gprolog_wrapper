

wrapper.o: wrapper.cc wrapper.h

example: example.o wrapper.o empty.o

CC=clang++
CXX=clang++

COMPILER_FLAGS=-pedantic -Wall -I/usr/lib/gprolog-iso/include

CFLAGS=-std=c11 $(COMPILER_FLAGS)
CXXFLAGS= -std=c++11 $(COMPILER_FLAGS)

LDFLAGS=-L/usr/lib/gprolog-iso

LDLIBS= -lbips_fd -lbips_pl -lengine_pl -lengine_fd \
	-lbips_fd -lbips_pl -lengine_pl -lengine_fd \
  -llinedit  -lm

empty.o: empty.pl
	gplc -c empty.pl


clean:
	-rm *.o 



