#!/bin/sh

make clean
make
cd apps
make clean
cd ..
make -C apps
make
