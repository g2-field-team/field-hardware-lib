CXX=g++
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./lib
BUILD_DIR = ./build
BIN_DIR = ./bin

# Locations and default files
PREFIX=/usr/local
LIBINSTALLDIR=$(PREFIX)/lib
INCINSTALLDIR=$(PREFIX)/include/g2field

DEPS=$(INC_DIR)/CommDriver.hh
CFLAGS = -I$(INC_DIR) -Wall
# CFLAGS = -I$(INC_DIR) -Wl,-rpath,$(LIB_DIR)
LIBFLAGS = -L$(LIB_DIR) -lCommDriver

# all: $(LIB_DIR)/libCommDriver.so
all: $(BUILD_DIR)/CommDriver.o

$(BUILD_DIR)/CommDriver.o: $(SRC_DIR)/CommDriver.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(LIB_DIR)/libCommDriver.so: $(BUILD_DIR)/CommDriver.o 
	$(CXX) -shared -fPIC -o $@ $^ 

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)
	install include/*.hh $(INCINSTALLDIR)
