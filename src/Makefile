CC = gcc
CFLAGS = -Wall
FLEX = flex
BISON = bison

all: parser

parser: parser.y scanner.l
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) parser.tab.c lex.yy.c -o parser

# Testes unitários de tokens do scanner
test-scanner: parser.y scanner.l test_runner.c
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) -DTEST_MODE lex.yy.c test_runner.c -o unit_tests
	./unit_tests

# Testes sintáticos do parser (arquivos .java)
test-parser: parser
	bash run_parser_tests.sh

# Roda todos os testes do projeto
test: test-scanner test-parser

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser unit_tests

.PHONY: all test-scanner test-parser test clean