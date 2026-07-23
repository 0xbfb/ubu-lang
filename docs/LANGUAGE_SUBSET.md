# Subconjunto executável — recovery-r1

O compilador reconstruído implementa um núcleo intencionalmente pequeno:

- declaração `module` e `import` preservada como metadado no C;
- funções `fn` com parâmetros tipados;
- retornos `i64`, `i32`, `bool`, `str`, `char` e `unit`;
- `let`, atribuição e expressão final como retorno;
- `if`, `else if`, `else` e `while` por indentação;
- operadores aritméticos, relacionais e booleanos básicos;
- `std.console.println(...)`;
- emissão de C standalone e compilação por GCC.

## Limitações

- imports ainda não são resolvidos nem ligados entre arquivos;
- ADTs, `match`, genéricos, lambdas e closures ainda não são baixados pelo `ucc0` reconstruído;
- os módulos de `lib/core` e `lib/std` preservam o contrato arquitetural, mas nem toda API é executável nesta recuperação;
- String, Vec, File e backend primitives precisam de implementação completa antes de um `ucc1` legítimo;
- não há garantia de equivalência com o código histórico perdido.
