#include "threads.h"
#include "rank.h"

#define NSLEEP 1000000 // 0.1s = 1E-1

// Variáveis globais
extern metroPista **pista;
extern ciclista *cab;
extern int d, n;
extern pthread_mutex_t mutex;
extern long int tempo;
extern ListaRank L;
extern Rank rankFinal;

// Para números negativos, mod é diferente do resultado do operador resto (%)
// Foi útil para passar os ciclistas para as pistas mais internas, pois
// (p->px - 1)%d não dava o resto, mas sim um número negativo.
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

void * competidor(void * arg)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    ciclista *p = (ciclista *) arg;

    if (DEBUG) {
        printf("Ciclista: %d pronto\n", p->num );
        if (DEBUG) fprintf(stderr, "Ciclista: %d pronto\n", p->num);
    }
    velocidade(p); // Velocidade inicial na primeira volta (30km/h)
    p->dt = 2 - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")
    while (true) {
        if (true) { // código da tarefa i
            if (DEBUG) fprintf(stderr, "\t(0) loop ciclista %d\n", p->num);
            p->roundFeito = false;
            int x = p->px;
            int y = p->py;
            if (p->dt > 0) { // A velocidade do ciclista requer mais de uma iteração para avançar
                (p->dt)--;
                if (DEBUG) fprintf(stderr, "\t(1 else) loop ciclista %d\n", p->num);
            }
            else { // A velocidade permite o ciclista avançar a cada iteração
                if (DEBUG) fprintf(stderr, "\t(1) loop ciclista %d\n", p->num);
                //pthread_mutex_lock(&mutex);
                //if (DEBUG) fprintf(stderr, "\t(LOCK) ciclista %d\n", p->num);

                pthread_mutex_lock(&pista[y][(x + 1)%d].mutex);
                if (pista[y][(x + 1)%d].ciclista == NULL && !pista[y][(x + 1)%d].ocupada) { // Caso 0: a posição da frente está livre
                    // anda pra frente
                    pista[y][(x + 1)%d].ciclista = p;
                    pthread_mutex_lock(&pista[y][x].mutex);
                    pista[y][x].ciclista = NULL;
                    pista[y][x].ocupada = false;
                    pthread_mutex_unlock(&pista[y][x].mutex);
                    p->px = (x + 1)%d;
                    pista[y][(x + 1)%d].ocupada = true;
                    pthread_mutex_unlock(&pista[y][(x + 1)%d].mutex);
                }
                else { // Caso 1: a posição da frente está ocupada
                    pthread_mutex_unlock(&pista[y][(x + 1)%d].mutex);
                    if (DEBUG) fprintf(stderr, "\tciclista %d (Caso 1)\n", p->num);
                    int j;
                    bool achouPistaExt = false; // Tem pista externa livre?
                    bool achouEspacoFrente = false; // Tem espaço na pista da frente?

                    for (int i = y + 1; i < 10; i++) { // Procura se há pista externa livre
                        if (pista[i][x].ciclista == NULL) {
                            achouPistaExt = true;
                            break;
                        }
                    }
                    for (j = 0; j < 10; j++) { // Procura se há espaço na linha da frente
                        if (pista[j][(x+1)%d].ciclista == NULL) {
                            achouEspacoFrente = true;
                            break;
                        }
                    }

                    // Caso 1.1: ultrapassagem é possível ---> ultrapassa com posição final (j,x+1); j é uma pista externa livre
                    if (achouPistaExt && achouEspacoFrente) pthread_mutex_lock(&pista[j][(x+1)%d].mutex);
                    if (achouPistaExt && achouEspacoFrente && !pista[j][(x+1)%d].ocupada) {
                        if (DEBUG) fprintf(stderr, "\tciclista %d (Caso 1.1)\n", p->num);
                        if (DEBUG) fprintf(stderr, "\tciclista %d (Caso 1.1) j: %d\n", p->num, j);
                        pista[j][(x+1)%d].ciclista = p;
                        pthread_mutex_lock(&pista[y][x].mutex);
                        pista[y][x].ciclista = NULL;
                        pista[y][x].ocupada = false;
                        pthread_mutex_unlock(&pista[y][x].mutex);
                        p->px = (x+1)%d;
                        p->py = j;
                        pista[j][(x+1)%d].ocupada = true;
                        pthread_mutex_unlock(&pista[j][(x+1)%d].mutex);
                    }

                    else { // Caso 1.2: ultrapassagem não é possível ---> unlock/sleep/lock até o cara da frente terminar o round
                            // esperar o cara da frente terminar o round leva a livelock (pode ser um cara que avançou e parou ali)
                            // verificar novamente se é possível ultrapassar
                            // se realmente não for possível, reduzir a velocidade
                            if (achouPistaExt && achouEspacoFrente) pthread_mutex_unlock(&pista[j][(x+1)%d].mutex);
                            int countErros = 0; // Eu fiz temporariamente para achar um livelock (depois eu removo)
                        while (pista[y][(x+1)%d].ciclista != NULL &&
                            pista[y][(x+1)%d].ciclista->roundFeito == false) {
                                if (DEBUG) fprintf(stderr, "\t(UNLOCK) ciclista %d (Caso 1.2) %d (original: %d)\n", p->num, pista[y][(x+1)%d].ciclista->num, pista[y][(x+1)%d].ciclista->roundFeito);
                                //pthread_mutex_unlock(&mutex);
                                nanosleep(&ts, NULL);
                                //pthread_mutex_lock(&mutex);
                                // if (DEBUG) fprintf(stderr, "\t(LOCK) ciclista %d (Caso 1.2) %d (original: %d)\n", p->num, pista[y][(x+1)%d]->num, pista[y][(x+1)%d]->roundFeito); //GDB SEGFAULT
                                countErros++;
                                if (countErros > 100) {
                                    if (DEBUG) fprintf(stderr, "\t(ERRO) ciclista %d (Caso 1.2) %d (original: %d)\n", p->num, pista[y][(x+1)%d].ciclista->num, pista[y][(x+1)%d].ciclista->roundFeito);
                                    visualizadorStderr();
                                    exit(1);
                                }
                        }
                        pthread_mutex_lock(&pista[y][(x+1)%d].mutex);
                        if (pista[y][(x+1)%d].ciclista == NULL && !pista[y][(x+1)%d].ocupada) { // casa da frente liberou
                            // anda pra frente
                            pista[y][(x+1)%d].ciclista = p;
                            pthread_mutex_lock(&pista[y][x].mutex);
                            pista[y][x].ciclista = NULL;
                            pista[y][x].ocupada = false;
                            pthread_mutex_unlock(&pista[y][x].mutex);
                            p->px = (x+1)%d;
                            pista[y][(x+1)%d].ocupada = true;
                            pthread_mutex_unlock(&pista[y][(x+1)%d].mutex);
                        }
                        else { // casa da frente está ocupada
                            pthread_mutex_unlock(&pista[y][(x+1)%d].mutex);
                            // não faz nada
                            if (DEBUG) fprintf(stderr, "\t(ERRO) ciclista %d (Caso 1.2) (VELOCIDADE REDUZIDA -> NÃO ANDOU)\n", p->num);
                        }
                    }
                    if (DEBUG) fprintf(stderr, "\tciclista %d (Caso 1.1 saída) j: %d\n", p->num, j);
                }
                if (DEBUG) fprintf(stderr, "\tciclista %d (saída 2) (%d, %d)\n", p->num, p->py, p->px);

                // Fim dos casos
                // Etapas finais da tarefa i
                if (p->px == 0) { // Verifica se completou uma volta para alterar a velocidade
                    if (DEBUG) fprintf(stderr, "Ciclista: %d (x == 0), volta: %d\n", p->num, p->voltas);
                    //if (DEBUG) printf(">.... insere ciclista %d, volta: %d\n", p->num, p->voltas);
                    InsereCiclista(L, n, p->voltas, p->num, tempo);
                    (p->voltas)++; // completou uma volta ou iniciou a corrida pós largada
                    velocidade(p);
                    // @@@ preciso registrar no ranqueamento com um mutex?
                }
                p->dt = 2 - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")

                // Ao fim de sua movimentação, o ciclista trata de ir para a pista mais interna possível
                //int yMaisInterno = p->py;
                // Procura se há pista mais interna livre (coloquei para poder pular ciclistas, pois estava gerando livelock1)
                // V2: verificar na subida se há algum ciclista que vai avançar (ainda não andou, mas dt == 0)
                // for (int i = 0; i < p->py; i++) {
                //     if (DEBUG) fprintf(stderr, "\tciclista %d (loop pista interna) (%d, %d) i:%d\n", p->num, p->py, p->px, i);
                //     if (pista[i][p->px] == NULL && (pista[i][mod(p->px - 1, d)] != NULL && pista[i][mod(p->px - 1, d)]->roundFeito == false &&  pista[i][mod(p->px - 1, d)]->dt == 0)) {
                //         if (DEBUG) fprintf(stderr, "\tciclista %d (loop pista interna) (%d, %d) -> i:%d\n", p->num, p->py, p->px, i);
                //         yMaisInterno = i;
                //         break;
                //     }
                // }

                // if (yMaisInterno < p->py) {
                //     pista[p->py][p->px] = NULL;
                //     pista[yMaisInterno][p->px] = p;
                //     p->py = yMaisInterno;
                // }
                //if (DEBUG) fprintf(stderr, "\t(UNLOCK) ciclista %d\n", p->num);
                //pthread_mutex_unlock(&mutex);
            }

            p->roundFeito = true;
            // // Teste sleep + subidas ao fim (sem a regra de ver se falta andar)
            // nanosleep(&ts, NULL);
            // pthread_mutex_lock(&mutex);
            // int yMaisInterno = p->py;
            //
            // if (yMaisInterno < p->py) {
            //     pista[p->py][p->px] = NULL;
            //     pista[yMaisInterno][p->px] = p;
            //     p->py = yMaisInterno;
            // }
            //pthread_mutex_unlock(&mutex);
        }

        // Barreira de sincronização
        p->arrive = 1;
        while (!p->Continue) usleep(1);
        if (DEBUG) fprintf(stderr, "(Continue) Ciclista: %d, volta: %d, vel: %d\n", p->num, p->voltas, p->velocidade);
        p->Continue = 0;
    }
  return NULL;
}

