parking : interface main
	gcc -o parking interface main

main : main.c
	gcc -c main.c -o main

interface : interface.c
	gcc -c interface.c -o interface