# Algoritmo Sequencial — BFS Flood Fill

tags: #sisop #trabalho1 #algoritmo #sequencial

---

## Ideia geral

Percorrer a matriz célula a célula. Ao encontrar um `1` ainda não visitado, incrementar o contador de objetos e disparar um **BFS** que marca **todas** as células conectadas (com conectividade-8).

## Estruturas de dados

```c
/* matriz binária de entrada */
int **matriz;          /* matriz[i][j] ∈ {0,1} */
int linhas, colunas;

/* matriz de visitados */
char **visitado;       /* char basta — economiza memória */

/* fila circular do BFS */
typedef struct {
    int r, c;
} Ponto;

Ponto *fila;
size_t fila_head, fila_tail, fila_cap;
```

**Nota C89:** usar `char **` alocado com `malloc` de ponteiro e depois cada linha. Não usar VLAs (`int mat[n][m]`).

## Pseudocódigo

```
contador = 0
para cada célula (r, c):
    se matriz[r][c] == 1 e não visitado[r][c]:
        contador++
        bfs(r, c)
retornar contador

bfs(r0, c0):
    push_fila(r0, c0)
    visitado[r0][c0] = 1
    enquanto fila não vazia:
        (r, c) = pop_fila()
        para cada (dr, dc) em vizinhança-8:
            nr = r + dr; nc = c + dc
            se dentro_limites(nr, nc)
               e matriz[nr][nc] == 1
               e não visitado[nr][nc]:
                visitado[nr][nc] = 1
                push_fila(nr, nc)
```

## Vizinhança-8

```c
static const int DR[8] = {-1,-1,-1, 0, 0, 1, 1, 1};
static const int DC[8] = {-1, 0, 1,-1, 1,-1, 0, 1};
```

## Complexidade

- **Tempo:** O(L × C) — cada célula é visitada uma vez.
- **Espaço:** O(L × C) para `visitado` + O(L × C) pior caso na fila (matriz toda preenchida com 1).

## Otimizações

1. **Fila estática pré-alocada:** alocar `fila` com capacidade `L × C` uma única vez → sem `realloc` durante o BFS.
2. **`char` para visitado:** um byte por célula em vez de `int` de 4 bytes.
3. **Sair cedo do `push` se já visitado:** poupa ciclos e evita entradas duplicadas na fila.

## Casos de borda a testar

| Caso | Comportamento esperado |
|---|---|
| Matriz toda 0 | 0 objetos |
| Matriz toda 1 | 1 objeto |
| Matriz 1×1 com `[1]` | 1 objeto |
| Matriz 1×1 com `[0]` | 0 objetos |
| Objeto tocando as 4 bordas | 1 objeto, sem crash |
| Diagonal isolada (só cantos conectados) | Contar como 1 objeto |

## Interface do módulo

```c
/* flood.h */
int contar_objetos_seq(int **matriz, int linhas, int colunas);
```

## Ver também

- [[04 - Algoritmo Paralelo]] — como reaproveitar o BFS por bloco
- [[06 - Plano de Testes]]
