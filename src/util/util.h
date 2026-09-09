/* ============================================================================
 * util.h - Helpers gerais do projeto
 *
 * Fornece:
 *   - Macros/funcoes para checar retornos POSIX (requisito 14 do enunciado)
 *   - Encerramento com mensagem de erro (die_if)
 *   - Medicao de tempo monotonico em milissegundos
 * ==========================================================================*/

#ifndef UTIL_H
#define UTIL_H

/* Encerra o programa com mensagem em stderr caso cond seja verdadeira.
 * Usa exit(EXIT_FAILURE). Nao retorna. */
void die_if(int cond, const char *msg);

/* Encerra o programa se rc != 0, imprimindo msg e strerror(rc).
 * Uso tipico com retornos de funcoes pthread_* (que retornam errno). */
void check_pthread(int rc, const char *msg);

/* Retorna o tempo atual em milissegundos (double, precisao de sub-ms)
 * usando CLOCK_MONOTONIC. Usado para medir intervalos. */
double tempo_agora_ms(void);

#endif /* UTIL_H */
