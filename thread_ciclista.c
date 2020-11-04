#include "thread_ciclista.h"
#include "rank.h"

#define NSLEEP 10
#define PROB_QUEBRA 0.05

// Variáveis globais
extern ciclista ***pista;
extern int d, n;
extern int dt_base;
extern _Atomic int nCiclistasAtivos;
extern bool ciclistaQuebrou;
extern bool tem90;
extern int nCiclista90;
extern bool esperandoSegundoUltimasVoltas;
extern _Atomic long long int tempo;
extern pthread_mutex_t **mutex;
extern pthread_mutex_t mutexInsere;
extern ListaRank L;
extern Rank rankFinal;
extern Rank rankQuebras;
extern long memTotal;

void * competidor(void * arg)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    // lê uso de memoria
    struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);
    memTotal += usage.ru_maxrss;

    ciclista *p = (ciclista *) arg;

    velocidade(p);
    p->dt = dt_base - p->velocidade;
    while (true) {
        if (true) { // Tarefa  da thread
            p->roundFeito = false;
            p->dt = p->dt - p->velocidade;
            if (p->dt <= 0) { // A velocidade permite o ciclista avançar a cada iteração
                bool avancou = false;
                int contTentativas = 0;
                while (contTentativas++ < 3) { // Tenta avançar algumas vezes, esperando que outro ciclista na frente ande
                    if (pthread_mutex_trylock(&mutex[p->py][(p->px + 1)%d]) == 0) {
                        if (pista[p->py][(p->px + 1)%d] == NULL) { // Caso 0: a posição da frente está livre
                            moveFrente(p);
                            pthread_mutex_unlock(&mutex[p->py][p->px]);
                            avancou = true;
                            break;
                        }
                    }
                    else
                        nanosleep(&ts, NULL);
                }
                if (!avancou) {
                    pthread_mutex_unlock(&mutex[p->py][(p->px + 1)%d]);
                    movePistaExterna(p);
                }
                if (p->px == 0)
                    tratalinhaDechegada(p);
                p->dt = dt_base; // atualiza dt para a pŕoxima iteração
            }
            p->roundFeito = true;
            nanosleep(&ts, NULL);
            movePistaInterna(p);
        }

        // Barreira de sincronização
        p->arrive = 1;
        while (!p->Continue) usleep(1);
        p->Continue = 0;
    }
  return NULL;
}

// Define a velocidade do ciclista
void velocidade(ciclista *p) {
    double prob = randReal(0, 1);
    if (tem90 && !esperandoSegundoUltimasVoltas)
        if (nCiclista90 == p->num) {
            p->velocidade = 3;
            return;
        }
    if (p->voltas < 1) p->velocidade = 1;
    else if (p->velocidade == 1 && prob < 0.8)
        p->velocidade = 2;
    else if (p->velocidade == 2 && prob < 0.4)
        p->velocidade = 1;
}

// Anda pra frente
void moveFrente(ciclista *p) {
    pista[p->py][(p->px+1)%d] = p;
    pista[p->py][p->px] = NULL;
    p->px = (p->px+1)%d;
}

// Procura uma pista externa para avançar
void movePistaExterna(ciclista *p) {
    int i, j;
    bool achouPistaExt = false;
    for (i = p->py + 1; i < 10; i++) { // Procura se há pista externa livre
        pthread_mutex_lock(&mutex[i][p->px]);
        if (pista[i][p->px] == NULL) {
            achouPistaExt = true;
            break;
        }
        pthread_mutex_unlock(&mutex[i][p->px]);
    }

    if (achouPistaExt) {
        pista[i][p->px] = p;
        pista[p->py][p->px] = NULL;
        p->py = i;
        pthread_mutex_unlock(&mutex[i][p->px]);
    }
    if (achouPistaExt) {
        for (j = 0; j < 10; j++) { // Procura se há espaço na linha da frente
            pthread_mutex_lock(&mutex[j][(p->px+1)%d]);
            if (pista[j][(p->px+1)%d] == NULL) {
                pista[j][(p->px+1)%d] = p;
                pista[p->py][p->px] = NULL;
                p->px = (p->px+1)%d;
                p->py = j;
                pthread_mutex_unlock(&mutex[j][p->px]);
                break;
            }
            pthread_mutex_unlock(&mutex[j][(p->px+1)%d]);
        }
    }
}

// Tratamento do ciclista na linha de chegada. Faz sorteio para ver se ele vai
// quebrar, se quebrar, ativa a flag p->quebrou. Caso contrário, insere-o no
// rank da volta completada e recalcula sua velocidade.
void tratalinhaDechegada(ciclista *p) {
    bool quebrou = false;
    (p->voltas)++; // completou uma volta ou iniciou a corrida pós largada
    if (p->voltas > 0 && nCiclistasAtivos > 5 && (p->voltas)%6 == 0) { // verificar se há quebra
        if(randReal(0, 1) < PROB_QUEBRA) {
            ciclistaQuebrou = true;
            nCiclistasAtivos--;
            p->quebrou = true;
            quebrou = true;
            printf("Quebra! O ciclista %d quebrou na volta %d.\n", p->num, p->voltas);
        }
    }
    if (!quebrou) {
        pthread_mutex_lock(&mutexInsere);
        InsereCiclista(L, n, p->voltas, p->num, tempo);
        pthread_mutex_unlock(&mutexInsere);
        velocidade(p);
    }
}

// Ao fim de sua movimentação, o ciclista trata de ir para a pista mais interna possível
void movePistaInterna(ciclista *p) {
    bool achou = false;
    int i = p->py;
    for ( ; i >= 0; i--) {
        pthread_mutex_lock(&mutex[i][p->px]);
        if (pista[i][p->px] == NULL) {
            achou = true;
            break;
        }
        pthread_mutex_unlock(&mutex[i][p->px]);
    }
    if (achou) {
        if (i < p->py) {
            pista[p->py][p->px] = NULL;
            pista[i][p->px] = p;
            p->py = i;
        }
        pthread_mutex_unlock(&mutex[i][p->px]);
    }
}
