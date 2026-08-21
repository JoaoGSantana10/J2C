# J2C (J to C)

## 📖 Sobre o projeto

Este repositório reúne o desenvolvimento do projeto prático da disciplina de Compiladores 1, do curso de Engenharia de Software da Universidade de Brasília (UnB) — Campus FGA Gama, turma 01, Grupo 7. 

Imagine pegar um programa escrito em uma linguagem orientada a objetos moderna — com décadas de boas práticas de engenharia de software embutidas — e traduzi-lo automaticamente para C, a linguagem que moldou praticamente toda a computação de sistemas desde a década de 1970 e que ainda hoje está por trás de sistemas operacionais, firmwares, bancos de dados e boa parte da infraestrutura crítica do mundo. É exatamente essa ponte entre dois paradigmas — o orientado a objetos e o procedural — que o JaCOBOL se propõe a construir.

Mais do que um exercício acadêmico, este projeto é uma imersão prática em como compiladores realmente funcionam por dentro: como um texto puro se transforma em tokens, como esses tokens ganham estrutura e significado através de uma gramática formal, como essa estrutura é validada semanticamente, e como ela é reorganizada e reescrita em uma linguagem-alvo com um modelo de execução fundamentalmente diferente — sem classes, sem objetos, sem herança, apenas funções, structs e memória gerenciada explicitamente.

Traduzir Java para C não é apenas trocar sintaxe: é resolver, de forma sistemática, o descasamento entre dois paradigmas. Uma classe Java precisa virar uma `struct` (seus atributos) associada a um conjunto de funções que recebem essa struct como parâmetro (seus métodos). Um objeto instanciado com `new` precisa virar uma alocação explícita de memória, com o programador do compilador decidindo como e quando ela será liberada. Um método que antes vivia "dentro" do objeto passa a ser uma função livre, identificada por um nome que evita colisões (algo como `NomeDaClasse_nomeDoMetodo`). É esse tipo de decisão de projeto — e não só a tradução literal de comandos — que torna este compilador um exercício real de engenharia de linguagens.

Desenvolvido na disciplina de **Compiladores 1** (UnB/FGA Gama, turma 01), o projeto tem como objetivo construir um compilador/transpilador capaz de traduzir um subconjunto da linguagem Java para código C equivalente e funcional, percorrendo — na prática, não só na teoria — todas as etapas clássicas da construção de um compilador:

1. **Análise Léxica** — tokenização do código-fonte Java, implementada com Flex, reconhecendo palavras-chave, identificadores, literais, operadores e delimitadores da linguagem
2. **Análise Sintática** — construção da Árvore Sintática Abstrata (AST) a partir dos tokens, implementada com Bison via uma gramática LALR(1) que descreve o subconjunto suportado de Java
3. **Análise Semântica** — verificação de tipos, resolução de escopo (incluindo o escopo implícito de classes e métodos), checagem de regras da linguagem e construção da tabela de símbolos
4. **Geração de Código Intermediário** (opcional, dependendo do escopo final da disciplina) — uma representação simplificada da AST que facilita a etapa seguinte
5. **Geração de Código C** — tradução da AST (ou da representação intermediária) para código C válido e compilável, incluindo a estratégia de mapeamento de classes para structs + funções

## 🎯 Escopo da linguagem-fonte

O compilador não pretende suportar Java completo — o foco é um subconjunto bem definido que cubra os conceitos fundamentais sem inviabilizar o projeto no prazo da disciplina. O escopo exato ainda está sendo refinado, mas inclui candidatos como: classes simples (sem herança múltipla), atributos e métodos, tipos primitivos, estruturas de controle (`if`/`else`, `while`, `for`), atribuições e expressões aritméticas/lógicas, e chamadas de método.
