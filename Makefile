# obsidian 2 wip makefile

default: all

# make directory if needed
DUMMY != mkdir -p build && mkdir -p build/deps

# compiler configuration

CC := g++

CFLAGS := -c -O3

ifeq ($(BUILD), debug)
    CFLAGS =: -c -DDEBUG -g
endif

LIBS := -lassimp

LD := ld

all:

# build process
# compile

build/src/%.o: src/%.cxx
	$(CC) $(CFLAGS) -o $@ $<

#build/deps/exoquant.o: deps/exoquant.c
#	$(CC) $(CFLAGS) -o $@ $<

#build/deps/exoquant.o: deps/lodepng.cpp
#	$(CC) $(CFLAGS) -o $@ $<

#link

