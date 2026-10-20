# Antigravity agent instructions — InternPath fresh build

## Project identity
InternPath is a real internship matching / skill-gap planning **product**.
The custom DS and algorithms are behind the scenes for our university project.
Do not add DSA labs, quizzes, algorithm visualizers, lecturer modes, mini-project
branding, or CLI controls to the product website.

## IMPORTANT: User's coding learning goal
Three students will implement required DSA **from scratch**. Never fill TODO
exercises automatically unless explicitly requested for that specific method.
Explain the algorithm, pseudocode, complexity, edge cases and a test first.
Use lecture-readable C++20: Allman braces, 4 spaces, one statement per line.
`using namespace std;` in `.cpp` is fine; don't pollute shared headers.

## Guardrails
- No std::vector, list, map, unordered_map, stack, queue, std::sort, etc.
  in application DSA implementation. Tests and third-party HTTP/JSON exempt.
- `std::string`, integer types, file I/O and safe standard language utilities okay.
- Backend services are independent of web/HTTP; website only consumes endpoints.
- No fake application data in user-visible results; real engine results only.
- No Qt/QML, previous UI code, AI features, CV parsing or public auth for now.
- Don't modify source before auditing it and obtaining user approval.
- Don't automatically push/merge; preserve other members' work.
- Give verified results, not claims about incomplete TODOs.

## Build acceptance
Core matching/date tests pass at M0. DSA exercise tests are opt-in and
**deliberately fail** until implementations exist. Don't disable them to green CI.
Read `docs/IMPLEMENTATION_ROADMAP.md` before proposing next tasks.
