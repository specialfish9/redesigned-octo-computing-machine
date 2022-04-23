#!/bin/bash

make all
make clean

tar -czf really-optimistic-computing-machine.tar.gz  .clang-format AUTHORS Doxyfile Makefile README.md assets/ config/ doc/ obj/ out/ output/ src/ test/
