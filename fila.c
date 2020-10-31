#include "fila.h"

void
Fila criaFila(int size) {
   queue = malloc(size*sizeof(par));
   ini = fim = 0;
}

bool filaVazia() {
   return ini == fim;
}

void insereFila(int nCiclista, int volta) {
   queue[fim++] = v;
}

vertex removeFila( void) {
   return queue[ini++];
}

void destroiFila() {
   free(queue);
}
