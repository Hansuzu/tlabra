#!/bin/bash
echo "Running cxxtestgen..."
cxxtestgen --error-printer -o test/tmp/vector_test_runner.cpp test/vector_tests.h

echo "Compiling..."
g++ test/tmp/vector_test_runner.cpp -o bin/test -Iinclude -O3 -std=c++17

RET=$?;
if [ $RET == 0 ] 
then 
    echo "running bin/test" 
    bin/test
else 
    echo "Compiler error.";
fi
