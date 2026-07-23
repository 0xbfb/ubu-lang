#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
forbidden = []
SCAN_ROOTS = [ROOT / "src", ROOT / "lib", ROOT / "examples"]
for path in sorted(path for base in SCAN_ROOTS if base.exists() for path in base.rglob("*.ubu")):
    text = path.read_text(encoding="utf-8")
    for token in ("printf", "fopen", "malloc", "pthread", "import core.intrinsics"):
        if token in text:
            forbidden.append({"path": str(path.relative_to(ROOT)), "token": token})
active_runtime = [p for p in (ROOT / "src" / "runtime").glob("*.[ch]")] if (ROOT / "src" / "runtime").exists() else []
for path in active_runtime:
    forbidden.append({"path": str(path.relative_to(ROOT)), "token": "active C runtime"})
status = "pass" if not forbidden else "fail"
report = {"schema": 1, "project": "UbuLang recovery-r1", "status": status, "forbidden": forbidden,
          "scope_note": "This gate verifies repository purity, not complete historical ucc-pre1 feature parity."}
out = ROOT / "build" / "pure-ubu-gate.json"
out.parent.mkdir(exist_ok=True)
out.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
print(f"pure-ubu-gate {status.upper()}: {len(forbidden)} forbidden findings")
raise SystemExit(0 if status == "pass" else 1)
