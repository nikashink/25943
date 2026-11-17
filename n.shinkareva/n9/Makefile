all: main

.PHONY: all clean run

main: main.c
	gcc main.c -o main -Wall -pedantic -Wextra

run: main
	./main

clean:
	rm -f main
