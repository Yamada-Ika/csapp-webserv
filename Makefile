build:
	gcc -o server main.c

run: build
	./server localhost

test:
	gcc utils.c utils_test.c -o utils_test
	./utils_test
	rm -f utils_test
