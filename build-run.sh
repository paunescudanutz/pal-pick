#!/bin/bash

rm bin/palpick
gcc src/*.c -lm -o bin/palpick ; ./bin/palpick
