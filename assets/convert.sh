#!/bin/sh

# To be called from root

IN="assets/bitmaps/"
OUT="bin/"

mkdir -p bin
./tools/bin/png2bin "$IN"font.png "$OUT"FONT.BIN
./tools/bin/png2bin "$IN"font_yellow.png "$OUT"FONT2.BIN
./tools/bin/png2bin "$IN"static_tiles.png "$OUT"STATIC.BIN
./tools/bin/png2bin "$IN"dynamic_tiles.png "$OUT"DYNAMIC.BIN
./tools/bin/png2bin "$IN"borders.png "$OUT"BORDERS.BIN

./tools/bin/tmx2bin ./bin/LEVELS.BIN ./assets/levels/*.tmx
