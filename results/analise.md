# Análise de desempenho

## Ambiente e método

- Matrizes: 1200×1200.
- Sistema: Windows, Intel Core i7-1255U (10 núcleos, 12 processadores lógicos).
- Compilador: GCC, flags `-std=c89 -Wall -Wextra -pedantic -O2`, link com `-pthread`.
- Paralelismo: BR×BC = 16×16, total de 256 blocos, com 4 e 8 threads.
  A configuração de 2 threads não foi medida.
- Repetições: 10 execuções por entrada e configuração; valores apresentados são medianas.
- Entradas:
  - **Fragmentada (51.284 objetos):** matriz pseudoaleatória, semente 123. Em blocos de 80×80 alternados em padrão de xadrez, cada célula tinha probabilidade 55% ou 15% de ser 1.
  - **Um componente grande (1 objeto):** matriz totalmente preenchida com 1 (densidade 100%).

Os arquivos de entrada foram gerados no diretório temporário para os testes e
não fazem parte do repositório.

## Resultados

Todos os tempos estão em milissegundos. “Paralelo (Fases 2–4)” inclui as
Fases 2, 3 e 4. “Paralelo combinado” é a mediana, calculada por execução,
de preparo + Fases 2–4; evita somar medianas de fases obtidas
separadamente.

| Entrada | Configuração | Objetos | Sequencial (contagem) | Preparo paralelo | Fase 2 | Fase 3 | Fase 4 | Paralelo (Fases 2–4) | Paralelo combinado |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Fragmentada | Sequencial | 51.284 | 64,267 | — | — | — | — | — | — |
| Fragmentada | Paralelo, 4 threads | 51.284 | — | 7,991 | 27,150 | 65,522 | 14,790 | 107,378 | 113,665 |
| Fragmentada | Paralelo, 8 threads | 51.284 | — | 7,160 | 17,127 | 53,743 | 13,530 | 85,350 | 91,780 |
| Um componente grande | Sequencial | 1 | 39,541 | — | — | — | — | — | — |
| Um componente grande | Paralelo, 4 threads | 1 | — | 6,102 | 22,137 | 87,556 | 4,410 | 115,135 | 121,367 |
| Um componente grande | Paralelo, 8 threads | 1 | — | 6,287 | 15,676 | 82,896 | 3,872 | 103,794 | 110,327 |

## O que cada tempo mede

- **Sequencial (contagem):** cronometra `flood_contar_seq`, incluindo a
  alocação e liberação de suas estruturas de trabalho. Não inclui leitura do
  arquivo nem liberação da matriz de entrada.
- **Preparo paralelo:** alocação de `labels`, divisão e configuração dos
  blocos, criação/inicialização da DSU, inicialização do mutex e alocação do
  vetor de threads. É mostrado à parte e somado ao tempo paralelo combinado.
- **Fase 2:** criação e junção das threads, retirada dos blocos da fila e
  rotulagem local via BFS.
- **Fase 3:** consolidação sequencial das fronteiras usando Union-Find.
- **Fase 4:** contagem dos representantes únicos.
- **Paralelo (Fases 2–4):** soma cronometrada de Fases 2, 3 e 4; não inclui
  preparo nem leitura.
- **Paralelo combinado:** preparo + Fases 2–4. Continua sem incluir leitura,
  destruição da DSU, liberação de estruturas e demais operações de limpeza.
- A leitura foi cronometrada separadamente pelo programa, mas não está na
  tabela para não misturar I/O e parsing com a comparação da contagem.

## Análise

O tempo combinado paralelo excedeu o tempo sequencial em todas as
configurações. Na matriz fragmentada, as medianas combinadas foram 1,77×
(4 threads) e 1,43× (8 threads) o tempo sequencial. Na matriz de um
componente, foram 3,07× e 2,79×, respectivamente.

A Fase 2 ficou mais rápida com 8 threads que com 4, nas duas entradas.
Entretanto, a Fase 3 foi o maior custo paralelo: 53,743–65,522 ms na matriz
fragmentada e 82,896–87,556 ms na matriz de um componente. Como essa fase é
sequencial e percorre os vizinhos rotulados para consolidar componentes, o
paralelismo da BFS não compensou seu custo neste algoritmo e nessas entradas.

Os resultados não confirmam a expectativa de que um componente grande, por
si só, faria esta implementação paralela superar a sequencial. Eles mostram
que o custo da consolidação e o trabalho redundante nas fronteiras precisam
ser considerados antes de concluir sobre o ganho de paralelismo. Os números
caracterizam apenas estas matrizes, parâmetros e ambiente; não são uma
estimativa universal de desempenho.

As contagens foram iguais entre as versões sequencial e paralela em ambas as
matrizes. Os 8 arquivos de teste pequenos do repositório também produziram
contagens iguais nas duas versões.
