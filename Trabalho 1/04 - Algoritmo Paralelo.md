# Algoritmo Paralelo — Pthreads + Blocos 2D

tags: #sisop #trabalho1 #algoritmo #paralelo #pthreads

---

## Visão geral em 3 fases

```
┌────────────────────────────────────────────────────────────┐
│  Fase 1: DECOMPOSIÇÃO                                       │
│  Dividir matriz L×C em BR × BC blocos retangulares          │
└────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────┐
│  Fase 2: LABELING LOCAL (PARALELO)                          │
│  N threads consomem blocos da fila e rodam BFS local        │
│  atribuindo labels ÚNICOS por bloco (namespace disjunto)    │
└────────────────────────────────────────────────────────────┘
                            ↓
┌────────────────────────────────────────────────────────────┐
│  Fase 3: CONSOLIDAÇÃO (SEQUENCIAL, RÁPIDA)                  │
│  Varrer fronteiras entre blocos e unir labels equivalentes  │
│  no Union-Find. Contar representantes únicos.               │
└────────────────────────────────────────────────────────────┘
```

## Fase 1 — Decomposição

- Entrada: matriz L×C, `n_threads`, `BR`, `BC`.
- Divide L em `BR` faixas (as últimas absorvem o resto se não divide) e C em `BC` faixas.
- Total de blocos: `BR × BC`. Pode ser **maior** que `n_threads` (fila de trabalho).

```c
typedef struct {
    int r0, r1;   /* linhas [r0, r1) */
    int c0, c1;   /* colunas [c0, c1) */
    int id_bloco;
} Bloco;
```

## Fase 2 — Labeling local (paralelo)

### Estruturas globais compartilhadas

```c
/* matriz de labels — cada célula recebe seu label global único */
int **labels;   /* alocada 1 vez, cada thread escreve APENAS na sua região */

/* Union-Find global — dimensionado com upper bound de labels possíveis */
DSU *dsu;

/* fila de blocos + contador atômico ou mutex */
Bloco *blocos;
int total_blocos;
int proximo_bloco;                    /* protegido por mutex */
pthread_mutex_t mutex_fila;

/* alocador de labels globais — cada thread reserva um range */
int proximo_label;                    /* protegido por mutex */
pthread_mutex_t mutex_label;
```

### Estratégia de labels

Para evitar contenção pesada, cada thread, ao processar um bloco, **reserva em massa** IDs de label:
1. Faz um pré-scan do bloco contando quantas células == 1 → upper bound de labels.
2. Adquire `mutex_label`, lê `proximo_label`, incrementa por esse upper bound, solta.
3. Usa esse range localmente sem tocar mais em nada global.

Alternativa mais simples: contar componentes no bloco primeiro, depois reservar exatamente esse número. Duas passadas mas sem desperdiço.

### Loop do worker

```c
void *worker(void *arg) {
    Bloco b;
    while (pegar_proximo_bloco(&b)) {
        /* BFS local: só visita células DENTRO do bloco */
        processar_bloco(b);
    }
    return NULL;
}
```

### `processar_bloco`

Reaproveita o BFS sequencial com **duas restrições**:
1. Vizinhança-8 só considera vizinho `(nr, nc)` se `b.r0 ≤ nr < b.r1` e `b.c0 ≤ nc < b.c1`.
2. Cada componente encontrado ganha um label global (do range reservado) e é escrito em `labels[r][c]`.

Nada além disso é tocado — sem mutex durante o BFS.

### Por que isso é livre de race conditions

- Blocos são **disjuntos** → duas threads nunca escrevem na mesma célula de `labels`.
- `visitado` pode ser dispensado: `labels[r][c] != 0` indica "já processado".
- Único ponto de sincronização: pegar próximo bloco + reservar range de labels. Ambos são O(1) por bloco.

## Fase 3 — Consolidação

Ver detalhes em [[05 - Consolidação com Union-Find]].

Em resumo:
1. Para cada par de blocos **adjacentes** (horizontal, vertical, diagonal), varrer as células de fronteira.
2. Se `labels[r][c] != 0` e um vizinho em outro bloco `labels[nr][nc] != 0` e são conectados na conectividade-8 (é fronteira!) → `dsu_union(labels[r][c], labels[nr][nc])`.
3. Contar quantos "roots" distintos existem entre todos os labels usados.

**Justificativa de manter Fase 3 sequencial:**
- É O(fronteiras) = O(L + C) por dimensão da grade — muito menor que O(L × C).
- Elimina sincronização complexa; união-find precisa de proteção se paralelizado.
- Amdahl: se essa fração serial é <5% do tempo total, o overhead da paralelização não compensa.

## Considerações de sincronização

| Ponto | Mecanismo | Justificativa |
|---|---|---|
| Pegar próximo bloco | `pthread_mutex_t mutex_fila` | Curto (incrementar índice + copiar bloco) |
| Reservar range de labels | `pthread_mutex_t mutex_label` | Curto (2 leituras + 1 escrita) |
| Escrever em `labels[r][c]` | **Nenhum** | Cada thread só escreve nas suas células |
| BFS local | **Nenhum** | Não acessa dados de outras threads |
| Union-Find na Fase 3 | **Nenhum** | Executa em thread única |

## Fluxo da `main` paralela

```c
int main(int argc, char *argv[]) {
    /* 1. Parse args, ler matriz */
    /* 2. Alocar labels[][], DSU, blocos[], mutexes */
    /* 3. Criar n_threads threads */
    /* 4. pthread_join em todas */
    /* 5. Consolidação sequencial (Fase 3) */
    /* 6. Contar roots únicos */
    /* 7. Liberar tudo */
    /* 8. Imprimir resultado + tempo */
}
```

## Ver também

- [[03 - Algoritmo Sequencial]] — BFS base reaproveitado
- [[05 - Consolidação com Union-Find]]
- [[06 - Plano de Testes]]
- [[12 - Escalabilidade — Lei de Amdahl]] — justifica manter Fase 3 sequencial
