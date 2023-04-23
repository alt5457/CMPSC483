#!/bin/bash

rm -rf output/
rm learningfactory.db
cd build
find . ! -name CMakeLists.txt -delete
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=ninja -G Ninja .
ninja
cd ..
./output/learning_factory
ls