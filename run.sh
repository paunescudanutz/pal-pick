#!/bin/bash

rm bin/palpick
gcc src/*.c -lm -o bin/palpick ; ./bin/palpick -x 1 --y 1 --width 50 --height 50 --first-separator "; " --second-separator "; " --label1 "R: " --label2 "G: " --label3 "B: "
