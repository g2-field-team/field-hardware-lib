# Grab the targets and sources as two batches
OBJECTS = $(patsubst src/%.cxx, build/%.o, $(wildcard src/*.cxx))
OBJ_VME = $(patsubst include/vme/%.c, build/%.o, $(wildcard include/vme/*.c))

# Locations and default files
PREFIX=/usr/local
LIBDIR=$(PREFIX)/lib
INCDIR=$(PREFIX)/include/g2field
CONFDIR=$(PREFIX)/etc/g2field
LOGDIR=/var/log/g2field

# Library info.
MAJOR=0
MINOR=1.0
SONAME=libg2fieldvme.so
ARNAME=libg2fieldvme.a
LIBNAME=$(SONAME).$(MAJOR).$(MINOR)

# Add a few flags to all compiler calls.
CXX += -O3 -fPIC -std=c++11
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
	LDCONFIG = ldconfig -n $(LIBDIR)
endif

all: $(OBJECTS) $(OBJ_VME) $(TARGETS) lib/$(ARNAME) lib/$(LIBNAME)

$(LOGFILE):
	@mkdir -p $(@D)
	@touch $@

$(CONFDIR):
	@mkdir -p $(@D)
	@cp -r config $(@D)/

build/%.o: src/%.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

build/%.o: include/vme/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

lib/$(ARNAME): $(OBJECTS) $(OBJ_VME)
	$(AR) -rcs $@ $+

lib/$(LIBNAME): $(OBJECTS) $(OBJ_VME)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LIBFLAGS) -o $@ $+ $(LIBS)

install:
	mkdir -p $(LIBDIR)
	install lib/* $(LIBDIR)
	ln -sf $(LIBDIR)/$(LIBNAME) $(LIBDIR)/$(SONAME)

	mkdir -p $(INCDIR) $(INCDIR)/vme
	install include/*.hh $(INCDIR)
	install include/vme/*.h $(INCDIR)/vme/

	mkdir -p -m a=rwx $(CONFDIR)
	install config/* $(CONFDIR)

	mkdir -p -m a=rwx $(LOGDIR)
	$(LDCONFIG)

uninstall:
	rm -f $(LIBDIR)/$(SONAME)*
	rm -rf $(INCDIR) $(LOGDIR) $(CONFDIR)

clean:
	rm -f $(TARGETS) build/*.o lib/lib*
