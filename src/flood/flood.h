/* ============================================================================
 * flood.h - Flood fill BFS com conectividade-8
 *
 * Conectividade-8 (vizinhanca de Moore): cada celula tem 8 vizinhos.
 *
 *      NW  N  NE
 *       \  |  /
 *      W - C - E
 *       /  |  \
 *      SW  S  SE
 *
 * Duas funcoes exportadas:
 *   - flood_contar_seq   : conta objetos na matriz inteira (sequencial)
 *   - flood_rotular_bloco: rotula um bloco especifico (usado no paralelo)
 * ==========================================================================*/

#ifndef FLOOD_H
#define FLOOD_H

/* Conta quantos componentes conexos (objetos) existem na matriz binaria
 * inteira, usando conectividade-8.
 *
 * Aloca memoria de trabalho internamente e libera antes de retornar.
 * Nao modifica a matriz de entrada. */
int flood_contar_seq(int **matriz, int linhas, int colunas);

/* Percorre APENAS o bloco [r0, r1) x [c0, c1) da matriz. Cada componente
 * encontrado dentro do bloco recebe um label inteiro sequencial comecando
 * em (proximo_label + 1). O rotulo e escrito em labels[r][c].
 *
 * Restringe a BFS ao bloco: nao propaga para celulas fora dele - mesmo
 * que estejam conectadas. A juncao entre blocos e responsabilidade da
 * fase de consolidacao (via DSU no programa paralelo).
 *
 * Parametros extras:
 *   linhas, colunas : dimensoes globais da matriz (para saber tamanho
 *                     total ao codificar celulas na fila)
 *
 * Retorna: quantos labels foram atribuidos neste bloco. */
int flood_rotular_bloco(int **matriz, int **labels,
                        int r0, int r1, int c0, int c1,
                        int linhas, int colunas,
                        int proximo_label);

#endif /* FLOOD_H */
