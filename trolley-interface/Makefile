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

DEPS=$(INC_DIR)/TCPDrivers.h $(INC_DIR)/TrolleyInterface.h $(INC_DIR)/Sg382Interface.h
CFLAGS = -I$(INC_DIR) -Wl,-rpath,$(LIB_DIR)
LIBFLAGS = -L$(LIB_DIR) -ltrolleyinterface -lsg382interface

all: $(BIN_DIR)/TestComm $(BIN_DIR)/TestCommSg382 $(LIB_DIR)/libtrolleyinterface.so $(LIB_DIR)/libsg382interface.so


$(LIB_DIR)/libtrolleyinterface.so: $(BUILD_DIR)/TCPDrivers.o $(BUILD_DIR)/TrolleyInterface.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(LIB_DIR)/libsg382interface.so: $(BUILD_DIR)/TCPDrivers.o $(BUILD_DIR)/Sg382Interface.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/TCPDrivers.o: $(SRC_DIR)/TCPDrivers.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/TrolleyInterface.o: $(SRC_DIR)/TrolleyInterface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/Sg382Interface.o: $(SRC_DIR)/Sg382Interface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`

$(BUILD_DIR)/TestCommSg382.o: $(SRC_DIR)/TestCommSg382.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libtrolleyinterface.so
	$(CXX) -o $@ $< $(CFLAGS)  $(LIBFLAGS) `root-config --libs`

$(BIN_DIR)/TestCommSg382: $(BUILD_DIR)/TestCommSg382.o $(LIB_DIR)/libsg382interface.so
	$(CXX) -o $@ $< $(CFLAGS)  $(LIBFLAGS) `root-config --libs`


.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)

	mkdir -p $(INCINSTALLDIR)
	install $(INC_DIR)/*.h $(INCINSTALLDIR)
