#include "thread_ciclista.h"
#include "rank.h"

#define DEBUG 1
#define DEBUG2 1
#define NSLEEP 100000000 // 0.1s = 1E-1
#define PROB_90 0.1 // @alterar para 0.1 -> probabilidade de um ciclista ter 90km/h nas últimas voltas
#define PROB_QUEBRA 0.001 // @alterar para 0.05 -> probabilidade de um ciclista quebrar ao completar uma volta


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
extern bool ultimasVoltas;
extern bool ciclistaQuebrou;

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
    p->dt = dt_base - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")
    while (true) {
        if (true) { // código da tarefa i
            p->linhaDeChegada = false;
            if (DEBUG2) fprintf(stderr, "(ini loop) Ciclista: %d, volta: %d, vel: %d, dt: %d\n", p->num, p->voltas, p->velocidade, p->dt);

            p->roundFeito = false;
            int x = p->px;
            int y = p->py;
            if (p->dt > 0) { // A velocidade do ciclista requer mais de uma iteração para avançar
                (p->dt)--;
                if (DEBUG2) fprintf(stderr, "\t(dt > 0 -> requer mais uma iteração para andar) loop ciclista %d\n", p->num);
            }
            else { // A velocidade permite o ciclista avançar a cada iteração
                if (DEBUG2) fprintf(stderr, "\t(1) loop ciclista %d\n", p->num);
                pthread_mutex_lock(&mutex);
                if (DEBUG2) fprintf(stderr, "\t(LOCK) ciclista %d\n", p->num);

                if (pista[y][(x + 1)%d] == NULL) { // Caso 0: a posição da frente está livre
                    moveFrente(p);
                }
                else { // Caso 1: a posição da frente está ocupada
                    moveTemp(p);
                }
                if (DEBUG2) fprintf(stderr, "\tciclista %d (saída 2) (%d, %d)\n", p->num, p->py, p->px);

                // Fim dos casos
                // Etapas finais da tarefa i
                if (p->px == 0) { // Verifica se completou uma volta para alterar a velocidade
                    p->linhaDeChegada = true;
                    (p->voltas)++; // completou uma volta ou iniciou a corrida pós largada
                    if (nCiclistasAtivos > 5 && (p->voltas)%6 == 0) { // verificar se há quebra
                        if(randReal(0, 1) < PROB_QUEBRA) {
                            ciclistaQuebrou = true;
                            p->quebrou = true;
                            fprintf(stderr, "Quebra! O ciclista %d quebrou na volta %d.\n", p->num, p->voltas);
                        }
                    }
                    else { // não quebrou
                        if (DEBUG) fprintf(stderr, "Ciclista: %d (x == 0), volta: %d\n", p->num, p->voltas);
                        printf(">.... insere ciclista %d, volta: %d\n", p->num, p->voltas);
                        InsereCiclista(L, n, p->voltas, p->num, tempo);
                        velocidade(p);
                        // @@@ preciso registrar no ranqueamento com um mutex?
                    }
                }
                p->dt = dt_base - p->velocidade; // atualiza dt para a pŕoxima iteração (para as 2 últimas voltas, devemos fazer "3 - p->velocidade")

                // Ao fim de sua movimentação, o ciclista trata de ir para a pista mais interna possível
                int yMaisInterno = p->py;
                // Procura se há pista mais interna livre (coloquei para poder pular ciclistas, pois estava gerando livelock1)
                // V2: verificar na subida se há algum ciclista que vai avançar (ainda não andou, mas dt == 0)
                for (int i = 0; i < p->py; i++) {
                    if (DEBUG2) fprintf(stderr, "\tciclista %d (loop pista interna) (%d, %d) i:%d\n", p->num, p->py, p->px, i);
                    if (pista[i][p->px] == NULL && (pista[i][mod(p->px - 1, d)] != NULL && pista[i][mod(p->px - 1, d)]->roundFeito == false &&  pista[i][mod(p->px - 1, d)]->dt == 0)) {
                        if (DEBUG2) fprintf(stderr, "\tciclista %d (loop pista interna) (%d, %d) -> i:%d\n", p->num, p->py, p->px, i);
                        yMaisInterno = i;
                        break;
                    }
                }

                if (yMaisInterno < p->py) {
                    pista[p->py][p->px] = NULL;
                    pista[yMaisInterno][p->px] = p;
                    p->py = yMaisInterno;
                }
                if (DEBUG) fprintf(stderr, "\t(UNLOCK) ciclista %d\n", p->num);
                pthread_mutex_unlock(&mutex);
            }
            if (DEBUG2) fprintf(stderr, "\t(fim tratamento dt) loop ciclista %d\n", p->num);

            p->roundFeito = true;
            // Teste sleep + subidas ao fim (sem a regra de ver se falta andar)
            nanosleep(&ts, NULL);
            pthread_mutex_lock(&mutex);
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
            pthread_mutex_unlock(&mutex);
        }
        if (DEBUG2) fprintf(stderr, "(arrive) Ciclista: %d, volta: %d, vel: %d, dt: %d\n", p->num, p->voltas, p->velocidade, p->dt);

        // Barreira de sincronização
        p->arrive = 1;
        while (!p->Continue) usleep(1);
        if (DEBUG2) fprintf(stderr, "(Continue) Ciclista: %d, volta: %d, vel: %d\n", p->num, p->voltas, p->velocidade);
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
    if (p->voltas <= 1) p->velocidade = 1; // velocidade na primeira volta é 30km/h
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

    if (DEBUG2) fprintf(stderr, "\tciclista %d (Caso 1)\n", p->num);
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
        if (DEBUG2) fprintf(stderr, "\tciclista %d (Caso 1.1)\n", p->num);
        if (DEBUG2) fprintf(stderr, "\tciclista %d (Caso 1.1) j: %d\n", p->num, j);
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
                if (DEBUG) fprintf(stderr, "\t(UNLOCK) ciclista %d (Caso 1.2) %d (original: %d)\n", p->num, pista[p->py][(p->px+1)%d]->num, pista[p->py][(p->px+1)%d]->roundFeito);
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
    if (DEBUG2) fprintf(stderr, "\tciclista %d (Caso 1.1 saída) j: %d\n", p->num, j);
}
