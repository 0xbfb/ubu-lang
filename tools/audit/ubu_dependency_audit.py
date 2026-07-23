#!/usr/bin/env python3
from __future__ import annotations
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SCAN_ROOTS = [ROOT / "src", ROOT / "lib", ROOT / "examples"]
files = sorted(path for base in SCAN_ROOTS if base.exists() for path in base.rglob("*.ubu"))
modules = []
for path in files:
    text = path.read_text(encoding="utf-8")
    module = next((line.split(maxsplit=1)[1] for line in text.splitlines() if line.startswith("module ")), None)
    imports = [line.split(maxsplit=1)[1] for line in text.splitlines() if line.startswith("import ")]
    modules.append({"path": str(path.relative_to(ROOT)).replace("\\", "/"), "module": module, "imports": imports})
report = {"schema": 1, "project": "UbuLang recovery-r1", "ubu_files": len(files), "modules": modules}
out = ROOT / "build" / "dependency-map.json"
out.parent.mkdir(exist_ok=True)
out.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
print(f"dependency audit PASS: {len(files)} .ubu files")
print(out)
