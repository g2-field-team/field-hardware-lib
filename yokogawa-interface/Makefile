# Yokogawa Makefile
# Copied from the vme-hardware setup; small modifications for the vxi-11 library   
# Grab the targets and sources as two batches
OBJECTS = $(patsubst src/%.cxx, build/%.o, $(wildcard src/*.cxx))
# objects for vxi11 
OBJ_VXI = $(patsubst include/vxi11/%.cc, build/%.o, $(wildcard include/vxi11/*.cc))

# Locations and default files
LIB_DIR=./lib
BUILD_DIR=./build
BIN_DIR=./bin
INC_VXI_DIR=./include/vxi11

# Library info.
MAJOR=0
MINOR=0.1
SONAME=libg2fieldyoko.so
ARNAME=libg2fieldyoko.a
LIBNAME=$(SONAME).$(MAJOR).$(MINOR)

# Add a few flags to all compiler calls.
CXX += -O3 -fPIC
CC += -O3 -fPIC

ifdef DEBUG
   CXX += -g -pg
   CC += -g -pg
endif

# Add local include directory
CPPFLAGS += -Iinclude

# Check for UPS boost
ifdef BOOST_INC
   CPPFLAGS += -I$(BOOST_INC)
   LIBFLAGS += -L$(BOOST_LIB)
endif

# ROOT libs and flags
CXXFLAGS += $(shell root-config --cflags)
LIBS += $(shell root-config --libs)

ifeq ($(shell uname -s), Darwin)
        LIBFLAGS = -shared -Wl,-install_name,$(SONAME).$(MAJOR)
else
        LIBFLAGS = -shared -Wl,-soname,$(SONAME).$(MAJOR)
        LDCONFIG = ldconfig
endif

all: $(OBJECTS) $(OBJ_VXI) $(TARGETS) lib/$(ARNAME) lib/$(LIBNAME)

# $(LOGFILE):
# 	@mkdir -p $(@D)
# 	@touch $@
# 
# $(CONFDIR):
# 	@mkdir -p $(@D)
# 	@cp -r config $(@D)/

build/%.o: src/%.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# build/%.o: include/vxi11/%.cc
# 	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
#
# VXI build rules  
build/vxi11_user.o: $(INC_VXI_DIR)/vxi11_user.cc $(INC_VXI_DIR)/vxi11.h
	$(CXX) $(CFLAGS) -c $< -o $@

INC_VXI_DIR/vxi11.h INC_VXI_DIR/vxi11_clnt.c INC_VXI_DIR/vxi11_xdr.c : $(INC_VXI_DIR)/vxi11.x
	rpcgen -M $(INC_VXI_DIR)/vxi11.x

lib/$(ARNAME): $(OBJECTS) $(OBJ_VXI)
	$(AR) -rcs $@ $+

lib/$(LIBNAME): $(OBJECTS) $(OBJ_VXI)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIBFLAGS) -o $@ $+ $(LIBS)

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
