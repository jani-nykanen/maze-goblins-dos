tools: png2bin tmx2bin

png2bin:
	mkdir -p ./tools/bin
	gcc -o ./tools/bin/$@ ./tools/$@.c -lm 

tmx2bin:
	mkdir -p ./tools/bin
	gcc -o ./tools/bin/$@ ./tools/$@.c -lm 


run:
	dosbox bin/game.exe

.PHONY: assets
assets:
	./assets/convert.sh
