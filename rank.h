// Estrutura de dados para armazenar ranks volta a volta
// As inserções devem ocorrer na ordem de ranqueamento dos ciclistas em uma dada volta
// A estrutura é ordenada pelo número de voltas (crescente)

#ifndef RANK_H
#define RANK_H

#include <stdlib.h>
#include <stdio.h>

typedef struct celRank {
    int volta;      // Número da volta
    int n;          // Número de ciclistas ativos nessa volta
    int *nCiclista; // Vetor de números dos ciclistas ordenados por posição naquela volta
    int size;       // Tamanho dos vetores alocados
    int *t;         // Para o ciclista de índice i em rank[], informa o tempo em que ele cruzou a linha de chegada nessa volta
} celRank;
typedef celRank *Rank;

//struct Lista ligada
typedef struct eloRank {
    Rank rank;
    struct eloRank *prox;
} eloRank;
typedef eloRank *ListaRank;

// Cria um Rank vazio e retorna um ponteiro para ele
Rank CriaRank(int volta, int size);

// Insere ciclista num Rank sem utilização de listas ligadas
// A estrutura foi aproveitada para armazenar o rank final
void InsereCiclistaRank(Rank R, int ciclista, int t);

// Insere um ciclista p no instante de tempo t em uma lista L
void InsereCiclista(ListaRank L, int size, int volta, int ciclista, int t);

// Cria uma lista ligada de Ranks, com primeiro rank de tamanho size
// e retorna um ponteiro para a lista criada
ListaRank CriaListaRank();

// Destrói um Rank
void DestroiRank(Rank R);

// Recebe uma ListaRank L e remove o rank do início da lista
ListaRank RemoveRank(ListaRank L);

// Recebe uma ListaRank L e remove os rank menores que volta
ListaRank RemoveRanksVolta(ListaRank L, int volta);

// Destrói uma lista de ranks
void DestroiListaRank(ListaRank L);

// Imprime a lista de rank de uma volta específica
void imprimeRank(ListaRank L, int volta);

// Imprime na saída de erro a lista de rank de uma volta específica
void imprimeStderrRank(ListaRank L, int volta);

// Imprime a lista de rank final
void imprimeRankFinal(Rank R);

// Imprime a lista de rank de quebras
void imprimeRankQuebras(Rank R);

// Imprime na saída de erro a lista de rank final
void imprimeStderrRankFinal(Rank R);

// Recebe uma ListaRank L e uma volta e retorna um ponteiro para o Rank
// dessa volta.
Rank BuscaRank(ListaRank L, int volta);

// Recebe uma lista de ranks por volta e retorna o último colocado ao término
// dessa volta. Se houver mais de um último colocado, sorteia um deles
int ultimoColocado(ListaRank L, int volta);

// Recebe uma rank e um ciclista e verifica se o ciclista está nesse rank
// Retorna 1 se estiver, 0 caso contrário
int buscaQuebra(Rank R, int nCiclista);

// Recebe uma lista de ranks por volta, uma volta e um número de ciclista.
// A função remove o ciclista do rank dessa volta e retorna um novo último colocado
int novoUltimoColocado(ListaRank L, int volta, int numCiclista);

// Recebe um rank e o número do ciclista vencedor. A função ajusta as posições
// do rank final para que o vencedor seja o primeiro colocado, pois sua thread
// foi a primeira a ser destruída na última volta.
void ajustaPrimeiroColocado(Rank R, int vencedor);

// Recebe uma lista de ranks por volta e retorna o primeiro colocado dessa volta.
int primeiroColocado(ListaRank L, int volta);

// Para debug
void imprimeVoltasListaRank(ListaRank L);

#endif
