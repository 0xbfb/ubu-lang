CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
BUILD := build
UCC := $(BUILD)/ucc

.PHONY: all build clean doctor test audit gate release-gate
all: build
build: $(UCC)
$(UCC): src/ucc0/main.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $< -o $@
clean:
	rm -rf $(BUILD)
doctor: build
	$(UCC) doctor
test: build
	python3 scripts/run_tests.py
audit:
	python3 tools/audit/ubu_dependency_audit.py
gate:
	python3 tools/audit/pure_ubu_gate.py
release-gate: test audit gate
