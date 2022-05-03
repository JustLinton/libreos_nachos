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

./nachos -ap ../test/exitTest.noff exitTest.noff
./nachos -ap ../test/joinTest.noff joinTest.noff
./nachos -ap ../test/yieldTest.noff yieldTest.noff
./nachos -ap ../test/fileTest.noff fileTest.noff
./nachos -ap ../test/sayHelloTest.noff sayHelloTest.noff
./nachos -x joinTest.noff
# ./nachos -x sayHelloTest.noff
# ./nachos -x fileTest.noff

# ./nachos -sh

# ./nachos -ap test/big test
# ./nachos -ap test/big test
# ./nachos -ap test/huge test
# ./nachos -ap test/big test
# ./nachos -ap test/big test