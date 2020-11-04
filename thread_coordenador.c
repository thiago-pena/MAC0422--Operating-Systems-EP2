#include "thread_coordenador.h"
#include "rank.h"

#define DEBUGVIEW 0
#define PROB_90 0.1 //probabilidade de um ciclista ter 90km/h nas últimas voltas
#define NSLEEP 1000

/* Variáveis globais */
extern ciclista ***pista;
extern ciclista *cab;
extern int d, n;
extern int dt_base;
extern _Atomic int nCiclistasAtivos, nQuebras;
extern int nVoltasTotal;
extern bool ciclistaQuebrou;
extern bool ultimasVoltas;
extern bool tem90;
extern int nCiclista90;
extern bool esperandoSegundoUltimasVoltas;
extern _Atomic long long int tempo;
extern pthread_mutex_t **mutex;
extern ListaRank L;
extern Rank rankFinal;
extern Rank rankQuebras;
extern long memTotal;
extern int debugParameter;

void * juiz(void * arg)
{
    /* Lê informações de memória e tempo */
    struct rusage usage;
    getrusage(RUSAGE_THREAD, &usage);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;

    ciclista *c = (ciclista *) arg;

    /* Flags auxiliares */
    int minVolta = 0; // Mínimo das voltas locais dos ciclistas
    int maxVolta = 0; // Máximo das voltas locais dos ciclistas
    int ultimaVoltaDeEliminacao = 0; // Menor volta local do coordenador
    int maiorVolta = 0; // Maior volta local do coordenador
    bool vencedorTerminouProva = false;
    int vencedor = -1;
    int primeiroUltimasVoltas = -1;

    while (true) {
        for (ciclista * p = c->prox; p != cab; p = p->prox) {
            while (p->arrive == 0) nanosleep(&ts, NULL);
            p->arrive = 0;
        }

        if (true) { // código do Coordenador
            if (ciclistaQuebrou) {
                eliminaQuebra(c);
                ciclistaQuebrou = false;
            }
            minVolta = maxVolta;
            for (ciclista * p = c->prox; p != cab; p = p->prox) {
                if (maxVolta < p->voltas) maxVolta = p->voltas;
                if (minVolta > p->voltas) minVolta = p->voltas;
            }
            if (maiorVolta != maxVolta) maiorVolta = maxVolta;
            if(!vencedorTerminouProva && maiorVolta >= nVoltasTotal) {
                    vencedor = primeiroColocado(L, maiorVolta);
                    eliminaCiclista(c, vencedor);
                    nCiclistasAtivos--;
                    printf("O ciclista %d venceu a corrida!!! \\o/\n\n", vencedor);
                    vencedorTerminouProva = true;
            }
            while (minVolta > 0 && ultimaVoltaDeEliminacao < minVolta) {
                if (ultimaVoltaDeEliminacao > 0 && nCiclistasAtivos > 1) {
                    printf("\n------------- Volta %d completada. -------------\n", ultimaVoltaDeEliminacao);
                    printf("Posicoes dos ciclistas na volta %d:\n", ultimaVoltaDeEliminacao);
                    imprimeRank(L, ultimaVoltaDeEliminacao);
                }
                ultimaVoltaDeEliminacao++;
                if (ultimaVoltaDeEliminacao%2 == 0) {
                    int ultimo = ultimoColocado(L, ultimaVoltaDeEliminacao);
                    eliminaCiclista(c, ultimo);
                    nCiclistasAtivos--;
                    if (nCiclistasAtivos == 0) { // Fim da prova
                        ajustaPrimeiroColocado(rankFinal, vencedor);
                        printf("\n------------- Volta %d completada. -------------\n", ultimaVoltaDeEliminacao);
                        printf("Posicoes dos ciclistas na volta %d:\n", ultimaVoltaDeEliminacao);
                        imprimeRank(L, ultimaVoltaDeEliminacao);
                        getrusage(RUSAGE_THREAD, &usage);
                        memTotal += usage.ru_maxrss;
                        pthread_exit(0);
                    }
                }
            }
            if (!ultimasVoltas && maiorVolta >= nVoltasTotal - 3) { // Sorteio de 90km/h
                ultimasVoltas = true;
                if (randReal(0, 1) < PROB_90) {
                    tem90 = true;
                    dt_base = 6;
                    primeiroUltimasVoltas = primeiroColocado(L, maiorVolta);
                    if (randReal(0, 1) < 0.5)
                        nCiclista90 = primeiroUltimasVoltas;
                    else
                        esperandoSegundoUltimasVoltas = true;
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
            if (debugParameter) visualizador();
            if (ultimaVoltaDeEliminacao > 0)
                L = RemoveRanksVolta(L, ultimaVoltaDeEliminacao);
        }

        for (ciclista * p = cab->prox; p != cab; p = p->prox) {
            p->Continue = 1;
        }
    }
}

// Imprime a visualização da pista na saída padrão
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

// Imprime a visualização da pista na saída de erros
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

// Recebe um inteiro nCiclista, temove-o da pista, remove o ciclista com essa
// numeração da estrutura de dados, insere-o no rank final, interrompe sua
// thread e libera a memória alocada por ele
void eliminaCiclista(ciclista *c, int nCiclista) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;
    ciclista *q, *anterior;
    anterior = c;
    for (q = c->prox; q != cab; q = q->prox) {
        if (q->num == nCiclista) break;
        anterior = q;
    }
    pista[q->py][q->px] = NULL;
    InsereCiclistaRank(rankFinal, q->num, tempo);
    pthread_cancel(q->id);
    anterior->prox = q->prox;
    nanosleep(&ts, NULL); // dorme para esperar a thread parar
    free(q);
}

// Verifica se há ciclistas com a flag p->quebrou e os elimina: remove-os da
// pista, remove-os da estrutura de dados, insere-os no rank de quebras,
// interrompe suas threads e libera a memória alocada por eles
void eliminaQuebra(ciclista *c) {
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = NSLEEP;
    ciclista *anterior = c;
    for (ciclista * p = c->prox; p != cab; p = p->prox) {
        if (p->quebrou) {
            InsereCiclistaRank(rankQuebras, p->num, p->voltas);
            ciclista *q = p;
            anterior->prox = p->prox;
            p = anterior;
            pista[q->py][q->px] = NULL; // limpa pista
            pthread_cancel(q->id); // interrompe a thread
            nanosleep(&ts, NULL); // dorme para esperar a thread parar
            free(q);
            nQuebras++;
            nVoltasTotal -= 2; // uma quebra diminui em 2 o número total de voltas da corrida
        }
        else
            anterior = p;
    }
}
