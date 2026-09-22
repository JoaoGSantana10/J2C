# Variáveis de compilação
CC = gcc
CFLAGS = -Wall
FLEX = flex
BISON = bison

# Compila o parser/compilador completo por padrão ao rodar 'make'
all: parser

# Compila o executável do parser (Bison + Flex)
parser: parser.y scanner.l
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) parser.tab.c lex.yy.c -o parser

# Compila e executa a suíte de testes unitários do scanner
test: parser.y scanner.l test_runner.c
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) -DTEST_MODE lex.yy.c test_runner.c -o unit_tests
	./unit_tests

# Limpa todos os arquivos gerados no build
clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser unit_tests

.PHONY: all test clean