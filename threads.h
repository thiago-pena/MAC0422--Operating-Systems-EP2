#ifndef THREADS_H
#define THREADS_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "tools.h"


/*Estrutura correspondente a um ciclista*/
typedef struct Ciclista ciclista;
struct Ciclista {
    int num;
    int px,py;
    int arrive;
    int Continue;
    int voltas;
    int velocidade; // 1: 30km/h, 2: 60km/h; 3: 90km/h
    int dt;
    int ultimo;
    int eliminado;
    pthread_t id;
    ciclista *prox;
    bool roundFeito; // cada ciclista indica se seu round já está completo
};

typedef struct MetroPista metroPista;
struct MetroPista {
    int ocupada;
    pthread_mutex_t mutex;
    ciclista *ciclista;
};


void * competidor(void * arg);
void * juiz(void * arg);
void visualizador();
void visualizadorStderr();
void velocidade(ciclista *p);

#endif
