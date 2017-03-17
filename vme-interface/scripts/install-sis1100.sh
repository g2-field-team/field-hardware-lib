#!/bin/bash

if [[ $EUID -ne 0 ]]; then
    echo "This script must be run as root"
    exit 1
fi

mkdir -p /tmp/sis1100-install && cd /tmp/sis1100-install
wget http://www.struck.de/sis1100-2.13-9.tar.gz
tar xf sis1100-2.13-9.tar.gz
cd sis1100-2.13-9/dev/pci
make
make install

