# **:checkered_flag:** Sprint 09

Período: 13/10/2026 a 19/10/2026

## Descrição

> Após ampliar o subconjunto de Java aceito pelo Parser na Sprint 08, o time iniciou a preparação para a Análise Semântica. O foco desta Sprint foi implementar a Tabela de Símbolos e definir a estrutura completa da Árvore Sintática Abstrata (AST), que servirá de base para as próximas fases do compilador. Paralelamente, a execução dos testes foi automatizada com integração contínua no GitHub Actions e o Parser e o Scanner receberam ajustes de robustez.
>
## Objetivos

* Automatizar a execução dos testes a cada push na branch main com GitHub Actions.
* Implementar a Tabela de Símbolos (symtab.c e symtab.h), incluindo suporte aos tipos long e float.
* Criar os testes da Tabela de Símbolos (test_symtab.c) e integrá-los ao Makefile.
* Definir os nós da AST (ast.h) para programa, classe, declarações, comandos, estruturas de controle, switch-case, expressões e literais.
* Implementar os construtores da AST, a impressão da árvore (ast_print) e a liberação recursiva de memória (ast_free).
* Ajustar o Parser e o Scanner: uso de %precedence e %empty no Bison, envio dos erros léxicos para stderr com contagem de erros e reconhecimento de literais double sem sufixo.

## Reuniões

### Reunião 1

**Data:** 14/10/2026  
**Local:** Sala de aula  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Configuração do workflow de testes no GitHub Actions.
* Estrutura e operações da Tabela de Símbolos.
* Modelagem dos nós da AST e divisão da implementação dos construtores.
* Ajustes no tratamento de erros léxicos e sintáticos.

## Finalização

> Os testes do Scanner, da Tabela de Símbolos e do Parser passaram a ser executados automaticamente pelo GitHub Actions a cada push na main. A Tabela de Símbolos foi entregue com sua suíte de testes, compilada com AddressSanitizer e UndefinedBehaviorSanitizer para detectar erros de memória. A AST foi definida e implementada com construtores, impressão e liberação de memória. O Parser e o Scanner foram ajustados com o PR #32. A integração da AST às ações do Parser ficou para as próximas Sprints.
> 

---

**Observações:**

> -
>