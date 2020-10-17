CC = gcc
CFLAGS = -Wall -pedantic -g -O0
RM = rm

ep2: ep2.c
	$(CC) $(CFLAGS) ep2.c -o ep2 -lpthread -lm

clean:
	$(RM) *.o ep2 
