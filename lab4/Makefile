CC = clang

all: clean main lib1 lib2
clean:
	rm -rf *.out *.o
lib1:
	$(CC) -o blocks.so -fPIC -shared blocks.c -lm
lib2:
	$(CC) -o doubles.so -fPIC -shared doubles.c -lm

main:
	$(CC) -o main.out -ldl main.c 
