TARGETS = $(patsubst src/%.cxx, bin/%, $(wildcard src/*.cxx))

CXX=g++ -std=c++11

ifdef BOOST_INC
	CXXFLAGS += -I$(BOOST_INC)
endif

# add ROOT libs/flags
CXXFLAGS += $(shell root-config --cflags)
LIBS += $(shell root-config --libs)

# Libraries && include flags
CXXFLAGS += -Wl,-rpath,/usr/local/lib
LIBS += -L/usr/local/lib -lg2fieldvme -lrt

all: $(TARGETS)

bin/%: src/%.cxx
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@