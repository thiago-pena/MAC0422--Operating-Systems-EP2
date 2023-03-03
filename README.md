# MAC0422 - Operating Systems / Sistemas Operacionais (2020)
Undergraduate subject of the Bachelor's Degree in Computer Science at IME-USP.

View [course description](https://uspdigital.usp.br/jupiterweb/obterDisciplina?nomdis=&sgldis=mac0422).

## EP2 - Segundo Semestre de 2020

Name | nUSP
--- | ---
[Pedro Fernandes](https://github.com/Pedro84skynet) | 5948611
[Thiago Benitez Pena](https://github.com/thiago-pena) | 6847829

## 1.CONTEÚDO

    Conteúdo do arquivo "ep1-pedro.thiago.tar.gz"

    Makefile
    apresentação.pdf
    LEIAME
    ep2.c
    rank.c
    rank.h
    thread_ciclista.c
    thread_ciclista.h
    thread_coordenador.c
    thread_coordenador.h
    tools.c
    tools.h

## 2.INSTRUÇÕES

  Observações inicias:

          A máquina cujo programa será executado dever ser compatível com
          as principais funções GNU/Linux em especial as referidas pela
          definição _GNU_SOURCE e as bibliotecas:

		stdio.h
		stdlib.h
 		pthread.h
 		stdbool.h
		unistd.h
		sys/resource.h>
		sys/time.h
		errno.h
		unistd.h
		stdbool.h


  ### 1.1   Para gerar os códigos binários abrir pasta desempacotada no shell e
        digitar "make"

          Exemplo: ~/dir1/dir2 >$ make

  ### 1.2   Para rodar o binário digitar "./ep1 d n" sendo d a distancia da pista
        desejada e n o número de ciclista entre 5 a 5 x d.

          Exemplo: ~/dir1/dir2 >$ ./ep1 250 500

  #### 1.4.1 Para rodar o programa no formato debug, inserir a flag -d no final
        digitando "./ep1 d n -d"

        Exemplo: ~/dir1/dir2 >$ ./ep1 250 500 -d

  #### 1.4.2 Para gerar um .txt adicional com informação de tempo real, tempo de
        usuário, tempo de sistema e uso de memória, digitar "./ep1 d n
        -benchmark "numero_qualquer"

          Exemplo: ~/dir1/dir2 >$ ./ep1 250 500 -benchmark 1

        a saída do benchmark.txt tera fila de 5 números correspondendo a numero
        de índice qualquer, tempo real (relógio), tempo de usuário, tempo de
        sistema e uso de memória respectivamente.

## 3.REFERÊNCIAS

   Várias referencias de uso das biblioteca pthread.h, time.h
   e sys/.... .h inspiradas mas não copiadas das notas de
   aula e sites:

              - https://pt.stackoverflow.com/
              - https://man7.org/
              - https://linux.die.net/

    Utilizamos os códigos para geração de número aleatórios usados na disciplina MAC0328 - Algoritmos em Grafos, também existentes na seguinte página do professor Paulo Feofiloff:
              - https://www.ime.usp.br/~pf/algoritmos/aulas/random.html

