#ifndef THREADS_H
#define THREADS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
    // usleep

/*Estrutura correspondente a um ciclista*/
typedef struct Ciclista ciclista;
struct Ciclista {
    int num;
    int px,py;
    int arrive;
    int Continue;
    int voltas;
    int velocidade; /* 1: 30km/h, 2: 60km/h; 3: 90km/h */
    int meiaVolta;
    int ultimo;
    int eliminado;
    pthread_t id;
    ciclista *prox;
};


void * competidor(void * arg);
void * juiz(void * arg);
void visualizador();
void velocidade(ciclista *p);

#endif
