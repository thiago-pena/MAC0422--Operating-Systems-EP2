#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define TRUE 1
#define DEBUG 1

void * competidor(void * arg);
void * juiz(void * arg);
void vizualizador();
pthread_mutex_t mutex;

/*Estrutura correspondente a um ciclista*/
typedef struct Ciclista ciclista;
struct Ciclista {
    int num;
    int px,py;
    int Pause;
    int Continue;
    int voltas;
    int velocidade;
    pthread_t id;
    ciclista *prox;
};
/*Variáveis globais*/
ciclista ***pista;
ciclista *cab;
int d, n;

int main(int argc, char const *argv[]) {

    d = atoi(argv[1]);
    n = atoi(argv[2]);

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
    int q = n/10;
    int r = n%10;
    int numComp = n;
    if (q > 0 && r == 0) {q--;r = 10;} /*Conserta caso r == 0*/
    for (int j = q; j > 0; j--) {
        for(int i = 0; i < 10; i++) {
            ciclista *novoCiclista;
            novoCiclista = malloc(sizeof(ciclista));
            pista[i][j] = novoCiclista;
            novoCiclista->num = numComp--;
            novoCiclista->Pause = 0;
            novoCiclista->Continue = 0;
            novoCiclista->px = j;
            novoCiclista->py = i;
            novoCiclista->voltas = 0;
            novoCiclista->prox = cab->prox;
            cab->prox = novoCiclista;
        }
    }
    for (int i = 0; i < r; i++) {
        ciclista *novoCiclista;
        novoCiclista = malloc(sizeof(ciclista));
        pista[i][0] = novoCiclista;
        novoCiclista->num = numComp--;
        novoCiclista->Pause = 0;
        novoCiclista->Continue = 0;
        novoCiclista->px = 0;
        novoCiclista->py = i;
        novoCiclista->voltas = 0;
        novoCiclista->prox = cab->prox;
        cab->prox = novoCiclista;
    }
    if (DEBUG) {
        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          printf("Ciclista %d vai competir\n",p->num);
        }
    }
    vizualizador();
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
    return 0;
}

void * competidor(void * arg)
{
  int cont;
  ciclista *p =(ciclista *) arg;
  srand(time(NULL));
  int casas;
  if (DEBUG)printf("Ciclista: %d pronto\n",p->num );
  while (1) {
      casas = rand()%2 + 1;
      p->velocidade = casas;
      while (casas) {
          /*Caso 1 a pista estiver livre: avança */
          if (pista[(p->py)][((p->px) +1)%d] == NULL) {

              pista[(p->py)][((p->px) +1)%d] = pista[p->py][p->px];
              pista[p->py][p->px] = NULL;
              p->px = ((p->px) +1)%d;
              if ((p->px)%d == 0) p->voltas = p->voltas + 1;
              casas--;

          } else {

            cont = 0;
              while (pista[(p->py)][((p->px) +1)%d] != NULL && cont < 20) {
                  cont++;
                  /*Caso 2 a pista estiver bloqueada com ciclista que já avançou */
                  /* e ultrapassagem não é possivel */
                  if (pista[(p->py)][((p->px) +1)%d] != NULL          &&
                      pista[(p->py)][((p->px) +1)%d]->Pause == 1      &&
                     (pista[((p->py) + 1)%10][((p->px) +1)%d] != NULL ||
                      p->py == 9)) {

                      casas = 0;
                      break;

                  }
                  /*Caso 3 a pista estiver bloqueada e a ultrapassagem é possivel */
                  else if (pista[(p->py)][((p->px) +1)%d] != NULL          &&
                           pista[(p->py)][((p->px) +1)%d]->Pause == 1      &&
                           pista[((p->py) + 1)%10][((p->px) +1)%d] == NULL &&
                           pista[p->py][((p->px) +2)%d] == NULL            &&
                           p->py != 9) {

                      pista[p->py][((p->px) +2)%d] = pista[p->py][p->px];
                      pista[p->py][p->px] = NULL;
                      p->px = ((p->px) +2)%d;
                      if ((p->px)%d == 0) p->voltas = p->voltas + 1;/*Defeito!*/
                      casas--;
                      break;

                  }
                  usleep(1);
              }
          }
          if (cont > 19) {printf("ciclista %d: travou\n",p->num );casas = 0;}
      }
      p->Pause = 1;
      while (!p->Continue) usleep(1);
      p->Continue = 0;
  }
}

void * juiz(void * arg)
{
    if (DEBUG) printf("Juiz está pronto\n");
    while (1) {
      ciclista *p =(ciclista *) arg;
      for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          while (!p->Pause) usleep(1);
          p->Pause = 0;
      }
      if (DEBUG) printf("Juiz: Todos ciclistas completaram rodada!\n\n");
      sleep(1);
      vizualizador();
      for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          p->Continue = 1;
      }
    }
}

void vizualizador()
{
  for (int i = 0; i < 10; i++) {
      for (int j = 0; j < d; j++) {
          if (pista[i][j] != NULL) printf(" %02d |",pista[i][j]->num);
          else printf("    |");
      }
      printf("\n");
      for (int j = 0; j < d; j++) {
        printf("-----");
      }
      printf("\n");
  }
  printf("\n\n\n\n");
}
