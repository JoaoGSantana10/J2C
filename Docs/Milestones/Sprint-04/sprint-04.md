# **:checkered_flag:** Sprint 04

Período: 10/09/2026 a 16/09/2026

## Descrição

> Nesta Sprint, o time avançou para a segunda fase do compilador: a Análise Sintática. O objetivo foi construir o Parser do J2C e integrá-lo ao Scanner gerado na Sprint anterior, permitindo que a cadeia de tokens fosse processada e validada contra as regras gramaticais do subconjunto Java, com foco em estruturas de declaração e funções.
> 

## Objetivos

- Criar a estrutura principal do analisador sintático (parser.y).
- Integrar o Scanner ao Parser para consumo sequencial de tokens (lex.yy.c, parser.tab.c e parser.tab.h).
- Implementar as regras sintáticas para a declaração de variáveis e funções.
- Adicionar o suporte gramatical ao token e comando de retorno (return).

## Reuniões

### Reunião 1

**Data:** 11/09/2026  
**Local:** Discord 
**Inicio:** 19:00  
**Encerramento:** 20:00

**Assuntos:**

* Estratégia de integração entre o Flex e o Bison.
* Desenho das regras de produção gramatical para as assinaturas de funções e variáveis.
* Tratamento e resolução de conflitos iniciais de shift/reduce no arquivo do Parser.

## Finalização

> O Parser foi criado e integrado de forma bem-sucedida ao fluxo do compilador. As bases sintáticas foram consolidadas com a implementação das regras que validam declarações de variáveis, estruturação de funções e o comando return. Os artefatos C foram gerados corretamente, finalizando a integração Léxico-Sintática sem pendências.
> 

---
