#include "thread_coordenador.h"
#include "rank.h"

#define DEBUG 1
#define DEBUG2 1
#define PROB_90 0.1 // @alterar para 0.1 -> probabilidade de um ciclista ter 90km/h nas últimas voltas
#define NSLEEP 100000 // 0.01s = 1E-2 = 10ms

// Variáveis globais
extern ciclista ***pista;
extern ciclista *cab;
extern int d, n;
extern _Atomic int nCiclistasAtivos;
extern bool tem90;
extern int nCiclista90; // número do ciclista que terá 90km/h, se ocorrer
extern int dt_base; // base do delta de velocidade (2 padrão, 3 se tiver ciclista a 90km/h
extern pthread_mutex_t mutex;
extern long int tempo;
extern ListaRank L;
extern Rank rankFinal;
extern Rank rankQuebras;
extern bool ultimasVoltas;
extern bool ciclistaQuebrou;

void * juiz(void * arg)
{
    if (DEBUG) printf("Juiz está pronto\n");

    ciclista *c = (ciclista *) arg;

    /*Flags auxiliares*/
    int minVolta = 0; // Mínimo das voltas locais dos ciclistas
    int maxVolta = 0; // Máximo das voltas locais dos ciclistas
    int ultimaVoltaDeEliminacao = 0; // Menor volta local do coordenador
    int maiorVolta = 0; // Maior volta local do coordenador

    while (true) {
        if (DEBUG2) fprintf(stderr, "\t\t\t(ini loop Coordenador)\n");
        for (ciclista * p = c->prox; p != cab; p = p->prox) {
            while (p->arrive == 0) usleep(1);
            p->arrive = 0;
        }

        if (true) { // if temporário (código da thread coordenadora)
            if (DEBUG2) fprintf(stderr, "\t\t\t(Coordenador teste quebra)\n");
            if (ciclistaQuebrou) // elimina todos os ciclistas que quebraram (se quebrou, está na linha de chegada)
                eliminaQuebra(c);
            eliminaCiclistaMarcado(c); // verifica se tem algum ciclista marcado para eliminação na linha de chegada

            minVolta = maxVolta;
            for (ciclista * p = c->prox; p != cab; p = p->prox) {
                if (maxVolta < p->voltas) maxVolta = p->voltas;
                if (minVolta > p->voltas) minVolta = p->voltas;
            }
            if (maiorVolta != maxVolta) maiorVolta = maxVolta;
            if (DEBUG2) fprintf(stderr, "\t\t\t(Coordenador teste) Voltas dos ciclistas ativos\n");
            imprimeVoltasCiclistas(c);
            if (maiorVolta > 0) imprimeVoltasListaRank(L);
            while (minVolta > 0 && ultimaVoltaDeEliminacao < minVolta) {
                imprimeRank(L, ultimaVoltaDeEliminacao);
                imprimeStderrRank(L, ultimaVoltaDeEliminacao);
                ultimaVoltaDeEliminacao++; // terminou uma volta
                if (ultimaVoltaDeEliminacao%2 == 0) { // Eliminação
                    int ultimo = ultimoColocado(L, ultimaVoltaDeEliminacao);
                    printf(">>>> Ultimo colocado (Eliminado): %d\n", ultimo);
                    eliminaCiclista(c, ultimo);
                    nCiclistasAtivos--;
                    fprintf(stderr, "imprimeRank final (volta %d) / total de ciclistas ativos: %d\n", ultimaVoltaDeEliminacao, nCiclistasAtivos);
                    imprimeRankFinal(rankFinal);

                    if (nCiclistasAtivos == 1) { // fim da corrida
                        ultimo = novoUltimoColocado(L, ultimaVoltaDeEliminacao, ultimo);
                        eliminaCiclista(c, ultimo);
                        fprintf(stderr, ">>>>>>>>>>>>> O ciclista %d foi o vencedor!\n", ultimo);
                        printf(">>>>>>>>>>>>> O ciclista %d foi o vencedor!\n", ultimo);
                        // return NULL;
                        pthread_exit(0);
                    }
                    else if (nCiclistasAtivos == 2) { // sorteio 90km/h
                        ultimasVoltas = true;
                        if (randReal(0, 1) < PROB_90) {
                            tem90 = true;
                            dt_base = 3;
                            if (randReal(0, 1) < 0.5) // 50% de chances de ser o 1º
                                nCiclista90 = c->prox->num;
                            else
                                nCiclista90 = c->prox->prox->num; // será o 2º
                        }
                    }
                    // Remover as voltas anteriores da ED
                        // Alternativa é eliminar as voltas ímpares assim que imprimir o rank
                        // Descobrir na ED os últimos colocados OK
                            // Se houver mais de um, sortear algum OK
                    // Eliminar o último colocado
                        // Eliminação: remover sua posição da pista, parar a thread, destrui-la, remover da lista de threads
                    // Eliminar da ED a volta mais antiga
                    if (DEBUG2) fprintf(stderr, "\t\t\t(Coordenador teste) @5\n");
                }
                ciclistaQuebrou = false;
                }
                tempo++;
                usleep(100000);
                printf("(\\/)\nmaiorVolta: %d, minVolta: %d, ultimaVoltaDeEliminacao: %d\n", maiorVolta, minVolta, ultimaVoltaDeEliminacao);
                fprintf(stderr, "(\\/)\nmaiorVolta: %d, minVolta: %d, ultimaVoltaDeEliminacao: %d\n", maiorVolta, minVolta, ultimaVoltaDeEliminacao);
                visualizador();
                visualizadorStderr();
                fprintf(stderr, "Teste RemoveRanksVolta ini\n");
                if (ultimaVoltaDeEliminacao > 0) {
                    fprintf(stderr, "volta ini: %d\n", L->rank->volta);
                    L = RemoveRanksVolta(L, ultimaVoltaDeEliminacao);
                    fprintf(stderr, "thread volta\n");
                    fprintf(stderr, "volta fim a: %d\n", L->rank == NULL);
                    fprintf(stderr, "volta fim: %d\n", L->rank->volta);
                }
                fprintf(stderr, "Teste RemoveRanksVolta fim\n");
            }

        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
            p->Continue = 1;
        }
        if (DEBUG2) fprintf(stderr, "Juiz loop fim\n\n");
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
          if (pista[i][j] != NULL) {
              printf("%02d",pista[i][j]->num);
              if (pista[i][j]->velocidade == 1)
                printf("|");
              else if (pista[i][j]->velocidade == 2)
                printf(")");
              else
                printf(">");
          }
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
void visualizadorStderr()
{
  for (int j = 0; j < d; j++) {
    fprintf(stderr, "---");
  }
  fprintf(stderr, "\n");
  for (int i = 0; i < 10; i++) {
      for (int j = 0; j < d; j++) {
          if (pista[i][j] != NULL) {
              fprintf(stderr, "%02d",pista[i][j]->num);
              if (pista[i][j]->velocidade == 1)
                fprintf(stderr, "|");
              else if (pista[i][j]->velocidade == 2)
                fprintf(stderr, ")");
              else
                fprintf(stderr, ">");
          }
          else fprintf(stderr, "  |");
      }
      fprintf(stderr, "\n");
      for (int j = 0; j < d; j++) {
        fprintf(stderr, "---");
      }
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n\n");
}

// Recebe um inteiro nCiclista, remove o ciclista com essa numeração da
// estrutura de dados, insere-o no rank final, interrompe sua thread e
// libera a memória alocada por ele
void eliminaCiclista(ciclista *c, int nCiclista) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;
    fprintf(stderr, "\t\t\t\t(coordenador) inicio função eliminaCiclista\n");
    ciclista *q, *anterior;
    anterior = c;
    for (q = c->prox; q != cab; q = q->prox) { // encontra a thread do ultimo colocado
        if (q->num == nCiclista) break;
        anterior = q;
    }
    pista[q->py][q->px] = NULL;
    InsereCiclistaRank(rankFinal, q->num, tempo);
    pthread_cancel(q->id);
    // Remover da lista de threads
    anterior->prox = q->prox;
    fprintf(stderr, "\t\t\t\t\t(coordenador) o ciclista %d foi eliminado <<<<<<<<<<\n", q->num);
    nanosleep(&ts, NULL); // dorme para esperar a thread parar (o loop de espera da thread dá uma leitura inválida no valgrind por causa do free)
    free(q);
    fprintf(stderr, "\t\t\t\t\t(coordenador) fim função eliminaCiclista\n");
}

// Recebe um inteiro nCiclista, remove da estrutura c um ciclista marcado para
// ser eliminado interrompe sua thread e libera a memória alocada por ele
void eliminaCiclistaMarcado(ciclista *c) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    fprintf(stderr, "\t\t\t\t(coordenador) inicio função eliminaCiclistaMarcado\n");
    ciclista *q, *anterior;
    anterior = c;
    for (q = c->prox; q != cab; q = q->prox) { // encontra a thread do ultimo colocado
        if (q->eliminar) {
            pista[q->py][q->px] = NULL;
            InsereCiclistaRank(rankFinal, q->num, tempo);
            pthread_cancel(q->id);
            // Remover da lista de threads
            anterior->prox = q->prox;
            nanosleep(&ts, NULL); // dorme para esperar a thread parar (o loop de espera da thread dá uma leitura inválida no valgrind por causa do free)
            free(q);
            break;
        }
        anterior = q;
    }
    fprintf(stderr, "\t\t\t\t\t(coordenador) fim função eliminaCiclistaMarcado\n");
}

