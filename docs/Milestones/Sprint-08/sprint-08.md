# **:checkered_flag:** Sprint 08

Período: 06/10/2026 a 12/10/2026

## Descrição

> Após a reorganização da suíte de testes e a inclusão de funções void e modificadores de acesso na Sprint 07, o time ampliou o subconjunto de Java aceito pelo Parser, adicionando construções necessárias para programas mais próximos de código Java real, como nomes qualificados, criação de objetos e tipos definidos pelo usuário. Paralelamente, a suíte de testes foi padronizada para que todos os casos estivessem dentro de uma public class, e os testes inválidos foram expandidos.
>
## Objetivos

* Adicionar o token KW_NEW e a regra de criação de objetos nas expressões.
* Permitir identificadores como tipo, possibilitando declarações com tipos definidos pelo usuário.
* Implementar nomes qualificados (ex.: System.out.println) nas chamadas de função.
* Padronizar todos os arquivos de teste com a declaração public class e unificar a nomenclatura dos arquivos.
* Ampliar os casos de teste válidos e inválidos do Parser.

## Reuniões

### Reunião 1

**Data:** 07/10/2026  
**Local:** Sala de aula  
**Inicio:** 14:00  
**Encerramento:** 15:50

**Assuntos:**

* Definição das novas funcionalidades do subconjunto Java (new, nomes qualificados e tipos definidos pelo usuário).
* Padronização da estrutura e da nomenclatura dos arquivos de teste.
* Revisão e ampliação dos casos de teste inválidos.

## Finalização

> O Parser passou a reconhecer nomes qualificados em chamadas de função, expressões de criação de objetos com new e identificadores como tipo. Os testes foram padronizados com public class e renomeados para um padrão único, e os casos válidos e inválidos foram ampliados, cobrindo erros como vírgulas soltas em argumentos, void usado como variável ou parâmetro, continue com valor e comandos sem ponto e vírgula. A automação dos testes com integração contínua e o início da preparação para a Análise Semântica ficaram para a próxima Sprint.
> 

---

**Observações:**

> -
>