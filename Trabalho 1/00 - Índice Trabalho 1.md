# Trabalho 1 — Contagem Paralela de Objetos

tags: #sisop #trabalho1 #pthreads #union-find

> **Disciplina:** Sistemas Operacionais 2026/II — Prof. Filipo Mór (PUCRS)
> **Modalidade:** Dupla | **Linguagem:** ANSI C (C89/C90) | **Peso:** 10 pts

---

## Notas do planejamento

1. [[01 - Enunciado e Critérios]] — o que o professor pede + como será avaliado
2. [[02 - Decisões de Arquitetura]] — stack técnica escolhida + justificativa
3. [[03 - Algoritmo Sequencial]] — BFS flood fill com conectividade-8
4. [[04 - Algoritmo Paralelo]] — decomposição por blocos 2D + Pthreads
5. [[05 - Consolidação com Union-Find]] — como unir objetos que atravessam blocos
6. [[06 - Plano de Testes]] — 5 matrizes obrigatórias + matriz de desempenho
7. [[07 - Divisão de Tarefas]] — split entre a dupla + cronograma
8. [[08 - Checklist de Entrega]] — repositório + slides + apresentação

## Referências ao conteúdo

- [[01 - Concorrência e Sincronização]] — mutex, race conditions
- [[06 - Algoritmos de Escalonamento]] — contexto de threads
- [[08 - Processos e Threads — Por Baixo do Capô]] — `pthread_create` vs `fork`
- [[09 - Custos Reais]] — por que threads ganham para esse workload
- [[11 - Hardware — SMT, NUMA e False Sharing]] — cuidado no padding do DSU
- [[12 - Escalabilidade — Lei de Amdahl]] — análise da aceleração

## Stack fechada

| Camada | Escolha | Justificativa |
|---|---|---|
| Paralelismo | **Pthreads** | Memória compartilhada natural, ~4× mais barato que fork |
| Decomposição | **Blocos 2D configuráveis** | Cobre encontro de 4 blocos + diagonal (Ex. 3) |
| Algoritmo local | **BFS iterativo (fila)** | Sem risco de stack overflow em matrizes grandes |
| Consolidação | **Union-Find (path compression + rank)** | O(α(N)) amortizado, determinístico |
| Build | **Makefile + gcc ANSI C89** | `-std=c89 -Wall -Wextra -pedantic -pthread` |
