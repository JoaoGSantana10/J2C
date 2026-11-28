# **:checkered_flag:** Sprint 15

Período: 24/11/2026 a 30/11/2026

## Descrição

> Sprint de implementação da geração de código C, a última etapa do compilador. Cada membro implementou a issue planejada na Sprint 14, e ao final o J2C passou a traduzir para C compilável 13 dos 14 programas válidos do projeto: o único que falha usa `new` de outra classe, e orientação a objetos está fora do escopo.
>
## Objetivos

* Criar a infraestrutura do gerador de código (`codegen.c`), integrada ao `./parser`, gerando o arquivo C sem deixar arquivos parciais em caso de erro (PR #49, issue #43).
* Traduzir declarações, tipos e funções: mapeamento de tipos Java → C, campos como variáveis globais, protótipos e `main` (PR #50, issue #44).
* Traduzir expressões, chamadas e atribuições, preservando a precedência da AST e adaptando os literais ao C (PR #51, issue #45).
* Traduzir as estruturas de controle: `if`/`else`, `while`, `do-while`, `for`, `switch`/`case`/`default`, `break` e `continue` (PRs #52, #54 e #55, issue #46).
* Traduzir `System.out.println`/`print` para `printf` com o formato do tipo e a leitura com `Scanner` (`nextInt` e `nextDouble`), com testes que executam o programa gerado e comparam a saída (PR #53, issue #47).
* Completar os testes do parser para `void`, incremento/decremento e `continue` (PR #57, issues #48 e #22).
* Atualizar o README com o que o compilador suporta e suas limitações (PRs #55 e #56).

## Reuniões

### Reunião 1

**Data:** 25/11/2026  
**Local:** Online  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Acompanhamento da implementação das issues de geração de código.
* Integração da tradução de controle de fluxo e de entrada e saída no gerador.
* Revisão e merge dos Pull Requests e preparação da entrega final.

## Finalização

> A geração de código foi integrada à main com os PRs #49 a #57. A tradução de controle de fluxo da PR #52 tinha ficado em um módulo separado (`traducao.c`), que o compilador não chamava; ela foi refeita dentro do `codegen.c` (PR #55), e o módulo duplicado foi removido em seguida. Ao final da Sprint, o `make test` passa em todas as suítes (scanner, tabela de símbolos, parser, AST e geração de código, com testes de execução do C gerado), e a CI está verde.
>
> Ficaram para versões futuras: arrays, orientação a objetos (classes como `structs`), concatenação de `String` no `println`, os métodos `next` e `nextLine` do `Scanner` e a checagem de `break`/`continue` fora de laço na análise semântica (hoje feita só na geração de código).
>

---

**Observações:**

> -
>
