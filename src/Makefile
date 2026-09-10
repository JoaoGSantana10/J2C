# Variáveis de compilação
CC = gcc
CFLAGS = -Wall
FLEX = flex

# Regra padrão (executada ao digitar apenas 'make')
all: scanner

# Compila o scanner usando o Flex e o GCC
scanner: scanner.l
	$(FLEX) scanner.l
	$(CC) $(CFLAGS) lex.yy.c -o scanner

# Limpa os arquivos gerados
clean:
	rm -f lex.yy.c scanner