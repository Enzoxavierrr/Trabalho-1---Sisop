/* ============================================================================
 * conta-objetos-sequencial.c
 *
 * Versao SEQUENCIAL: um unico fluxo de execucao percorre a matriz binaria
 * e conta os componentes conexos usando conectividade-8 via flood fill BFS.
 *
 * Uso:
 *   ./conta-objetos-sequencial <arquivo_matriz>
 * ==========================================================================*/

#include "matriz.h"
#include "util.h"
#include "flood.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int   **matriz;
    int    linhas;
    int    colunas;
    int    objetos;
    double t_leitura_ini;
    double t_leitura_fim;
    double t_conta_ini;
    double t_conta_fim;

    if (argc != 2) {
        fprintf(stderr, "uso: %s <arquivo_matriz>\n", argv[0]);
        return EXIT_FAILURE;
    }

    /* Fase 1: leitura do arquivo. */
    t_leitura_ini = tempo_agora_ms();
    matriz = matriz_ler_arquivo(argv[1], &linhas, &colunas);
    t_leitura_fim = tempo_agora_ms();

    /* Fase 2: contagem de objetos.
     * A leitura e a contagem sao cronometradas separadamente porque so a
     * contagem e comparavel com a versao paralela - a leitura e identica nas
     * duas e, em matrizes grandes, domina o tempo de parede (parsing de
     * milhoes de caracteres). Misturar as duas medidas esconderia o efeito
     * do paralelismo. */
    t_conta_ini = tempo_agora_ms();
    objetos = flood_contar_seq(matriz, linhas, colunas);
    t_conta_fim = tempo_agora_ms();

    /* Relatorio. */
    printf("arquivo         : %s\n", argv[1]);
    printf("dimensoes       : %d x %d\n", linhas, colunas);
    printf("objetos         : %d\n", objetos);
    printf("tempo leitura   : %.3f ms\n", t_leitura_fim - t_leitura_ini);
    printf("tempo contagem  : %.3f ms\n", t_conta_fim - t_conta_ini);

    matriz_liberar(matriz, linhas);
    return EXIT_SUCCESS;
}
