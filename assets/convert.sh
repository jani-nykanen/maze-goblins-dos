#!/bin/sh

# To be called from root

IN="assets/bitmaps/"
OUT="bin/"

mkdir -p bin
./tools/bin/png2bin "$IN"font.png "$OUT"FONT.BIN
./tools/bin/png2bin "$IN"static_tiles.png "$OUT"STATIC.BIN

./tools/bin/tmx2bin ./bin/LEVELS.BIN ./assets/levels/*.tmx
