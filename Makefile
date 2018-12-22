CC=gcc

CFLAGS=-Wall -Wextra -Werror -O0 -g -std=c11 -pthread
LDFLAGS=-lm

.PHONY: all

all: aluno grade

disk.o: disk.h disk.c 
	$(CC) $(CFLAGS) -c disk.c

fs.o: fs.h fs.c 
	$(CC) $(CFLAGS) -c fs.c

testAluno: disk.o fs.o testAluno.c
	$(CC) $(CFLAGS) -o testAluno disk.o fs.o testAluno.c $(LDFLAGS) 

test: disk.o fs.o test.c
	$(CC) $(CFLAGS) -o test disk.o fs.o test.c $(LDFLAGS) 

aluno: testAluno
	./testAluno

grade: test
	./test


clean:
	rm -rf *.o testAluno test 
