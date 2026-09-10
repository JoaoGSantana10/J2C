# Variáveis de compilação
CC = gcc
CFLAGS = -Wall
FLEX = flex

# Executado ao rodar apenas 'make'
all: scanner

# Compila o scanner executável principal
scanner: scanner.l
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) lex.yy.c -o scanner

# Compila e roda a suíte de testes unitários
test:
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) -DTEST_MODE lex.yy.c test_runner.c -o unit_tests
	./unit_tests

# Limpa todos os arquivos gerados
clean:
	rm -f lex.yy.c scanner unit_tests

.PHONY: all test clean