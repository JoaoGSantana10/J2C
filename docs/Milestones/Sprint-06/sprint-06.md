# **:checkered_flag:** Sprint 06

Período: 22/09/2025 a 28/09/2025

## Descrição

> Após a consolidação da documentação na Sprint 05, o time retomou o desenvolvimento do compilador com foco na etapa de Análise Sintática. O objetivo desta Sprint foi evoluir o Parser para suportar estruturas de controle e expressões com a precedência de operadores correta, além de consolidar uma suíte de testes automatizada para validar o Scanner e o Parser em conjunto.
>
## Objetivos

* Adicionar suporte a estruturas de controle na gramática do Parser (if/else, laços, etc.).
* Implementar precedência e associatividade de operadores para expressões aritméticas, lógicas e relacionais, resolvendo a ambiguidade clássica do dangling else.
* Habilitar mensagens de erro sintático mais detalhadas no Bison (parse.error verbose).
* Atualizar a suíte de testes do Scanner e criar a suíte de testes do Parser, com casos válidos e inválidos.
* Automatizar a execução dos testes via Makefile e script dedicado (run_parser_tests.sh).

## Reuniões



### Reunião 1

**Data:** 22/09/2025  
**Local:**  Discord
**Inicio:** 19:00  
**Encerramento:** 20:00

**Assuntos:**

* Definição da tabela de precedência e associatividade dos operadores.
* Planejamento dos casos de teste válidos e inválidos para o Parser.
* Divisão das tarefas de atualização da suíte de testes do Scanner.

## Finalização

> O Parser passou a suportar estruturas de controle e expressões com precedência de operadores corretamente, incluindo o tratamento do dangling else. Foi entregue uma suíte de testes própria para o Parser (casos válidos e inválidos), além da atualização da suíte do Scanner, reduzindo o risco de regressões na análise sintática. A Análise Semântica e a Geração de Código ficaram para as próximas Sprints.
> 

---

**Observações:**

> -
>