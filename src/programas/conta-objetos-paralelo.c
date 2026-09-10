/* ============================================================================
 * conta-objetos-paralelo.c
 *
 * Versao PARALELA: usa Pthreads. A matriz e dividida em BR x BC blocos
 * retangulares que sao processados por N threads via fila de trabalho
 * (work stealing simples com mutex).
 *
 * Fluxo:
 *   Fase 1  Divide a matriz em blocos e reserva um intervalo de labels
 *           para cada bloco (sem contencao entre threads).
 *   Fase 2  N threads rodam BFS restrita ao seu bloco, rotulando as
 *           celulas de cada componente local com um label unico.
 *   Fase 3  Consolidacao sequencial: para cada celula rotulada, verifica
 *           vizinhos-8. Se dois labels distintos sao vizinhos, unir na
 *           DSU. Isso mescla componentes que atravessam fronteiras de
 *           bloco (H, V ou diagonais).
 *   Fase 4  Conta representantes unicos na DSU = numero final de objetos.
 *
 * Uso:
 *   ./conta-objetos-paralelo <arquivo_matriz> <n_threads> [<BR> <BC>]
 * ==========================================================================*/

#define _POSIX_C_SOURCE 200112L

#include "matriz.h"
#include "util.h"
#include "flood.h"
#include "dsu.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* ---------------------------------------------------------------------------
 * Tipos internos
 * -------------------------------------------------------------------------*/

/* Descreve um bloco (regiao retangular da matriz) que sera processado
 * por uma thread. O range de labels [label_inicial+1, label_inicial+cells]
 * e reservado exclusivamente para este bloco. */
typedef struct {
    int r0;
    int r1;
    int c0;
    int c1;
    int label_inicial;
} Bloco;

/* Contexto compartilhado por todas as threads worker. Todas leem os mesmos
 * ponteiros; somente 'proximo_bloco' e mutex-protegido. */
typedef struct {
    int             **matriz;
    int             **labels;
    int               linhas;
    int               colunas;
    Bloco            *blocos;
    int               total_blocos;
    int              *proximo_bloco;   /* protegido por 'mutex_fila'         */
    pthread_mutex_t  *mutex_fila;
} WorkerCtx;

/* Offsets da vizinhanca-8, usados na fase de consolidacao. */
static const int DR8[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
static const int DC8[8] = {-1,  0,  1, -1, 1, -1, 0, 1};

/* ---------------------------------------------------------------------------
 * Worker: pega blocos da fila e os processa ate a fila esvaziar
 * -------------------------------------------------------------------------*/
static void *worker(void *arg)
{
    WorkerCtx *ctx = (WorkerCtx *) arg;
    int idx;

    for (;;) {
        /* Pega o proximo bloco disponivel. */
        check_pthread(pthread_mutex_lock(ctx->mutex_fila), "mutex_lock");
        idx = *(ctx->proximo_bloco);
        (*(ctx->proximo_bloco))++;
        check_pthread(pthread_mutex_unlock(ctx->mutex_fila), "mutex_unlock");

        if (idx >= ctx->total_blocos) {
            break; /* fila esvaziou -> encerra */
        }

        /* Processa o bloco: BFS local rotulando cada componente com um
         * label unico do range reservado. Nao ha race porque blocos sao
         * disjuntos - duas threads nunca escrevem na mesma celula de
         * ctx->labels. */
        (void) flood_rotular_bloco(
            ctx->matriz, ctx->labels,
            ctx->blocos[idx].r0, ctx->blocos[idx].r1,
            ctx->blocos[idx].c0, ctx->blocos[idx].c1,
            ctx->linhas, ctx->colunas,
            ctx->blocos[idx].label_inicial);
    }
    return NULL;
}

/* ---------------------------------------------------------------------------
 * Fase 3: varre a matriz de labels e unifica vizinhos-8 com labels distintos
 *
 * Como cada bloco foi rotulado independentemente, componentes que
 * atravessam fronteiras entre blocos receberam labels DIFERENTES em cada
 * bloco. Esta funcao percorre todos os pares (celula, vizinho-8) e:
 *   - Se ambos tem label > 0 e sao DIFERENTES, chama dsu_unir.
 *   - Se sao iguais (mesmo bloco), dsu_unir e no-op (rapido).
 *
 * Executa sequencialmente: e O(L*C) e nao vale a pena paralelizar
 * (Amdahl: a fase 2 domina). Alem disso, simplifica a sincronizacao.
 * -------------------------------------------------------------------------*/
static void consolidar_fronteiras(int **labels, int linhas, int colunas,
                                  DSU *dsu)
{
    int i;
    int j;
    int d;
    int ni;
    int nj;

    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            if (labels[i][j] == 0) {
                continue;
            }
            for (d = 0; d < 8; d++) {
                ni = i + DR8[d];
                nj = j + DC8[d];
                if (ni < 0 || ni >= linhas) continue;
                if (nj < 0 || nj >= colunas) continue;
                if (labels[ni][nj] == 0) continue;
                /* dsu_unir e no-op se labels ja pertencem ao mesmo grupo. */
                dsu_unir(dsu, labels[i][j], labels[ni][nj]);
            }
        }
    }
}

