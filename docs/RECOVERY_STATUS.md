# Recovery status

## What this package is

A clean, executable reconstruction produced from surviving UbuLang/UCC documentation and artifact manifests.

## What it is not

It is not the original `ubulang-ucc-pre1.zip` and is not byte-identical to it. The original binary artifact was not present in File Library, Google Drive, the active sandbox, or the previously empty `0xbfb/ubu-lang` repository.

## Implemented

- `ucc0` bootstrap compiler in C;
- `.ubu` to standalone C lowering for a documented executable subset;
- functions, integer/bool/string scalars, `let`, assignment, `if`, `else`, `while`, arithmetic, comparisons and console output;
- Windows and Unix build/test/gate scripts;
- `.ubu` core/std recovery tree;
- dependency audit and structural pure-Ubu gate;
- four executable examples and integration tests.

## Not yet reconstructed

- full module linking and namespace resolution;
- ADT/match lowering;
- generic Option/Result/Vec semantics;
- file and string primitive lowering;
- historical optimization passes and exact benchmark harness;
- exact source, tests and reports from `ucc-pre1`.
