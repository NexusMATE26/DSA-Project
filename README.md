# InternPath — fresh C++20 project (Milestone 0 starter)

InternPath is a practical **internship matching and skill-gap planning web
application**. The seven handwritten data structures plus sorting/searching
will eventually power real student and administrator features. **The website
itself will never be a DSA learning tool.**

## Current status: scaffold, NOT a finished application

**Implemented and verified now:** CMake C++20 project, data model/contracts,
real deterministic weighted matching rule, basic date validation, simple
developer smoke command and two passing core tests. A small example dataset is
included for future CSV integration.

**Intentionally not implemented yet:** DS/algo TODOs, application services,
CSV reader/writer, HTTP server and website. TODO functions throw `logic_error`.
These are your **coursework tasks**. The page in `web/` is only a placeholder.

## Requirements
- CMake 3.20+ and a C++20-capable compiler.
- The core scaffold needs no third-party libraries and no internet.
- Windows: Visual Studio 2022 developer terminal or another CMake C++20 toolchain.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

On multi-config Windows builds, use `cmake --build build --config Debug`
and `ctest --test-dir build -C Debug --output-on-failure`.

Run the developer smoke program:
- Linux/macOS: `./build/internpath_smoke`
- Windows: `build\\Debug\\internpath_smoke.exe` or your generator's output path.

### Exercise tests (expected to FAIL before team codes TODOs)

```bash
cmake -S . -B build -DINTERNPATH_ENABLE_EXERCISE_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Start with TODO A01 / A02. After finishing one module, use only the relevant
exercise executable or test to avoid confusing unfinished modules with bugs.

## Most important reading order
1. `AGENTS.md` and `ANTIGRAVITY_START_PROMPT.md`.
2. `docs/DOMAIN_RULES.md` and `docs/DSA_MAPPING.md`.
3. `docs/HEADER_CONTRACTS.md` and `docs/IMPLEMENTATION_ROADMAP.md`.
4. Source files assigned to your member.

## Product / academic boundary
- Product users see profiles, internships, recommendations, learning plans,
  applications and administration.
- DSA explanations/complexity tests belong to code, tests, reports and viva.
- CLI/smoke executable is **developer-only**, not a user product screen.
- No real password/auth system in this offline demo; never deploy as-is.

## Current development responsibilities
- A: dynamic arrays, lists, stack/queue, sorting, waiting list and undo.
- B: hashing, AVL, searching, registries and CSV.
- C: graph, cycle checking, topological planning, matching and API integration.

We are **starting fresh**; older V2 ZIPs are references only, not files to merge.
