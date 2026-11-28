# **:checkered_flag:** Sprint 12

Período: 03/11/2026 a 09/11/2026

## Descrição

> Com a estratégia definida na Sprint 11, o time implementou as ações do Parser que constroem a AST durante a análise sintática. Também ajustou a comunicação entre o Scanner e o Parser, terminou a migração dos testes para o formato com classe e atualizou a documentação das Sprints 07 a 11.
>
## Objetivos

* Fazer as regras de declarações e comandos do `parser.y` construírem os nós da AST, com a `%union`, os `%type` e o envio do lexema de identificadores e literais (PR #34).
* Declarar o `YYSTYPE yylval` para o Scanner enviar os valores dos tokens ao Parser (PR #36).
* Concluir a migração dos testes do parser para o formato com `public class`, criar um programa Java completo como teste ponta a ponta e corrigir o `make test` (PR #37).
* Criar testes automatizados da forma da AST para expressões (PR #38).
* Documentar as Sprints 07 a 11 (PRs #35, #39 e #40).

## Reuniões

> A preencher pelo time: data, local, horário e assuntos das reuniões desta Sprint.

## Finalização

> O Parser passou a construir a AST para declarações, comandos e expressões, e os testes do parser foram todos migrados para o formato com classe, com um programa ponta a ponta. Foram integrados à main os PRs #34, #35, #36, #37, #39 e #40. O PR #38, com os testes da forma da AST, foi aberto em 07/11 e ficou para ser integrado na Sprint 14.
>

---

**Observações:**

> -
>
