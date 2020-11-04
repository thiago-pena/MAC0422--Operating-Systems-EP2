#include "thread_ciclista.h"
#include "rank.h"

#define DEBUG 0
#define DEBUG2 0
#define DEBUGMUTEX 0
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

void * competidor(void * arg) {
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
        if (true) { // código da tarefa i
            if (DEBUG) printf("loop ciclista %d, volta: %d, x: %d, v: %d\n", p->num, p->voltas, p->px, p->velocidade);
            if (DEBUG) fprintf(stderr, "ciclista %d, volta: %d, x: %d, v: %d\n", p->num, p->voltas, p->px, p->velocidade);
            p->linhaDeChegada = false;
            p->roundFeito = false;
            p->dt = p->dt - p->velocidade;
            if (p->dt <= 0) { // A velocidade permite o ciclista avançar a cada iteração
                bool avancou = false;

                int contTentativas = 0;
                while (contTentativas++ < 3) {
                    if (pthread_mutex_trylock(&mutex[p->py][(p->px + 1)%d]) == 0) {
                        if (DEBUGMUTEX) fprintf(stderr, "LOCK1   (%d, %d) ciclista %d, volta: %d\n", p->py, (p->px + 1)%d, p->num, p->voltas);
                        if (pista[p->py][(p->px + 1)%d] == NULL) { // Caso 0: a posição da frente está livre
                            moveFrente(p);
                            if (DEBUGMUTEX) fprintf(stderr, "UNLOCK1 (%d, %d) ciclista %d, volta: %d\n", p->py, p->px, p->num, p->voltas);
                            pthread_mutex_unlock(&mutex[p->py][p->px]);
                            avancou = true;
                            break;
                        }
                    }
                    else
                        nanosleep(&ts, NULL);
                }
                if (!avancou) {// Caso 1: a posição da frente está ocupada
                    if (DEBUGMUTEX) fprintf(stderr, "UNLOCK1 (%d, %d) ciclista %d, volta: %d\n", p->py, (p->px + 1)%d, p->num, p->voltas);
                    pthread_mutex_unlock(&mutex[p->py][(p->px + 1)%d]);
                    moveTemp(p);
                }
                if (p->voltas >= 4) {
                    if (DEBUGMUTEX) fprintf(stderr, "[DEBUG erro] ciclista: %d, p->voltas: %d, p->px: %d\n", p->num, p->voltas, p->px);
                }
                if (p->px == 0)// Verifica se completou uma volta para alterar a velocidade
                    tratalinhaDechegada(p);
                p->dt = dt_base; // atualiza dt para a pŕoxima iteração
                // movePistaInterna(p);
            }
            p->roundFeito = true;
            nanosleep(&ts, NULL);
            movePistaInterna2(p);
        }

        // Barreira de sincronização
        p->arrive = 1;
        while (!p->Continue) usleep(1);
        p->Continue = 0;
    }
  return NULL;
}

/* Calcula probabilidade e muda velocidade */
void velocidade(ciclista *p) {
    double prob = randReal(0, 1);
    if (tem90 && !esperandoSegundoUltimasVoltas)
        if (nCiclista90 == p->num) {
            p->velocidade = 3;
            if (DEBUG)printf("Ciclista %d comecou a pedalar a 90km/h (ciclista: %d, nCiclista90: %d)\n", p->num, p->num, nCiclista90);
            return;
        }
    if (p->voltas < 1) p->velocidade = 1;
    else if (p->velocidade == 1 && prob < 0.8)
        p->velocidade = 2;
    else if (p->velocidade == 2 && prob < 0.4)
        p->velocidade = 1;
}

// anda pra frente
void moveFrente(ciclista *p) {
    pista[p->py][(p->px+1)%d] = p;
    pista[p->py][p->px] = NULL;
    p->px = (p->px+1)%d;
}

