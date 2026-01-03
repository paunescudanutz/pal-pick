#!/bin/bash

rm app.exe
# gcc *.c -lm -o app.exe -g; ./app.exe -h --pre "Vec3(" --post ")" --first-separator ", " --second-separator ", "
gcc *.c -lm -o app.exe ; ./app.exe -x 1 --y 1 --width 50 --height 50 --first-separator "; " --second-separator "; " --label1 "R: " --label2 "G: " --label3 "B: "
