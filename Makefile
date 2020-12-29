SHELL := bash

run:
	./build/main

test: build
	./runtests.sh

build:
	mkdir build
	bison -d -o build/parser.c src/parser.y
	flex -i -o build/scanner.c src/scanner.l
	gcc -std=c99 -pedantic -Isrc/ -Ibuild/ -o build/main src/*.c src/*.h build/*.c build/*.h

clean:
	rm -rf build