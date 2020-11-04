#ifndef THREAD_CICLISTA_H
#define THREAD_CICLISTA_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "tools.h"

/* Estrutura correspondente a um ciclista */
typedef struct Ciclista ciclista;
struct Ciclista {
    int num;
    int px,py;
    int arrive;
    int Continue;
    int voltas;
    int velocidade; // 1: 30km/h, 2: 60km/h; 3: 90km/h
    int dt;
    pthread_t id;
    ciclista *prox;
    bool roundFeito; // cada ciclista indica se seu round já está completo
    bool quebrou;
};

// Função da thread dos ciclistas
void * competidor(void * arg);

// Define a velocidade do ciclista
void velocidade(ciclista *p);

// Anda pra frente
void moveFrente(ciclista *p);

// Procura uma pista externa para avançar
void movePistaExterna(ciclista *p);

// Tratamento do ciclista na linha de chegada. Faz sorteio para ver se ele vai
// quebrar, se quebrar, ativa a flag p->quebrou. Caso contrário, insere-o no
// rank da volta completada e recalcula sua velocidade.
void tratalinhaDechegada(ciclista *p);

// Ao fim de sua movimentação, o ciclista trata de ir para a pista mais interna possível
void movePistaInterna(ciclista *p);

#endif
