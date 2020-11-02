
/*  Funções para geração de números aleatórios foram copiadas da biblioteca
    utilizada pelo professor Paulo Feofiloff durante a disciplina
    MAC0328 - Algoritmos em Grafos
*/

#include "tools.h"


// Função privada auxiliar. Devolve um inteiro aleatório entre a e b
// inclusive, ou seja, no intervalo fechado [a,b]. Vamos supor que
// a <= b e que b-a <= RAND_MAX. (O código foi copiado da biblioteca
    // random de Eric Roberts.)
int randInteger( int a, int b) {
    double d = (double) rand() / ((double) RAND_MAX + 1);
    // 0 <= d < 1
    int k = d * (b - a + 1); // 0 <= k <= b-a
    return a + k;
}

// Função privada auxiliar. Devolve um número real aleatório no
// intervalo semi-aberto [a,b). O código foi copiado da biblioteca
// random de Eric Roberts.
double randReal( double a, double b) {
    double d = (double) rand() / ((double) RAND_MAX + 1);
    // 0 <= d < 1
    return a + d * (b - a);
}

void trocaInt(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}
