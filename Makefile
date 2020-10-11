all: memoryship.c
	gcc -pedantic -Wall -o memoryship memoryship.c

.PHONY: clean

clean:
	rm -f memoryship
