/* ============================================================================
 * matriz.c - Leitura de matriz binaria em DOIS formatos suportados
 *
 * Formato 1 - .txt (nosso formato interno, uma linha "L C" + linhas):
 *
 *   5 5
 *   1 1 0 0 0
 *   1 1 0 0 0
 *   ...
 *
 * Formato 2 - .c (formato do editor do professor, https://filipomor.com/editor-tabelas-c):
 *
 *   #define LINHAS 10
 *   #define COLUNAS 10
 *   int Tabela[LINHAS][COLUNAS] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
 *                                  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 *                                  ...};
 *
 * A funcao publica matriz_ler_arquivo() auto-detecta o formato pela
 * extensao do arquivo:
 *   - .c ou .h -> formato do professor
 *   - qualquer outra -> nosso formato .txt
 * ==========================================================================*/

#define _POSIX_C_SOURCE 200112L

#include "matriz.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------------------------------------------------------------------------
 * Utilitario: verifica se s termina com o sufixo dado
 * -------------------------------------------------------------------------*/
static int termina_com(const char *s, const char *sufixo)
{
    size_t ls;
    size_t lsuf;
    ls = strlen(s);
    lsuf = strlen(sufixo);
    if (lsuf > ls) return 0;
    return strcmp(s + ls - lsuf, sufixo) == 0;
}

/* ---------------------------------------------------------------------------
 * Utilitario comum aos dois formatos: aloca a matriz L x C dinamicamente
 * -------------------------------------------------------------------------*/
static int **alocar_matriz(int linhas, int colunas)
{
    int **m;
    int   i;
    m = (int **) malloc(((size_t) linhas) * sizeof(int *));
    die_if(m == NULL, "malloc vetor de linhas");
    for (i = 0; i < linhas; i++) {
        m[i] = (int *) malloc(((size_t) colunas) * sizeof(int));
        die_if(m[i] == NULL, "malloc linha da matriz");
    }
    return m;
}

/* ---------------------------------------------------------------------------
 * Parser do formato .txt
 * -------------------------------------------------------------------------*/
static int **ler_formato_txt(const char *caminho, int *out_linhas, int *out_colunas)
{
    FILE *fp;
    int   linhas;
    int   colunas;
    int   i;
    int   j;
    int   lido;
    int   valor;
    int **matriz;

    fp = fopen(caminho, "r");
    die_if(fp == NULL, "nao foi possivel abrir arquivo de matriz (.txt)");

    lido = fscanf(fp, "%d %d", &linhas, &colunas);
    die_if(lido != 2, "formato .txt invalido: esperava 'linhas colunas' na 1a linha");
    die_if(linhas <= 0 || colunas <= 0, "dimensoes devem ser positivas");

    matriz = alocar_matriz(linhas, colunas);
    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            lido = fscanf(fp, "%d", &valor);
            die_if(lido != 1, "faltam valores no arquivo .txt");
            die_if(valor != 0 && valor != 1, "valor invalido no .txt (esperado 0 ou 1)");
            matriz[i][j] = valor;
        }
    }

    die_if(fclose(fp) != 0, "fclose falhou");
    *out_linhas = linhas;
    *out_colunas = colunas;
    return matriz;
}

/* ---------------------------------------------------------------------------
 * Parser do formato .c do professor
 *
 * Etapas:
 *   1. Le linha por linha ate encontrar "#define LINHAS <N>" e
 *      "#define COLUNAS <N>". Ordem nao importa.
 *   2. Avanca ate o primeiro caractere '{' (inicio do inicializador).
 *   3. Le exatamente LINHAS * COLUNAS caracteres '0' ou '1', ignorando
 *      qualquer outro caractere (virgulas, espacos, quebras, '}').
 *
 * O nome da variavel ("Tabela") e ignorado - qualquer nome funciona.
 * -------------------------------------------------------------------------*/
static int **ler_formato_c(const char *caminho, int *out_linhas, int *out_colunas)
{
    FILE *fp;
    int   linhas;
    int   colunas;
    int   achou_linhas;
    int   achou_colunas;
    int   c;
    int   i;
    int   j;
    int **matriz;
    char  linha[512];

    linhas = 0;
    colunas = 0;
    achou_linhas = 0;
    achou_colunas = 0;

    fp = fopen(caminho, "r");
    die_if(fp == NULL, "nao foi possivel abrir arquivo de matriz (.c)");

    /* Fase 1: procura os #define LINHAS e #define COLUNAS.
     * sscanf com %d ignora espacos em branco antes do numero. */
    while (fgets(linha, sizeof(linha), fp) != NULL) {
        if (!achou_linhas) {
            if (sscanf(linha, " #define LINHAS %d", &linhas) == 1) {
                achou_linhas = 1;
            }
        }
        if (!achou_colunas) {
            if (sscanf(linha, " #define COLUNAS %d", &colunas) == 1) {
                achou_colunas = 1;
            }
        }
        if (achou_linhas && achou_colunas) {
            break;
        }
    }
    die_if(!achou_linhas, "formato .c invalido: nao encontrou '#define LINHAS'");
    die_if(!achou_colunas, "formato .c invalido: nao encontrou '#define COLUNAS'");
    die_if(linhas <= 0 || colunas <= 0, "dimensoes .c devem ser positivas");

    /* Fase 2: avanca ate o primeiro '{' (inicio do inicializador). */
    do {
        c = fgetc(fp);
        die_if(c == EOF, "formato .c invalido: nao encontrou '{' apos os defines");
    } while (c != '{');

    /* Aloca a matriz agora que sabemos as dimensoes. */
    matriz = alocar_matriz(linhas, colunas);

    /* Fase 3: le LINHAS * COLUNAS valores. Aceita qualquer caractere
     * entre eles (virgula, espaco, tab, quebra de linha), so olha 0/1. */
    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            do {
                c = fgetc(fp);
                die_if(c == EOF, "formato .c: arquivo terminou antes do esperado");
                die_if(c == '}', "formato .c: fechou antes de completar a matriz");
            } while (c != '0' && c != '1');
            matriz[i][j] = c - '0';
        }
    }

    die_if(fclose(fp) != 0, "fclose falhou");
    *out_linhas = linhas;
    *out_colunas = colunas;
    return matriz;
}

/* ---------------------------------------------------------------------------
 * Funcoes publicas
 * -------------------------------------------------------------------------*/
int **matriz_ler_arquivo(const char *caminho, int *out_linhas, int *out_colunas)
{
    /* Auto-deteccao pelo sufixo do nome do arquivo. */
    if (termina_com(caminho, ".c") || termina_com(caminho, ".h")) {
        return ler_formato_c(caminho, out_linhas, out_colunas);
    }
    return ler_formato_txt(caminho, out_linhas, out_colunas);
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
