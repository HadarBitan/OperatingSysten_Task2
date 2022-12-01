CC = gcc
AR = ar
FLAGS = -Wall -g

OBJECTS_Shell = myshell.o

all: myshell mytee mync

mytee: mytee.o
	$(CC) $(FLAGS) -o mytee mytee.o

mync: mync.o 
	$(CC) $(FLAGS) -o mync mync.o

myshell: $(OBJECTS_Shell)
	$(CC) $(FLAGS) -o myshell $(OBJECTS_Shell) 

.PHONY: clean all

clean:
	rm -f *.o *.so myshell mytee mync