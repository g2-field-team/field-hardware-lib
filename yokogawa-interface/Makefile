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

DEPS=$(INC_DIR)/YokogawaInterface.hh $(INC_DIR)/vxi11_user.h
CFLAGS = -I$(INC_DIR) -Wl,-rpath,$(LIB_DIR)
LIBFLAGS = -L$(LIB_DIR) -lyokogawainterface

all: $(BIN_DIR)/TestComm $(LIB_DIR)/libyokogawainterface.so

# VXI build
$(BUILD_DIR)/vxi11_user.o: $(SRC_DIR)/vxi11_user.cc $(INC_DIR)/vxi11_user.h $(INC_DIR)/vxi11.h
	$(CXX) $(CFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR)/vxi11_clnt.o: $(SRC_DIR)/vxi11_clnt.c $(INC_DIR)/vxi11.h 
	$(CXX) $(CFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR)/vxi11_xdr.o: $(SRC_DIR)/vxi11_xdr.c $(INC_DIR)/vxi11.h 
	$(CXX) $(CFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR)/YokogawaInterface.o: $(SRC_DIR)/YokogawaInterface.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS)

$(LIB_DIR)/libyokogawainterface.so: $(BUILD_DIR)/YokogawaInterface.o $(BUILD_DIR)/vxi11_user.o $(BUILD_DIR)/vxi11_clnt.o $(BUILD_DIR)/vxi11_xdr.o
	$(CXX) -shared -fPIC -o $@ $^ 

$(BUILD_DIR)/TestComm.o: $(SRC_DIR)/TestComm.cxx $(DEPS)
	$(CXX) -o $@ -fPIC -c $< $(CFLAGS) `root-config --cflags`

$(BIN_DIR)/TestComm: $(BUILD_DIR)/TestComm.o $(LIB_DIR)/libyokogawainterface.so
	$(CXX) -o $@ $< $(CFLAGS)  $(LIBFLAGS) `root-config --libs`

.PHONY: clean
.PHONY: install

clean:
	rm $(BUILD_DIR)/* $(BIN_DIR)/* $(LIB_DIR)/*

install:
	mkdir -p $(LIBINSTALLDIR)
	install $(LIB_DIR)/* $(LIBINSTALLDIR)

	install include/*.hh $(INCINSTALLDIR)

