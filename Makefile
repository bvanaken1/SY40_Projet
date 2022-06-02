all: parking main

parking: interface.c
	gcc interface.c -o parking

main: main.c
	gcc main.c -o main -pthread