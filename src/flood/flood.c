/* ============================================================================
 * flood.c - BFS iterativo com conectividade-8
 *
 * A BFS e iterativa (usa fila explicita) e nao recursiva. Isso evita
 * estourar a pilha em matrizes grandes com objetos que ocupam muitas
 * celulas - conforme requisito 41 do enunciado.
 * ==========================================================================*/

#include "flood.h"
#include "util.h"

#include <stdlib.h>

/* Offsets dos 8 vizinhos (conectividade-8, vizinhanca de Moore).
 * DR = delta row, DC = delta col.
 * Ordem: NW, N, NE, W, E, SW, S, SE. */
static const int DR[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
static const int DC[8] = {-1,  0,  1, -1, 1, -1, 0, 1};

int flood_contar_seq(int **matriz, int linhas, int colunas)
{
    char **visitado;
    int   *fila;
    int    head;
    int    tail;
    int    contador;
    int    i;
    int    j;
    int    d;
    int    k;
    int    r;
    int    c;
    int    nr;
    int    nc;
    size_t cap;

    /* Matriz de visitados (1 byte por celula = economico). */
    visitado = (char **) malloc(((size_t) linhas) * sizeof(char *));
    die_if(visitado == NULL, "malloc visitado");
    for (i = 0; i < linhas; i++) {
        visitado[i] = (char *) calloc((size_t) colunas, sizeof(char));
        die_if(visitado[i] == NULL, "calloc visitado linha");
    }

    /* Fila do BFS. Pior caso: matriz inteira e um so objeto -> L*C celulas.
     * Aloca uma unica vez, sem realloc dentro do BFS.
     * Codifica (r,c) como um unico int: r*colunas + c. */
    cap = (size_t) linhas * (size_t) colunas;
    fila = (int *) malloc(cap * sizeof(int));
    die_if(fila == NULL, "malloc fila BFS");

    contador = 0;

    /* Varre matriz na ordem raster (esquerda-direita, cima-baixo). */
    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {

            /* Pula celulas de fundo e ja visitadas. */
            if (matriz[i][j] != 1 || visitado[i][j]) {
                continue;
            }

            /* Encontrou o primeiro pixel de um NOVO objeto. */
            contador++;

            /* Reinicia a fila (reaproveita a alocacao). */
            head = 0;
            tail = 0;

            /* Insere a semente e marca como visitada. */
            fila[tail++] = i * colunas + j;
            visitado[i][j] = 1;

            /* BFS: enquanto houver celulas na fila, expande vizinhos-8. */
            while (head < tail) {
                k = fila[head++];
                r = k / colunas;
                c = k % colunas;

                for (d = 0; d < 8; d++) {
                    nr = r + DR[d];
                    nc = c + DC[d];

                    /* Rejeita fora da matriz. */
                    if (nr < 0 || nr >= linhas) continue;
                    if (nc < 0 || nc >= colunas) continue;

                    /* Rejeita fundo ou ja visitado. */
                    if (matriz[nr][nc] != 1 || visitado[nr][nc]) continue;

                    /* Aceita: marca e enfileira. */
                    visitado[nr][nc] = 1;
                    fila[tail++] = nr * colunas + nc;
                }
            }
        }
    }

    /* Libera memoria de trabalho. */
    for (i = 0; i < linhas; i++) {
        free(visitado[i]);
    }
    free(visitado);
    free(fila);

    return contador;
}

int flood_rotular_bloco(int **matriz, int **labels,
                        int r0, int r1, int c0, int c1,
                        int linhas, int colunas,
                        int proximo_label)
{
    int   *fila;
    int    head;
    int    tail;
    int    label_atual;
    int    atribuidos;
    int    i;
    int    j;
    int    d;
    int    k;
    int    r;
    int    c;
    int    nr;
    int    nc;
    size_t cap_bloco;

    (void) linhas; /* nao usado diretamente, mas mantido na API por clareza */

    /* Capacidade maxima da fila: numero de celulas do bloco. */
    cap_bloco = (size_t) (r1 - r0) * (size_t) (c1 - c0);
    fila = (int *) malloc(cap_bloco * sizeof(int));
    die_if(fila == NULL, "malloc fila bloco");

    label_atual = proximo_label;
    atribuidos = 0;

    for (i = r0; i < r1; i++) {
        for (j = c0; j < c1; j++) {

            /* Ignora fundo e celulas ja rotuladas. */
            if (matriz[i][j] != 1 || labels[i][j] != 0) {
                continue;
            }

            /* Novo componente local: consome o proximo label do range. */
            label_atual++;
            atribuidos++;

            head = 0;
            tail = 0;

            fila[tail++] = i * colunas + j;
            labels[i][j] = label_atual;

            /* BFS restrito ao bloco. */
            while (head < tail) {
                k = fila[head++];
                r = k / colunas;
                c = k % colunas;

                for (d = 0; d < 8; d++) {
                    nr = r + DR[d];
                    nc = c + DC[d];

                    /* Fica dentro do bloco - IMPORTANTE! */
                    if (nr < r0 || nr >= r1) continue;
                    if (nc < c0 || nc >= c1) continue;

                    /* Fundo ou ja rotulado -> nao propaga. */
                    if (matriz[nr][nc] != 1 || labels[nr][nc] != 0) continue;

                    labels[nr][nc] = label_atual;
                    fila[tail++] = nr * colunas + nc;
                }
            }
        }
    }

    free(fila);
    return atribuidos;
}
