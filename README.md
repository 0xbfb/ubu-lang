# UbuLang / UCC recovery-r1

Executable recovery baseline for the UbuLang language and UCC compiler chain.

> This is a reconstruction, not the lost original `ucc-pre1` archive. See `docs/RECOVERY_STATUS.md`.

## Architecture

```text
.ubu source -> ucc0 (C bootstrap compiler) -> standalone generated C -> GCC -> native binary
```

No maintained public C runtime is linked into generated programs.

## Linux/macOS

```bash
make build
make doctor
make test
make release-gate
./build/ucc emit-c examples/gcd.ubu -o build/gcd.c
./build/ucc build examples/gcd.ubu -o build/gcd
./build/gcd
```

## Windows (MinGW GCC + Python)

```bat
build.bat
doctor.bat
test.bat
release-gate.bat
build\ucc.exe emit-c examples\gcd.ubu -o build\gcd.c
build\ucc.exe build examples\gcd.ubu -o build\gcd.exe
build\gcd.exe
```

## Supported recovery subset

- `fn`, typed parameters and return types;
- `let`, assignment;
- integer, bool, char and string scalar expressions;
- `if`, `else if`, `else`, `while`;
- testes executáveis para hello, GCD, soma de quadrados e condicionais;
- final-expression returns;
- `std.console.println(...)`;
- standalone C emission.

Imports and module declarations are preserved as metadata comments in generated C; full multi-file linking remains pending.

## Estado da recuperação

O ZIP histórico original não foi recuperado. Esta base foi reconstruída a partir da arquitetura e dos contratos documentados, com validação própria. Consulte `docs/RECOVERY_STATUS.md` e `docs/VALIDATION.md`.
