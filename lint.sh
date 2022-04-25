#!/bin/bash -v
set -v
make clean
make
./nachos -f
./nachos -mkdir home
./nachos -mkdir usr
./nachos -mkdir etc
./nachos -mkdir var
./nachos -mkdir bin

./nachos -ap ../test/halt.noff halt.noff
./nachos -x halt.noff

# ./nachos -sh

# ./nachos -ap test/big test
# ./nachos -ap test/big test
# ./nachos -ap test/huge test
# ./nachos -ap test/big test
# ./nachos -ap test/big test