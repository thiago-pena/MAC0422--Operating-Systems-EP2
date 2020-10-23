# so-ep2

Link da apresentação:
https://docs.google.com/presentation/d/1xRl5Mm4w-hyP7T1xzA5vDjIK1Wc0GqihGoLxspg3UN4/edit#slide=id.g53c4c165e2_0_6
------------------------------------------------------------------------------------------------
Alterações do commit "Diversas modificações - ver README"
*Separação do ep2.c nos seguintes arquivos:
    *ep2.c agora só contém a main e a declaração de variáveis globais
    *as demais funções que estavam em ep2.c foram passadas para:
        *threads.c:
            *contém as funções das threads e do coordenador
            *contém também algumas poucas funções (depois eu quero organizar melhor)

*Adicão de novos arquivos:
    *tools.c: contém as funções de cálculo de probabilidades do PF
    *rank.c: contém uma estrutura de dados para armazenar as listas com as posições
             dos ciclistas a cada volta.

*Implementações diversas
    *Regras de ultrapassagem
    *Eliminação do último colocado a cada 2 voltas (tomando como referência a volta
     do ciclistas mais lento -> a confirmar se está correto)
        A eliminação de um ciclista inclui:
            (1) Remoção da marcação do ciclista na pista
            (2) Remoção da thread da lista ligada circular de threads
            (3) Inserção do ciclista no rank final
            (4) Interrupção definitiva da execução da thread com 'pthread_cancel'
    *contagem do tempo (por enquanto é a contagem de iterações)
    *alterei a volta inicial de -1 para 0, para que as eliminações ocorram em
     voltas pares.
    *Criação de uma estrutura de dados para armazenar as posições a cada volta,
     com os tempos em que um ciclista cruzou a linha de chegada e a posição dele.
    *Essa mesma estrutura também serve para guardar as posições finais de cada
     ciclista.

*OBS: como tem dependências no Makefile, é melhor remover o 'ep2' sempre que for
      dar make, pois como não ajustei o Makefile, ele acha que o executável
      está atualizado, mesmo que não esteja.
*OBS2: Não me preocupei muito com free's por enquanto.
