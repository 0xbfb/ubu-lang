# Validação da reconstrução

**Data UTC:** 2026-07-23T12:56:12Z  
**Ambiente:** Debian, GCC 14.2.0, Python 3.13.5

## Comandos executados

```bash
make clean
make build
make test
make release-gate
make doctor
./build/ucc --version
./build/ucc emit-c examples/conditionals.ubu -o build/conditionals-manual.c
gcc -std=c11 -Wall -Wextra -Wpedantic build/conditionals-manual.c -o build/conditionals-manual
./build/conditionals-manual
```

## Resultado

```text
build: PASS, zero warnings
hello: PASS
GCD: PASS, saída 21
sum_squares(1_000_000): PASS, saída 333333833333500000
conditionals: PASS, saída -1
invalid syntax: PASS, cabeçalho de função malformado rejeitado
dependency audit: PASS, 21 arquivos .ubu
pure-ubu-gate: PASS, 0 achados proibidos
doctor: PASS
```

O gate comprova apenas a integridade desta reconstrução. Ele não comprova paridade integral com o `ucc-pre1` histórico.
