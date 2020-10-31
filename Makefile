CC = gcc
CFLAGS = -Wall -pedantic -g -O0
RM = rm

ep2: ep2.o thread_ciclista.o thread_coordenador.o tools.o rank.o
	$(CC) ep2.o thread_ciclista.o thread_coordenador.o tools.o rank.o -o ep2 -lpthread -lm

ep2.o: ep2.c thread_ciclista.h thread_coordenador.h rank.h
	$(CC) $(CFLAGS) -c ep2.c

thread_ciclista.o: thread_ciclista.c thread_ciclista.h rank.h
	$(CC) $(CFLAGS) -c thread_ciclista.c

thread_coordenador.o: thread_coordenador.c thread_coordenador.h rank.h
	$(CC) $(CFLAGS) -c thread_coordenador.c

rank.o: rank.c rank.h tools.h
	$(CC) $(CFLAGS) -c rank.c

tools.o: tools.c tools.h
	$(CC) $(CFLAGS) -c tools.c

clean:
	$(RM) *.o *~ ep2
