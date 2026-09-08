# Consolidação com Union-Find

tags: #sisop #trabalho1 #union-find #dsu #consolidacao

> **Vale 1,5 pt** — critério isolado no enunciado. É o ponto mais delicado.

---

## O problema

Após a Fase 2, temos a matriz `labels[][]` onde cada componente **local** de cada bloco tem um ID único. Mas um objeto físico pode ter partes em **múltiplos blocos** — cada parte recebeu um label diferente.

**Exemplo (fronteira vertical):**
```
Bloco A (col 0-3) | Bloco B (col 4-7)
                  |
    0  1  1  0    | 0  0  0  0
    0  0  1  1    | 1  0  0  0        ← célula (1,3) do A conecta com (1,4) do B
    0  0  0  0    | 0  0  0  0
```
Se A rotulou o objeto como 5 e B rotulou o objeto começando na coluna 4 como 12, precisamos fazer **union(5, 12)** para reconhecer que é o mesmo objeto.

## Estrutura Union-Find (DSU)

```c
typedef struct {
    int *parent;   /* parent[i] = pai do nó i (i se é raiz) */
    int *rank;     /* rank[i] = altura estimada da árvore */
    int capacidade;
} DSU;

DSU *dsu_create(int capacidade);
int  dsu_find(DSU *d, int x);          /* com path compression */
void dsu_union(DSU *d, int a, int b);  /* union by rank */
void dsu_destroy(DSU *d);
```

### Path compression

```c
int dsu_find(DSU *d, int x) {
    if (d->parent[x] != x) {
        d->parent[x] = dsu_find(d, d->parent[x]);  /* achata caminho */
    }
    return d->parent[x];
}
```

**Nota:** essa é a única recursão do projeto e é rasa (nunca ultrapassa log*(N) ≈ 5 na prática) — aceita pelo enunciado.

Alternativa iterativa (mais seguro em C89):
```c
int dsu_find(DSU *d, int x) {
    int root, y, next;
    root = x;
    while (d->parent[root] != root) root = d->parent[root];
    y = x;
    while (d->parent[y] != root) {
        next = d->parent[y];
        d->parent[y] = root;
        y = next;
    }
    return root;
}
```

### Union by rank

```c
void dsu_union(DSU *d, int a, int b) {
    int ra = dsu_find(d, a);
    int rb = dsu_find(d, b);
    if (ra == rb) return;
    if (d->rank[ra] < d->rank[rb]) {
        d->parent[ra] = rb;
    } else if (d->rank[ra] > d->rank[rb]) {
        d->parent[rb] = ra;
    } else {
        d->parent[rb] = ra;
        d->rank[ra]++;
    }
}
```

## Fase 3 — Varredura de fronteiras

### Pares de blocos a verificar

Para cada bloco `(bi, bj)` na grade `BR × BC`, verificar:

| Vizinho | Existe se | Fronteira a varrer |
|---|---|---|
| **Direita** `(bi, bj+1)` | `bj+1 < BC` | Coluna `c1-1` do bloco atual vs. coluna `c1` do vizinho, para todas as linhas |
| **Abaixo** `(bi+1, bj)` | `bi+1 < BR` | Linha `r1-1` do atual vs. linha `r1` do vizinho, para todas as colunas |
| **Diagonal SE** `(bi+1, bj+1)` | ambos existem | 1 célula: `(r1-1, c1-1)` do atual vs. `(r1, c1)` do vizinho |
| **Diagonal SW** `(bi+1, bj-1)` | ambos existem | 1 célula: `(r1-1, c0)` do atual vs. `(r1, c0-1)` do vizinho |

**Nota:** as diagonais NE↔SW e NW↔SE são cobertas naturalmente por essas 4 direções — cada par de blocos diagonais é analisado uma única vez pelo canto SE ou SW do bloco "de cima".

### Pseudocódigo da consolidação

```
para bi em [0, BR):
    para bj em [0, BC):
        se bj+1 < BC:  /* fronteira vertical à direita */
            verificar_fronteira_vertical(bi, bj, bi, bj+1)

        se bi+1 < BR:  /* fronteira horizontal abaixo */
            verificar_fronteira_horizontal(bi, bj, bi+1, bj)

        se bi+1 < BR e bj+1 < BC:  /* canto SE (diagonal) */
            unir_se_conectados(r1-1, c1-1, r1, c1)

        se bi+1 < BR e bj-1 >= 0:  /* canto SW (diagonal) */
            unir_se_conectados(r1-1, c0, r1, c0-1)

verificar_fronteira_vertical(A, B):
    /* A à esquerda, B à direita — compartilham colunas c1_A e c0_B (c1_A == c0_B) */
    para r em [max(r0_A, r0_B), min(r1_A, r1_B)):
        se labels[r][c1_A - 1] != 0:
            /* vizinhos-8 em B: (r-1, c0_B), (r, c0_B), (r+1, c0_B) */
            para cada vizinho válido:
                se labels[nr][nc] != 0:
                    dsu_union(labels[r][c1_A - 1], labels[nr][nc])
```

### Casos de canto (encontro de 4 blocos)

O **Exemplo 3** do enunciado testa exatamente isso. As 4 uniões diagonais + 4 uniões ortogonais entre os 4 blocos vizinhos garantem que qualquer conectividade-8 é capturada.

## Contagem final

```c
int contar_roots_unicos(DSU *d, int *labels_usados, int n_labels) {
    int contador = 0;
    /* Marca cada raiz uma única vez */
    char *ja_contou = calloc(d->capacidade, sizeof(char));
    int i, root;
    for (i = 0; i < n_labels; i++) {
        root = dsu_find(d, labels_usados[i]);
        if (!ja_contou[root]) {
            ja_contou[root] = 1;
            contador++;
        }
    }
    free(ja_contou);
    return contador;
}
```

## Dimensionamento do DSU

Upper bound de labels = número total de células com valor 1 (cada célula poderia ser um objeto isolado). Alocar `DSU` com essa capacidade + margem.

Alternativa: cada thread reporta quantos labels usou; após join, sabemos o total exato e podemos alocar o DSU antes das uniões. Isso adiciona uma barreira mas economiza memória.

## Por que isso está correto

**Invariante:** dois pixels `p` e `q` da matriz original com valor 1 pertencem ao mesmo objeto **se e somente se** `dsu_find(labels[p]) == dsu_find(labels[q])` após a Fase 3.

- **Interior de um bloco:** garantido pelo BFS local — ambos recebem o mesmo label.
- **Fronteira entre blocos:** garantido pela Fase 3 — se vizinhos-8, `union` os equipara.

Prova por indução sobre o caminho de conexão: qualquer caminho de `p` a `q` no grafo de conectividade-8 é uma sequência de arestas; cada aresta é (a) interna a algum bloco ou (b) atravessa uma fronteira. Ambos os casos garantem união dos labels.

## Ver também

- [[04 - Algoritmo Paralelo]] — contexto da Fase 3
- [[01 - Concorrência e Sincronização]] — mutex vs. atomic
- [[07 - Deadlock]] — como evitar (não usamos locks aninhados)
