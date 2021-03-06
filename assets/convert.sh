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
./tools/bin/png2bin "$IN"logo_flat.png "$OUT"LOGO.BIN -RLE
./tools/bin/png2bin "$IN"intro1.png "$OUT"INTRO1.BIN -RLE
./tools/bin/png2bin "$IN"intro2.png "$OUT"INTRO2.BIN -RLE
./tools/bin/png2bin "$IN"ending.png "$OUT"ENDING.BIN -RLE
./tools/bin/png2bin "$IN"start.png "$OUT"START.BIN -RLE

./tools/bin/mono2src "$IN"loading.png src/loading.h

./tools/bin/tmx2bin ./bin/LEVELS.BIN ./assets/levels/*.tmx
