---
name: Tarefa
about: Tarefa do compilador J2C (léxico, sintático, semântico, geração de código, testes ou docs)
title: "[TAREFA] "
labels: ""
assignees: ""
---

## 📝 Descrição
<!-- Explique o problema, a necessidade ou a funcionalidade de forma detalhada -->


## 🧩 Fase do compilador
- [ ] 🔤 Analisador léxico (`scanner.l`)
- [ ] 🌳 Analisador sintático (`parser.y`)
- [ ] 🧠 Análise semântica (tabela de símbolos, tipos, escopo)
- [ ] ⚙️ Geração de código C
- [ ] 🧪 Testes
- [ ] 📚 Documentação / infraestrutura (Makefile, CI, repo)

## 💻 Exemplo de entrada e saída esperada
<!-- Trecho de Java (subconjunto do J2C) e o resultado esperado -->

**Entrada (Java):**
```java
// exemplo
```

**Saída esperada (C ou mensagem de erro):**
```c
// exemplo
```

## 👥 Responsáveis
- **Autor:**
- **Revisor:**

## 🚦 Prioridade
- [ ] 🔴 Alta
- [ ] 🟡 Média
- [ ] 🟢 Baixa

## ✅ Tarefas
- [ ] Tarefa 1
- [ ] Tarefa 2
- [ ] Tarefa 3

## 📌 Definition of Ready (DoR)
- [ ] Critérios de aceitação definidos
- [ ] Trecho do subconjunto de Java afetado identificado (tokens, regras ou construções)
- [ ] Estimativa inicial atribuída
- [ ] Dependências de outras tarefas/fases identificadas

## 🏁 Definition of Done (DoD)
- [ ] Código implementado e revisado
- [ ] `make` compila sem erros nem warnings (Flex, Bison e gcc)
- [ ] Sem novos conflitos shift/reduce no Bison
- [ ] Casos válidos e inválidos testados (ex.: `echo '...' | ./parser`)
- [ ] Testes automatizados adicionados ou atualizados
- [ ] Documentação atualizada (se aplicável)
- [ ] PR aprovado por pelo menos um revisor

## 🔗 Referências
<!-- Links, issues relacionadas, PRs, trechos da especificação da linguagem -->
