#include "thread_ciclista.h"
#include "rank.h"

#define DEBUG 1
#define DEBUG2 1
#define NSLEEP 100000000 // 0.1s = 1E-1
#define PROB_QUEBRA 0.05 // @alterar para 0.05 -> probabilidade de um ciclista quebrar ao completar uma volta

// Variáveis globais
extern ciclista ***pista;
extern ciclista *cab;
extern int d, n;
extern _Atomic int nCiclistasAtivos;
extern _Atomic bool tem90;
extern int nCiclista90; // número do ciclista que terá 90km/h, se ocorrer
extern int dt_base; // base do delta de velocidade (2 padrão, 3 se tiver ciclista a 90km/h
extern pthread_mutex_t mutex;
extern _Atomic long long int tempo;
extern ListaRank L;
extern Rank rankFinal;
extern Rank rankQuebras;
extern bool ultimasVoltas;
extern bool ciclistaQuebrou;

void * competidor(void * arg)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    ciclista *p = (ciclista *) arg;

    if (DEBUG) {
        printf("Ciclista: %d pronto\n", p->num );
        fprintf(stderr, "Ciclista: %d pronto\n", p->num);
    }
    velocidade(p); // Velocidade inicial na primeira volta (30km/h)
    p->dt = dt_base - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")
    while (true) {
        if (true) { // código da tarefa i
            p->linhaDeChegada = false;
            p->roundFeito = false;
            if (p->dt > 0) { // A velocidade do ciclista requer mais de uma iteração para avançar
                (p->dt)--;
            }
            else { // A velocidade permite o ciclista avançar a cada iteração
                pthread_mutex_lock(&mutex);
                if (pista[p->py][(p->px + 1)%d] == NULL) // Caso 0: a posição da frente está livre
                    moveFrente(p);
                else // Caso 1: a posição da frente está ocupada
                    moveTemp(p);
                if (p->voltas >= 4) {
                    fprintf(stderr, "[DEBUG erro] ciclista: %d, p->voltas: %d, p->px: %d\n", p->num, p->voltas, p->px);
                }
                if (p->px == 0)// Verifica se completou uma volta para alterar a velocidade
                    tratalinhaDechegada(p);
                p->dt = dt_base - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")
                movePistaInterna(p);
                pthread_mutex_unlock(&mutex);
            }
            p->roundFeito = true;
            nanosleep(&ts, NULL);
            pthread_mutex_lock(&mutex);
            movePistaInterna2(p);
            pthread_mutex_unlock(&mutex);
        }

        // Barreira de sincronização
        p->arrive = 1;
        while (!p->Continue) usleep(1);
        p->Continue = 0;
    }
  return NULL;
}

/* Calcula probabilidade e muda velocidade */
void velocidade(ciclista *p)
{
    double prob = randReal(0, 1);
    if (DEBUG) fprintf(stderr, "Função velocidade (prob: %lf)\n", prob);
    if (tem90 && nCiclista90 == p->num)
        p->velocidade = 3;
    if (p->voltas < 1) p->velocidade = 1; // velocidade na primeira volta é 30km/h
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

    int j;
    bool achouPistaExt = false; // Tem pista externa livre?
    bool achouEspacoFrente = false; // Tem espaço na pista da frente?
    for (int i = p->py + 1; i < 10; i++) { // Procura se há pista externa livre
        if (pista[i][p->px] == NULL) {
            achouPistaExt = true;
            break;
        }
    }
    for (j = 0; j < 10; j++) { // Procura se há espaço na linha da frente
        if (pista[j][(p->px+1)%d] == NULL) {
            achouEspacoFrente = true;
            break;
        }
    }

    // Caso 1.1: ultrapassagem é possível ---> ultrapassa com posição final (j,x+1); j é uma pista externa livre
    if (achouPistaExt && achouEspacoFrente) {
        pista[j][(p->px+1)%d] = p;
        pista[p->py][p->px] = NULL;
        p->px = (p->px+1)%d;
        p->py = j;
    }
    else { // Caso 1.2: ultrapassagem não é possível ---> unlock/sleep/lock até o cara da frente terminar o round
            // esperar o cara da frente terminar o round leva a livelock (pode ser um cara que avançou e parou ali)
            // verificar novamente se é possível ultrapassar
            // se realmente não for possível, reduzir a velocidade
        while (pista[p->py][(p->px+1)%d] != NULL &&
            pista[p->py][(p->px+1)%d]->roundFeito == false) {
                pthread_mutex_unlock(&mutex);
                nanosleep(&ts, NULL);
                pthread_mutex_lock(&mutex);
        }
        if (pista[p->py][(p->px+1)%d] == NULL) { // casa da frente liberou
            // anda pra frente
            pista[p->py][(p->px+1)%d] = p;
            pista[p->py][p->px] = NULL;
            p->px = (p->px+1)%d;
        }
        else { // casa da frente está ocupada
            // não faz nada
            if (DEBUG) fprintf(stderr, "\tciclista %d (Caso 1.2) (VELOCIDADE REDUZIDA -> NÃO ANDOU)\n", p->num);
        }
    }
}

void tratalinhaDechegada(ciclista *p) {
    bool quebrou = false;
    if (DEBUG) fprintf(stderr, "Ciclista: %d (funcao tratalinhaDechegada), volta: %d, p->px: %d\n", p->num, p->voltas, p->px);
    p->linhaDeChegada = true;
    (p->voltas)++; // completou uma volta ou iniciou a corrida pós largada
    if (p->voltas > 0 && nCiclistasAtivos > 5 && (p->voltas)%6 == 0) { // verificar se há quebra
        if(randReal(0, 1) < PROB_QUEBRA) {
            ciclistaQuebrou = true;
            nCiclistasAtivos--;
            p->quebrou = true;
            quebrou = true;
            fprintf(stderr, "Quebra! O ciclista %d quebrou na volta %d.\n", p->num, p->voltas);
        }
    }
    if (!quebrou) { // não quebrou
        if (DEBUG) fprintf(stderr, "Ciclista: %d (x = 0), volta: %d\n", p->num, p->voltas);
        printf(">.... insere ciclista %d, volta: %d\n", p->num, p->voltas);
        fprintf(stderr, ">.... insere ciclista %d, volta: %d\n", p->num, p->voltas);
        InsereCiclista(L, n, p->voltas, p->num, tempo);
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
}

void movePistaInterna2(ciclista *p) {
    int yMaisInterno = p->py;
    for (int i = p->py; i >= 0; i--) {
        if (pista[i][p->px] == NULL) {
            yMaisInterno = i;
            break;
        }
    }
    if (yMaisInterno < p->py) {
        pista[p->py][p->px] = NULL;
        pista[yMaisInterno][p->px] = p;
        p->py = yMaisInterno;
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
