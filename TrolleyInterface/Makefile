CXX=g++
DEPS=TCPDrivers.h Device.h
INCLUDEDIR=./
CFLAGS=-I$(INCLUDEDIR)

all: TestComm TCPDrivers.o Device.o

TCPDrivers.o: TCPDrivers.cxx TCPDrivers.h
	$(CXX) -o $@ -c $< $(CFLAGS)

Device.o: Device.cxx $(DEPS)
	$(CXX) -o $@ -c $< $(CFLAGS)

TestComm.o: TestComm.cxx $(DEPS)
	$(CXX) -o $@ -c $< $(CFLAGS)

TestComm: TestComm.o Device.o TCPDrivers.o
	$(CXX) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm *.o
