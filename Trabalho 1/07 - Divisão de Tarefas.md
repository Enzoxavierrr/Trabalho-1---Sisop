# Divisão de Tarefas — Dupla

tags: #sisop #trabalho1 #cronograma #dupla

> **Dupla:** Enzo Xavier e Rafael Menchik Birmann.
> **A definir:** prazo de entrega no Moodle.

---

## Princípio da divisão

**Cada pessoa domina uma metade + as duas revisam a metade da outra.**

O enunciado exige (item 13, "Autoria e domínio"): *ambos devem ser capazes de explicar todo o código*. Então dividir por módulos mas fazer **pair review** obrigatório antes de cada commit importante.

## Contribuições já realizadas por Rafael

- Executou os benchmarks nas matrizes de 1200×1200, com 10 repetições por
  configuração, e coletou os resultados para 4 e 8 threads.
- Adicionou a medição dos tempos de preparo e das Fases 2, 3 e 4 no programa
  paralelo.
- Escreveu a análise de desempenho em `results/analise.md` e atualizou a seção
  correspondente no README com o resumo dos resultados.

## Divisão de responsabilidades

| Área | Enzo | Rafael |
|---|---|---|
| Setup (Makefile, estrutura, README inicial) | ✅ | 👀 revisa |
| Módulo `matriz.c` (I/O) | 👀 | ✅ |
| Módulo `dsu.c` (Union-Find) | ✅ | 👀 |
| Módulo `flood.c` (BFS sequencial + local) | 👀 | ✅ |
| `conta-objetos-sequencial.c` | 👀 | ✅ |
| `conta-objetos-paralelo.c` (Fase 2 — labeling) | ✅ | 👀 |
| Consolidação (Fase 3 — fronteiras) | ✅ | 👀 |
| Instrumentação dos tempos por fase no programa paralelo | 👀 | ✅ |
| Execução dos benchmarks e coleta de resultados | 👀 | ✅ |
| Análise de desempenho (`results/analise.md`) e resumo no README | 👀 | ✅ |
| Slides | ✅ + ✅ | ✅ + ✅ |
| Ensaio da apresentação | ✅ + ✅ | ✅ + ✅ |

**Legenda:** ✅ = responsável, 👀 = revisor obrigatório antes do merge.

## Cronograma (7 dias)

> Assumindo entrega em ~1 semana. Ajustar para o prazo real.

### Dia 1 — Fundação (~2h)
- [ ] Criar repositório GitHub (público)
- [ ] Setup: Makefile + estrutura de pastas + `.gitignore`
- [ ] README esqueleto (título, autores, comandos)
- [ ] Módulo `matriz.c/.h` — ler matriz de arquivo texto
- [ ] Módulo `util.c/.h` — helpers de erro + timing (`clock_gettime`)
- [ ] Commit: "setup inicial + módulo matriz"

### Dia 2 — Sequencial completo (~3h)
- [ ] Módulo `flood.c/.h` — BFS iterativo com vizinhança-8
- [ ] `conta-objetos-sequencial.c` — main
- [ ] Testar com 5 matrizes obrigatórias → todas devem dar o número esperado
- [ ] Adicionar as 5 matrizes de teste em `tests/`
- [ ] Commit: "versão sequencial funcional + testes básicos"

### Dia 3 — Union-Find + fronteiras (~3h)
- [ ] Módulo `dsu.c/.h` com testes unitários simples
- [ ] Esqueleto do paralelo: leitura da matriz + decomposição em blocos
- [ ] Reaproveitar BFS para versão "por bloco" (recebe bounding box)
- [ ] Commit: "DSU + decomposição em blocos"

### Dia 4 — Paralelo Fase 2 (~4h)
- [ ] Criar workers + fila de blocos + mutex
- [ ] Labeling com IDs globais únicos
- [ ] Testar Fase 2 isoladamente (sem consolidação — deve dar contagem inflada = número de labels alocados)
- [ ] Commit: "fase 2 do paralelo (labeling local)"

### Dia 5 — Consolidação Fase 3 (~4h)
- [ ] Varrer fronteiras H/V + cantos diagonais
- [ ] Chamar `dsu_union` nas conexões cross-block
- [ ] Contar roots únicos
- [ ] **Verificar que a paralela dá o MESMO número da sequencial** para as 5 matrizes
- [ ] Commit: "consolidação + paralela produz resultado correto"

### Dia 6 — Desempenho (~3h)
- [ ] Gerar `tests/m_grande.txt` (2000×2000 aleatória)
- [ ] Script `run_benchmarks.sh`
- [ ] Coletar dados (10 repetições cada config)
- [ ] Gerar gráficos (Python/matplotlib ou LibreOffice)
- [ ] Escrever `results/analise.md`
- [ ] Commit: "análise de desempenho completa"

### Dia 7 — Polimento + apresentação (~4h)
- [ ] Finalizar README (compilação, execução, arquitetura, autoria)
- [ ] Revisar todos os warnings — deve estar 100% limpo
- [ ] Rodar valgrind / verificar leaks
- [ ] Fazer slides (`slides/apresentacao.pdf`)
- [ ] Ensaiar apresentação (7-10 min)
- [ ] Commit final + tag `v1.0`
- [ ] Postar link no Moodle

## Regras de trabalho conjunto

1. **Branches por feature:** `feat/sequencial`, `feat/dsu`, `feat/paralelo-fase2`, etc.
2. **Pull request obrigatório** — o revisor lê e aprova antes de merge.
3. **Um commit ≠ 200 linhas cegas.** Commits pequenos, mensagem no imperativo.
4. **Reunião de sincronização diária** de 15 min (mesmo remota) para desbloqueio.
5. **Nada de código copiado sem entender** — se o colega escreveu e você não entende, ele explica antes do merge.
6. **Both compile + test antes de push:** `make && ./programa tests/m1_5x5.txt`.

## Ferramentas

- **Git/GitHub** — código + PRs
- **Obsidian (essas notas)** — planejamento e docs vivas
- **Discord/WhatsApp** — sync rápido
- **Google Slides ou Keynote** → exportar para PDF

## Ver também

- [[08 - Checklist de Entrega]]
- [[01 - Enunciado e Critérios]]
