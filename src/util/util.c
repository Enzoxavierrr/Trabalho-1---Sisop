/* ============================================================================
 * util.c - Implementacao dos helpers gerais
 * ==========================================================================*/

/* _POSIX_C_SOURCE define nivel POSIX exposto pelos headers do sistema.
 * 200112L habilita clock_gettime, strdup e outras chamadas POSIX.1-2001
 * mesmo compilando com -std=c89 (que sozinho esconde essas APIs).       */
#define _POSIX_C_SOURCE 200112L

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void die_if(int cond, const char *msg)
{
    if (cond) {
        fprintf(stderr, "erro: %s\n", msg);
        exit(EXIT_FAILURE);
    }
}

void check_pthread(int rc, const char *msg)
{
    /* Funcoes pthread_* nao setam errno: elas retornam o codigo direto.
     * Por isso usamos strerror(rc) em vez de perror.                    */
    if (rc != 0) {
        fprintf(stderr, "erro: %s: %s\n", msg, strerror(rc));
        exit(EXIT_FAILURE);
    }
}

double tempo_agora_ms(void)
{
    struct timespec ts;
    int rc;

    rc = clock_gettime(CLOCK_MONOTONIC, &ts);
    die_if(rc != 0, "clock_gettime falhou");

    /* Converte segundos + nanossegundos para milissegundos em double.
     * 1s = 1000ms; 1ns = 1e-6ms.                                        */
    return ((double) ts.tv_sec) * 1000.0 + ((double) ts.tv_nsec) / 1.0e6;
}
