# Contagem Paralela de Objetos em Matriz Binária

**Disciplina:** Sistemas Operacionais — 2026/II
**Professor:** Filipo Mór
**Instituição:** PUCRS — Escola Politécnica

Autoria e matrículas: ver o [relatório técnico](RelatorioTecnico).

---

## Sumário

1. [O problema](#1-o-problema)
2. [O que é conectividade-8](#2-o-que-é-conectividade-8)
3. [Como o programa identifica objetos](#3-como-o-programa-identifica-objetos)
4. [Estrutura do projeto](#4-estrutura-do-projeto)
5. [Como compilar](#5-como-compilar)
6. [Como executar](#6-como-executar)
7. [Formato do arquivo de matriz](#7-formato-do-arquivo-de-matriz)
8. [Como funciona a versão sequencial](#8-como-funciona-a-versão-sequencial)
9. [Como funciona a versão paralela](#9-como-funciona-a-versão-paralela)
10. [Testes obrigatórios e resultados](#10-testes-obrigatórios-e-resultados)
11. [Análise de desempenho](#11-análise-de-desempenho)
12. [Decisões técnicas](#12-decisões-técnicas)
13. [Referências](#13-referências)

---

## 1. O problema

Uma **imagem binária** é uma matriz onde cada célula vale `0` (fundo) ou `1`
(pixel de primeiro plano). Um **objeto** é uma região de células vizinhas
que valem `1`. Nosso programa recebe essa matriz e retorna **quantos
objetos distintos existem**.

Exemplo intuitivo (matriz 5×5):

```
1 1 0 0 0
1 1 0 0 0     → 3 objetos:
0 0 0 1 0        (A) o bloco 2×2 no canto superior esquerdo
0 0 0 1 0        (B) o segmento vertical no meio
1 0 0 0 0        (C) o pixel isolado no canto inferior esquerdo
```

O trabalho pede **duas implementações** funcionalmente equivalentes:

- **Sequencial:** um fluxo de execução único percorre a matriz inteira.
- **Paralela:** múltiplas threads dividem o trabalho e depois consolidam
  o resultado.

Ambas devem produzir **exatamente o mesmo número** de objetos para toda
matriz de entrada.

## 2. O que é conectividade-8

Duas células vizinhas podem estar conectadas de dois jeitos diferentes:

**Conectividade-4** (só ortogonal): 4 vizinhos por célula.

```
    N
    |
W - C - E
    |
    S
```

**Conectividade-8** (Moore neighborhood): 8 vizinhos por célula, incluindo
as diagonais. **É a que nosso trabalho usa.**

```
NW  N  NE
  \ | /
W - C - E
  / | \
SW  S  SE
```

Consequência prática: dois pixels conectados **apenas em diagonal** ainda
pertencem ao mesmo objeto.

```
1 0     ← esses dois pixels formam UM objeto só
0 1        (não dois), porque estão conectados na diagonal
```

## 3. Como o programa identifica objetos

O algoritmo se chama **flood fill via BFS (Busca em Largura)**, e é o mesmo
princípio do balde de tinta do Paint. Passo a passo, na versão sequencial:

1. **Varre a matriz** célula por célula, da esquerda para a direita,
   de cima para baixo.
2. Quando encontra uma célula com valor `1` que ainda **não foi visitada**:
   - Contador de objetos `+= 1`.
   - Inicia uma **BFS** partindo dessa célula.
3. A BFS "espalha" pelo objeto: pega a célula da fila, marca como visitada,
   e enfileira todos os 8 vizinhos que também valem `1` e ainda não foram
   visitados.
4. Quando a fila esvazia, o objeto inteiro está marcado. A varredura
   continua procurando o próximo `1` não-visitado.

**Exemplo visual (matriz 4×4, conectividade-8):**

```
Matriz:            Passo a passo:

1 1 0 0            (0,0) = 1 e não visitado → objeto 1 encontrado!
1 0 0 1            BFS visita: (0,0)→(0,1)→(1,0) → fila esvazia.
0 0 0 1            Marca todas.
0 1 1 0
                   (1,3) = 1 e não visitado → objeto 2 encontrado!
                   BFS visita: (1,3)→(2,3) → fila esvazia.
                   
                   (3,1) = 1 → objeto 3! BFS: (3,1)→(3,2). Fila esvazia.
                   
                   Total: 3 objetos.
```

Na versão paralela, esse mesmo princípio é aplicado por **múltiplas threads
em paralelo, cada uma cuidando de uma região da matriz**. Depois, uma etapa
de consolidação junta objetos que atravessaram fronteiras entre regiões.
Detalhes na [seção 9](#9-como-funciona-a-versão-paralela).

## 4. Estrutura do projeto

```
Trabalho-1---Sisop/
├── README.md              ← este arquivo
├── Makefile               ← receita de compilação
├── .gitignore             ← ignora binários, .o, .DS_Store
├── src/                   ← todo o código-fonte C
│   ├── programas/         ← programas principais (têm main)
│   │   ├── conta-objetos-sequencial.c
│   │   └── conta-objetos-paralelo.c
│   ├── util/              ← helpers: erros POSIX + medição de tempo
│   │   ├── util.h
│   │   └── util.c
│   ├── matriz/            ← leitura e liberação da matriz binária
│   │   ├── matriz.h
│   │   └── matriz.c
│   ├── flood/             ← BFS com conectividade-8
│   │   ├── flood.h
│   │   └── flood.c
│   └── dsu/               ← Union-Find para consolidação paralela
│       ├── dsu.h
│       └── dsu.c
├── tests/                 ← matrizes de teste (formato .c)
│   ├── m2_6x8.c         (4 objetos)
│   ├── m3_8x8.c         (5 objetos)
│   ├── m4_9x12.c        (6 objetos)
│   ├── m5_12x12.c       (7 objetos)
│   ├── tabela1_10x10.c  (5 objetos)
│   ├── tabela2_10x10.c  (4 objetos)
│   └── tabela3_10x10.c  (6 objetos)
├── results/               ← dados de benchmark e análise
├── slides/                ← apresentação em PDF (10 min)
└── Testes/                ← matrizes-fonte no formato do editor do professor
```

Cada módulo (`util/`, `matriz/`, `flood/`, `dsu/`) tem seu próprio par
`.h` + `.c`:

- **`.h`** = *cardápio* — declara quais funções o módulo oferece.
- **`.c`** = *cozinha* — implementa as funções.

Isso permite reuso: os dois programas principais (`conta-objetos-sequencial`
e `conta-objetos-paralelo`) importam os mesmos módulos sem duplicar código.

## 5. Como compilar

Requisitos: `cc` (gcc ou clang) e biblioteca `pthread`, em Linux ou macOS.

```bash
make                # compila as duas versões
make sequencial     # compila apenas a versão sequencial
make paralelo       # compila apenas a versão paralela
make clean          # remove binários e artefatos
make test           # roda ambas em todas as matrizes obrigatórias
make help           # lista todos os targets
```

Flags de compilação (todas exigidas pelo enunciado):

```
-std=c89 -Wall -Wextra -pedantic -O2 -pthread
```

Compila **sem warnings** com essas flags.

## 6. Como executar

### Versão sequencial

```bash
./conta-objetos-sequencial <arquivo_matriz>
```

Exemplo:

```bash
./conta-objetos-sequencial tests/m3_8x8.c
```

Saída:

```
arquivo         : tests/m3_8x8.c
dimensoes       : 8 x 8
objetos         : 5
tempo leitura   : 0.043 ms
tempo contagem  : 0.005 ms
```

### Versão paralela

```bash
./conta-objetos-paralelo <arquivo_matriz> <n_threads> [<BR> <BC>]
```

- `n_threads` — quantidade de threads trabalhadoras (obrigatório).
- `BR` `BC` — grade de blocos: `BR` faixas de linhas × `BC` faixas de
  colunas (opcional; padrão 2×2).

Exemplos:

```bash
# 4 threads, grade padrão 2x2 (4 blocos):
./conta-objetos-paralelo tests/m3_8x8.c 4

# 8 threads, grade 4x4 (16 blocos):
./conta-objetos-paralelo tests/m3_8x8.c 8 4 4

# 2 threads processando 6 blocos (2 threads pegam blocos da fila):
./conta-objetos-paralelo tests/m3_8x8.c 2 3 2
```

## 7. Formato do arquivo de matriz

O programa **auto-detecta o formato pela extensão**:

- **`.c` ou `.h`** — formato do [editor do professor](https://filipomor.com/editor-tabelas-c).
  **É o formato principal** — é como o professor entrega as matrizes de teste.
- **outra extensão** — formato texto simples (compatibilidade).

### Formato `.c` do professor (principal)

```c
#define LINHAS 5
#define COLUNAS 5
int Tabela[LINHAS][COLUNAS] = {1, 1, 0, 0, 0,
                               1, 1, 0, 0, 0,
                               0, 0, 0, 1, 0,
                               0, 0, 0, 1, 0,
                               1, 0, 0, 0, 0,}
```

O parser é **liberal**: aceita qualquer espaçamento, comentários entre
os defines, nome de variável diferente de "Tabela", vírgula sobrando no
final. Basta ter os `#define LINHAS` e `#define COLUNAS`, seguidos de
um `{ ... }` com os valores 0/1.

### Formato texto simples (fallback)

```
5 5
1 1 0 0 0
1 1 0 0 0
0 0 0 1 0
0 0 0 1 0
1 0 0 0 0
```

Primeira linha: dimensões. Demais: valores separados por espaço.

## 8. Como funciona a versão sequencial

**Arquivo:** `src/flood/flood.c` (função `flood_contar_seq`) +
`src/programas/conta-objetos-sequencial.c`.

### Estruturas de dados

- `int **matriz` — matriz binária de entrada (não modificada).
- `char **visitado` — matriz do mesmo tamanho marcando quem já foi visto
  pela BFS. Usamos `char` (1 byte) em vez de `int` (4 bytes) para
  economizar memória.
- `int *fila` — fila circular do BFS, alocada uma única vez com capacidade
  máxima `L × C`. Cada célula `(r, c)` é codificada como `r * C + c`
  para caber num único `int`.

### Fluxo

```
1. Aloca 'visitado' e 'fila' (uma vez só).
2. Para cada célula (i, j) da matriz:
     Se matriz[i][j] == 1 e não foi visitada:
       objetos += 1
       BFS partindo de (i, j):
         Enfileira (i, j) e marca como visitada.
         Enquanto a fila não esvazia:
           Desenfileira (r, c).
           Para cada um dos 8 vizinhos (nr, nc):
             Se está dentro da matriz, é 1, e não foi visitado:
               Marca e enfileira.
3. Libera memória.
4. Retorna objetos.
```

### Complexidade

- **Tempo:** O(L × C) — cada célula é processada uma vez.
- **Espaço:** O(L × C) para `visitado` + O(L × C) no pior caso para
  a fila (matriz inteira sendo um único objeto).

### Por que BFS e não DFS?

DFS recursiva seria mais curta em linhas, mas o enunciado (requisito 41)
diz **"evitar recursão excessiva"**. Uma matriz grande com um objeto que
ocupa milhares de células estouraria a pilha da thread. BFS iterativa
usa uma fila alocada no heap, sem esse problema.

## 9. Como funciona a versão paralela

**Arquivo:** `src/programas/conta-objetos-paralelo.c`.

A versão paralela executa em **4 fases**. As fases 2 e 3 são as centrais.

### Visão geral

```
┌──────────────────────────────────────────────────────────────────┐
│  Fase 1 — Divisão (main thread)                                  │
│  Divide a matriz L×C em BR×BC blocos retangulares.               │
│  Reserva um intervalo de labels globais para cada bloco.         │
└──────────────────────────────────────────────────────────────────┘
                                ↓
┌──────────────────────────────────────────────────────────────────┐
│  Fase 2 — Labeling local (N threads em paralelo)                 │
│  Cada thread pega blocos da fila e roda BFS restrita ao bloco.   │
│  Cada componente local recebe um label único do range reservado. │
└──────────────────────────────────────────────────────────────────┘
                                ↓
┌──────────────────────────────────────────────────────────────────┐
│  Fase 3 — Consolidação (main thread, sequencial)                 │
│  Varre toda a matriz de labels. Para cada vizinho-8 com label    │
│  diferente, chama dsu_unir. Isso mescla objetos que atravessam   │
│  fronteiras entre blocos.                                        │
└──────────────────────────────────────────────────────────────────┘
                                ↓
┌──────────────────────────────────────────────────────────────────┐
│  Fase 4 — Contagem final (main thread)                           │
│  Conta quantas raízes distintas existem na DSU.                  │
└──────────────────────────────────────────────────────────────────┘
```

### Por que blocos 2D e não faixas de linhas?

O enunciado usa a palavra "encontro de 4 blocos com conectividade
diagonal" no Exemplo 3. Se dividíssemos só por faixas de linhas, esse
cenário nunca seria testado — o algoritmo poderia estar errado sem que
percebêssemos. Blocos 2D forçam a tratar fronteiras horizontais,
verticais **e diagonais**.

### Por que a Fase 3 é sequencial?

- É **rápida** — O(L × C), enquanto a Fase 2 é O(L × C / N).
- Paralelizar exigiria proteger a DSU com mutex, o que geraria contenção.
- Pela Lei de Amdahl, deixar uma fração pequena serial (< 10%) não
  compromete significativamente a aceleração.

### Sincronização

| Ponto | Mecanismo | Justificativa |
|---|---|---|
| Pegar próximo bloco da fila | `pthread_mutex_t` | Curto: só incrementa um índice |
| Escrever em `labels[r][c]` na Fase 2 | **Nenhum** | Blocos são disjuntos — impossível haver conflito |
| Alocar labels | **Nenhum** | Range reservado antes de iniciar as threads |
| DSU na Fase 3 | **Nenhum** | Roda em thread única |

**Sem race conditions.** Sem deadlocks (só um mutex, sem locks aninhados).
Sem atualizações perdidas.

### Union-Find (DSU)

**Arquivo:** `src/dsu/dsu.c`.

Estrutura clássica que responde duas perguntas em O(α(N)) amortizado
(praticamente constante):

- `find(x)` — qual é o "representante" (raiz) do conjunto que contém `x`?
- `unir(a, b)` — junta os conjuntos que contêm `a` e `b`.

Usamos duas otimizações fundamentais:

- **Path compression** em `find`: cada nó do caminho aponta direto para
  a raiz, achatando a árvore.
- **Union by rank** em `unir`: a árvore mais baixa vira filha da mais alta.

## 10. Testes obrigatórios e resultados

Matrizes do enunciado (m2–m5) + 3 tabelas 10×10 do editor do professor
**passam** em ambas as versões:

| Arquivo | Dimensões | Esperado | Sequencial | Paralelo (2t) | Paralelo (4t, 2×2) | Paralelo (4t, 3×3) |
|---|---|---|---|---|---|---|
| `m2_6x8.c`         | 6×8    | 4 | ✅ 4 | ✅ 4 | ✅ 4 | ✅ 4 |
| `m3_8x8.c`         | 8×8    | 5 | ✅ 5 | ✅ 5 | ✅ 5 | ✅ 5 |
| `m4_9x12.c`        | 9×12   | 6 | ✅ 6 | ✅ 6 | ✅ 6 | ✅ 6 |
| `m5_12x12.c`       | 12×12  | 7 | ✅ 7 | ✅ 7 | ✅ 7 | ✅ 7 |
| `tabela1_10x10.c`  | 10×10  | 5 | ✅ 5 | ✅ 5 | ✅ 5 | ✅ 5 |
| `tabela2_10x10.c`  | 10×10  | 4 | ✅ 4 | ✅ 4 | ✅ 4 | ✅ 4 |
| `tabela3_10x10.c`  | 10×10  | 6 | ✅ 6 | ✅ 6 | ✅ 6 | ✅ 6 |

Para reproduzir:

```bash
make test
```

### Verificação cruzada ampliada

Além do `make test`, a contagem da versão paralela foi comparada com a
sequencial em **21 matrizes × 10 configurações** de threads/grade (210
execuções paralelas), incluindo 10 casos de borda: matriz
1×1, linha e coluna únicas, toda 0, toda 1, moldura tocando as 4 bordas,
xadrez diagonal (1 objeto só, por conectividade-8), pontos isolados e
listras. Todas as 210 execuções produziram a mesma contagem.

Análise dinâmica, nas duas versões, sem nenhum achado:

| Ferramenta | Resultado |
|---|---|
| `valgrind --leak-check=full` | 0 erros, "no leaks are possible" |
| `valgrind --tool=helgrind` | 0 erros (sem condições de corrida) |
| `valgrind --tool=drd` | 0 erros (sem condições de corrida) |

## 11. Análise de desempenho

Os benchmarks foram executados em matrizes de 1200×1200, com 10 repetições
por configuração. Os resultados completos, incluindo o perfil das entradas,
escopo dos cronômetros e medianas por fase, estão em
[results/analise.md](results/analise.md).

Resumo: a contagem sequencial teve mediana de 64,267 ms na matriz
fragmentada e 39,541 ms na matriz de um único componente. Com BR×BC = 16×16
(256 blocos), o tempo paralelo combinado de preparo + Fases 2–4 foi maior
que o sequencial nas duas entradas, tanto com 4 quanto com 8 threads.
A consolidação sequencial (Fase 3) foi o maior custo paralelo medido.
Portanto, nestes testes, aumentar o número de threads reduziu o tempo da
Fase 2, mas não foi suficiente para superar o custo de consolidação.

### Segunda medição (Linux, i7-13620H, 16 threads)

Repetição independente em outra máquina, com 5 matrizes de 2000×2000 e
4000×4000 e configurações de 1 a 16 threads. Confirma a conclusão acima e
quantifica a causa:

- A **Fase 2 (paralela) escala bem**: 5,4×–7,2× de 1 para 16 threads.
- As **fases seriais** (preparo + Fases 3 e 4) representam **56%–72%** do
  trabalho e, sozinhas, custam **mais que a versão sequencial inteira**.
- Pela Lei de Amdahl, isso limita a aceleração a ~1,4×–1,8× sobre o tempo de
  1 thread, que já é 2,2×–3,3× o da sequencial. Resultado: **nenhuma
  configuração superou a sequencial**; o melhor caso foi `S = 0,74`
  (4000×4000, 16 threads).
- A Fase 3 chega a **piorar 23%** com mais threads: mais blocos significam
  mais fronteiras e mais uniões a executar numa fase que é serial.
- Causa raiz: a Fase 3 varre as `L × C` células quando bastariam as células
  de fronteira entre blocos. Restringi-la às fronteiras derrubaria a fração
  serial de ~0,6 para ~0,1, elevando o teto de Amdahl de ~1,6× para ~10×.

Detalhamento, tabelas por fase, descrição das matrizes usadas e caminho de
otimização em [results/analise.md](results/analise.md); dados brutos (uma
linha por execução) em [results/resultados.csv](results/resultados.csv). O CSV
resumido por amostra e configuração, junto com os gráficos reproduzíveis, está
na seção "Amostras e gráficos" da análise.

## 12. Decisões técnicas

| Camada | Escolha | Justificativa |
|---|---|---|
| Paralelismo | **Pthreads** | Memória compartilhada natural, ~4× mais barato que `fork` |
| Decomposição | **Blocos 2D** | Estressa fronteiras H, V e diagonais |
| Algoritmo local | **BFS iterativo** | Evita estouro de pilha (req. 41) |
| Consolidação | **Union-Find sequencial** | O(α(N)) amortizado, sem contenção |
| Alocador de labels | **Ranges reservados** | Zero contenção durante Fase 2 |
| Estrutura de código | **Modular por responsabilidade** | Reuso entre seq e paralelo |
| Padrão de linguagem | **ANSI C89 estrito** | Requisito 10 do enunciado |
| Verificação de erros | **Todos os retornos POSIX** | Requisito 14 do enunciado |

## 13. Referências

- Enunciado: `Trabalho_Pratico_Processos_Threads_Contagem_Objetos.pdf`
- Editor de matrizes do professor:
  <https://filipomor.com/editor-tabelas-c>
- Union-Find (Wikipedia):
  <https://en.wikipedia.org/wiki/Disjoint-set_data_structure>
- Connected-component labeling (Wikipedia):
  <https://en.wikipedia.org/wiki/Connected-component_labeling>

---

_Trabalho acadêmico — Pontifícia Universidade Católica do Rio Grande do Sul._
