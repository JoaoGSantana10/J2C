# **:checkered_flag:** Sprint 07

Período: 29/09/2026 a 05/10/2026

## Descrição

> Após evoluir o Parser com estruturas de controle e precedência de operadores na Sprint 06, o time deu continuidade à Análise Sintática ampliando a gramática para cobrir funções sem retorno e modificadores de acesso, além de corrigir o tratamento de listas de parâmetros e argumentos. Paralelamente, o repositório passou por uma limpeza dos arquivos gerados automaticamente e a suíte de testes do Parser foi reorganizada em testes unitários menores e mais específicos.
>
## Objetivos

* Remover do repositório os arquivos gerados pelo Flex e pelo Bison (lex.yy.c, parser.tab.c e parser.tab.h) e os binários compilados, criando um .gitignore para evitar que voltem a ser versionados.
* Corrigir as regras gramaticais de parâmetros e argumentos para exigir a separação por vírgula.
* Adicionar o token KW_VOID para permitir a declaração de funções sem retorno.
* Implementar no Parser e no Scanner o suporte aos modificadores de acesso (public, private e protected).
* Reorganizar a suíte de testes do Parser em testes unitários de casos válidos e inválidos (continue, incremento/decremento, funções void, chamadas com argumentos e vírgulas soltas).
* Revisar o template de Issues para refletir as fases do compilador J2C.

## Reuniões

### Reunião 1

**Data:** 30/09/2026  
**Local:** Sala de aula  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Limpeza dos arquivos gerados automaticamente e definição do .gitignore.
* Definição das regras gramaticais para funções void e modificadores de acesso.
* Correção das regras de parâmetros e argumentos separados por vírgula.
* Divisão dos novos casos de teste unitários válidos e inválidos.
* Revisão do template de Issues do repositório.

## Finalização

> O repositório passou a versionar apenas o código-fonte do compilador, sem arquivos gerados. O Parser ganhou suporte a funções void e a modificadores de acesso, e as listas de parâmetros e argumentos passaram a ser validadas corretamente. A suíte de testes foi reestruturada em testes unitários e atualizada após a introdução dos modificadores. O template de Issues foi revisado, incluindo a fase do compilador, Definition of Ready e Definition of Done. A ampliação do subconjunto Java (criação de objetos e nomes qualificados) ficou para a próxima Sprint.
> 

---

**Observações:**

> -
>