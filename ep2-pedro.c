#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#define TRUE 1
#define DEBUG 1

/*Estrutura correspondente a um ciclista*/
typedef struct Ciclista ciclista;
struct Ciclista {
    int num;
    int px,py;
    int Pause;
    int Continue;
    int voltas;
    int velocidade;
    int meiaVolta;
    int ultimo;
    int eliminado;
    pthread_t id;
    ciclista *prox;
};

/*Thread do ciclista*/
void * competidor(void * arg);

/*Thread do coordenador de threads*/
void * juiz(void * arg);

void vizualizador();

/*Calcula probabilidade e muda velocidade*/
void velocidade(ciclista *p);

/*Elimina último ciclista*/
ciclista * eliminador(ciclista *c);

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
            novoCiclista->Pause = 0;
            novoCiclista->Continue = 0;
            novoCiclista->px = d -1 -j;
            novoCiclista->py = i;
            novoCiclista->voltas = -1;
            novoCiclista->meiaVolta = 0;
            novoCiclista->ultimo = 0;
            novoCiclista->eliminado = 0;
            novoCiclista->prox = cab->prox;
            cab->prox = novoCiclista;
        }
    }
    for (int i = 0; i < r; i++) {
        ciclista *novoCiclista;
        novoCiclista = malloc(sizeof(ciclista));
        pista[i][d -q -1] = novoCiclista;
        novoCiclista->num = numComp++;
        novoCiclista->Pause = 0;
        novoCiclista->Continue = 0;
        novoCiclista->px = d -1 -q;
        novoCiclista->py = i;
        novoCiclista->voltas = -1;
        novoCiclista->meiaVolta = 0;
        novoCiclista->ultimo = 0;
        novoCiclista->eliminado = 0;
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
  ciclista* q;

  int casas;
  if (DEBUG)printf("Ciclista: %d pronto\n",p->num );
  while (!p->eliminado) {

      /*Verefica o ciclista esta com a flag ultimo e ira cruzar a linha*/
      if (p->ultimo && pista[(p->py)][((p->px) +1)%d] == 0) {
        p->eliminado = 1; //Eliminado ...sai do loop
        break;
      }

      /*Verefica se esta em meia volta: 0.5m*/
      if (!p->meiaVolta) velocidade(p); //Calcula as probabilidades de velocidade
      if (p->velocidade == 0) {
          p->meiaVolta == 1; // se meia volta (30kmh) somente levanta a flag

      } else {

          /*Caso 1 a pista estiver livre: avança */
          if (pista[(p->py)][((p->px) +1)%d] == NULL) {

              pista[(p->py)][((p->px) +1)%d] = pista[p->py][p->px]; //  marca a posição da frente
              pista[p->py][p->px] = NULL; // desmarca a posição anterior
              p->px = ((p->px) +1)%d; // x++
              if ((p->px)%d == 0) p->voltas = p->voltas + 1; // passei para o final (volta++)
              casas--;

          } else {
              /*ATENCAO: existe algum caso nao coberto pelos proximos que faz */
              /*         com que entre em loop infinito, criei esse cont <20  */
              /*         para detectar essa situacao ate a gente achar o erro */
              cont = 0;
              while (pista[(p->py)][((p->px) +1)%d] != NULL && cont < 20) {
                  cont++;

                  /*Caso 2 a pista estiver bloqueada com ciclista que já avançou */
                  /* e ultrapassagem não é possivel */
                  if (pista[(p->py)][((p->px) +1)%d] != NULL          && // posição da frente está ocupada
                      pista[(p->py)][((p->px) +1)%d]->Pause == 1      && // o ciclista da frente já andou nessa iteração (ERRO)
                     (pista[((p->py) + 1)%10][((p->px) +1)%d] != NULL ||
                      p->py == 9)) {
                      p->velocidade = 0; //Limita a velocidade para 30,enunciado.
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
                      p->px = ((p->px) +2)%d; //"Pulo" da ultrapassagem
                      if ((p->px)%d == 0) p->voltas = p->voltas + 1;/*Defeito!*/
                      casas--;
                      break;

                  }
                  usleep(1);
              }
          }
          //if (cont > 19) {printf("ciclista %d: travou\n",p->num );casas = 0;}
      }
      p->Pause = 1;
      while (!p->Continue) usleep(1);
      p->Continue = 0;
  }
  /*A propria thread faz a eliminacao do ciclista*/
  if (p->eliminado) {
      pista[p->py][p->px] = NULL;
      for (q = cab; q->prox != p ; q = q->prox) {}
      q->prox = p->prox;
      free(p);
  }
}

void * juiz(void * arg)
{
    if (DEBUG) printf("Juiz está pronto\n");

    ciclista *c =(ciclista *) arg;
    ciclista *eliminado;

    /*Flags auxiliares*/
    int menorVolta = 0;
    int voltas = 0;
    int atento = 0; //atencao do juiz para cada 2 voltas.
    int cont = 0;

    while (1) {

      for (ciclista * p = c->prox; p != cab; p = p->prox) {
          while (!p->Pause && !p->eliminado) usleep(1); //!p->eliminado evita a possibilidade de uma thread zumbi travar tudo.
          if (p->voltas > voltas) {
              voltas = p->voltas;
              if (voltas%2 == 0 && voltas > 1) atento = 1; //caso mult2, juiz atento
          }
          p->Pause = 0;
      }

      /*Caso atento, alguem sera eliminado*/
      if (atento) {
          cont = 0; // número de candidatos a serem eliminados [Pena]
          menorVolta = voltas;
          /*verifica se alguem ficou em ultimo*/
          for (ciclista * p = c->prox; p != cab; p = p->prox) {
              if (p->voltas < menorVolta) {
                  menorVolta = p->voltas;
                  eliminado = p;
                  cont = 1; // Caso existir um unico valor menor esse cont permanecera = 1
              } else if (p->voltas == menorVolta) {
                  cont = 0; // Caso houver dois ou mais valores menores nao ha ultimo
              }
          }
          if (cont = 1) {
            eliminado->ultimo = 1; //Levanta a flag de ultimo, a thread fara o resto.
            atento = 0;
          }
      }

      usleep(100000);
      vizualizador();
      if (DEBUG) printf("Juiz: ...volta: %d\n\n",voltas);
      for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          p->Continue = 1;
      }

    }
}

void vizualizador()
{
  for (int j = 0; j < d; j++) {
    printf("---");
  }
  printf("\n");
  for (int i = 0; i < 10; i++) {
      for (int j = 0; j < d; j++) {
          if (pista[i][j] != NULL) printf("%02d|",pista[i][j]->num);
          else printf("  |");
      }
      printf("\n");
      for (int j = 0; j < d; j++) {
        printf("---");
      }
      printf("\n");
  }
  printf("\n\n");
}

/*Calcula probabilidade e muda velocidade*/
void velocidade(ciclista *p)
{
    int prob;
    srand(time(NULL));
    prob  = rand()%10;

    if (p->velocidade == 0) {
        if (prob >= 2) p->velocidade = 1;
    } else if (p->velocidade == 1) {
        if (prob < 4) p->velocidade = 0;
    }
}