void moveTemp(ciclista *p) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    int i, j;
    bool achouPistaExt = false; // Tem pista externa livre?
    bool achouEspacoFrente = false; // Tem espaço na pista da frente?
    for (i = p->py + 1; i < 10; i++) { // Procura se há pista externa livre
        pthread_mutex_lock(&mutex[i][p->px]);
        if (DEBUGMUTEX) fprintf(stderr, "LOCK2i   (%d, %d) ciclista %d, volta: %d\n", i, p->px, p->num, p->voltas);
        if (pista[i][p->px] == NULL) {
            achouPistaExt = true;
            break;
        }
        if (DEBUGMUTEX) fprintf(stderr, "UNLOCK2i (%d, %d) ciclista %d, volta: %d\n", i, p->px, p->num, p->voltas);
        pthread_mutex_unlock(&mutex[i][p->px]);
    }

    if (achouPistaExt) {
        if (DEBUGMUTEX) fprintf(stderr, "ciclista %d, volta: %d || (%d, %d)->(%d, %d)\n", p->num, p->voltas, p->py, p->px, i, p->px);
        pista[i][p->px] = p;
        pista[p->py][p->px] = NULL;
        p->py = i;
        if (DEBUGMUTEX) fprintf(stderr, "UNLOCK2i (%d, %d) ciclista %d, volta: %d\n", i, p->px, p->num, p->voltas);
        pthread_mutex_unlock(&mutex[i][p->px]);
    }
    if (achouPistaExt) {
        for (j = 0; j < 10; j++) { // Procura se há espaço na linha da frente
            pthread_mutex_lock(&mutex[j][(p->px+1)%d]);
            if (DEBUGMUTEX) fprintf(stderr, "LOCK2j   (%d, %d) ciclista %d, volta: %d\n", j, (p->px+1)%d, p->num, p->voltas);
            if (pista[j][(p->px+1)%d] == NULL) {
                if (DEBUGMUTEX) fprintf(stderr, "ciclista %d, volta: %d || (%d, %d)->(%d, %d)\n", p->num, p->voltas, p->py, p->px, j, (p->px+1)%d);
                achouEspacoFrente = true;
                pista[j][(p->px+1)%d] = p;
                pista[p->py][p->px] = NULL;
                p->px = (p->px+1)%d;
                p->py = j;
                if (DEBUGMUTEX) fprintf(stderr, "UNLOCK2j (%d, %d) ciclista %d, volta: %d\n", j, p->px, p->num, p->voltas);
                pthread_mutex_unlock(&mutex[j][p->px]);
                break;
            }
            if (DEBUGMUTEX) fprintf(stderr, "UNLOCK2j (%d, %d) ciclista %d, volta: %d\n", j, (p->px+1)%d, p->num, p->voltas);
            pthread_mutex_unlock(&mutex[j][(p->px+1)%d]);
        }
    }
}

void tratalinhaDechegada(ciclista *p) {
    bool quebrou = false;
    // if (DEBUG) fprintf(stderr, "Ciclista: %d (funcao tratalinhaDechegada), volta: %d, p->px: %d\n", p->num, p->voltas, p->px);
    p->linhaDeChegada = true;
    (p->voltas)++; // completou uma volta ou iniciou a corrida pós largada
    if (p->voltas > 0 && nCiclistasAtivos > 5 && (p->voltas)%6 == 0) { // verificar se há quebra
        if(randReal(0, 1) < PROB_QUEBRA) {
            ciclistaQuebrou = true;
            nCiclistasAtivos--;
            p->quebrou = true;
            quebrou = true;
            if (DEBUG) printf("Quebra! O ciclista %d quebrou na volta %d.\n", p->num, p->voltas);
            if (DEBUG) fprintf(stderr, "Quebra! O ciclista %d quebrou na volta %d.\n", p->num, p->voltas);
        }
    }
    if (!quebrou) {
        pthread_mutex_lock(&mutexInsere);
        InsereCiclista(L, n, p->voltas, p->num, tempo);
        pthread_mutex_unlock(&mutexInsere);
        velocidade(p);
        // @@@ preciso registrar no ranqueamento com um mutex?
    }
}

// Ao fim de sua movimentação, o ciclista trata de ir para a pista mais interna possível
// Procura se há pista mais interna livre (coloquei para poder pular ciclistas, pois estava gerando livelock1)
// V2: verificar na subida se há algum ciclista que vai avançar (ainda não andou, mas dt == 0)
void movePistaInterna(ciclista *p) {
    int yMaisInterno = p->py;
    for (int i = 0; i < p->py; i++) {
        if (pista[i][p->px] == NULL && (pista[i][mod(p->px - 1, d)] != NULL && pista[i][mod(p->px - 1, d)]->roundFeito == false &&  pista[i][mod(p->px - 1, d)]->dt == 0)) {
            yMaisInterno = i;
            break;
        }
    }

    if (yMaisInterno < p->py) {
        pista[p->py][p->px] = NULL;
        pista[yMaisInterno][p->px] = p;
        p->py = yMaisInterno;
    }
    // pthread_mutex_lock(&mutex[][]);
    // pthread_mutex_unlock(&mutex[][]);
}


void movePistaInterna2(ciclista *p) {
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

// Para números negativos, mod é diferente do resultado do operador resto (%)
// Foi útil para passar os ciclistas para as pistas mais internas, pois
// (p->px - 1)%d não dava o resto, mas sim um número negativo.
int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}
