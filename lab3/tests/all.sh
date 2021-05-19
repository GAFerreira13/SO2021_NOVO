#!/bin/sh
#Loading code
cd ..
sudo ./remove.sh serp
make clean
make
sudo ./load.sh serp
cd tests/
make clean
make
