#include "rank.h"
#include "tools.h"

// Cria um Rank vazio e retorna um ponteiro para ele
Rank CriaRank(int volta, int size) {
    Rank R = malloc(sizeof(celRank));
    R->volta = volta;
    R->n = 0;
    R->nCiclista = malloc(size * sizeof(int));
    R->t = malloc(size * sizeof(int));
    R->size = size;
    return R;
}

// Insere ciclista num Rank sem utilização de listas ligadas
// A estrutura foi aproveitada para armazenar o rank final
void InsereCiclistaRank(Rank R, int ciclista, int t) {
    R->nCiclista[R->n] = ciclista;
    R->t[R->n] = t;
    (R->n)++;
}


// Insere um ciclista p no instante de tempo t em uma lista L
void InsereCiclista(ListaRank L, int size, int volta, int ciclista, int t) {
    Rank R = NULL; // Rank da volta correspondente
    if (L->rank == NULL) { // lista vazia
        L->rank = CriaRank(volta, size);
        R = L->rank;
    }
    else { // lista não vazia
        ListaRank Latual = L;
        while (1) { // busca se já existe um rank para essa volta
            if (Latual->rank->volta == volta) {
                R = Latual->rank;
                break;
            }
            if (Latual->prox == NULL) break;
            else Latual = Latual->prox;
        }
        if (R == NULL) { // Não existe, precisa criar
            ListaRank Lnovo = CriaListaRank();
            R = CriaRank(volta, size);
            Latual->prox = Lnovo;
            Lnovo->rank = R;
        }
    }
    R->nCiclista[R->n] = ciclista;
    R->t[R->n] = t;
    (R->n)++;
}


// Cria uma lista ligada de Ranks, com primeiro rank de tamanho size
// e retorna um ponteiro para a lista criada
ListaRank CriaListaRank() {
    ListaRank L = malloc(sizeof(ListaRank));
    L->rank = NULL;
    L->prox = NULL;
    return L;
}

// Destrói um Rank
void DestroiRank(Rank R) {
    free(R->nCiclista);
    free(R->t);
    free(R);
}

// Recebe uma ListaRank L e remove o rank do início da lista
ListaRank RemoveRank(ListaRank lista) {
    Rank rank = lista->rank;
    ListaRank l = lista;
    lista = lista->prox;
    DestroiRank(rank);
    free(l);
    return lista;
}

// Recebe uma ListaRank L e remove os rank menores que volta
ListaRank RemoveRanksVolta(ListaRank L, int volta) {
    if (L == NULL) return NULL;
    if (L != NULL && L->rank != NULL && L->prox != NULL && L->rank->volta < volta) {
        L = RemoveRank(L);
    }
    return L;
}

// Destrói uma lista de ranks
void DestroiListaRank(ListaRank L) {
    while (L->rank != NULL)
        RemoveRank(L);
}

// Imprime a lista de rank de uma volta específica
void imprimeRank(ListaRank L, int volta) {
    Rank R = NULL;
    for (ListaRank L1 = L; L1 != NULL; L1 = L1->prox)
        if (L1->rank->volta == volta) {
            R = L1->rank;
            break;
        }
    if (R == NULL) {
        printf("ERRO! Volta não encontrada na lista de ranks. (volta %d)\n", volta);
        return;
    }
    printf("Ciclic\tPos\tTempo\n");
    for (int i = 0; i < R->n; i++) {
        printf("%d\t%d\t%d\n", R->nCiclista[i], i+1, R->t[i]);
    }
    printf("\n");
}

// Imprime na saíade de erro a lista de rank de uma volta específica
void imprimeStderrRank(ListaRank L, int volta) {
    Rank R = NULL;
    for (ListaRank L1 = L; L1 != NULL; L1 = L1->prox)
    if (L1->rank->volta == volta) {
        R = L1->rank;
        break;
    }
    if (R == NULL) {
        fprintf(stderr, "ERRO! Volta não encontrada na lista de ranks. (volta %d)\n", volta);
        return;
    }
    fprintf(stderr, "Ciclic\tPos\tTempo\n");
    for (int i = 0; i < R->n; i++) {
        fprintf(stderr, "%d\t%d\t%d\n", R->nCiclista[i], i+1, R->t[i]);
    }
    fprintf(stderr, "\n");
}

// Imprime a lista de rank final
void imprimeRankFinal(Rank R) {
    printf("Ciclic\tPos\tTempo\n");
    for (int i = R->n - 1; i >= 0; i--) {
        printf("%d\t%d\t%d\n", R->nCiclista[i], R->n - i, R->t[i]);
    }
    printf("\n");
}

// Imprime a lista de rank final
void imprimeStderrRankFinal(Rank R) {
    fprintf(stderr, "Ciclic\tPos\tTempo\n");
    for (int i = R->n - 1; i >= 0; i--) {
        fprintf(stderr, "%d\t%d\t%d\n", R->nCiclista[i], R->n - i, R->t[i]);
    }
    fprintf(stderr, "\n");
}

// Recebe uma ListaRank L e uma volta e retorna um ponteiro para o Rank
// dessa volta.
Rank BuscaRank(ListaRank L, int volta) {
    Rank R = NULL;
    ListaRank Latual = L;
    while (1) { // busca se já existe um rank para essa volta
        if (Latual->rank->volta == volta) {
            R = Latual->rank;
            break;
        }
        if (Latual->prox == NULL) break;
        else Latual = Latual->prox;
    }
    return R;
}


// Recebe uma lista de ranks por volta e retorna o último colocado ao término
// dessa volta. Se houver mais de um último colocado, sorteia um deles
int ultimoColocado(ListaRank L, int volta) {
    int a, b;
    Rank R = BuscaRank(L, volta);
    if (R == NULL) {
        printf("ERRO! Volta não encontrada na função ultimoColocado. (volta %d)\n", volta);
        exit(1);
    }
    a = b = R->n - 1; // último índice
    while (R->t[a] == R->t[b])
        a--;
    a++;
    if (a == b) // só há um último colocado
        return R->nCiclista[a];
    return R->nCiclista[randInteger(a, b)];
}

// Recebe uma lista de ranks por volta, uma volta e um número de ciclista.
// A função remove o ciclista do rank dessa volta e retorna um novo último colocado
int novoUltimoColocado(ListaRank L, int volta, int numCiclista) {
    Rank R = BuscaRank(L, volta);
    if (R == NULL) {
        printf("ERRO! Volta não encontrada na função novoUltimoColocado. (volta %d)\n", volta);
        exit(1);
    }
    int i;
    while (i < R->n && R->nCiclista[i] != numCiclista)
        i++;
    if (i >= R->n) {
        printf("ERRO! numCiclista na encontrado na função novoUltimoColocado. (volta %d)\n", volta);
        exit(1);
    }
    for ( ; i + 1 < R->n; i++) {
        R->nCiclista[i] = R->nCiclista[i + 1];
        R->t[i] = R->t[i + 1];
    }
    (R->n)--;
    return ultimoColocado(L, volta);
}

// Para debug
void imprimeVoltasListaRank(ListaRank L) {
    fprintf(stderr, "Voltas em listaRank: ");
    for (ListaRank L1 = L; L1 != NULL; L1 = L1->prox) {
        fprintf(stderr, "%d ", L1->rank->volta);
    }
    fprintf(stderr, "\n");
}
