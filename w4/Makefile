clean : main
	rm *.o	

main : main.o a.o b.o
	gcc main.o a.o b.o -o main

main.o :
	gcc main.c -c

a.o :
	gcc a.c -c

b.o : 
	gcc b.c -c

