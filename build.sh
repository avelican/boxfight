#!/bin/bash

mkdir -p out

g++ -Iinclude -std=c++14 -x c++ -c src/console.cpp -o out/console.o


g++ -Iinclude -std=c++14 src/main.cpp -o out/main.exe `sdl2-config --cflags --libs`

# g++ -Iinclude -std=c++14 out/console.o src/console_test.cpp -o out/console_test.exe
# g++ -Iinclude -std=c++14 out/console.o src/rng_test.cpp -o out/rng_test.exe

