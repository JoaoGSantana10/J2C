# ☕ J2C — Compilador / Transpilador de Java para C

![Java](https://img.shields.io/badge/Java-ED8B00?style=for-the-badge&logo=openjdk&logoColor=white)
![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-blue?style=for-the-badge)

> Um compilador/transpilador de código-fonte Java para código C legível e executável, desenvolvido como projeto acadêmico da disciplina de Compiladores.

---

## 📌 Sumário

- [Sobre o Projeto](#-sobre-o-projeto)
- [🎯 Objetivos e Escopo](#-objetivos-e-escopo)
- [🛠️ Arquitetura e Tecnologias](#️-arquitetura-e-tecnologias)
- [🚀 Configuração do Ambiente e Testes](#-configuração-do-ambiente-e-testes)
  - [Pré-requisitos](#pré-requisitos)
  - [Instalação e Build](#instalação-e-build)
  - [Compilando e Executando o Código C Gerado](#compilando-e-executando-o-código-c-gerado)
- [🤝 Equipe](#equipe)

---

## 📖 Sobre o Projeto

O **J2C** é um transpilador (compilador de código para código) que traduz um subconjunto da linguagem **Java** para código-fonte em **C** (`ANSI C` / `C99`) eficiente e legível.

O projeto implementa todas as etapas clássicas do pipeline de compilação:

1. **Análise Léxica (_Scanning_):** Reconhecimento de tokens e remoção de espaços/comentários.
2. **Análise Sintática (_Parsing_):** Validação gramatical e construção da Árvore de Sintaxe Abstrata (_AST - Abstract Syntax Tree_).
3. **Análise Semântica (_Semantic Analysis_):** Tabela de símbolos, checagem e inferência de tipos, verificação de escopo e identificação de erros de tipo.
4. **Geração de Código (_Code Generation_):** Mapeamento e tradução dos nós da AST para sintaxe C correspondente (ex: mapeamento de classes para `structs`, métodos para funções).

---

## 🎯 Objetivos e Escopo

### Funcionalidades do Subconjunto Java Suportado:

- [x] **Tipos Primitivos:** `int`, `double`, `boolean`, `char`.
- [x] **Estruturas de Controle:** `if`, `else`, `while`, `for`.
- [x] **Funções e Métodos:** Parâmetros, valores de retorno e recursão.
- [x] **Entrada e Saída:** Reconhecimento de `System.out.println(...)`.
- [x] **Estruturas de Dados:** Reconhecimento de arrays unidimensionais.
- [ ] **Geração de Código Completa:** Tradução dos nós da AST para C.
- [ ] **Orientação a Objetos:** Mapeamento de Classes e Objetos para `structs` e ponteiros em C.

> **Estado atual da geração:** o gerador implementa `NODE_PROGRAM`, `NODE_CLASS`, `NODE_VAR_DECL`, `NODE_FUNC_DECL`, `NODE_PARAM`, `NODE_BLOCK`, `NODE_RETURN`, as expressões (`NODE_BIN_OP`, `NODE_UNARY_OP`, `NODE_CALL`, `NODE_IDENTIFIER` e literais), `NODE_ASSIGN` e `NODE_EXPR_STMT`, além de `System.out.println`/`print` e da leitura com `Scanner` (`nextInt` e `nextDouble`). Outros `new` e chamadas qualificadas são rejeitados com erro. As estruturas de controle (`if`, laços, `switch`) ainda não possuem tradução C.

---

## 🛠️ Arquitetura e Tecnologias

- **Linguagem do Compilador:** C
- **Analisador Léxico:** Flex
- **Analisador Sintático:** Bison
- **Linguagem de Entrada:** Subconjunto de Java
- **Linguagem Alvo:** C (compatível com GCC / Clang)

---

## 🚀 Configuração do Ambiente e Testes

### Pré-requisitos

Certifique-se de possuir os seguintes componentes instalados e configurados no seu ambiente de desenvolvimento:

1. **Compilador C (GCC ou Clang)**
   - **Linux / macOS:** GCC ou Clang pré-instalado (ou via `sudo apt install build-essential`).
   - **Windows:** MinGW-w64, MSVC ou ambiente WSL.
   - Teste de instalação:
     ```bash
     gcc --version
     ```

2. **Flex, Bison e Make**
   - **Debian/Ubuntu:**
     ```bash
     sudo apt install build-essential flex bison
     ```
   - **macOS:**
     ```bash
     brew install flex bison make
     ```
   - Teste de instalação:
     ```bash
     flex --version
     bison --version
     make --version
     ```

---

### Instalação e Build

1. **Clone o repositório do projeto:**
   ```bash
   git clone https://github.com/JoaoGSantana10/J2C.git
   cd J2C/src
   make
   ```

Isso gera o executável `./parser`. Os artefatos gerados podem ser removidos com:

```bash
make clean
```

Para executar todas as suítes disponíveis:

```bash
make test
```

Para executar apenas os testes de geração de código:

```bash
make test-codegen
```

### Compilando e Executando o Código C Gerado

Para transpilar um programa escrito em Java para C:

```bash
./parser saida.c < caminho/para/MeuPrograma.java
```

Para testar o resultado final no ambiente C:

```bash
# Compilar o código C gerado
gcc saida.c -o MeuPrograma

# Executar o programa
./MeuPrograma
```

`./parser` recebe o caminho do arquivo C de saída como primeiro argumento e lê o código Java pela entrada padrão (`stdin`). Executar `./parser` sem argumento valida e imprime a AST, mas não gera código C.

O gerador cria o arquivo de saída com este contrato:

- um comentário indicando que o conteúdo foi gerado pelo J2C e os `#include` de `<stdio.h>`, `<stdlib.h>` e `<stdbool.h>`;
- o nome da classe Java como comentário (`/* Classe Nome */`);
- campos da classe como variáveis globais, na ordem: campos, protótipos das funções, definições das funções;
- métodos como funções C (sem parâmetros vira `(void)`); `public`, `private`, `protected` e `static` não são traduzidos;
- `public static void main(String[] args)` vira `int main(void)` com `return 0;` ao final (os `args` são ignorados); sem `main` no Java, é gerada uma `main` vazia.

Mapeamento de tipos: `int`→`int`, `long`→`long long`, `float`→`float`, `double`→`double`, `boolean`→`bool`, `char`→`char`, `void`→`void`, `String`→`char *` e `String[]`→`char **`. Tipos de classe ainda não são suportados e geram erro.

A geração atual cobre declarações, tipos, funções e expressões. Outros nós válidos da AST (`if`, laços, `switch` etc.) ainda não têm tradução e fazem o gerador retornar erro sem criar ou preservar um arquivo C parcial. Também é erro: nome ausente ou igual a palavra reservada de C, nome repetido entre campos e funções (C não tem sobrecarga), parâmetro ou variável `void` e `main` com assinatura diferente de `main(String[] args)`.

#### Expressões

- **Precedência e associatividade:** toda subexpressão binária é escrita entre parênteses, seguindo a AST. `a + b * c` vira `a + (b * c)` e `a - b - c` vira `(a - b) - c`, então o significado não depende da tabela de precedência do C.
- **Unários:** `++`/`--` respeitam prefixo e pósfixo (`is_postfix`). Um unário aplicado a outro unário ou a uma expressão binária ganha parênteses: `-(-a)`, `-(a + b)`.
- **Literais:** `10L` vira `10LL`; `3.14d` vira `3.14` e `5d` vira `5.0`; `5f` vira `5.0f`; `true`/`false` vêm de `<stdbool.h>`; `'\'` e `'''` recebem escape. Em `String`, `\uXXXX` vira os bytes UTF-8 em octal e octais do Java são normalizados para 3 dígitos.
- **Chamadas e atribuições:** chamadas simples (`f(a, b + 1)`) e `=`, `+=`, `-=`. O número de argumentos é conferido com a declaração da função.
- **Campos:** aceitam inicializadores constantes (literais e operadores, como `-1` ou `60 * 60`), porque variável global em C só aceita constante.

São rejeitados com erro, em vez de gerar C que compila com outro significado:

- concatenação de `String` com `+` ou `+=` (em C seria aritmética de ponteiro);
- `%` com `float`/`double` (não existe em C);
- `++`/`--` em uma variável usada de novo na mesma expressão, como `i++ + i` ou `i = i++` (Java avalia da esquerda para a direita; em C é comportamento indefinido);
- função `void` usada como valor, nome de função usado como variável e variável local com o mesmo nome de uma função chamada;
- `new` fora de `Scanner sc = new Scanner(System.in)` (classes ainda não são traduzidas) e chamadas qualificadas que não sejam de entrada e saída, como `Math.max`.

#### Entrada e saída

- **`System.out.println(x)` / `System.out.print(x)`** viram `printf`, com o formato escolhido pelo tipo de `x`: `int`→`%d`, `long`→`%lld`, `float`→`%f`, `double`→`%lf`, `char`→`%c`, `String`→`%s`. `boolean` é impresso como `true`/`false`, igual ao Java. `println()` sem argumento vira `printf("\n")`.
- **`Scanner`:** `Scanner sc = new Scanner(System.in);` (local ou campo) não gera variável em C, só um comentário. `sc.nextInt()` e `sc.nextDouble()` chamam as funções `j2c_ler_int()` e `j2c_ler_double()`, geradas no início do arquivo quando o programa usa `Scanner`. `sc.close()` não faz nada.

Limitações conhecidas:

- `println` aceita um argumento só: a concatenação (`"n = " + n`) ainda gera erro, assim como um valor cujo tipo o gerador não consegue descobrir;
- `double` e `float` saem com 6 casas (`2.5` vira `2.500000`), diferente do Java;
- do `Scanner`, só `nextInt` e `nextDouble` (`next` e `nextLine` geram erro); entrada inválida ou fim da entrada devolve 0, em vez da exceção do Java;
- arrays ainda não são suportados.

Para rodar os testes de ponta a ponta (Java → C → `gcc`):

```bash
make test-codegen-e2e
```

Além de compilar o C gerado, esses testes executam `tests/codegen/execucao/*.java`: o C gerado é linkado com um driver em C (arquivo `.c` de mesmo nome) que chama as funções e confere se o resultado é o mesmo do Java. Os programas de `tests/codegen/saida/*.java` são compilados e executados de verdade: a saída é comparada com o arquivo `.saida` de mesmo nome, e o `.entrada`, quando existe, é usado como entrada padrão.

## Equipe

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/macedoPedro20">
        <img src="https://github.com/macedoPedro20.png" width="100px;" alt="Pedro Macedo"/><br />
        <sub><b>Pedro Macedo</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/JoaoGSantana10">
        <img src="https://github.com/JoaoGSantana10.png" width="100px;" alt="João Santana"/><br />
        <sub><b>João Santana</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/gusmoles">
        <img src="https://github.com/gusmoles.png" width="100px;" alt="Guilherme Gusmão"/><br />
        <sub><b>Guilherme Gusmão</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/RodrigoCBarbosa">
        <img src="https://github.com/RodrigoCBarbosa.png" width="100px;" alt="Rodrigo Barbosa"/><br />
        <sub><b>Rodrigo Barbosa</b></sub>
      </a>
    </td>
    <td align="center">
      <a href="https://github.com/Nic0laslc">
        <img src="https://github.com/Nic0laslc.png" width="100px;" alt="Rodrigo Barbosa"/><br />
        <sub><b>Nicolas Lopes</b></sub>
      </a>
    </td>
  </tr>
</table>

