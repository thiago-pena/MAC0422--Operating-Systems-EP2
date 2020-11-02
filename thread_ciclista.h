#ifndef THREAD_CICLISTA_H
#define THREAD_CICLISTA_H

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
    bool quebrou;
    bool linhaDeChegada;
    bool eliminar; // o ciclista deve ser eliminado assim que passar pela linha de chegada
    bool vel90; // marca o ciclista para ter 90km/h nas 2 últimas voltas
};


void * competidor(void * arg);
void velocidade(ciclista *p);
void moveFrente(ciclista *p);
void moveTemp(ciclista *p);
void tratalinhaDechegada(ciclista *p);
void movePistaInterna(ciclista *p);
void movePistaInterna2(ciclista *p);
int mod(int a, int b);

#endif
