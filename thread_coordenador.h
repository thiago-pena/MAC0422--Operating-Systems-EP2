#ifndef THREAD_COORDENADOR_H
#define THREAD_COORDENADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include "tools.h"
#include "thread_ciclista.h"

void * juiz(void * arg);
void visualizador();
void visualizadorStderr();
void eliminaQuebra(ciclista *c);
void eliminaCiclista(ciclista *c, int nCiclista);
void eliminaCiclistaMarcado(ciclista *c);
void imprimeVoltasCiclistas(ciclista *c);

#endif
