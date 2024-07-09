build:
	gcc -Wall quadtree.c -o quadtree -lm -std=c99

run:
	./quadtree

clean:
	rm -rf quadtree