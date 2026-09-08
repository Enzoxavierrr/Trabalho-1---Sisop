# Decisões de Arquitetura

tags: #sisop #trabalho1 #arquitetura

---

## Stack escolhida

### 1. Pthreads (não processos)

**Escolha:** `pthread_create` + `pthread_join` + `pthread_mutex_t`.

**Por quê:**
- Memória compartilhada é natural — a matriz e o DSU vivem no mesmo espaço de endereçamento, sem `shm_open`/`mmap`.
- Custo de criação ≈ 18µs (vs. 73µs do `fork`) — melhor para matrizes menores onde overhead pesa.
- Sincronização com `pthread_mutex_t` é direta; não precisa de semáforos POSIX nomeados.
- Ver [[09 - Custos Reais]].

**Trade-off aceito:** perdemos o isolamento de processos, mas ganhamos simplicidade e desempenho. Documentar essa decisão no README com dados dos custos.

### 2. Decomposição em blocos 2D

**Escolha:** matriz dividida em uma grade `BR × BC` de blocos retangulares configuráveis via CLI.

**Por quê:**
- O enunciado ilustra grids 2×2 e 3×3 e o **Exemplo 3 exige tratar encontro de 4 blocos com conectividade diagonal**.
- Faixas de linhas simplificam demais e podem esconder o problema real.
- Blocos 2D forçam a tratar 4 tipos de fronteira: horizontal, vertical e 2 tipos de diagonal (canto NE↔SW e NW↔SE).
- Trabalhadores atuam em blocos disjuntos → nenhuma escrita concorrente na matriz de labels dentro dos blocos.

**Alternativas descartadas:**
- **Faixas de linhas:** simples mas não estressa o caso "encontro de 4 blocos".
- **Fila dinâmica de blocos (work-stealing):** melhor balanceamento mas complexidade alta para o prazo.

### 3. Algoritmo local: BFS iterativo

**Escolha:** flood fill via BFS com fila circular alocada uma vez por thread.

**Por quê:**
- Requisito 41 do enunciado: **evitar recursão excessiva**.
- Matriz de desempenho pode ter milhares de linhas — DFS recursivo estoura a pilha.
- BFS com `int *queue; size_t head, tail;` é O(N) em memória e O(N) em tempo por componente.
- Vizinhança-8 é 8 checagens de limite + push condicional.

### 4. Consolidação: Union-Find (Disjoint Set Union)

**Escolha:** DSU global com path compression + union by rank.

**Por quê:**
- Complexidade amortizada O(α(N)) — praticamente constante.
- Determinístico: mesma matriz → mesmo número de objetos.
- Fase de merge de fronteiras é **pequena** (só as bordas) → pode ser sequencial sem custo relevante → **elimina necessidade de mutex complexo no merge**.
- Ver [[05 - Consolidação com Union-Find]] para detalhes.

## Estrutura de arquivos

```
Trabalho 1/
├── README.md
├── Makefile
├── src/
│   ├── conta-objetos-sequencial.c   (main sequencial)
│   ├── conta-objetos-paralelo.c     (main paralelo)
│   ├── matriz.h / matriz.c          (I/O de matriz binária)
│   ├── dsu.h    / dsu.c             (Union-Find)
│   ├── flood.h  / flood.c           (BFS flood fill + labeling local)
│   └── util.h   / util.c            (timing, args, log_erro)
├── tests/
│   ├── m1_5x5.txt
│   ├── m2_6x8.txt
│   ├── m3_8x8.txt
│   ├── m4_9x12.txt
│   ├── m5_12x12.txt
│   └── m_grande_2000x2000.txt
├── results/
│   ├── resultados.csv
│   └── analise.md
└── slides/
    └── apresentacao.pdf
```

## Interface CLI

```
# Sequencial
./conta-objetos-sequencial <arquivo_matriz>

# Paralelo
./conta-objetos-paralelo <arquivo_matriz> <n_threads> [<blocos_linha> <blocos_coluna>]
# Se blocos não informados → escolhe automaticamente próximo do sqrt(n_threads)
```

## Formato do arquivo de matriz

Texto simples:
```
<linhas> <colunas>
0 1 0 1 0
1 1 0 0 0
...
```

Escolha por texto (não binário) porque:
- Fácil versionar no git com diff legível.
- Trivial de gerar/editar à mão.
- Overhead de parsing é irrelevante frente ao processamento.

## Regras de qualidade que vamos seguir

1. **Toda** chamada POSIX tem retorno verificado — helpers `check_pthread(rc, "msg")` centralizam.
2. Sem `malloc` sem `free` correspondente — checar no fim da main.
3. `pthread_join` em todas as threads criadas.
4. Compilar sem **nenhum** warning com `-Wall -Wextra -pedantic`.
5. C89 estrito: declarar variáveis no topo do bloco, sem `//`, sem C99 array VLA.
6. Comentários explicam **por quê**, não o quê.

## Cuidados de hardware

- **False sharing** ([[11 - Hardware — SMT, NUMA e False Sharing]]):
  - Contadores por thread devem estar em cache lines separadas — usar padding ou array com stride ≥ 64 bytes.
  - Ex: `struct { long count; char pad[64 - sizeof(long)]; } thread_local[MAX_THREADS];`

## Ver também

- [[03 - Algoritmo Sequencial]]
- [[04 - Algoritmo Paralelo]]
- [[05 - Consolidação com Union-Find]]
