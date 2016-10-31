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

DEPS=$(INC_DIR)/TCPDrivers.h $(INC_DIR)/TrolleyInterface.h
CFLAGS = -I$(INC_DIR)
LIBFLAGS = -L$(LIB_DIR) -lTrolleyInterface

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libTrolleyInterface.so


$(LIB_DIR)/libTrolleyInterface.so: $(BUILD_DIR)/TCPDrivers.o $(BUILD_DIR)/TrolleyInterface.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/TCPDrivers.o: $(SRC_DIR)/TCPDrivers.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/TrolleyInterface.o: $(SRC_DIR)/TrolleyInterface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libTrolleyInterface.so
	$(CXX) -o $@ $< $(CFLAGS) $(LIBFLAGS) `root-config --libs`

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)

	mkdir -p $(INCINSTALLDIR)
	install $(INC_DIR)/*.h $(INCINSTALLDIR)
