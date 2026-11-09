# **:checkered_flag:** Sprint 10

Período: 20/10/2026 a 26/10/2026

## Descrição

> Após a criação da Tabela de Símbolos e da estrutura da AST na Sprint 09, o time refinou a AST para prepará-la para ser construída pelas ações do Parser. O foco foi o gerenciamento de memória das strings, a tipagem dos operadores com enums e a criação de funções auxiliares para modificadores e nomes qualificados. Paralelamente, as branches da Tabela de Símbolos e do Analisador Sintático foram integradas à main.
>
## Objetivos

* Fazer os construtores da AST assumirem a posse das strings recebidas, simplificando a liberação de memória.
* Simplificar a estrutura dos nós: remover o nome do nó de programa, mantendo-o apenas na classe, e criar o nó NODE_DEFAULT para o default do switch.
* Criar enums para os operadores binários, unários e de atribuição (BinaryOpType, UnaryOpType e AssignOpType).
* Implementar funções auxiliares para o Parser: ast_join_qualified, para montar nomes qualificados, e ast_set_modifiers, para atribuir modificadores a variáveis e funções.
* Integrar à main as branches da Tabela de Símbolos (PR #31) e do Analisador Sintático (PR #33).

## Reuniões

### Reunião 1

**Data:** 21/10/2026  
**Local:** Sala de aula  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Estratégia de posse e liberação de memória das strings na AST.
* Revisão da estrutura dos nós e tipagem dos operadores.
* Planejamento da integração da AST com as ações do Parser.
* Revisão e merge dos Pull Requests pendentes.

## Finalização

> A AST foi refatorada para gerenciar a posse das strings, os operadores passaram a ser representados por enums e foram criadas as funções auxiliares necessárias para as ações do Parser. As branches da Tabela de Símbolos e do Analisador Sintático foram integradas à main com os PRs #31 e #33. A implementação das ações do Parser que constroem a AST ficou para as próximas Sprints.
> 

---

**Observações:**

> -
>