TARGETS = $(patsubst src/%.cxx, bin/%, $(wildcard src/*.cxx))

CXX=g++
CXXFLAGS += $(shell root-config --cflags)
LIBS += $(shell root-config --libs) -lrt -l:/usr/local/lib/libg2fieldvme.so.0
#LIBS += $(shell root-config --libs) -lrt -L/usr/local/lib -lg2fieldvme

all: $(TARGETS)

bin/%: src/%.cxx
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@