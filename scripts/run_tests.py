#!/usr/bin/env python3
from __future__ import annotations
import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EXE = ".exe" if os.name == "nt" else ""
UCC = ROOT / "build" / f"ucc{EXE}"
CASES = [
    ("hello", "hello from UbuLang recovery-r1\n"),
    ("gcd", "21\n"),
    ("sum_squares", "333333833333500000\n"),
    ("conditionals", "-1\n"),
]

def run(cmd: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, cwd=ROOT, text=True, capture_output=True, check=False)

if not UCC.exists():
    print(f"missing compiler: {UCC}", file=sys.stderr)
    raise SystemExit(2)

for name, expected in CASES:
    source = ROOT / "examples" / f"{name}.ubu"
    binary = ROOT / "build" / f"{name}{EXE}"
    result = run([str(UCC), "build", str(source), "-o", str(binary)])
    if result.returncode:
        print(result.stdout); print(result.stderr, file=sys.stderr); raise SystemExit(1)
    executed = run([str(binary)])
    if executed.returncode or executed.stdout != expected:
        print(f"FAIL {name}: expected={expected!r}, got={executed.stdout!r}, stderr={executed.stderr!r}")
        raise SystemExit(1)
    print(f"PASS {name}: {executed.stdout.strip()}")

invalid = ROOT / "build" / "invalid.ubu"
invalid.write_text("fn broken\n", encoding="utf-8")
invalid_result = run([str(UCC), "emit-c", str(invalid), "-o", str(ROOT / "build" / "invalid.c")])
if invalid_result.returncode == 0:
    print("FAIL invalid syntax: compiler accepted malformed function header")
    raise SystemExit(1)
print("PASS invalid syntax: rejected malformed function header")
invalid.unlink(missing_ok=True)
(ROOT / "build" / "invalid.c").unlink(missing_ok=True)

for tool in (ROOT / "tools" / "audit" / "ubu_dependency_audit.py", ROOT / "tools" / "audit" / "pure_ubu_gate.py"):
    result = run([sys.executable, str(tool)])
    print(result.stdout, end="")
    if result.returncode:
        print(result.stderr, file=sys.stderr); raise SystemExit(result.returncode)
print(f"{len(CASES)} executable cases + 1 negative compiler check PASS")
