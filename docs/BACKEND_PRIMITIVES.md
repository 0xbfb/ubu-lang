# Backend primitives

Backend primitives are compiler-owned operations, not a maintained public C runtime.
The recovery tree declares the intended boundary for console, files, strings and vectors.
Only a narrow console `println` path is currently lowered by `ucc0`.
