TARGETS = $(patsubst src/%.cxx, bin/%, $(wildcard src/*.cxx))

CXX=g++

ifdef BOOST_INC
	CXXFLAGS += -I$(BOOST_INC)
endif

# add ROOT libs/flags
CXXFLAGS += $(shell root-config --cflags)
LIBS += $(shell root-config --libs)

# Libraries && include flags
CXXFLAGS += -Wl,-rpath,/usr/local/lib
LIBS += -L/usr/local/lib -l:/usr/local/lib/libg2fieldvme.so.0

all: $(TARGETS)

bin/%: src/%.cxx
	$(CXX) $(CXXFLAGS) $(LIBS) $< -o $@