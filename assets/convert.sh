#!/bin/sh

# To be called from root

IN="assets/bitmaps/"
OUT="bin/"

mkdir -p bin
./tools/bin/png2bin "$IN"parrot.png "$OUT"PARROT.BIN -RLE
