# Enunciado e Critérios

tags: #sisop #trabalho1 #requisitos

---

## Problema

Dada uma matriz binária (0 = fundo, 1 = objeto), contar quantos **componentes conexos** existem usando **conectividade-8** (vizinhança de Moore — 8 vizinhos, inclui diagonais).

**Duas versões obrigatórias:**
1. **Sequencial** — referência de correção
2. **Paralela** — pelo menos 2 unidades concorrentes (processos POSIX, Pthreads ou híbrido)

## Requisitos técnicos essenciais

| Requisito | Detalhe |
|---|---|
| Linguagem | **ANSI C89/C90** — sem `//`, sem `for (int i...)`, sem declaração no meio |
| Plataforma | Linux ou macOS (nada exclusivo do Windows) |
| Flags | `-std=c89 -Wall -Wextra -pedantic -pthread` deve compilar limpo |
| Retornos POSIX | **Todos** os retornos das chamadas POSIX devem ser verificados |
| Recursão | **Evitar recursão excessiva** — usar BFS iterativo |
| Configurável | Quantidade de threads deve ser parametrizável via CLI |

## Requisitos da paralela (críticos)

1. **Distribuir trabalho real** — criar threads sem paralelizar não conta
2. **Preservar conexões** horizontais, verticais e **diagonais**
3. Produzir **exatamente** o mesmo resultado da versão sequencial
4. Sem: contagem duplicada, race conditions, deadlock, atualizações perdidas
5. Consolidar objetos que atravessam fronteiras entre blocos
6. Tratar o **encontro de 4 blocos** com conectividade diagonal

## Matrizes obrigatórias

| Ex. | Dimensões | Objetos esperados | Situação testada |
|---|---|---|---|
| 1 | 5×5 | 3 | Identificação básica |
| 2 | 6×8 | 4 | Fronteiras horizontais e verticais |
| 3 | 8×8 | 5 | **Encontro de 4 blocos + diagonal** |
| 4 | 9×12 | 6 | Objetos irregulares em 3×3 blocos |
| 5 | 12×12 | 7 | Travessia diagonal longa entre blocos |

**Plus:** pelo menos uma matriz maior para medir desempenho.

## Análise de desempenho

- Rodar sequencial + paralela nos mesmos dados
- Testar **pelo menos 2** quantidades de threads (ex: 2, 4, 8)
- Múltiplas medições → reportar média (ou mediana, mais robusto)
- Calcular **aceleração**: `S = T_sequencial / T_paralelo`
- **Explicar** casos em que a paralela é mais lenta (overhead > ganho)

## Critérios de avaliação (10 pts)

| Pts | Critério | Como maximizar |
|---|---|---|
| 2,0 | Correção seq + paralela + conectividade-8 | Testes exaustivos, mesma saída |
| 1,5 | Decomposição + paralelismo efetivo | Blocos 2D configuráveis |
| 1,5 | Sincronização sem race | Mutex mínimo, DSU + memória local por thread |
| **1,5** | **Consolidação de objetos entre regiões** | **Union-Find bem justificado — o ponto mais difícil** |
| 1,0 | Testes obrigatórios + desempenho | 5 matrizes + análise + gráficos |
| 1,0 | Qualidade ANSI C + tratamento de erros | Checar retornos, liberar recursos |
| 0,5 | Organização do repo | README, Makefile, estrutura clara |
| 1,0 | Apresentação (10 min) | Ensaiar, ter demo pronta |

## Entrega

- **GitHub público** (link enviado pelo Moodle)
- README.md com descrição, autoria, compilação, execução, arquitetura
- Makefile ou instruções reproduzíveis
- Matrizes de teste + resultados
- Análise de desempenho
- **Slides em PDF**
- Apresentação em aula (até 10 min) — **ambos da dupla participam**

## Ver também

- [[02 - Decisões de Arquitetura]]
- [[06 - Plano de Testes]]
- [[08 - Checklist de Entrega]]
