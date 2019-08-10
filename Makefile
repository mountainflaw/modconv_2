#modconv_2 Makefile

#Compiler
CC = g++

#Source directories
BASEDIR = ./
SRCDIR = $(BASEDIR)src/
DEPSDIR = $(BASEDIR)deps/
OUTPUTDIR = build
OUTPUTNAME = modconv2
OUTPUTPATH = $(BASEDIR)$(OUTPUTDIR)/$(OUTPUTNAME)

#Project information
PROJECT = modconv 2
AUTHOR = red
BUILDS = release debug debug_all debug_optimizer debug_output

####################
#Build Configuration
####################
#Target files
SRCFILES = $(SRCDIR)main.cxx $(SRCDIR)f3d.cxx $(SRCDIR)collision.cxx $(SRCDIR)file.cxx
DEPFILES = $(DEPSDIR)lodepng.cpp

#Compiler flags
CFLAGS = -O3
FORCEFLAGS = -Wall -Werror -std=c++17 -Wno-unused-function
LIBS = -lassimp -lstdc++fs

#Debug Flags
DEBUGFLAGS = -g -fsanitize=address,undefined
####################

#Release recipes
all: clean createdir compile

release: all

redskin: SRCFILES += $(SRCDIR)redskin/redskin.cxx
redskin: CFLAGS = $(DEBUGFLAGS) -DBUILD_REDSKIN
redskin: all

debug: CFLAGS = $(DEBUGFLAGS)
debug: all

#Main recipes
compile: $(SRCFILES)
	$(CC) $(CFLAGS) $(FORCEFLAGS) $(SRCFILES) $(DEPFILES) -o $(OUTPUTPATH) $(LIBS)

clean:
	rm -r $(OUTPUTDIR) &> /dev/null

createdir:
	mkdir $(OUTPUTDIR) &> /dev/null

#Print help menu
help:
	$(info ======== $(PROJECT) ========)
	$(info author: $(AUTHOR))
	$(info ===========================)
	$(info Build types: $(BUILDS))
