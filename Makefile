CC = gcc
CFLAGS = -Wall -pedantic -g -O0
RM = rm

ep2: ep2.o threads.o tools.o rank.o
	$(CC) ep2.o threads.o tools.o rank.o -o ep2 -lpthread -lm

ep2.o: ep2.c threads.h rank.h
	$(CC) $(CFLAGS) -c ep2.c

threads.o: threads.c threads.h rank.h
	$(CC) $(CFLAGS) -c threads.c

rank.o: rank.c rank.h tools.h
	$(CC) $(CFLAGS) -c rank.c

tools.o: tools.c tools.h
	$(CC) $(CFLAGS) -c tools.c

clean:
	$(RM) *.o *~ ep2
