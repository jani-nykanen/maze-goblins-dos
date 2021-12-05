tools: png2bin

png2bin:
	mkdir -p ./tools/bin
	gcc -o ./tools/bin/$@ ./tools/$@.c -lm 
