/* ============================================================================
 * dsu.h - Union-Find (Disjoint Set Union)
 *
 * Estrutura classica para responder rapidamente duas perguntas:
 *   1) find(x) : qual e o "representante" (raiz) do conjunto que contem x?
 *   2) unir(a, b) : unifica os conjuntos que contem a e b.
 *
 * Usado na versao paralela para juntar labels de objetos que atravessam
 * fronteiras entre blocos. Complexidade amortizada: O(alpha(N)) por
 * operacao (praticamente constante).
 * ==========================================================================*/

#ifndef DSU_H
#define DSU_H

typedef struct {
    int *parent;      /* parent[i] = pai de i (i se e raiz)               */
    int *rank;        /* estimativa da altura da arvore com raiz i        */
    int  capacidade;  /* quantos elementos cabem [0, capacidade)           */
} DSU;

/* Cria uma DSU com 'capacidade' elementos, cada um em seu proprio conjunto. */
DSU *dsu_criar(int capacidade);

/* Retorna a raiz do conjunto que contem x. Aplica path compression. */
int dsu_find(DSU *d, int x);

/* Une os conjuntos que contem a e b (nao faz nada se ja iguais).
 * Usa union by rank para manter as arvores baixas. */
void dsu_unir(DSU *d, int a, int b);

/* Libera toda a memoria da DSU. */
void dsu_destruir(DSU *d);

#endif /* DSU_H */
