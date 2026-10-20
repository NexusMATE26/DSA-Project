# Implementation roadmap — do not skip the acceptance gates

## M0 — current package (all three)
- [x] CMake C++20 scaffold; core models; matching rule sample; test framework.
- [x] Student-friendly DSA exercises and public contracts.
- [x] Small seed dataset and web folder placeholder.
- [ ] Team agrees on signatures and coding conventions; push repository to GitHub.
- [ ] Confirm compiler/CMake on all three development computers.

## M1 — working infrastructure and basic DSA (all three)
- [ ] A: DynamicArray (first), linked lists, stack, circular queue.
- [ ] B: hashing, hash table, sorting-independent AVL, searching.
- [ ] C: graph representation and DFS/Kahn; test pure matching.
- [ ] Infrastructure spike: install pinned HTTP/JSON libraries; GET /api/health
      and serve one static page (NO UI design yet).
- Gate: all required DSA tests pass with sanitizers; basic HTTP smoke works.

## M2 — algorithms
- [ ] A: stable merge sort + insertion sort; app-ranking comparator.
- [ ] B: hash collision/re-hash tests; AVL delete/range; binary lower bound.
- [ ] C: cycle prevention, reverse prerequisite closure, learning stages.
- Gate: deterministic examples and edge cases pass.

## M3 — real application services
- [ ] B: AppState record owner, registry indexes, catalogue, CSV snapshot.
- [ ] C: student matching and recommendation services, learning planner.
- [ ] A: undo, ranked waiting lists, applications, promotions.
- Gate: end-to-end test works in C++ with no web layer.

## M4 — HTTP/JSON, real student/admin workflow
- [ ] Expose validated service DTOs via HTTP; local-only bind.
- [ ] Unit + integration + persistence roundtrip + API smoke tests.
- Gate: browser fetch can run the entire scenario.

## M5 — website design (shared)
- [ ] Only then create product screens: profile, explore, match, plan,
      applications, admin. UI must not show DSA course topics.
- Gate: product flows work end-to-end on three laptops.

## M6 — university deliverables (shared)
- [ ] Two-page DOCX proposal and independent technical explanation/diagrams.
- [ ] Benchmarks/complexity notes for the viva OUTSIDE the website.
- [ ] Clean demo dataset, reliability and presentation rehearsal.

## Git workflow
- main protected/stable; feature/a-..., feature/b-..., feature/c-... branches.
- Freeze public contracts before parallel feature implementation.
- PR with code + tests; another member reviews; never overwrite others' work.
- Keep a working build after each merge.
