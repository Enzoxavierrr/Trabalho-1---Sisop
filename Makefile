# =============================================================================
# Trabalho 1 - Sistemas Operacionais - PUCRS 2026/II
# Contagem paralela de objetos em matriz binaria (conectividade-8)
# =============================================================================

# Compilador e flags conforme exigencia do enunciado (ANSI C89)
CC      = cc
CFLAGS  = -std=c89 -Wall -Wextra -pedantic -O2
LDFLAGS = -pthread

# -I diz ao compilador: "quando um .c fizer #include, procure os .h aqui".
# Precisamos listar cada subpasta que contem um header do projeto.
INCLUDES = -Isrc/util -Isrc/matriz -Isrc/flood -Isrc/dsu

# Diretorio raiz do codigo
SRC_DIR = src

# Modulos compartilhados entre as duas versoes (cada um numa subpasta)
COMMON_SRC = $(SRC_DIR)/util/util.c     \
             $(SRC_DIR)/matriz/matriz.c \
             $(SRC_DIR)/dsu/dsu.c       \
             $(SRC_DIR)/flood/flood.c

COMMON_HDR = $(SRC_DIR)/util/util.h     \
             $(SRC_DIR)/matriz/matriz.h \
             $(SRC_DIR)/dsu/dsu.h       \
             $(SRC_DIR)/flood/flood.h

# Fontes de cada versao (main + modulos)
SEQ_SRC = $(SRC_DIR)/programas/conta-objetos-sequencial.c $(COMMON_SRC)
PAR_SRC = $(SRC_DIR)/programas/conta-objetos-paralelo.c   $(COMMON_SRC)

# Binarios finais (ficam na raiz para facilitar a execucao)
SEQ_BIN = conta-objetos-sequencial
PAR_BIN = conta-objetos-paralelo

# -----------------------------------------------------------------------------
# Targets principais
# -----------------------------------------------------------------------------

.PHONY: all sequencial paralelo clean test test-seq test-par help

all: sequencial paralelo

sequencial: $(SEQ_BIN)

paralelo: $(PAR_BIN)

$(SEQ_BIN): $(SEQ_SRC) $(COMMON_HDR)
	$(CC) $(CFLAGS) $(INCLUDES) $(SEQ_SRC) -o $@ $(LDFLAGS)

$(PAR_BIN): $(PAR_SRC) $(COMMON_HDR)
	$(CC) $(CFLAGS) $(INCLUDES) $(PAR_SRC) -o $@ $(LDFLAGS)

# -----------------------------------------------------------------------------
# Utilitarios
# -----------------------------------------------------------------------------

clean:
	rm -f $(SEQ_BIN) $(PAR_BIN)
	find $(SRC_DIR) -name "*.o" -delete
	rm -rf *.dSYM

test: test-seq test-par

test-seq: sequencial
	@echo "== Testes sequencial =="
	@for m in tests/m*.txt; do \
		[ -f "$$m" ] && ./$(SEQ_BIN) "$$m"; \
	done

test-par: paralelo
	@echo "== Testes paralelo (4 threads) =="
	@for m in tests/m*.txt; do \
		[ -f "$$m" ] && ./$(PAR_BIN) "$$m" 4; \
	done

help:
	@echo "Targets disponiveis:"
	@echo "  make                Compila as duas versoes"
	@echo "  make sequencial     Compila apenas a versao sequencial"
	@echo "  make paralelo       Compila apenas a versao paralela"
	@echo "  make test           Roda ambas nas matrizes de tests/"
	@echo "  make clean          Remove binarios e artefatos"
	@echo ""
	@echo "Uso dos binarios:"
	@echo "  ./conta-objetos-sequencial <arquivo_matriz>"
	@echo "  ./conta-objetos-paralelo   <arquivo_matriz> <n_threads> [BR BC]"
