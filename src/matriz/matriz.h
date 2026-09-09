/* ============================================================================
 * matriz.h - Leitura, alocacao e liberacao de matrizes binarias
 *
 * Formato do arquivo texto de entrada:
 *   Linha 1 : "<linhas> <colunas>"
 *   Linha N : L linhas com C valores {0,1} separados por espacos ou tabs
 *
 * A matriz e armazenada como int** (array de ponteiros p/ linhas).
 * Escolha justificada em [[02 - Decisoes de Arquitetura]]: evita VLAs
 * (proibido em C89) e permite acesso natural matriz[i][j].
 * ==========================================================================*/

#ifndef MATRIZ_H
#define MATRIZ_H

/* Le uma matriz binaria de um arquivo texto.
 *
 * Parametros de saida (por ponteiro):
 *   out_linhas   : quantidade de linhas
 *   out_colunas  : quantidade de colunas
 *
 * Retorna:
 *   ponteiro para matriz alocada dinamicamente (int**).
 *   Encerra o programa via die_if em caso de qualquer erro.
 *
 * Contrato: o chamador deve chamar matriz_liberar() para desalocar. */
int **matriz_ler_arquivo(const char *caminho, int *out_linhas, int *out_colunas);

/* Libera uma matriz previamente alocada por matriz_ler_arquivo. */
void matriz_liberar(int **matriz, int linhas);

/* Imprime a matriz em stdout (usado para debug de matrizes pequenas). */
void matriz_imprimir(int **matriz, int linhas, int colunas);

#endif /* MATRIZ_H */
