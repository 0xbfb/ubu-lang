# AGENTS.md — UbuLang / UCC recovery-r1

## Natureza do pacote

Este repositório é uma reconstrução funcional criada porque o ZIP histórico `ubulang-ucc-pre1.zip` não estava recuperável. Nunca o apresente como cópia byte a byte da release original.

## Regras arquiteturais

1. `src/ucc0` é apenas o compilador bootstrap em C.
2. Programas `.ubu` devem gerar C autossuficiente; não crie `src/runtime` público.
3. `lib/core` e `lib/std` permanecem em `.ubu`.
4. Operações de plataforma devem ser documentadas como backend primitives.
5. Não crie `src/ucc1` antes de cumprir `docs/UCC1_START_CRITERIA.md`.
6. Imports decorativos e recursos não implementados devem ser declarados explicitamente como pendência, nunca como suporte concluído.

## Método de desenvolvimento

- Adicione teste reproduzível antes ou junto de cada expansão do compilador.
- Execute `make release-gate` no Unix ou `release-gate.bat` no Windows.
- Compile o C gerado também com `-Wall -Wextra -Wpedantic` quando alterar o lowering.
- Preserve exemplos mínimos de regressão.
- Não inclua `.git`, caches, executáveis ou diretórios de build em releases limpas.

## Entregas

Toda entrega deve registrar:

- versão e base usada;
- arquivos alterados;
- testes executados e resultados;
- limitações reais;
- SHA-256 dos artefatos;
- distinção entre código reconstruído e artefato histórico.
