CFLAGS = -Wall -W -lm -g

Pairing : main.o parse.o line.o pair.o glif.o
	gcc -o Pairing main.o parse.o line.o pair.o glif.o $(CFLAGS)

main.o : 
	gcc -c main.c $(CFLAGS)

parse.o : 
	gcc -c parse.c $(CFLAGS)

line.o :
	gcc -c line.c $(CFLAGS)

pair.o : 
	gcc -c pair.c $(CFLAGS)

glif.o : 
	gcc -c glif.c $(CFLAGS)

clean :
	rm main.o parse.o line.o pair.o glif.o Pairing

