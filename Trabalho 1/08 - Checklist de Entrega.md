# Checklist de Entrega Final

tags: #sisop #trabalho1 #entrega #checklist

---

## Repositório GitHub (público)

- [ ] Repositório criado com nome descritivo (ex: `sisop-contagem-objetos-paralela`)
- [ ] Visibilidade: **público**
- [ ] Ambos os autores como colaboradores/commiters

### Arquivos obrigatórios

- [ ] `README.md` completo
- [ ] `Makefile` — compila `sequencial` e `paralelo` com `make`
- [ ] `src/conta-objetos-sequencial.c` + helpers
- [ ] `src/conta-objetos-paralelo.c` + helpers
- [ ] `tests/` — 5 matrizes obrigatórias + matriz de desempenho
- [ ] `results/` — CSV + análise em markdown
- [ ] `slides/apresentacao.pdf`

### README.md deve conter

- [ ] Título + descrição breve do problema
- [ ] **Autoria** (nomes + matrículas)
- [ ] Como compilar (`make`)
- [ ] Como executar cada versão (com exemplo)
- [ ] **Arquitetura da solução** (link para as decisões do Obsidian se público, senão resumir)
- [ ] Formato do arquivo de entrada
- [ ] Estratégia de decomposição
- [ ] Estratégia de consolidação
- [ ] Bibliotecas/referências externas usadas
- [ ] Tabela de resultados das 5 matrizes obrigatórias
- [ ] Resumo da análise de desempenho

## Qualidade do código

- [ ] Compila com `cc -std=c89 -Wall -Wextra -pedantic -pthread` **sem warnings**
- [ ] Todas as chamadas POSIX têm retorno verificado (fork, pthread_*, mmap, etc.)
- [ ] Sem memory leaks — verificado com `valgrind --leak-check=full`
- [ ] Sem race conditions — verificado com `valgrind --tool=helgrind`
- [ ] Todo recurso alocado é liberado (mutex_destroy, free, pthread_join)
- [ ] Sem `//` — apenas `/* */`
- [ ] Sem VLAs — apenas `malloc`
- [ ] Sem declaração de variável no meio de bloco
- [ ] Nomes significativos + comentários no *por quê*

## Testes de correção

- [ ] Matriz 1 (5×5) → 3 objetos ✓ seq ✓ par
- [ ] Matriz 2 (6×8) → 4 objetos ✓ seq ✓ par
- [ ] Matriz 3 (8×8) → 5 objetos ✓ seq ✓ par
- [ ] Matriz 4 (9×12) → 6 objetos ✓ seq ✓ par
- [ ] Matriz 5 (12×12) → 7 objetos ✓ seq ✓ par
- [ ] Casos borda: matriz toda 0, toda 1, 1×1, objeto tocando todas as bordas
- [ ] Rodar paralela com **múltiplas configurações** (2, 4, 8 threads) — mesmo resultado

## Análise de desempenho

- [ ] Matriz grande gerada (≥ 1000×1000)
- [ ] Sequencial rodada N ≥ 10 vezes → mediana
- [ ] Paralela rodada em pelo menos 2 configurações de threads
- [ ] `results/resultados.csv` com todos os dados brutos
- [ ] `results/analise.md` com:
  - [ ] Tempos absolutos (tabela)
  - [ ] Aceleração calculada `S = T_seq / T_par`
  - [ ] Eficiência `E = S / n_threads`
  - [ ] Gráfico(s) de aceleração vs. threads
  - [ ] **Discussão** — por que a aceleração não é linear (Amdahl, overhead, contenção)
  - [ ] **Casos em que a paralela é mais lenta** (matrizes pequenas) — explicar

## Apresentação (10 min)

- [ ] Slides em PDF prontos em `slides/apresentacao.pdf`
- [ ] Roteiro seguindo distribuição do enunciado:
  - [ ] 1 min — Problema + estratégia
  - [ ] 2 min — Sequencial + referência de correção
  - [ ] 2 min — Decomposição + threads + sincronização
  - [ ] 2 min — Consolidação + demo ao vivo
  - [ ] 2 min — Testes + desempenho
  - [ ] 1 min — Conclusões
- [ ] **Ambos** integrantes falam
- [ ] Ambos preparados para responder perguntas sobre **qualquer parte** do código
- [ ] Ambiente da demo preparado (matrizes prontas, comandos testados)
- [ ] Backup do slide + backup do código clonado localmente (sem depender de internet no dia)

## Entrega no Moodle

- [ ] Link do repositório GitHub enviado no Moodle
- [ ] Dentro do prazo
- [ ] Verificar se o link abre em janela anônima (repo realmente público)

## Após a entrega

- [ ] Tag `v1.0-entrega` no git
- [ ] Nenhum commit após o prazo (evita polêmica sobre "entregou depois")

## Ver também

- [[00 - Índice Trabalho 1]]
- [[01 - Enunciado e Critérios]]
- [[07 - Divisão de Tarefas]]