/* Conta quantas raizes DISTINTAS aparecem entre todos os labels usados. */
static int contar_representantes(int **labels, int linhas, int colunas,
                                 DSU *dsu, int total_labels)
{
    char *ja_visto;
    int   i;
    int   j;
    int   raiz;
    int   contador;

    ja_visto = (char *) calloc((size_t) (total_labels + 1), sizeof(char));
    die_if(ja_visto == NULL, "calloc ja_visto");

    contador = 0;
    for (i = 0; i < linhas; i++) {
        for (j = 0; j < colunas; j++) {
            if (labels[i][j] == 0) {
                continue;
            }
            raiz = dsu_find(dsu, labels[i][j]);
            if (!ja_visto[raiz]) {
                ja_visto[raiz] = 1;
                contador++;
            }
        }
    }

    free(ja_visto);
    return contador;
}

/* ---------------------------------------------------------------------------
 * main
 * -------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int             **matriz;
    int             **labels;
    int               linhas;
    int               colunas;
    int               n_threads;
    int               br;
    int               bc;
    int               total_blocos;
    int               total_labels;
    int               cur_label;
    int               objetos;
    int               proximo_bloco;
    int               i;
    int               bi;
    int               bj;
    Bloco            *blocos;
    DSU              *dsu;
    pthread_t        *threads;
    pthread_mutex_t   mutex_fila;
    WorkerCtx         ctx;
    double            t_leitura_ini;
    double            t_leitura_fim;
    double            t_paralelo_ini;
    double            t_paralelo_fim;

    /* ------------------------------------------------------------------
     * 1) Parse de argumentos
     * ----------------------------------------------------------------*/
    if (argc != 3 && argc != 5) {
        fprintf(stderr,
                "uso: %s <arquivo_matriz> <n_threads> [<BR> <BC>]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    n_threads = atoi(argv[2]);
    if (n_threads < 1) {
        fprintf(stderr, "erro: n_threads deve ser >= 1\n");
        return EXIT_FAILURE;
    }
    br = (argc == 5) ? atoi(argv[3]) : 2;
    bc = (argc == 5) ? atoi(argv[4]) : 2;
    if (br < 1 || bc < 1) {
        fprintf(stderr, "erro: BR e BC devem ser >= 1\n");
        return EXIT_FAILURE;
    }

    /* ------------------------------------------------------------------
     * 2) Leitura da matriz
     * ----------------------------------------------------------------*/
    t_leitura_ini = tempo_agora_ms();
    matriz = matriz_ler_arquivo(argv[1], &linhas, &colunas);
    t_leitura_fim = tempo_agora_ms();

    /* Ajusta grade se maior que a matriz. */
    if (br > linhas) br = linhas;
    if (bc > colunas) bc = colunas;

    /* ------------------------------------------------------------------
     * 3) Aloca matriz de labels (inicialmente 0 = "sem label")
     * ----------------------------------------------------------------*/
    labels = (int **) malloc(((size_t) linhas) * sizeof(int *));
    die_if(labels == NULL, "malloc labels");
    for (i = 0; i < linhas; i++) {
        labels[i] = (int *) calloc((size_t) colunas, sizeof(int));
        die_if(labels[i] == NULL, "calloc labels linha");
    }

    /* ------------------------------------------------------------------
     * 4) Divide em BR x BC blocos e reserva range de labels por bloco
     *
     * Cada bloco recebe um range de labels [label_inicial+1 ..
     * label_inicial+celulas_do_bloco]. Isso garante que labels sao
     * globais unicos SEM contencao (nada de mutex durante a fase 2).
     * ----------------------------------------------------------------*/
    total_blocos = br * bc;
    blocos = (Bloco *) malloc(((size_t) total_blocos) * sizeof(Bloco));
    die_if(blocos == NULL, "malloc blocos");

    cur_label = 0; /* label 0 = fundo/sem label; blocos comecam a partir de 1 */
    for (bi = 0; bi < br; bi++) {
        for (bj = 0; bj < bc; bj++) {
            int r0;
            int r1;
            int c0;
            int c1;
            int idx;
            int celulas;

            /* Divisao balanceada: as bordas absorvem o resto da divisao. */
            r0 = bi * linhas / br;
            r1 = (bi + 1) * linhas / br;
            c0 = bj * colunas / bc;
            c1 = (bj + 1) * colunas / bc;
            idx = bi * bc + bj;
            celulas = (r1 - r0) * (c1 - c0);

            blocos[idx].r0 = r0;
            blocos[idx].r1 = r1;
            blocos[idx].c0 = c0;
            blocos[idx].c1 = c1;
            blocos[idx].label_inicial = cur_label;

            cur_label += celulas;
        }
    }
    total_labels = cur_label;

    /* ------------------------------------------------------------------
     * 5) Cria DSU com capacidade para todos os labels (mais o 0)
     * ----------------------------------------------------------------*/
    dsu = dsu_criar(total_labels + 1);

    /* ------------------------------------------------------------------
     * 6) Prepara mutex, contexto e cria threads
     * ----------------------------------------------------------------*/
    check_pthread(pthread_mutex_init(&mutex_fila, NULL), "mutex_init");
    proximo_bloco = 0;

    ctx.matriz        = matriz;
    ctx.labels        = labels;
    ctx.linhas        = linhas;
    ctx.colunas       = colunas;
    ctx.blocos        = blocos;
    ctx.total_blocos  = total_blocos;
    ctx.proximo_bloco = &proximo_bloco;
    ctx.mutex_fila    = &mutex_fila;

    threads = (pthread_t *) malloc(((size_t) n_threads) * sizeof(pthread_t));
    die_if(threads == NULL, "malloc threads");

    /* ------------------------------------------------------------------
     * 7) Executa o trabalho paralelo + consolidacao + contagem
     *    (medimos so a parte que compete com a sequencial)
     * ----------------------------------------------------------------*/
    t_paralelo_ini = tempo_agora_ms();

    for (i = 0; i < n_threads; i++) {
        check_pthread(pthread_create(&threads[i], NULL, worker, &ctx),
                      "pthread_create");
    }
    for (i = 0; i < n_threads; i++) {
        check_pthread(pthread_join(threads[i], NULL), "pthread_join");
    }

    /* Fase 3: consolidar labels que atravessam blocos. */
    consolidar_fronteiras(labels, linhas, colunas, dsu);

    /* Fase 4: contar raizes unicas. */
    objetos = contar_representantes(labels, linhas, colunas, dsu,
                                    total_labels);

    t_paralelo_fim = tempo_agora_ms();

    /* ------------------------------------------------------------------
     * 8) Relatorio
     * ----------------------------------------------------------------*/
    printf("arquivo         : %s\n", argv[1]);
    printf("dimensoes       : %d x %d\n", linhas, colunas);
    printf("threads         : %d\n", n_threads);
    printf("grade de blocos : %d x %d (%d blocos)\n", br, bc, total_blocos);
    printf("objetos         : %d\n", objetos);
    printf("tempo leitura   : %.3f ms\n", t_leitura_fim - t_leitura_ini);
    printf("tempo paralelo  : %.3f ms\n", t_paralelo_fim - t_paralelo_ini);

    /* ------------------------------------------------------------------
     * 9) Cleanup (libera todos os recursos)
     * ----------------------------------------------------------------*/
    check_pthread(pthread_mutex_destroy(&mutex_fila), "mutex_destroy");
    dsu_destruir(dsu);
    for (i = 0; i < linhas; i++) {
        free(labels[i]);
    }
    free(labels);
    free(blocos);
    free(threads);
    matriz_liberar(matriz, linhas);

    return EXIT_SUCCESS;
}
