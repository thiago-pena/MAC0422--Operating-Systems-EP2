#include "thread_coordenador.h"
#include "rank.h"

#define DEBUG 1
#define DEBUG2 0
#define DEBUG3 0
#define DEBUG4 1
#define DEBUGVOLTAS 1
#define DEBUGVIEW 1
#define DEBUGMUTEX 0
#define PROB_90 0.9999 // @alterar para 0.1 -> probabilidade de um ciclista ter 90km/h nas últimas voltas
#define NSLEEP 100000 // 0.01s = 1E-2 = 10ms

// Variáveis globais
extern ciclista ***pista;
extern ciclista *cab;
extern int d, n;
extern _Atomic int nCiclistasAtivos, nEliminados, nQuebras;
extern int nVoltasTotal;
extern bool tem90;
extern int nCiclista90; // Número do ciclista que vai pedalar a 90km/h
extern bool esperandoSegundoUltimasVoltas;
extern int dt_base; // base do delta de velocidade (2 padrão, 3 se tiver ciclista a 90km/h
extern pthread_mutex_t mutex;
extern pthread_mutex_t **mutex2;
extern _Atomic long long int tempo;
extern ListaRank L;
extern Rank rankFinal;
extern Rank rankQuebras;
extern bool ultimasVoltas;
extern bool ciclistaQuebrou;

