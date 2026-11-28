# **:checkered_flag:** Sprint 13

Período: 10/11/2026 a 16/11/2026

## Descrição

> Com a AST sendo construída pelo Parser, o time trabalhou em duas frentes: tornar a árvore visível, com uma impressão formatada, e iniciar a análise semântica de escopos usando a Tabela de Símbolos.
>
## Objetivos

* Implementar a impressão da árvore sintática após um `yyparse()` bem-sucedido, com indentação e conectores (`├──` / `└──`), junto com o script e o alvo `test-ast` para validar a impressão (PR #41).
* Criar o esqueleto da análise semântica, usando a Tabela de Símbolos para empilhar escopos, buscar símbolos e detectar redeclarações, incluindo os escopos do `for` e do `switch` (PR #42).
* Adicionar testes de análise semântica.

## Reuniões

### Reunião 1

**Data:** 11/11/2026  
**Local:** Online  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Formato da impressão da árvore sintática.
* Estratégia da análise semântica de escopos com a Tabela de Símbolos.
* Acompanhamento das tarefas em andamento.

## Finalização

> A impressão da AST e a primeira versão da análise semântica foram implementadas e abertas para revisão nos PRs #41 (12/11) e #42 (15/11). Não houve integração na main durante a Sprint: os dois PRs foram revisados e integrados no início da Sprint 14.
>

---

**Observações:**

> -
>
