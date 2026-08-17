# JaCOBOL
 
📖 Sobre o projeto

Imagine pegar um programa escrito hoje, em uma linguagem orientada a objetos, com décadas de boas práticas de engenharia de software embutidas — e transformá-lo automaticamente em COBOL, a linguagem que nasceu em 1959 e que, até hoje, processa a maior parte das transações bancárias do planeta. É exatamente essa ponte entre dois mundos que o JaCOBOL se propõe a construir.

Mais do que um exercício acadêmico, este projeto é uma imersão prática em como compiladores realmente funcionam por dentro: como um texto puro se transforma em tokens, como esses tokens ganham estrutura e significado, e como essa estrutura é reorganizada e reescrita em uma linguagem-alvo completamente diferente — com outra sintaxe, outro paradigma e outras convenções.

Desenvolvido na disciplina de Compiladores 1 (UnB/FGA Gama, turma 01), o projeto tem como objetivo construir um compilador/transpilador capaz de traduzir um subconjunto da linguagem Java para código COBOL equivalente e funcional, percorrendo — na prática, não só na teoria — todas as etapas clássicas da construção de um compilador:

Análise Léxica — tokenização do código-fonte Java
Análise Sintática — construção da Árvore Sintática Abstrata (AST)
Análise Semântica — verificação de tipos, escopo e regras da linguagem
Geração de Código Intermediário (opcional, dependendo do escopo da disciplina)
Geração de Código COBOL — tradução da AST para código COBOL válido
