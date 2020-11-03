#ifndef THREAD_COORDENADOR_H
#define THREAD_COORDENADOR_H
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>
#include "tools.h"
#include "thread_ciclista.h"

void * juiz(void * arg);
void visualizador();
void visualizadorStderr();
void eliminaQuebra(ciclista *c);
void eliminaCiclista(ciclista *c, int nCiclista);
void imprimeVoltasCiclistas(ciclista *c);
void imprimeMutexLocked();

#endif
