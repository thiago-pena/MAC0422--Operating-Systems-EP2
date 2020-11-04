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

// Função da thread coordenadora
void * juiz(void * arg);

// Imprime a visualização da pista na saída padrão
void visualizador();

// Imprime a visualização da pista na saída de erros
void visualizadorStderr();

// Recebe um inteiro nCiclista, temove-o da pista, remove o ciclista com essa
// numeração da estrutura de dados, insere-o no rank final, interrompe sua
// thread e libera a memória alocada por ele
void eliminaCiclista(ciclista *c, int nCiclista);

// Verifica se há ciclistas com a flag p->quebrou e os elimina: remove-os da
// pista, remove-os da estrutura de dados, insere-os no rank de quebras,
// interrompe suas threads e libera a memória alocada por eles
void eliminaQuebra(ciclista *c);


#endif