void * juiz(void * arg)
{
    ciclista *c = (ciclista *) arg;

    /*Flags auxiliares*/
    int minVolta = 0; // Mínimo das voltas locais dos ciclistas
    int maxVolta = 0; // Máximo das voltas locais dos ciclistas
    int ultimaVoltaDeEliminacao = 0; // Menor volta local do coordenador
    int maiorVolta = 0; // Maior volta local do coordenador
    bool vencedorTerminouProva = false;
    int vencedor = -1;
    int primeiroUltimasVoltas = -1;

    while (true) {
        for (ciclista * p = c->prox; p != cab; p = p->prox) {
            while (p->arrive == 0) usleep(1);
            p->arrive = 0;
        }

        if (true) { // if temporário (código da thread coordenadora)
            if (ciclistaQuebrou) // elimina todos os ciclistas que quebraram (se quebrou, está na linha de chegada)
                eliminaQuebra(c);
            minVolta = maxVolta;
            for (ciclista * p = c->prox; p != cab; p = p->prox) {
                if (maxVolta < p->voltas) maxVolta = p->voltas;
                if (minVolta > p->voltas) minVolta = p->voltas;
            }
            if (maiorVolta != maxVolta) maiorVolta = maxVolta;
            if (DEBUGVOLTAS) printf("(imprimeVoltasCiclistas) Voltas dos ciclistas ativos, nCiclistasAtivos: %d, nQuebras: %d, n - 2 -2*nQuebras: %d, nVoltasTotal: %d\n", nCiclistasAtivos, nQuebras, 2*n - 2 -2*nQuebras, nVoltasTotal);
            if (DEBUGVOLTAS) imprimeVoltasCiclistas(c);
            if (DEBUG3 && maiorVolta > 0) imprimeVoltasListaRank(L);

            if(!vencedorTerminouProva && maiorVolta >= nVoltasTotal) { // Vencedor terminou a prova (usei >= pois pode haver um ciclista muito rápido que esteja muitas voltas à frente, antes de sabermos nVoltasTotal com absoluta certeza)
                    vencedor = primeiroColocado(L, maiorVolta);
                    eliminaCiclista(c, vencedor);
                    nCiclistasAtivos--;
                    nEliminados++;
                    if (DEBUG) fprintf(stderr, ">>>>>>>>>>>>> O ciclista %d foi o vencedor!\n", vencedor);
                    if (DEBUG) printf(">>>>>>>>>>>>> O ciclista %d foi o vencedor!\n", vencedor);
                    vencedorTerminouProva = true;
            }
            while (minVolta > 0 && ultimaVoltaDeEliminacao < minVolta) {
                // imprimeRank(L, ultimaVoltaDeEliminacao);
                imprimeStderrRank(L, ultimaVoltaDeEliminacao);
                ultimaVoltaDeEliminacao++; // terminou uma volta
                printf("ultimaVoltaDeEliminacao: %d\n", ultimaVoltaDeEliminacao);
                if (ultimaVoltaDeEliminacao%2 == 0) { // Eliminação
                    int ultimo = ultimoColocado(L, ultimaVoltaDeEliminacao);
                    if (DEBUG) printf(">>>> Ultimo colocado (Eliminado): %d\n", ultimo);
                    eliminaCiclista(c, ultimo);
                    nCiclistasAtivos--;
                    nEliminados++;
                    if (nCiclistasAtivos == 0) { // Fim da prova
                        ajustaPrimeiroColocado(rankFinal, vencedor);
                        pthread_exit(0);
                    }
                }
                ciclistaQuebrou = false;
            }
            if (!ultimasVoltas && maiorVolta >= nVoltasTotal - 3) { // Sorteio de 90km/h
                printf("SORTEIO TEM90\n");
                ultimasVoltas = true;
                if (randReal(0, 1) < PROB_90) {
                    tem90 = true;
                    dt_base = 6;
                    primeiroUltimasVoltas = primeiroColocado(L, maiorVolta);
                    if (randReal(0, 1) < 0.5) {
                        nCiclista90 = primeiroUltimasVoltas;
                        printf("\t(sorteio tem 90) Primeiro colocado sorteado (nCiclista90: %d)\n", nCiclista90);
                    }
                    else {
                        esperandoSegundoUltimasVoltas = true;
                        printf("\t(sorteio tem 90) Segundo colocado sorteado (nCiclista90: %d)\n", nCiclista90);
                    }
                }
            }
            if (esperandoSegundoUltimasVoltas) {
                for (ciclista * p = c->prox; p != cab; p = p->prox) {
                    if (p->num != primeiroUltimasVoltas && p->voltas >= nVoltasTotal - 3) {
                        nCiclista90 = p->num;
                        esperandoSegundoUltimasVoltas = false;
                    }
                }
            }
            if (tem90) tempo += 20;
            else tempo += 60;
            usleep(10000);
            if (DEBUG2) printf("(\\/)\nmaiorVolta: %d, minVolta: %d, ultimaVoltaDeEliminacao: %d\n", maiorVolta, minVolta, ultimaVoltaDeEliminacao);
            if (DEBUG2) fprintf(stderr, "(\\/)\nmaiorVolta: %d, minVolta: %d, ultimaVoltaDeEliminacao: %d\n", maiorVolta, minVolta, ultimaVoltaDeEliminacao);
            if (DEBUGVIEW) printf("dt_base: %d\n", dt_base);
            if (DEBUGVIEW) visualizador();
            if (DEBUG2) visualizadorStderr();
            if (ultimaVoltaDeEliminacao > 0) {
                L = RemoveRanksVolta(L, ultimaVoltaDeEliminacao);
            }
        }
        if (DEBUGMUTEX) imprimeMutexLocked(); // debug de mutexes


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
    nanosleep(&ts, NULL); // dorme para esperar a thread parar (o loop de espera da thread dá uma leitura inválida no valgrind por causa do free)
    free(q);
}

void eliminaQuebra(ciclista *c) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;
    ciclista *anterior = c;
    for (ciclista * p = c->prox; p != cab; p = p->prox) {
        if (p->quebrou) {
            InsereCiclistaRank(rankQuebras, p->num, p->voltas);
            ciclista *q = p;
            anterior->prox = p->prox; // Remover da lista de threads
            p = anterior;
            pista[q->py][q->px] = NULL; // limpa pista
            pthread_cancel(q->id); // interrompe a thread
            nanosleep(&ts, NULL); // dorme para esperar a thread parar (o loop de espera da thread dá uma leitura inválida no valgrind por causa do free)
            free(q);
            nQuebras++;
            nVoltasTotal -= 2; // uma quebra diminui em 2 o número total de voltas da corrida
        }
        else
        anterior = p;
    }
}

// para Debug apenas (remover)
void imprimeVoltasCiclistas(ciclista *c) {
    printf("[DEBUG] voltas de cada ciclista\n");
    for (ciclista * p = c->prox; p != cab; p = p->prox) {
        printf("\tciclista %d, volta: %d\n", p->num, p->voltas);
    }
}

void imprimeMutexLocked() {
    fprintf(stderr, "Teste mutexes\n");
    for (int j = 0; j < 10; j++)
        for (int i = 0; i < d; i++)
            if (pthread_mutex_trylock(&mutex2[j][i]) == 0)
                pthread_mutex_unlock(&mutex2[j][i]);
            else
                fprintf(stderr, "\tmutex (%d, %d) locked\n", j, i);
}
