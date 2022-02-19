#!/bin/bash

make all
make clean

tar -czf really-optimistic-computing-machine.tar.gz src/ Makefile AUTHORS README.md obj/ output/ .clang-format