void eliminaQuebra(ciclista *c) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;
    fprintf(stderr, "\t\t\t\t(coordenador) inicio função eliminaQuebra\n");
    ciclista *anterior = c;
    for (ciclista * p = c->prox; p != cab; p = p->prox) {
        if (p->quebrou) {
            InsereCiclistaRank(rankQuebras, p->num, p->voltas);
            if (p->eliminar) {
                fprintf(stderr, "\t(coordenador) novo último colocado (por quebra)\n");
                // int novoUltimoColocado(ListaRank L, int volta, int numCiclista);
                // procura o novo último colocado
                // marca ele para ser eliminado (a volta deve ser ultimaVoltaDeEliminacao)
                    // p->eliminar
            }
            fprintf(stderr, "\t(coordenador) eliminação da quebra ciclista %d\n", p->num);
            ciclista *q = p;
            anterior->prox = p->prox; // Remover da lista de threads
            p = anterior;
            pista[q->py][q->px] = NULL; // limpa pista
            pthread_cancel(q->id); // interrompe a thread
            nanosleep(&ts, NULL); // dorme para esperar a thread parar (o loop de espera da thread dá uma leitura inválida no valgrind por causa do free)
            free(q);
        }
        else
            anterior = p;
    }
    fprintf(stderr, "\t\t\t\t\t(coordenador) fim função eliminaQuebra\n");
}

// para Debug apenas (remover)
void imprimeVoltasCiclistas(ciclista *c) {
    fprintf(stderr, "[DEBUG] voltas de cada ciclista\n");
    for (ciclista * p = c->prox; p != cab; p = p->prox) {
        if (DEBUG2) {
            fprintf(stderr, "\tciclista %d, volta: %d\n", p->num, p->voltas);
        }
    }
}
