#!/usr/bin/python3

#
# This file generates an array for fixed-precision
# sine function values, to be used in the C code
#

import math
import os

os.chdir(os.path.dirname(os.path.abspath(__file__)))

PRECISION = 6

sine = []
m = math.pow(2, PRECISION)
for angle in range(0, 360):
    sine.append(int(m * math.sin(angle / 180.0 * math.pi)))

f = open("../src/sine.h", "w")

f.write("#define SINE_PRECISION " + str(PRECISION) + "\n")

f.write("static const short SINE_TABLE[] = {")
for i in range(0, len(sine)):
    f.write(str(sine[i]))
    if i != len(sine)-1:
        f.write(",")

f.write("};\n")
f.close()
