#include "threads.h"

#define DEBUG 1
#include "tools.h"

ciclista ***pista;
ciclista *cab;
int d, n;



void * competidor(void * arg)
{
  int cont;
  ciclista *p =(ciclista *) arg;
  ciclista* q;

  int casas;
  if (DEBUG) {
      printf("Ciclista: %d pronto\n", p->num );
      fprintf(stderr, "Ciclista: %d pronto\n", p->num);
  }
  while (!p->eliminado) {
      if (true) { // código da tarefa i
          /*Verifica o ciclista esta com a flag ultimo e ira cruzar a linha*/
          if (p->ultimo && pista[(p->py)][((p->px) +1)%d] == 0) {
              p->eliminado = 1; //Eliminado ...sai do loop
              break;
          }
          /*Verifica se esta em meia volta: 0.5m*/
          if (!p->meiaVolta) velocidade(p); //Calcula as probabilidades de velocidade
          if (p->velocidade == 0) {
              p->meiaVolta == 1; //se meia volta (30kmh) somente levanta a flag

          } else {

              /*Caso 1 a pista estiver livre: avança */
              if (pista[(p->py)][((p->px) +1)%d] == NULL) {

                  pista[(p->py)][((p->px) +1)%d] = pista[p->py][p->px];
                  pista[p->py][p->px] = NULL;
                  p->px = ((p->px) +1)%d;
                  if ((p->px)%d == 0) p->voltas = p->voltas + 1;
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
                      if (pista[(p->py)][((p->px) +1)%d] != NULL          &&
                      pista[(p->py)][((p->px) +1)%d]->arrive == 1      &&
                      (pista[((p->py) + 1)%10][((p->px) +1)%d] != NULL ||
                      p->py == 9)) {
                          p->velocidade = 0; //Limita a velocidade para 30,enunciado.
                          casas = 0;
                          break;

                      }
                      /*Caso 3 a pista estiver bloqueada e a ultrapassagem é possivel */
                      else if (pista[(p->py)][((p->px) +1)%d] != NULL          &&
                      pista[(p->py)][((p->px) +1)%d]->arrive == 1      &&
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

      }

      p->arrive = 1;
      while (!p->Continue) usleep(1);
      if (DEBUG) fprintf(stderr, "Ciclista: %d, volta: %d, vel: %d\n", p->num, p->voltas, p->velocidade);
      p->Continue = 0;
  }
  /*A propria thread faz a eliminacao do ciclista*/
  if (p->eliminado) {
      pista[p->py][p->px] = NULL;
      for (q = cab; q->prox != p ; q = q->prox) {}
      q->prox = p->prox;
      free(p);
  }
  return NULL;
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

    while (true) {

      for (ciclista * p = c->prox; p != cab; p = p->prox) {
          while (!p->arrive && !p->eliminado) usleep(1); //!p->eliminado evita a possibilidade de uma thread zumbi travar tudo.
          if (p->voltas > voltas) {
              voltas = p->voltas;
              if (voltas%2 == 0 && voltas > 1) atento = 1; //caso mult2, juiz atento
          }
          p->arrive = 0;
      }

      /*Caso atento, alguem sera eliminado*/
      if (atento) {
          cont = 0;
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
          if (cont == 1) {
            eliminado->ultimo = 1; //Levanta a flag de ultimo, a thread fara o resto.
            atento = 0;
          }
      }

      usleep(100000);
      visualizador();
      if (DEBUG) {
          printf("Juiz: ...volta: %d\n\n",voltas);
          fprintf(stderr, "Juiz: ...volta: %d\n",voltas);
      }
      for (ciclista * p = cab->prox; p != cab; p = p->prox) {
          p->Continue = 1;
      }

    }
}


void visualizador()
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


/* Calcula probabilidade e muda velocidade */
void velocidade(ciclista *p)
{
    int prob;
    srand(time(NULL));
    prob  = rand()%10;
    // prob = randReal(0, 1);

    if (p->velocidade == 0) {
        if (prob >= 2) p->velocidade = 1;
    } else if (p->velocidade == 1) {
        if (prob < 4) p->velocidade = 0;
    }
}
