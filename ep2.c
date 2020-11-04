#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "thread_ciclista.h"
#include "thread_coordenador.h"
#include "rank.h"

/* Variáveis globais */
ciclista ***pista;
ciclista *cab;
int d, n;
int dt_base = 2;
_Atomic int nCiclistasAtivos, nQuebras;
int nVoltasTotal;
bool ciclistaQuebrou;
bool ultimasVoltas = false; // Indica que um ciclista começou as duas últimas voltas
bool tem90 = false; // Indica se algum ciclista vai pedalar a 90km/h
int nCiclista90 = -1; // Número do ciclista que vai pedalar a 90km/h
bool esperandoSegundoUltimasVoltas = false; // Esperando o segundo nas últimas voltas para pedalar a 90km/h
_Atomic long long int tempo = 0;
pthread_mutex_t **mutex;
pthread_mutex_t mutexInsere;
Rank rankFinal;
Rank rankQuebras;
ListaRank L;
long memTotal;

void destroiPista();
double elapsedTime(struct timeval a,struct timeval b);
double elapsedTime2(struct timespec a,struct timespec b);


int main(int argc, char const *argv[]) {
    srand(time(NULL));

    /* Inicializa o tempo real */
    struct timespec iniR, endR;

    // Caso flag benchmark captura informações de tempo e memória
    struct rusage usage;
    struct timeval ini, end, iniS, endS;
    if (argc > 3 && !strcmp("-benchmark", argv[3])) {
      clock_gettime(CLOCK_REALTIME, &iniR);
      getrusage(RUSAGE_SELF, &usage);
      ini = usage.ru_utime;
      iniS = usage.ru_stime;
    }


    d = atoi(argv[1]);
    n = atoi(argv[2]);

    /* Inicialização de variáveis */
    nCiclistasAtivos = n;
    nQuebras = 0;
    nVoltasTotal = 2*n - 2;

    /* Inicialização do mutex de ranks */
    pthread_mutex_init(&mutexInsere, NULL);

    /* Cria lista de Ranks por volta */
    L = CriaListaRank();
    /* Cria lista de Rank final */
    rankFinal = CriaRank(0, n);
    rankQuebras = CriaRank(0, n);

    /* Cria pista como uma matriz de ponteiros e uma matriz de mutex para ela */
    pista = malloc(10 * sizeof(ciclista**));
    mutex = malloc(10 * sizeof(pthread_mutex_t *));
    for (int i = 0; i < 10; i++) {
        pista[i] = malloc(d * sizeof(ciclista*));
        mutex[i] = malloc(d * sizeof(pthread_mutex_t));
    }

    /* Inicializa pista e seus mutexes */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < d; j++) {
            pista[i][j] = NULL;
            pthread_mutex_init(&mutex[i][j], NULL);
        }
    }

    /* Cria os ciclistas e posiciona-os na pista */
    cab = malloc(sizeof(ciclista));
    cab->prox = cab;
    int q = n/5;
    int r = n%5;
    int numComp = 1;
    if (q > 0 && r == 0) { q--; r = 5;} /* Conserta caso r == 0 */
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
            novoCiclista->roundFeito = false;
            novoCiclista->velocidade = 1;
            novoCiclista->quebrou = false;
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
        novoCiclista->roundFeito = false;
        novoCiclista->velocidade = 1;
        novoCiclista->quebrou = false;
        novoCiclista->prox = cab->prox;
        cab->prox = novoCiclista;
    }
    //visualizador();
    /* Cria thread coordenadora de threads */
    pthread_t coord;
    if (pthread_create(&coord, NULL, juiz, (void*) cab)) {
        printf("\n ERROR creating thread juiz\n");
        exit(1);
    }
    for (ciclista * p = cab->prox; p != cab; p = p->prox) {
      if (pthread_create(&p->id, NULL, competidor, (void*) p)) {
          printf("\n ERROR creating thread %ld\n",p->id);
          exit(1);
      }
    }

    for (ciclista * p = cab->prox; p != cab; p = p->prox) {
      if (pthread_join(p->id, NULL)) {
          printf("\n ERROR joining thread %ld\n", p->id);
          exit(1);
      }
    }

    if (pthread_join(coord, NULL)) {
        printf("\n ERROR joining thread juiz\n");
        exit(1);
    }

    printf("Rank final\n");
    imprimeRankFinal(rankFinal);
    if (nQuebras > 0) {
        printf("\nRank de quebras\n");
        imprimeRankQuebras(rankQuebras);
    }
    else
        printf("Nenhum ciclista quebrou.\n");
    printf("Fim do ep\n");

    pthread_mutex_destroy(&mutexInsere);
    destroiPista();
    DestroiListaRank(L);
    DestroiRank(rankFinal);
    DestroiRank(rankQuebras);
    free(cab);
    /* Grava no final informações de benchmark em arquivo .txt */
    if (argc > 3 && !strcmp("-benchmark",argv[3])) {
      clock_gettime(CLOCK_REALTIME, &endR);
      getrusage(RUSAGE_SELF, &usage);
      memTotal += usage.ru_maxrss;
      printf("Uso de memória: %ld kilobytes\n", memTotal);
      end = usage.ru_utime;
      endS = usage.ru_stime;
      FILE *fp;
      fp = fopen ("benchmark.txt","a");
      if (fp!=NULL)
      {
        fprintf(fp,"%02d %1.3lf %1.3lf %1.3lf %ld\n",atoi(argv[4]),elapsedTime2(iniR, endR),elapsedTime(ini, end), elapsedTime(iniS, endS), memTotal);
        fclose (fp);
      }
    }

    return 0;
}

void destroiPista() {
    for (int i = 0; i < 10; i++) {
        free(pista[i]);
    }
    free(pista);
}

/* Calcula o tempo na struct timeval */
double elapsedTime(struct timeval a,struct timeval b)
{
    long seconds = b.tv_sec - a.tv_sec;
    long microseconds = b.tv_usec - a.tv_usec;
    double elapsed = seconds + (double)microseconds/1000000;
    return elapsed;
}

double elapsedTime2(struct timespec a,struct timespec b)
{
    long seconds = b.tv_sec - a.tv_sec;
    long nanoseconds = b.tv_nsec - a.tv_nsec;
    double elapsed = seconds + (double)nanoseconds/1000000000;
    return elapsed;
}
