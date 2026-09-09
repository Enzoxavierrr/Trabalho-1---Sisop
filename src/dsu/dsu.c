/* ============================================================================
 * dsu.c - Implementacao do Union-Find
 * ==========================================================================*/

#include "dsu.h"
#include "util.h"

#include <stdlib.h>

DSU *dsu_criar(int capacidade)
{
    DSU *d;
    int  i;

    d = (DSU *) malloc(sizeof(DSU));
    die_if(d == NULL, "malloc DSU");

    d->parent = (int *) malloc(((size_t) capacidade) * sizeof(int));
    die_if(d->parent == NULL, "malloc DSU.parent");

    d->rank = (int *) calloc((size_t) capacidade, sizeof(int));
    die_if(d->rank == NULL, "calloc DSU.rank");

    d->capacidade = capacidade;

    /* Cada elemento comeca em seu proprio conjunto (parent[i] = i). */
    for (i = 0; i < capacidade; i++) {
        d->parent[i] = i;
    }
    return d;
}

int dsu_find(DSU *d, int x)
{
    int raiz;
    int y;
    int prox;

    /* Passo 1: sobe ate encontrar a raiz. */
    raiz = x;
    while (d->parent[raiz] != raiz) {
        raiz = d->parent[raiz];
    }

    /* Passo 2: path compression - faz todo no do caminho apontar direto
     * para a raiz. Achata a arvore, acelerando finds futuros. */
    y = x;
    while (d->parent[y] != raiz) {
        prox = d->parent[y];
        d->parent[y] = raiz;
        y = prox;
    }
    return raiz;
}

void dsu_unir(DSU *d, int a, int b)
{
    int ra;
    int rb;

    ra = dsu_find(d, a);
    rb = dsu_find(d, b);

    if (ra == rb) {
        return; /* ja pertencem ao mesmo conjunto */
    }

    /* Union by rank: a arvore mais baixa vira filha da mais alta.
     * Isso mantem a profundidade em O(log N) sem path compression, e
     * praticamente constante quando combinado com path compression. */
    if (d->rank[ra] < d->rank[rb]) {
        d->parent[ra] = rb;
    } else if (d->rank[ra] > d->rank[rb]) {
        d->parent[rb] = ra;
    } else {
        d->parent[rb] = ra;
        d->rank[ra]++;
    }
}

void dsu_destruir(DSU *d)
{
    if (d == NULL) {
        return;
    }
    free(d->parent);
    free(d->rank);
    free(d);
}
