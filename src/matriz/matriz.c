/* ============================================================================
 * matriz.c - Implementacao de I/O de matriz binaria
 * ==========================================================================*/

#define _POSIX_C_SOURCE 200112L

#include "matriz.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>

int **matriz_ler_arquivo(const char *caminho, int *out_linhas, int *out_colunas)
{
    FILE *fp;
    int linhas;
    int colunas;
    int i;
    int j;
    int lido;
    int valor;
    int **matriz;

    fp = fopen(caminho, "r");
    die_if(fp == NULL, "nao foi possivel abrir arquivo de matriz");

    /* Le dimensoes da primeira linha. */
    lido = fscanf(fp, "%d %d", &linhas, &colunas);
    die_if(lido != 2, "formato invalido: esperava 'linhas colunas' na 1a linha");
    die_if(linhas <= 0 || colunas <= 0, "dimensoes devem ser positivas");

    /* Aloca vetor de ponteiros para as linhas. */
    matriz = (int **) malloc(((size_t) linhas) * sizeof(int *));
    die_if(matriz == NULL, "malloc falhou para vetor de linhas");

    /* Aloca cada linha e le seus valores. */
    for (i = 0; i < linhas; i++) {
        matriz[i] = (int *) malloc(((size_t) colunas) * sizeof(int));
        die_if(matriz[i] == NULL, "malloc falhou para linha da matriz");

        for (j = 0; j < colunas; j++) {
            lido = fscanf(fp, "%d", &valor);
            die_if(lido != 1, "faltam valores no arquivo da matriz");
            die_if(valor != 0 && valor != 1, "valor invalido (esperado 0 ou 1)");
            matriz[i][j] = valor;
        }
    }

    die_if(fclose(fp) != 0, "fclose falhou");

    *out_linhas = linhas;
    *out_colunas = colunas;
    return matriz;
}

void matriz_liberar(int **matriz, int linhas)
{
    int i;

    if (matriz == NULL) {
        return;
    }
    for (i = 0; i < linhas; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

void matriz_imprimir(int **matriz, int linhas, int colunas)
{
    int i;
    int j;

    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
}
