# Plano de Testes

tags: #sisop #trabalho1 #testes #desempenho

---

## Fase 1 — Testes de correção (obrigatórios)

As 5 matrizes do enunciado. Cada uma vira um arquivo em `tests/`.

### Matriz 1 — Identificação básica (5×5, 3 objetos)

```
5 5
1 1 0 0 0
1 1 0 0 0
0 0 0 1 0
0 0 0 1 0
1 0 0 0 0
```

**Objetos esperados:** 3
**Testar com:** paralelo em grade 2×2 (matriz 5×5 → blocos de 3×3, 3×2, 2×3, 2×2 aprox.)

### Matriz 2 — Fronteiras H/V (6×8, 4 objetos)

```
6 8
0 0 0 0 0 0 1 1
0 1 1 1 1 0 1 0
0 0 1 1 0 0 0 0
0 0 0 1 1 0 0 0
0 0 0 0 1 0 0 1
1 1 0 0 0 0 1 1
```

**Objetos esperados:** 4
**Testar com:** grade 2×2 (blocos 3×4)

### Matriz 3 — Encontro de 4 blocos + diagonal (8×8, 5 objetos) ⚠️ **CRÍTICO**

```
8 8
1 1 0 0 0 0 0 0
1 0 0 0 0 0 0 0
0 0 0 0 0 0 1 0
0 0 0 1 1 0 1 0
0 0 0 1 1 0 0 0
0 0 0 0 0 0 0 0
0 0 1 0 0 0 0 1
0 0 1 0 0 0 1 1
```

**Objetos esperados:** 5
**Testar com:** grade 2×2 (blocos 4×4) — objeto central `(3,3)-(3,4)-(4,3)-(4,4)` toca os 4 blocos

### Matriz 4 — Objetos irregulares (9×12, 6 objetos)

```
9 12
0 1 1 0 0 0 0 0 0 0 1 0
0 0 1 1 1 1 0 0 0 1 1 0
0 0 0 0 0 1 0 0 0 0 0 0
0 0 0 0 0 1 1 0 0 0 0 0
0 1 0 0 0 0 1 0 0 1 0 0
0 1 1 0 0 0 0 0 1 1 0 0
0 0 1 1 0 0 0 0 1 0 0 0
0 0 0 1 0 0 0 1 1 0 0 0
0 0 0 0 0 1 0 0 0 0 0 1
```

**Objetos esperados:** 6
**Testar com:** grade 3×3 (blocos 3×4)

### Matriz 5 — Diagonal longa (12×12, 7 objetos)

```
12 12
1 0 0 0 0 1 1 1 1 0 1 1
0 1 0 0 0 1 0 0 1 0 1 0
0 0 1 0 0 0 0 0 0 0 0 0
0 0 0 1 0 0 0 0 0 0 0 0
0 0 0 0 1 0 0 0 0 0 0 0
1 1 0 0 0 1 0 0 0 0 0 0
1 0 0 0 0 0 1 0 0 0 0 0
0 0 0 1 1 0 0 1 0 0 0 0
0 0 0 1 1 0 0 0 1 0 0 0
0 0 0 0 0 0 0 0 0 1 0 0
0 1 0 0 0 0 0 0 0 0 1 0
0 1 1 0 0 0 1 0 0 0 0 1
```

**Objetos esperados:** 7
**Testar com:** grade 3×3 (blocos 4×4) — diagonal longa atravessa 3 blocos

## Fase 2 — Matriz de desempenho

**Tamanho alvo:** 2000×2000 (4M células).

**Estratégias de geração:**
1. **Aleatória com densidade controlada** (script Python que salva em .txt):
   - Densidade 30% de 1s
   - Semente fixa para reprodutibilidade
2. **Padrão estruturado:** grades ou espirais que forçam objetos longos entre blocos
3. **Pior caso adversarial:** matriz que gera muitos objetos pequenos travessando fronteiras

Salvar em `tests/m_grande_2000x2000.txt` (comprimir se ficar grande demais para o git — `.gitignore` o descomprimido).

## Fase 3 — Bateria de desempenho

| Config | n_threads | grade | Repetições |
|---|---|---|---|
| Seq | — | — | 10 |
| P1 | 2 | 2×1 | 10 |
| P2 | 4 | 2×2 | 10 |
| P3 | 8 | 4×2 | 10 |
| P4 | 16 | 4×4 | 10 |

Reportar **mediana** (mais robusta que média para outliers de escalonamento).

## Métricas

- **T_seq:** tempo mediano da versão sequencial
- **T_par(n):** tempo mediano da versão paralela com `n` threads
- **Aceleração:** `S(n) = T_seq / T_par(n)`
- **Eficiência:** `E(n) = S(n) / n`

Fórmula do enunciado: `S = T_sequencial / T_paralelo`.

## Formato do CSV

```csv
matriz,dimensoes,versao,n_threads,grade,rep,tempo_ms,objetos
m1_5x5,5x5,seq,1,-,1,0.012,3
m1_5x5,5x5,seq,1,-,2,0.011,3
...
m_grande,2000x2000,par,4,2x2,1,187.3,15234
```

## O que analisar no relatório

1. **Correção:** todas as 5 matrizes obrigatórias batem com esperado?
2. **Consistência seq vs. paralela:** mesma matriz produz mesmo número em todas as configurações?
3. **Ganho de aceleração** vs. número de threads — plotar
4. **Eficiência decrescente** com mais threads — Lei de Amdahl ([[12 - Escalabilidade — Lei de Amdahl]])
5. **Overhead das matrizes pequenas:** provavelmente T_par > T_seq nas matrizes pequenas por causa de custos fixos
6. **Anomalias:** casos em que mais threads pioram — explicar (contenção, cache thrashing, false sharing)

## Automação sugerida

Script `run_benchmarks.sh`:
```bash
#!/bin/bash
for matriz in tests/*.txt; do
    for rep in 1 2 3 4 5 6 7 8 9 10; do
        ./conta-objetos-sequencial $matriz  # loga em CSV
        for nt in 2 4 8; do
            ./conta-objetos-paralelo $matriz $nt  # loga em CSV
        done
    done
done
```

O programa deve **logar o tempo em CSV via stderr ou arquivo** — não misturar com a saída do "objetos encontrados".

## Ver também

- [[01 - Enunciado e Critérios]]
- [[12 - Escalabilidade — Lei de Amdahl]]
- [[08 - Checklist de Entrega]]
