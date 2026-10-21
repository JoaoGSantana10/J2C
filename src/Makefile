CC = gcc
CFLAGS = -Wall
FLEX = flex
BISON = bison

all: parser

parser: parser.y scanner.l symtab.c symtab.h
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) parser.tab.c lex.yy.c symtab.c -o parser

# Testes unitários de tokens do scanner
test-scanner: parser.y scanner.l test_runner.c
	$(BISON) -d parser.y
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) -DTEST_MODE lex.yy.c test_runner.c -o unit_tests
	./unit_tests

# Testes unitários da tabela de símbolos
test-symtab: test_symtab.c symtab.c symtab.h
	$(CC) $(CFLAGS) -Wextra -fsanitize=address,undefined test_symtab.c symtab.c -o unit_tests_symtab
	./unit_tests_symtab

# Testes sintáticos do parser (arquivos .java)
test-parser: parser
	bash run_parser_tests.sh

# Roda todos os testes do projeto
test: test-scanner test-symtab test-parser

clean:
	rm -f lex.yy.c parser.tab.c parser.tab.h parser unit_tests unit_tests_symtab

.PHONY: all test-scanner test-symtab test-parser test clean