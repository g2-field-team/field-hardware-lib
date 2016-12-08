#####################################################################
#
#  Name:         Makefile
#  Created by:   Matthias Smith
#
#  Contents:     Makefile for vme utilities/tests.
#                
#
#####################################################################

UTILITIES = $(patsubst src/%.cxx,bin/%,$(wildcard src/*.cxx))

#-----------------------------------------
# Set compilers
CC = gcc
CXX = g++

CXXFLAGS = -fpermissive -std=c++1y
LIBS = -lm -lz -lpthread -lrt

# add ROOT libs/flags
CXXFLAGS += $(shell root-config --cflags)
LIBS += $(shell root-config --libs)

# Libraries && include flags
CXXFLAGS += -Wl,-rpath,/usr/local/lib
LIBS += -L/usr/local/lib -lg2fieldvme

#-------------------------------------------------------------------
# Finally we have the actual make directives.

all: $(UTILITIES)

bin/%: src/%.cxx
	$(CXX) $< -o $@ $(CXXFLAGS) $(LIBS) 

clean:
	rm -f *~ bin/*
