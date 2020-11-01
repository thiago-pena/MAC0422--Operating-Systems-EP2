#include <stdio.h>
#include <stdlib.h>
    // malloc
    // fprintf
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
    // sleep
#include <stdbool.h>
#include "thread_ciclista.h"
#include "thread_coordenador.h"
#include "rank.h"


#define TRUE 1
#define DEBUG 1
#define SEED 2


/* Variáveis globais */
ciclista ***pista;
ciclista *cab;
int d, n;
_Atomic int nCiclistasAtivos;
bool ultimasVoltas = false;
bool tem90 = false;
int nCiclista90 = -1; // número do ciclista que terá 90km/h, se ocorrer
int dt_base = 2; // base do delta de velocidade (2 padrão, 3 se tiver ciclista a 90km/h)
bool ciclistaQuebrou;
pthread_mutex_t mutex;
int maiorVolta, menorVolta;
long int tempo = 1; // A primeira iteração ocorre primeiro nas threads, depois o coordenador incrementa o tempo
ListaRank L;
Rank rankFinal;
Rank rankQuebras;

void destroiPista();

int main(int argc, char const *argv[]) {
    srand(SEED); // seed da bib rand

    d = atoi(argv[1]);
    n = atoi(argv[2]);
    nCiclistasAtivos = n;
    if (n <= 2) ultimasVoltas = true;
    pthread_mutex_init(&mutex, NULL);

    // Cria lista de Ranks por volta
    L = CriaListaRank();
    // Cria lista de Rank final
    rankFinal = CriaRank(0, n);
    rankQuebras = CriaRank(0, n);

    /*Cria pista como uma matriz de ponteiros*/
    pista = malloc(10 * sizeof(ciclista**));
    for (int i = 0; i < 10; i++) {
        pista[i] = malloc(d * sizeof(ciclista*));
    }

    /*Declara posições da pista vazias como NULLs*/
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < d; j++) {
            pista[i][j] = NULL;
        }
    }

    /*Cria os ciclistas e posiciona-os na pista*/
    cab = malloc(sizeof(ciclista));
    cab->prox = cab;
    int q = n/5;
    int r = n%5;
    int numComp = 1;
    if (q > 0 && r == 0) { q--;r = 5;} /*Conserta caso r == 0*/
    for (int j = 0; j < q; j++) {
        for(int i = 0; i < 5; i++) {
            ciclista *novoCiclista;
            novoCiclista = malloc(sizeof(ciclista));
            pista[i][(d -1) -j] = novoCiclista;
            novoCiclista->num = numComp++;
            novoCiclista->arrive = 0;
            novoCiclista->Continue = 0;
            novoCiclista->px = d -1 -j;
            novoCiclista->py = i;
            novoCiclista->voltas = -1;
            novoCiclista->ultimo = 0;
            novoCiclista->eliminado = 0;
            novoCiclista->roundFeito = false;
            novoCiclista->velocidade = 1;
            novoCiclista->quebrou = false;
            novoCiclista->linhaDeChegada = false;
            novoCiclista->eliminar = false;
            novoCiclista->prox = cab->prox;
            cab->prox = novoCiclista;
        }
    }
    for (int i = 0; i < r; i++) {
        ciclista *novoCiclista;
        novoCiclista = malloc(sizeof(ciclista));
        pista[i][d -q -1] = novoCiclista;
        novoCiclista->num = numComp++;
        novoCiclista->arrive = 0;
        novoCiclista->Continue = 0;
        novoCiclista->px = d -1 -q;
        novoCiclista->py = i;
        novoCiclista->voltas = -1;
        novoCiclista->ultimo = 0;
        novoCiclista->eliminado = 0;
        novoCiclista->roundFeito = false;
        novoCiclista->velocidade = 1;
        novoCiclista->quebrou = false;
        novoCiclista->linhaDeChegada = false;
        novoCiclista->eliminar = false;
        novoCiclista->prox = cab->prox;
        cab->prox = novoCiclista;
    }
    if (DEBUG) {
        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          printf("Ciclista %d vai competir\n",p->num);
        }
    }
    visualizador();
    sleep(4);
    /*Cria thread coordenadora de threads*/
    pthread_t coord;
    if (pthread_create(&coord, NULL, juiz, (void*) cab)) {
        printf("\n ERROR creating thread juiz\n");
        exit(1);
    }
    printf("foi criação do juiz\n");
    for (ciclista * p = cab->prox; p != cab; p = p->prox) {
      if (pthread_create(&p->id, NULL, competidor, (void*) p)) {
          printf("\n ERROR creating thread %ld\n",p->id);
          exit(1);
      }
    }

    for (ciclista * p = cab->prox; p != cab; p = p->prox) {
      if (pthread_join(p->id, NULL)) {
          printf("\n ERROR joining thread %ld\n",p->id);
          exit(1);
      }
    }

    if (pthread_join(coord, NULL)) {
        printf("\n ERROR joining thread juiz\n");
        exit(1);
    }

    printf("Rank final\n");
    imprimeRankFinal(rankFinal);
    printf("\nRank de quebras\n");
    imprimeRankFinal(rankQuebras);
    printf("Fim do ep\n");

    pthread_mutex_destroy(&mutex);
    destroiPista();
    DestroiListaRank(L);
    DestroiRank(rankFinal);
    DestroiRank(rankQuebras);
    free(cab);
    return 0;
}

void destroiPista() {
    for (int i = 0; i < 10; i++) {
        free(pista[i]);
    }
    free(pista);
}
