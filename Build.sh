#!/usr/bin/env sh

g++ --std=c++17 -lopencv_core -lopencv_imgproc -lopencv_imgcodecs Solver.cpp -O3 -o Solver