void * juiz(void * arg)
{
    if (DEBUG) printf("Juiz está pronto\n");

    ciclista *c =(ciclista *) arg;

    /*Flags auxiliares*/
    int minVolta = 0; // Mínimo das voltas locais dos ciclistas
    int maxVolta = 0; // Máximo das voltas locais dos ciclistas
    int menorVolta = 0; // Menor volta local do coordenador
    int maiorVolta = 0; // Maior volta local do coordenador
    bool terminouVolta = false;

    while (true) {
        for (ciclista * p = c->prox; p != cab; p = p->prox) {
            while (p->arrive == 0) usleep(1);
            p->arrive = 0;
        }

        if (true) { // if temporário (código da thread coordenadora)
            minVolta = maxVolta;
            for (ciclista * p = c->prox; p != cab; p = p->prox) {
                if (maxVolta < p->voltas) maxVolta = p->voltas;
                if (minVolta > p->voltas) minVolta = p->voltas;
            }
            terminouVolta = false;
            if (maiorVolta != maxVolta) {
                maiorVolta = maxVolta;
            }
            if (menorVolta != minVolta) {
                menorVolta = minVolta;
                terminouVolta = true;
            }
            usleep(60000);
            if (terminouVolta) visualizador();
            // visualizador();
            if (DEBUG && terminouVolta) visualizadorStderr();
            if (DEBUG) {
                printf("Juiz: maxVolta: %d, minVolta: %d, terminouVolta: %d\n\n", maxVolta, minVolta, terminouVolta);
            if (DEBUG) fprintf(stderr, "Juiz: maxVolta: %d, minVolta: %d\n\n", maxVolta, minVolta);
            }
            if (terminouVolta) {
                if (DEBUG)printf("imprimeRank (volta %d)\n", menorVolta - 1);
                imprimeRank(L, menorVolta - 1);
                if (menorVolta%2 == 0) {
                    int ultimo = ultimoColocado(L, menorVolta - 1);
                    if (DEBUG)printf("Ultimo colocado: %d\n", ultimo);

                    ciclista *q, *anterior;
                    anterior = c;
                    for (q = c->prox; q != cab; q = q->prox) { // encontra a thread do ultimo colocado
                        if (q->num == ultimo) break;
                        anterior = q;
                    }
                    pista[q->py][q->px].ciclista = NULL;
                    pista[q->py][q->px].ocupada = false;
                    InsereCiclistaRank(rankFinal, q->num, tempo);
                    pthread_cancel(q->id);
                    // Remover da lista de threads
                    anterior->prox = q->prox;
                    free(q);
                    if (DEBUG)printf("imprimeRank final (volta %d)\n", menorVolta - 1);
                    imprimeRankFinal(rankFinal);

                    // Remover as voltas anteriores da ED
                        // Alternativa é eliminar as voltas ímpares assim que imprimir o rank
                        // Descobrir na ED os últimos colocados OK
                            // Se houver mais de um, sortear algum OK
                    // Eliminar o último colocado
                        // Eliminação: remover sua posição da pista, parar a thread, destrui-la, remover da lista de threads
                    // Eliminar da ED a volta mais antiga
                }
            }
            tempo++;
        }

        /*Busca pistas internas para os ciclistas*/
        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
            if (p->py != 0 && pista[(p->py) -1][p->px].ciclista == NULL) {
              pista[p->py][p->px].ciclista = NULL;
              pista[p->py][p->px].ocupada = false;
              pista[(p->py) -1 ][p->px].ciclista = p;
              pista[(p->py) -1 ][p->px].ocupada = true;
              p->py = (p->py) -1;
            }
        }
        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
            p->Continue = 1;
        }
        if (DEBUG) fprintf(stderr, "Juiz loop fim\n\n");
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
          if (pista[i][j].ciclista != NULL) {
              printf("%02d",pista[i][j].ciclista->num);
              if (pista[i][j].ciclista->velocidade == 1)
                printf("|");
              else if (pista[i][j].ciclista->velocidade == 2)
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
  fprintf(stderr, "\n");
  for (int i = 0; i < d; i++) {
      for (int j = 0; j < 10; j++) {
          if (pista[j][i].ciclista != NULL) {
              fprintf(stderr, "%03d|",pista[j][i].ciclista->num);
          }
          else fprintf(stderr, "   |");
      }
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n\n");
}

/* Calcula probabilidade e muda velocidade */
void velocidade(ciclista *p)
{
    double prob = randReal(0, 1);
    if (DEBUG) fprintf(stderr, "Função velocidade (prob: %lf)\n", prob);
    if (p->voltas <= 1) p->velocidade = 1; // velocidade na primeira volta é 30km/h
    else if (p->velocidade == 1 && prob < 0.8)
        p->velocidade = 2;
    else if (p->velocidade == 2 && prob < 0.4)
        p->velocidade = 1;
}
