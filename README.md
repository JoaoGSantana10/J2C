# ☕ Java2C — Compilador / Transpilador de Java para C

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
  - [Como Executar o Compilador](#como-executar-o-compilador)
  - [Compilando e Executando o Código C Gerado](#compilando-e-executando-o-código-c-gerado)
- [🤝 Equipe](#-equipe)

---

## 📖 Sobre o Projeto

O **Java2C** é um transpilador (compilador de código para código) que traduz um subconjunto da linguagem **Java** para código-fonte em **C** (`ANSI C` / `C99`) eficiente e legível.

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
- [x] **Entrada e Saída:** Tradução automática de `System.out.println(...)` para `printf(...)` com formatação adequada.
- [x] **Estruturas de Dados:** Arrays unidimensionais com gerenciamento básico de memória em C.
- [ ] **Orientação a Objetos:** Mapeamento de Classes e Objetos para `structs` e ponteiros em C.

---

## 🛠️ Arquitetura e Tecnologias

- **Linguagem do Compilador:** Java (JDK 17+)
- **Gerador de Parser/Analisador:** ANTLR4 (ou analisador sintático descendente recursivo próprio)
- **Linguagem Alvo:** C (compatível com `GCC` / `Clang`)

---

## 🚀 Configuração do Ambiente e Testes

### Pré-requisitos

Certifique-se de possuir os seguintes componentes instalados e configurados no seu ambiente de desenvolvimento:

1. **Java Development Kit (JDK 17 ou superior)**
   - Download: [OpenJDK](https://adoptium.net/) ou Oracle JDK
   - Teste de instalação:
     ```bash
     java -version
     ```

2. **Compilador C (GCC ou Clang)**
   - **Linux / macOS:** GCC ou Clang pré-instalado (ou via `sudo apt install build-essential`).
   - **Windows:** MinGW-w64, MSVC ou ambiente WSL.
   - Teste de instalação:
     ```bash
     gcc --version
     ```

---

### Instalação e Build

1. **Clone o repositório do projeto:**
   ```bash
   git clone [https://github.com/seu-usuario/java2c.git](https://github.com/seu-usuario/java2c.git)
   cd j2c
   ```

### Compilando e Executando o Código C Gerado

Para transpilar um programa escrito em Java para C:

```bash
java -jar target/java2c.jar caminho/para/MeuPrograma.java -o saida/MeuPrograma.c
```

Para testar o resultado final no ambiente C:

```bash
# Compilar o código C gerado
gcc saida/MeuPrograma.c -o saida/MeuPrograma

# Executar o programa
./saida/MeuPrograma
```

## Equipe

<img src="https://github.com/macedoPedro20">
<img src="https://github.com/JoaoGSantana10">
<img src="https://github.com/gusmoles">
<img src="https://github.com/RodrigoCBarbosa">
